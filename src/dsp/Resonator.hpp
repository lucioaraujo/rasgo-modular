#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// RESONATOR — ressoador modal excitado externamente (Módulo 55)
// ============================================================================
//
// O `MATTER` é voz AUTO-CONTIDA (exciter + física próprios). O `RESONATOR`
// é o Rings/Elements no modo "ressoador": um banco de ≤ 24 modos afinados
// que um sinal EXTERNO bate/arqueia — você toca com o que quiser (DRUM,
// NOISE, uma voz). As 3 saídas `low`/`mid`/`high` se CRUZAM quando você
// varre `tilt` — a relação entre elas é o processo (Three Sisters).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/55_resonator.md`.
//
// - `freq` (20–5000 Hz, +CV 1V/oct)  fundamental do banco
// - `structure` (0–1)                harmônico ↔ esticado/inarmônico
// - `partials` (1–24)                quantos modos
// - `decay` (0–1)                    tempo de anel (pluck ↔ drone)
// - `damp` (0–1)                     agudos decaem antes dos graves
// - `tilt` (−1..1)                   grave forte ↔ agudo forte (cruza low/high)
// - `position` (0–1)                 pente de pluck (0,5 = só ímpares)
// - `mix` (0–1)                      seco ↔ ressoado
//
// `in` livre → ruído interno de −36 dB arqueia o banco (modo autônomo).
// `strike` = impulso do exciter embutido. `mix=0` → bypass. Determinístico
// (ruído semeado). `process()` não aloca (coefs recalc só na mudança).

