#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/Oversampler.hpp"

#include <array>
#include <cmath>

// ============================================================================
// WASP — Filtro áspero (Módulo 32)
// ============================================================================
//
// O `FILTER` (Módulo 2) é o multimodo LIMPO. Este é o oposto: 12 dB/oitava
// com o GRÃO do EDP Wasp (1978) — onde os "amp-ops" do Sallen-Key são
// inversores CMOS 4069 que ceifam DURO e ASSIMÉTRICO. É a voz que grita:
// distorce quando ressoa, a auto-oscilação é reedy, o `drive` transforma o
// filtro num waveshaper com corte.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/32_wasp.md`.
//
// Núcleo: SVF TPT/trapezoidal de Simper/Cytomic (paper público, já reescrito
// no `FILTER`) — mesma base estável de 2 polos que auto-oscila (amortecimento
// negativo perto de resonance=1), com um ceifador NO LAÇO muito mais duro e
// ASSIMÉTRICO (`grit` abaixa o joelho até virar quase-quadrada; `bias` dá o
// teto assimétrico do inversor CMOS). Fontes ESTUDADAS (conceito, não código):
//   - circuito do EDP Wasp (análises independentes, René Schmitz, DIY);
//   - inversor CMOS 4069 como amplificador (curva íngreme, assimetria);
//   - SVF TPT não-linear (Zavalishin / Simper-Cytomic).
// NÃO consultado: service manual da Doepfer (cliente-only), plugin VCV
// "Doepfer" (proprietário) — ver `PESQUISA_MODULOS.md §2.2`.
//
// Determinístico: o `drift` é um seno, sem RNG.

