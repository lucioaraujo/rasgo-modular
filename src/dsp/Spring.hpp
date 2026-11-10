#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/DelayRead.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// SPRING (#68) — reverb de mola (2026-11-09, aprovado pelo autor a partir do
// estudo do Cardinal, dossies/ESTUDO_cardinal_catalogo_e_licencas.md §4b).
//
// O som da mola de amplificador: o impulso vira um "chirp" — os graves chegam
// depois dos agudos (dispersão) — que se repete a cada ida e volta na mola.
// Formulação pública: V. Välimäki, J. Parker, J. S. Abel, "Parametric Spring
// Reverberation Effect", JAES 58(7/8), 2010 — um laço com atraso, uma cascata
// de all-pass "esticados" (H(z) = (a + z^-K)/(1 + a·z^-K), cada um atrasa mais
// os graves) e um passa-baixa. Conceito lido no Autinn Coil (GPL-3.0+);
// nenhum código copiado.
//
// Duas molas de comprimentos diferentes (razão 1 : 1,137) — em estéreo, uma
// em cada canal; em mono, somadas.
//
// Desvio Rasgo:
//  (a) KICK — um gatilho que SACODE o tanque: um estalo de ruído (semeado)
//      entra direto nas molas — o "crash" de quem chuta o amplificador;
//  (b) TENS por CV — a tensão (o comprimento do laço) pode andar, e a mola
//      inteira desafina e geme (com suavização, sem clique).
//
// Parâmetros: DECAY (realimentação), TENS (laço de 25 a 75 ms), DAMP (o tom:
// passa-baixa de 6 kHz a 1,2 kHz), DRIP (quanto chirp: o coeficiente dos
// all-pass), MIX. process() não aloca (as linhas são alocadas no prepare()).
namespace rasgo::modular {

class Spring final : public Signal {
public:
    static constexpr int kStages = 12;     // all-pass por mola
    static constexpr int kStretch = 3;     // atraso de cada all-pass (amostras)

    Spring() : Signal(
        {{"in", PortKind::Audio, ""},
         {"kick", PortKind::Control, "trig"},
         {"tension_cv", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""},
         {"wet", PortKind::Audio, ""}},
        {{"decay", 0.0f, 0.97f, 0.85f, ""},
         {"tension", 0.0f, 1.0f, 0.5f, ""},
         {"damp", 0.0f, 1.0f, 0.4f, ""},
         {"drip", 0.0f, 1.0f, 0.6f, ""},
         {"mix", 0.0f, 1.0f, 0.35f, ""}}) {}

