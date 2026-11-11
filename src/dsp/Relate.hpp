#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// RELATE (#84) — os atos entre duas vozes (2026-11-11; dossiê
// `dossies/84_relate.md`; estudo da família §4.3 e §4.4: "o que o Modular não
// tem: hoje um cabo só leva sinal"; "avance em todos eles").
//
// Duas vozes entram (A, B) e saem transformadas pela RELAÇÃO entre elas —
// conceitos do Asterismus (sombra, hoquetus, contágio, atrito/reparo) e do
// `InteractionProcessor` do Antitotem (estados de uma relação), mesmo autor;
// conceito, não código. Cada voz tem um seguidor de envelope (5 ms / 80 ms).
// VERBO:
//   0 SOMBRA   — B só soa onde A soou, um TEMPO depois (a resposta com
//                tempo-luz): OB = B · env(A, t − TEMPO);
//   1 HOQUETO  — uma cala quando a outra soa (a que soa mais, ganha): OA = A ·
//                (1 − INT·b/(a+b)), OB = B · (1 − INT·a/(a+b));
//   2 CONTÁGIO — o envelope lento de cada uma contamina a outra (τ = 0,2 s +
//                4·TEMPO): as duas passam a respirar juntas;
//   3 ATRITO   — onde as duas soam juntas, o MIX ganha o produto A·B (o
//                atrito) na medida do encontro; quando o encontro passa, o
//                atrito se repara devagar (TEMPO);
//   4 ANEL     — o MIX é o anel A·B puro (o estado mais simples da relação).
// Saídas: OA, OB, MIX ((OA + OB)/2, mais o atrito ou o anel) e ENC (o
// encontro: quanto as duas soam ao mesmo tempo, 0–1). Atraso do SOMBRA até
// 1 s, alocado em prepare(); process() não aloca. Determinístico.
namespace rasgo::modular {

class Relate final : public Signal {
public:
    Relate() : Signal(
        {{"a", PortKind::Audio, ""},
         {"b", PortKind::Audio, ""}},
        {{"out_a", PortKind::Audio, ""},
         {"out_b", PortKind::Audio, ""},
         {"mix", PortKind::Audio, ""},
         {"meeting", PortKind::Control, ""}},
        {{"verb", 0.0f, 4.0f, 0.0f, ""},
         {"intensity", 0.0f, 1.0f, 0.8f, ""},
         {"time", 0.0f, 1.0f, 0.15f, "s"}}) {}

    std::string type() const override { return "RELATE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "RELATE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "relate", "", 2.5f, 6.0f, 35.6f);
        p.add(Widget::Kind::Knob, "VERBO", "verb", 6.0f, 30.0f);
        p.add(Widget::Kind::Knob, "INT", "intensity", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TEMPO", "time", 15.0f, 52.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 7.0f, 90.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 21.0f, 90.0f);
        p.add(Widget::Kind::Jack, "OA", "out:out_a", 7.0f, 104.0f);
        p.add(Widget::Kind::Jack, "OB", "out:out_b", 21.0f, 104.0f);
        p.add(Widget::Kind::Jack, "MIX", "out:mix", 7.0f, 120.0f);
        p.add(Widget::Kind::Jack, "ENC", "out:meeting", 21.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        envLine_.assign(static_cast<std::size_t>(sr_) + 2, 0.0f);   // 1 s do envelope de A
        w_ = 0;
        ea_ = eb_ = sa_ = sb_ = friction_ = 0.0f;
        att_ = 1.0f - std::exp(-1.0f / (0.005f * sr_));
        rel_ = 1.0f - std::exp(-1.0f / (0.080f * sr_));
    }

    float meeting() const noexcept { return meet_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4 || envLine_.empty()) return;
        const AudioBlock* a = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* b = inputs.size() > 1 ? inputs[1] : nullptr;
        const int verb = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("verb"), 0.0f), 0.0f, 4.0f)));
        const float inten = std::clamp(safe(parameterValue("intensity"), 0.8f), 0.0f, 1.0f);
        const float time = std::clamp(safe(parameterValue("time"), 0.15f), 0.0f, 1.0f);
        const std::size_t lag = std::min(envLine_.size() - 1, static_cast<std::size_t>(time * sr_));
        const float slow = 1.0f - std::exp(-1.0f / ((0.2f + 4.0f * time) * sr_));
        const float repair = 1.0f - std::exp(-1.0f / ((0.05f + 2.0f * time) * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            const float a0 = a ? safe(a->at(0, f), 0.0f) : 0.0f;
            const float b0 = b ? safe(b->at(0, f), 0.0f) : 0.0f;
            follow(ea_, std::fabs(a0));
            follow(eb_, std::fabs(b0));
            sa_ += (ea_ - sa_) * slow;
            sb_ += (eb_ - sb_) * slow;
            envLine_[w_] = ea_;
            const float eaLate = envLine_[(w_ + envLine_.size() - lag) % envLine_.size()];
            w_ = (w_ + 1) % envLine_.size();
            const float sum = ea_ + eb_ + 1.0e-6f;
            meet_ = std::min(ea_, eb_) / (std::max(ea_, eb_) + 1.0e-6f);
            if (ea_ + eb_ < 1.0e-4f) meet_ = 0.0f;
            // o atrito sobe com o encontro e se repara devagar
            friction_ += ((meet_ > friction_) ? (meet_ - friction_) * 0.01f
                                              : (meet_ - friction_) * repair);

            float ga = 1.0f, gb = 1.0f, ring = 0.0f;
            switch (verb) {
            case 0: {   // SOMBRA
                const float shade = std::min(1.0f, eaLate / (std::max(ea_, sa_) + 1.0e-3f));
                gb = (1.0f - inten) + inten * std::min(1.0f, eaLate * 4.0f) * std::max(shade, 0.0f);
                break;
            }
            case 1:     // HOQUETO
                ga = 1.0f - inten * eb_ / sum;
                gb = 1.0f - inten * ea_ / sum;
                break;
            case 2: {   // CONTÁGIO
                const float m = std::max({sa_, sb_, 1.0e-4f});
                ga = (1.0f - inten) + inten * sb_ / m;
                gb = (1.0f - inten) + inten * sa_ / m;
                break;
            }
            case 3:     // ATRITO
                ring = inten * friction_;
                break;
            default:    // ANEL
                ring = inten;
                break;
            }
            for (std::size_t c = 0; c < channels; ++c) {
                const float ac = a ? safe(a->at(a->channels() > c ? c : 0, f), 0.0f) : 0.0f;
                const float bc = b ? safe(b->at(b->channels() > c ? c : 0, f), 0.0f) : 0.0f;
                const float oa = ac * ga, ob = bc * gb;
                float mix = 0.5f * (oa + ob);
                if (ring > 0.0f) mix = mix * (1.0f - (verb == 4 ? ring : 0.0f)) + ring * std::tanh(ac * bc * 2.0f);
                outputs[0].at(c, f) = oa;
                outputs[1].at(c, f) = ob;
                outputs[2].at(c, f) = std::clamp(mix, -2.0f, 2.0f);
                outputs[3].at(c, f) = meet_;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    void follow(float& e, const float x) const noexcept {
        e += (x - e) * (x > e ? att_ : rel_);
    }

    float sr_ = 48000.0f;
    std::vector<float> envLine_;
    std::size_t w_ = 0;
    float ea_ = 0, eb_ = 0, sa_ = 0, sb_ = 0, friction_ = 0, meet_ = 0;
    float att_ = 0.1f, rel_ = 0.01f;
};

}  // namespace rasgo::modular