namespace rasgo::modular {

class Wasp final : public Signal {
public:
    Wasp()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"cutoff_mod", PortKind::Control, "v/oct"},
               {"res_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"cutoff", 20.0f, 24000.0f, 700.0f, "Hz"},
               {"resonance", 0.0f, 1.0f, 0.35f, ""},
               {"mode", 0.0f, 1.0f, 0.0f, ""},
               {"drive", 0.1f, 8.0f, 1.0f, ""},
               {"grit", 0.0f, 1.0f, 0.45f, ""},
               {"bias", -1.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "WASP"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "WASP", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "response", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "CUT", "cutoff", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RESO", "resonance", 30.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 8.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 30.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GRIT", "grit", 8.0f, 70.0f);
        p.add(Widget::Kind::Knob, "BIAS", "bias", 30.0f, 70.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 19.0f, 87.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 6.0f, 113.0f);
        p.add(Widget::Kind::Jack, "FC", "in:cutoff_mod", 18.0f, 113.0f);
        p.add(Widget::Kind::Jack, "Q", "in:res_mod", 30.0f, 113.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 42.0f, 113.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        osSr_ = sr_ * 2.0f;  // o núcleo não-linear roda a 2×
        smoothCutoff_ = parameterValue("cutoff");
        paramCoeff_ = std::exp(-1.0f / (0.005f * sr_));
        nyquist_ = sr_ * 0.45f;
        driftPhase_ = 0.0f;
        driftInc_ = 2.0f * 3.14159265f * 0.03f
                    * static_cast<float>(blockSize) / sr_;
        os_.reset();
        for (auto& s : state_) {
            s = State{};
            s.ic1 = 1e-3f;  // semente inaudível: arranca a auto-oscilação
        }
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const AudioBlock* in = inputs[0];
        const AudioBlock* cutoffMod = inputs[1];
        const AudioBlock* resMod = inputs[2];
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float cutoffParam = parameterValue("cutoff");
        const float resParam = parameterValue("resonance");
        const float modeParam = clampf(parameterValue("mode"), 0.0f, 1.0f);
        const float driveParam = clampf(parameterValue("drive"), 0.1f, 8.0f);
        const float grit = clampf(parameterValue("grit"), 0.0f, 1.0f);
        const float bias = clampf(parameterValue("bias"), -1.0f, 1.0f);
        const float drift = clampf(parameterValue("drift"), 0.0f, 1.0f);

        const float inGain = 1.0f + (driveParam - 0.1f) * 4.5f;
        const float th = 1.0f - grit * 0.8f;
        // estágio de saída (o inversor de saída do Wasp também ceifa) —
        // aqui os harmônicos gerados NÃO voltam a ser filtrados, então é o
        // que dá o "buzz" reedy à auto-oscilação e o grão ao áudio filtrado
        const float outGain = 1.0f + grit * 2.0f;

        driftPhase_ += driftInc_;
        if (driftPhase_ > 6.2831853f) driftPhase_ -= 6.2831853f;
        const float driftOct = drift * 0.15f * std::sin(driftPhase_);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            smoothCutoff_ =
                cutoffParam + paramCoeff_ * (smoothCutoff_ - cutoffParam);
            float fc = smoothCutoff_;
            if (cutoffMod != nullptr)
                fc *= std::exp2(cutoffMod->at(0, frame) + driftOct);
            else if (driftOct != 0.0f)
                fc *= std::exp2(driftOct);
            fc = clampf(fc, 20.0f, nyquist_);

            float res = resParam + (resMod ? resMod->at(0, frame) : 0.0f);
            res = clampf(res, 0.0f, 1.0f);
            const float q = 0.5f * std::exp2(res * 8.0f);
            // amortecimento negativo só perto de resonance=1 -> auto-oscila
            const float oscPush =
                res > 0.88f ? (res - 0.88f) * 8.0f : 0.0f;
            const float k = 1.0f / q - oscPush * oscPush * 0.05f;
            // coefs do SVF no RATE DOBRADO (o núcleo não-linear roda a 2×)
            const float g = std::tan(3.14159265f * fc / osSr_);
            const float a1 = 1.0f / (1.0f + g * (g + k));

            for (std::size_t c = 0; c < channels; ++c) {
                State& st = state_[c];
                float x = in ? in->at(c, frame) : 0.0f;
                if (!std::isfinite(x)) x = 0.0f;

                // núcleo não-linear (entrada + SVF TPT com wsat no laço +
                // mistura de modo + wsat de saída), rodado a 2× pelo
                // oversampler — é o que aliasa com `grit`/`drive` altos
                const auto core = [&](const float xs) noexcept -> float {
                    const float xin = wsat(xs * inGain, th, bias);
                    const float v1 = a1 * (st.ic1 + g * (xin - st.ic2));
                    const float v2 = st.ic2 + g * v1;
                    st.ic1 = 2.0f * wsat(v1, th, bias) - st.ic1;
                    st.ic2 = 2.0f * wsat(v2, th, bias) - st.ic2;
                    const float lp = v2;
                    const float bp = v1;
                    const float hp = xin - k * v1 - v2;
                    const float ym = modeParam < 0.5f
                        ? lp + (bp - lp) * (modeParam * 2.0f)
                        : bp + (hp - bp) * ((modeParam - 0.5f) * 2.0f);
                    return wsat(ym * outGain, th, bias);
                };
                const float y = os_.process(c, x, core);

                // bloqueador de DC (o bias assimétrico injeta offset)
                const float dc = y - st.dcX1 + 0.9985f * st.dcY1;
                st.dcX1 = y;
                st.dcY1 = dc;

                out.at(c, frame) = softLimit(dc);
            }
        }
    }

private:
    struct State {
        float ic1 = 0.0f;
        float ic2 = 0.0f;
        float dcX1 = 0.0f;
        float dcY1 = 0.0f;
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // ceifador com joelho ajustável (linear até ±knee, soft-clip pro teto) e
    // ASSIMÉTRICO por `bias` — o inversor CMOS não comuta em Vdd/2 exato;
    // `bias` > 0 baixa o teto positivo e sobe o negativo (harmônicos pares).
    static float wsat(const float v, const float th, const float bias) noexcept {
        const bool neg = v < 0.0f;
        const float a = neg ? -v : v;
        const float ceil = clampf(neg ? 1.0f + bias * 0.42f : 1.0f - bias * 0.42f,
                                  0.35f, 1.65f);
        const float knee = th * ceil;
        if (a <= knee) return v;
        const float over = (a - knee) / (ceil - knee + 0.05f);
        const float y = knee + (ceil - knee) * (1.0f - std::exp(-over));
        return neg ? -y : y;
    }

    static float softLimit(const float v) noexcept {
        if (v > 0.85f) return 0.85f + 0.1f * std::tanh((v - 0.85f) * 5.0f);
        if (v < -0.85f) return -0.85f + 0.1f * std::tanh((v + 0.85f) * 5.0f);
        return v;
    }

    float sr_ = 48000.0f;
    float osSr_ = 96000.0f;
    float smoothCutoff_ = 700.0f;
    float paramCoeff_ = 0.0f;
    float nyquist_ = 21600.0f;
    float driftPhase_ = 0.0f;
    float driftInc_ = 0.0f;
    std::array<State, AudioBlock::maxChannels> state_{};
    Oversampler2x os_{};
};

}  // namespace rasgo::modular