    std::string type() const override { return "SPRING"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "SPRING", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "spring", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "DECAY", "decay", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TENS", "tension", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damp", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIP", "drip", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "KICK", "in:kick", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "TENS", "in:tension_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "WET", "out:wet", 19.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        const auto maxLen = static_cast<std::size_t>(0.09f * sr_) + 8;
        for (auto& s : springs_) {
            s.line.assign(maxLen, 0.0f);
            s.w = 0;
            for (auto& st : s.ap) { st.x.fill(0.0f); st.y.fill(0.0f); st.i = 0; }
            s.lp = 0.0f;
            s.dcx = s.dcy = 0.0f;
        }
        rng_ = 0x5B21A6C0FFEE0068ULL;
        prevKick_ = 0.0f;
        kickLeft_ = 0;
        kickEnv_ = 0.0f;
        tensionSm_ = -1.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2 || springs_[0].line.empty()) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* kick = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* tcv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float decay = std::clamp(safe(parameterValue("decay"), 0.85f), 0.0f, 0.97f);
        const float tensionK = std::clamp(safe(parameterValue("tension"), 0.5f), 0.0f, 1.0f);
        const float damp = std::clamp(safe(parameterValue("damp"), 0.4f), 0.0f, 1.0f);
        const float drip = std::clamp(safe(parameterValue("drip"), 0.6f), 0.0f, 1.0f);
        const float mix = std::clamp(safe(parameterValue("mix"), 0.35f), 0.0f, 1.0f);
        const float a = 0.35f + 0.4f * drip;                   // coeficiente dos all-pass
        const float cutoff = 6000.0f * std::exp2(-2.3f * damp); // 6 kHz → ~1,2 kHz
        const float lpCoef = 1.0f - std::exp(-6.2831853f * cutoff / sr_);
        const float smooth = 1.0f - std::exp(-1.0f / (0.05f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const std::size_t len = springs_[0].line.size();
        for (std::size_t f = 0; f < frames; ++f) {
            float x = 0.0f;
            if (in) {
                x = safe(in->at(0, f), 0.0f);
                if (in->channels() > 1) x = 0.5f * (x + safe(in->at(1, f), 0.0f));
            }
            if (kick) {
                const float k = safe(kick->at(0, f), 0.0f);
                if (prevKick_ < 0.5f && k >= 0.5f) {
                    kickLeft_ = static_cast<int>(0.03f * sr_);
                    kickEnv_ = 0.9f;
                }
                prevKick_ = k;
            }
            float shake = 0.0f;
            if (kickLeft_ > 0) {
                shake = kickEnv_ * (2.0f * unit() - 1.0f);
                kickEnv_ *= 0.9993f;
                --kickLeft_;
            }
            const float cv = tcv ? std::clamp(safe(tcv->at(0, f), 0.0f), -1.0f, 1.0f) : 0.0f;
            const float target = std::clamp(tensionK + 0.5f * cv, 0.0f, 1.0f);
            if (tensionSm_ < 0.0f) tensionSm_ = target;
            tensionSm_ += (target - tensionSm_) * smooth;
            const float baseDelay = (0.025f + 0.05f * tensionSm_) * sr_;

            std::array<float, 2> wet{};
            for (int s = 0; s < 2; ++s) {
                auto& sp = springs_[static_cast<std::size_t>(s)];
                const float d = std::min(static_cast<float>(len - 4), baseDelay * (s == 0 ? 1.0f : 1.137f));
                // leitura fracionária por Lagrange cúbico (10/11/2026; era
                // linear, que abafava os agudos conforme a tensão andava —
                // ver `DelayRead.hpp`); a tensão anda sem clique
                float y = readDelayLagrange(sp.line.data(), len, sp.w, d);
                for (auto& st : sp.ap) y = st.run(y, a);
                sp.lp += (y - sp.lp) * lpCoef;
                // anti-DC no laço
                const float dc = sp.lp - sp.dcx + 0.995f * sp.dcy;
                sp.dcx = sp.lp; sp.dcy = dc;
                float v = x + shake + decay * dc;
                if (!std::isfinite(v)) v = 0.0f;
                sp.line[sp.w] = std::clamp(v, -4.0f, 4.0f);
                sp.w = (sp.w + 1) % len;
                wet[static_cast<std::size_t>(s)] = dc;
            }
            for (std::size_t c = 0; c < channels; ++c) {
                const float dry = in ? safe(in->at(std::min<std::size_t>(c, in->channels() - 1), f), 0.0f) : 0.0f;
                const float w = channels > 1 ? wet[std::min<std::size_t>(c, 1)] : 0.5f * (wet[0] + wet[1]);
                outputs[0].at(c, f) = std::clamp((1.0f - mix) * dry + mix * w, -4.0f, 4.0f);
                outputs[1].at(c, f) = std::clamp(w, -4.0f, 4.0f);
            }
        }
    }

private:
    // All-pass "esticado": y[n] = a·x[n] + x[n−K] − a·y[n−K].
    struct Stage {
        std::array<float, kStretch> x{}, y{};
        int i = 0;
        float run(float in, float a) noexcept {
            const float xk = x[static_cast<std::size_t>(i)], yk = y[static_cast<std::size_t>(i)];
            const float out = a * in + xk - a * yk;
            x[static_cast<std::size_t>(i)] = in;
            y[static_cast<std::size_t>(i)] = out;
            i = (i + 1) % kStretch;
            return out;
        }
    };
    struct OneSpring {
        std::vector<float> line;
        std::size_t w = 0;
        std::array<Stage, kStages> ap{};
        float lp = 0.0f, dcx = 0.0f, dcy = 0.0f;
    };

    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::array<OneSpring, 2> springs_{};
    std::uint64_t rng_ = 0x5B21A6C0FFEE0068ULL;
    float prevKick_ = 0.0f;
    int kickLeft_ = 0;
    float kickEnv_ = 0.0f;
    float tensionSm_ = -1.0f;
};

} // namespace rasgo::modular