namespace rasgo::modular {

class Resonator final : public Signal {
public:
    Resonator()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"strike", PortKind::Control, "trig"},
               {"freq_mod", PortKind::Control, "v/oct"}},
              {{"low", PortKind::Audio, ""},
               {"mid", PortKind::Audio, ""},
               {"high", PortKind::Audio, ""}},
              {{"freq", 20.0f, 5000.0f, 220.0f, "Hz"},
               {"structure", 0.0f, 1.0f, 0.0f, ""},
               {"partials", 1.0f, 24.0f, 12.0f, ""},
               {"decay", 0.0f, 1.0f, 0.5f, ""},
               {"damp", 0.0f, 1.0f, 0.3f, ""},
               {"tilt", -1.0f, 1.0f, 0.0f, ""},
               {"position", 0.0f, 1.0f, 0.3f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "RESONATOR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "RESONATOR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "reso", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "STRC", "structure", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "PRTS", "partials", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DECAY", "decay", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damp", 24.0f, 54.0f);
        p.add(Widget::Kind::Knob, "TILT", "tilt", 40.0f, 54.0f);
        p.add(Widget::Kind::Knob, "POS", "position", 8.0f, 78.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 24.0f, 78.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "STRK", "in:strike", 20.0f, 100.0f);
        p.add(Widget::Kind::Jack, "FQM", "in:freq_mod", 32.0f, 100.0f);
        p.add(Widget::Kind::Jack, "LOW", "out:low", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "MID", "out:mid", 20.0f, 120.0f);
        p.add(Widget::Kind::Jack, "HIGH", "out:high", 32.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        for (int k = 0; k < kMax; ++k) {
            y1_[k] = y2_[k] = 0.0f;
            a1_[k] = a2_[k] = b0_[k] = g_[k] = 0.0f;
        }
        prevStrike_ = 0.0f;
        strikeEnv_ = 0.0f;
        rng_ = 0x5E50A70DE1A1C0FFULL;
        cP_ = -1;
        coefPrimed_ = false;
        rebuild(12, 220.0f, 0.0f, 0.5f, 0.3f, 0.0f, 0.3f);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& lo = outputs[0];
        AudioBlock& mi = outputs[1];
        AudioBlock& hi = outputs[2];
        const std::size_t frames = lo.frames();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* strikeB = inputs[1];
        const AudioBlock* fmB = inputs[2];

        const int P = static_cast<int>(std::lround(
            clampf(parameterValue("partials"), 1.0f, 24.0f)));
        const float structure = clamp01(parameterValue("structure"));
        const float decay = clamp01(parameterValue("decay"));
        const float damp = clamp01(parameterValue("damp"));
        const float tilt = clampf(parameterValue("tilt"), -1.0f, 1.0f);
        const float position = clamp01(parameterValue("position"));
        const float mix = clamp01(parameterValue("mix"));
        const float fmCv = fmB ? fmB->at(0, 0) : 0.0f;
        const float freq = clampf(
            parameterValue("freq") * std::pow(2.0f, fmCv), 10.0f,
            sr_ * 0.48f);

        if (P != cP_ || freq != cFreq_ || structure != cStruct_
            || decay != cDecay_ || damp != cDamp_ || tilt != cTilt_
            || position != cPos_)
            rebuild(P, freq, structure, decay, damp, tilt, position);
        if (!coefPrimed_) {
            for (int k = 0; k < kMax; ++k) {
                a1_[k] = ta1_[k]; a2_[k] = ta2_[k];
                b0_[k] = tb0_[k]; g_[k] = tg_[k];
            }
            coefPrimed_ = true;
        }
        // ~8 ms de deslize por amostra pros coeficientes modais — sem
        // chirp/estalo quando freq/estrutura/decay movem ao vivo
        const float cs = 1.0f - std::exp(-1.0f / (0.008f * sr_));

        const int loEnd = std::max(1, P / 3);
        const int miEnd = std::max(loEnd + 1, (2 * P) / 3);
        const float loN = 26.0f / std::sqrt(static_cast<float>(loEnd));
        const float miN = 26.0f / std::sqrt(
            static_cast<float>(std::max(1, miEnd - loEnd)));
        const float hiN = 26.0f / std::sqrt(
            static_cast<float>(std::max(1, P - miEnd)));

        const float burstDecay = std::exp(-1.0f / (0.003f * sr_));
        for (std::size_t f = 0; f < frames; ++f) {
            float x = inB ? inB->at(0, f)
                          : (rnd11() * 0.45f);
            const float st = strikeB ? strikeB->at(0, f) : 0.0f;
            if (st >= 0.5f && prevStrike_ < 0.5f) strikeEnv_ = 1.0f;
            prevStrike_ = st;
            if (strikeEnv_ > 1.0e-4f) {          // maço: rajada de ruído ~3 ms
                x += strikeEnv_ * rnd11() * 60.0f;
                strikeEnv_ *= burstDecay;
            }

            float bl = 0.0f, bm = 0.0f, bh = 0.0f;
            for (int k = 0; k < P; ++k) {
                a1_[k] += (ta1_[k] - a1_[k]) * cs;
                a2_[k] += (ta2_[k] - a2_[k]) * cs;
                b0_[k] += (tb0_[k] - b0_[k]) * cs;
                g_[k]  += (tg_[k]  - g_[k])  * cs;
                const float y = a1_[k] * y1_[k] + a2_[k] * y2_[k]
                              + b0_[k] * x;
                y2_[k] = y1_[k];
                y1_[k] = y;
                const float yn = y * g_[k];       // normaliza o ganho ressonante
                if (k < loEnd) bl += yn;
                else if (k < miEnd) bm += yn;
                else bh += yn;
            }
            bl = std::tanh(bl * loN);
            bm = std::tanh(bm * miN);
            bh = std::tanh(bh * hiN);

            const float oL = x + (bl - x) * mix;
            const float oM = x + (bm - x) * mix;
            const float oH = x + (bh - x) * mix;
            for (std::size_t c = 0; c < lo.channels(); ++c) lo.at(c, f) = oL;
            for (std::size_t c = 0; c < mi.channels(); ++c) mi.at(c, f) = oM;
            for (std::size_t c = 0; c < hi.channels(); ++c) hi.at(c, f) = oH;
        }
    }

private:
    static constexpr int kMax = 24;

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rnd11() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(static_cast<std::int32_t>(rng_ >> 32))
            / 2147483648.0f;
    }

    void rebuild(const int P, const float f0, const float structure,
                 const float decay, const float damp, const float tilt,
                 const float position) noexcept {
        cP_ = P; cFreq_ = f0; cStruct_ = structure; cDecay_ = decay;
        cDamp_ = damp; cTilt_ = tilt; cPos_ = position;
        const float B = structure * structure * 0.02f;
        const float baseT = 0.03f + decay * decay * 3.0f;
        const float nyq = sr_ * 0.45f;
        for (int k = 0; k < P; ++k) {
            const float ki = static_cast<float>(k + 1);
            float fk = ki * f0 * std::sqrt(1.0f + B * ki * ki);
            float fade = 1.0f;
            if (fk > nyq) { fade = std::max(0.0f, 1.0f - (fk - nyq) / nyq); fk = nyq; }
            const float tK = baseT
                * (1.0f - damp * (1.0f - 1.0f / std::sqrt(ki)));
            const float r = std::exp(-1.0f / std::max(1.0e-4f, tK * sr_));
            const float w = 6.2831853f * fk / sr_;
            ta1_[k] = 2.0f * r * std::cos(w);
            ta2_[k] = -r * r;
            float amp = std::pow(ki, -0.5f + tilt * 1.2f);
            amp *= std::fabs(std::sin(3.14159265f * ki * position));
            // normaliza pra o GANHO EM REGIME (excitação contínua) do modo
            // ser ~amp — assim `tilt` controla a amplitude direto, sem o
            // pólo perto de 1 dando um boost enorme aos graves
            tb0_[k] = amp;
            tg_[k] = (1.0f - r) * 2.0f * std::sin(w) * fade;   // 1/ganho ressonante
        }
    }

    float y1_[kMax] = {}, y2_[kMax] = {};
    float a1_[kMax] = {}, a2_[kMax] = {}, b0_[kMax] = {}, g_[kMax] = {};
    // alvos dos coeficientes (recalc na mudança de param); os de cima
    // deslizam por amostra pra cá — anti-zíper
    float ta1_[kMax] = {}, ta2_[kMax] = {}, tb0_[kMax] = {}, tg_[kMax] = {};
    bool coefPrimed_ = false;
    float prevStrike_ = 0.0f;
    float strikeEnv_ = 0.0f;
    std::uint64_t rng_ = 0x5E50A70DE1A1C0FFULL;
    int cP_ = -1;
    float cFreq_ = 0, cStruct_ = 0, cDecay_ = 0, cDamp_ = 0, cTilt_ = 0,
          cPos_ = 0;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
