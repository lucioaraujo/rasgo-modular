#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// PRISM (#71) — o som através de um prisma (2026-11-09, ideia do autor:
// "ideias ópticas, refração").
//
// Num prisma, cada cor tem um índice de refração: o azul anda mais devagar
// que o vermelho dentro do vidro e sai noutro ângulo (dispersão normal). O
// PRISM faz isso com o som: separa a entrada em 6 bandas (passa-faixas de
// variável de estado, centros 120 Hz … 11 kHz), atrasa cada banda conforme o
// seu "índice" e abre as bandas em leque no estéreo — um arco-íris no tempo e
// no espaço. Ideia e implementação próprias (o Cardinal não tem módulos de
// óptica).
//
//   atraso(banda b) = DISP · forma(b)       forma: CURVE > 0 → agudos mais
//                                           lentos (normal); < 0 → graves
//                                           (anômala); 0 → todas iguais
//
// Desvio Rasgo:
//  (a) dispersão ANÔMALA (CURVE negativa) — o que o vidro comum não faz;
//  (b) FEEDBACK por banda — cada cor volta ao prisma: arco-íris em cascata,
//      ecos que se separam em cores a cada volta.
//
// Saídas: OUT (seco/molhado por MIX, em estéreo o leque SPREAD), WET.
// As linhas são alocadas no prepare(); process() não aloca.
namespace rasgo::modular {

class Prism final : public Signal {
public:
    static constexpr int kBands = 6;

    Prism() : Signal(
        {{"in", PortKind::Audio, ""},
         {"disp_cv", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""},
         {"wet", PortKind::Audio, ""}},
        {{"disp", 0.0f, 400.0f, 120.0f, "ms"},
         {"curve", -1.0f, 1.0f, 1.0f, ""},
         {"spread", 0.0f, 1.0f, 0.7f, ""},
         {"feedback", 0.0f, 0.85f, 0.3f, ""},
         {"mix", 0.0f, 1.0f, 0.5f, ""}}) {}

    std::string type() const override { return "PRISM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "PRISM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "prism", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "DISP", "disp", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CURVE", "curve", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FDBK", "feedback", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "DISP", "in:disp_cv", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "WET", "out:wet", 19.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        const auto len = static_cast<std::size_t>(0.42f * sr_) + 8;
        static constexpr float kCenters[kBands] = {120.0f, 300.0f, 750.0f, 1900.0f, 4700.0f, 11000.0f};
        for (int b = 0; b < kBands; ++b) {
            auto& band = bands_[static_cast<std::size_t>(b)];
            band.line.assign(len, 0.0f);
            band.w = 0;
            band.lo = band.bp = 0.0f;
            const float fc = std::min(kCenters[b], 0.4f * sr_);
            band.g = std::tan(3.14159265f * fc / sr_);
            band.delaySm = -1.0f;
        }
    }

    // A forma da dispersão (0–1) da banda b — exposta para os testes.
    static float shape(int b, float curve) noexcept {
        const float x = static_cast<float>(b) / static_cast<float>(kBands - 1);   // 0 grave … 1 agudo
        if (curve >= 0.0f) return curve * x + (1.0f - curve) * 0.5f;
        return -curve * (1.0f - x) + (1.0f + curve) * 0.5f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2 || bands_[0].line.empty()) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* dcv = inputs.size() > 1 ? inputs[1] : nullptr;

        const float dispK = std::clamp(safe(parameterValue("disp"), 120.0f), 0.0f, 400.0f);
        const float curve = std::clamp(safe(parameterValue("curve"), 1.0f), -1.0f, 1.0f);
        const float spread = std::clamp(safe(parameterValue("spread"), 0.7f), 0.0f, 1.0f);
        const float fb = std::clamp(safe(parameterValue("feedback"), 0.3f), 0.0f, 0.85f);
        const float mix = std::clamp(safe(parameterValue("mix"), 0.5f), 0.0f, 1.0f);
        const float k = 1.0f / 1.4f;   // Q ≈ 1,4 (bandas largas, que se tocam)
        const float smooth = 1.0f - std::exp(-1.0f / (0.05f * sr_));
        const std::size_t len = bands_[0].line.size();

        std::array<float, kBands> panL{}, panR{};
        for (int b = 0; b < kBands; ++b) {
            const float x = (static_cast<float>(b) / static_cast<float>(kBands - 1) - 0.5f) * spread;   // −0,5..0,5
            const float a = (x + 0.5f) * 1.5707963f;
            panL[static_cast<std::size_t>(b)] = std::cos(a);
            panR[static_cast<std::size_t>(b)] = std::sin(a);
        }

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            float x = 0.0f;
            if (in) {
                x = safe(in->at(0, f), 0.0f);
                if (in->channels() > 1) x = 0.5f * (x + safe(in->at(1, f), 0.0f));
            }
            const float cv = dcv ? std::clamp(safe(dcv->at(0, f), 0.0f), -1.0f, 1.0f) : 0.0f;
            const float disp = std::clamp(dispK + 200.0f * cv, 0.0f, 400.0f);
            float wl = 0.0f, wr = 0.0f;
            for (int b = 0; b < kBands; ++b) {
                auto& band = bands_[static_cast<std::size_t>(b)];
                const float target = 0.001f * disp * shape(b, curve) * sr_ + 1.0f;
                if (band.delaySm < 0.0f) band.delaySm = target;
                band.delaySm += (target - band.delaySm) * smooth;
                float rp = static_cast<float>(band.w) - band.delaySm;
                while (rp < 0.0f) rp += static_cast<float>(len);
                const auto i0 = static_cast<std::size_t>(rp) % len;
                const auto i1 = (i0 + 1) % len;
                const float fr = rp - std::floor(rp);
                const float delayed = band.line[i0] + (band.line[i1] - band.line[i0]) * fr;
                // passa-faixa SVF (Zavalishin, TPT) na entrada + a realimentação da banda
                const float v = x + fb * delayed;
                const float hp = (v - (k + band.g) * band.bp - band.lo) / (1.0f + band.g * (band.g + k));
                const float bpo = band.g * hp + band.bp;
                band.bp = band.g * hp + bpo;
                const float lpo = band.g * bpo + band.lo;
                band.lo = band.g * bpo + lpo;
                // k·bp = passa-faixa normalizado (ganho 1 no centro): a
                // realimentação abaixo de 1 é estável
                const float bpOut = std::isfinite(bpo) ? k * bpo : 0.0f;
                if (!std::isfinite(band.bp) || !std::isfinite(band.lo)) { band.bp = 0.0f; band.lo = 0.0f; }
                band.line[band.w] = std::clamp(bpOut, -4.0f, 4.0f);
                band.w = (band.w + 1) % len;
                wl += delayed * panL[static_cast<std::size_t>(b)];
                wr += delayed * panR[static_cast<std::size_t>(b)];
            }
            for (std::size_t c = 0; c < channels; ++c) {
                const float dry = in ? safe(in->at(std::min<std::size_t>(c, in->channels() - 1), f), 0.0f) : 0.0f;
                const float w = channels > 1 ? (c == 0 ? wl : wr) : 0.7071f * (wl + wr);
                outputs[0].at(c, f) = std::clamp((1.0f - mix) * dry + mix * w, -4.0f, 4.0f);
                outputs[1].at(c, f) = std::clamp(w, -4.0f, 4.0f);
            }
        }
    }

private:
    struct Band {
        std::vector<float> line;
        std::size_t w = 0;
        float g = 0.1f, lo = 0.0f, bp = 0.0f;
        float delaySm = -1.0f;
    };
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    std::array<Band, kBands> bands_{};
};

} // namespace rasgo::modular
