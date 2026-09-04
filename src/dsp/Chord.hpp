#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// CHORD — VCO parafônico (Módulo 26)
// ============================================================================
//
// O `OSC` é monofônico. `HARMONY` gera CV de acordes mas nada RENDERIZA
// um acorde como áudio. `CHORD` empilha 2–4 vozes de uma base 1 V/oct,
// com o formato por parâmetro OU por CV (casável com o `HARMONY` no
// controle → progressões que tocam sozinhas).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/26_chord.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Mutable Plaits (modelo "chord"); Harmonaig / Chord Machine;
//   - super-saw (JP-8000) — serras destoadas = coro;
//   - PolyBLEP (Välimäki/Finke); tabelas de acorde (fato musical).

namespace rasgo::modular {

class Chord final : public Signal {
public:
    Chord()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"chord_cv", PortKind::Control, ""},
               {"fm", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 16.0f, 4000.0f, 110.0f, "Hz"},
               {"chord", 0.0f, 1.0f, 0.3f, ""},
               {"voices", 2.0f, 4.0f, 3.0f, ""},
               {"inversion", 0.0f, 1.0f, 0.0f, ""},
               {"voicing", 0.0f, 1.0f, 0.0f, ""},
               {"detune", 0.0f, 1.0f, 0.15f, ""},
               {"wave", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CHORD"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "CHORD", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "voices", "", 2.5f, 7.0f, 46.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "CHORD", "chord", 28.0f, 26.0f);
        p.add(Widget::Kind::Knob, "VOX", "voices", 48.0f, 26.0f);
        p.add(Widget::Kind::Knob, "INV", "inversion", 8.0f, 48.0f);
        p.add(Widget::Kind::Knob, "DTUNE", "detune", 28.0f, 48.0f);
        p.add(Widget::Kind::Knob, "WAVE", "wave", 48.0f, 48.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 8.0f, 70.0f);
        p.add(Widget::Kind::Knob, "VLEAD", "voicing", 28.0f, 70.0f);
        p.add(Widget::Kind::Jack, "PITCH", "in:pitch", 6.0f, 110.0f);
        p.add(Widget::Kind::Jack, "CHRD", "in:chord_cv", 20.0f, 110.0f);
        p.add(Widget::Kind::Jack, "FM", "in:fm", 34.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 48.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        for (int k = 0; k < 4; ++k) {
            phase_[k] = 0.13f * static_cast<float>(k);   // fases distintas
            driftCur_[k] = driftTgt_[k] = 0.0f;
        }
        driftCounter_ = 0;
        driftInterval_ = static_cast<std::uint32_t>(std::max(1.0f, sr_ / 6.0f));
        rng_ = 0xCBF29CE484222325ULL;
        glideC_ = 1.0f - std::exp(-1.0f / (0.04f * sr_));  // ~40 ms
        for (int k = 0; k < 4; ++k) voiceSemi_[k] = voiceTarget_[k] = 0.0f;
        prevCi_ = -1;
        prevInvN_ = -1;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        static constexpr int kChords[10][4] = {
            {0, 0, 0, 0},     // uníssono
            {0, 12, 0, 12},   // oitavas
            {0, 7, 12, 19},   // quinta / power
            {0, 4, 7, 12},    // maior
            {0, 3, 7, 12},    // menor
            {0, 5, 7, 12},    // sus4
            {0, 4, 7, 11},    // maj7
            {0, 3, 7, 10},    // min7
            {0, 3, 6, 9},     // dim
            {0, 4, 7, 14},    // add9
        };

        const float freq = parameterValue("freq");
        const float chordP = clamp01(parameterValue("chord"));
        const int nv = clampi(static_cast<int>(
            std::lround(parameterValue("voices"))), 2, 4);
        const float inv01 = clamp01(parameterValue("inversion"));
        const bool voiceLead = parameterValue("voicing") >= 0.5f;
        const float detune = clamp01(parameterValue("detune"));
        const float wave = clamp01(parameterValue("wave"));
        const float drift = clamp01(parameterValue("drift"));
        const float driftAmp = 0.04f * drift;
        const float norm = 0.5f / std::sqrt(static_cast<float>(nv));

        const AudioBlock* pitchIn = inputs[0];
        const AudioBlock* chordCv = inputs[1];
        const AudioBlock* fmIn = inputs[2];

        for (std::size_t f = 0; f < frames; ++f) {
            if (drift > 0.0f && ++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                for (int k = 0; k < 4; ++k) driftTgt_[k] = noise() * driftAmp;
            }
            for (int k = 0; k < 4; ++k)
                driftCur_[k] += (driftTgt_[k] - driftCur_[k]) * 0.003f;

            const float pv = pitchIn != nullptr ? pitchIn->at(0, f) : 0.0f;
            const float base = freq * fastExp2(pv);
            const float cc = chordCv != nullptr ? chordCv->at(0, f) : 0.0f;
            const int ci = clampi(static_cast<int>(
                std::lround((chordP + cc) * 9.0f)), 0, 9);
            const int invN = clampi(static_cast<int>(std::lround(inv01 * 3.0f)),
                                    0, 3);
            const float fm = fmIn != nullptr ? fmIn->at(0, f) : 0.0f;

            // ---- alvos das vozes (paralelo vs condução de vozes) ----
            float rawT[4];
            for (int k = 0; k < 4; ++k)
                rawT[k] = static_cast<float>(kChords[ci][k])
                    + (k < invN ? 12.0f : 0.0f);
            if (!voiceLead) {
                for (int k = 0; k < nv; ++k) voiceTarget_[k] = rawT[k];
            } else if (ci != prevCi_ || invN != prevInvN_) {
                // cada voz vai pro tom do novo acorde MAIS PRÓXIMO do que
                // ela toca agora (ajuste de oitava), atribuição gulosa
                bool claimed[4] = {false, false, false, false};
                for (int k = 0; k < nv; ++k) {
                    int best = 0;
                    float bestDist = 1.0e9f, bestVal = rawT[0];
                    for (int j = 0; j < nv; ++j) {
                        if (claimed[j]) continue;
                        const float oct =
                            std::round((voiceSemi_[k] - rawT[j]) / 12.0f);
                        const float cand = rawT[j] + 12.0f * oct;
                        const float d = std::fabs(cand - voiceSemi_[k]);
                        if (d < bestDist) { bestDist = d; best = j; bestVal = cand; }
                    }
                    claimed[best] = true;
                    voiceTarget_[k] = bestVal;
                }
            }
            prevCi_ = ci;
            prevInvN_ = invN;
            const float glideCoef = voiceLead ? glideC_ : 1.0f;
            for (int k = 0; k < nv; ++k)
                voiceSemi_[k] += (voiceTarget_[k] - voiceSemi_[k]) * glideCoef;

            float acc = 0.0f;
            for (int k = 0; k < nv; ++k) {
                float semi = voiceSemi_[k];
                semi += detune * 0.25f
                    * (static_cast<float>(k) - 0.5f * static_cast<float>(nv - 1));
                semi += driftCur_[k];

                float fk = base * fastExp2(semi * (1.0f / 12.0f));
                fk += fm * base * 0.5f;
                fk = fk < 0.0f ? 0.0f : (fk > sr_ * 0.48f ? sr_ * 0.48f : fk);
                const float inc = fk / sr_;

                phase_[k] += inc;
                phase_[k] -= std::floor(phase_[k]);

                // serra com PolyBLEP na descontinuidade
                float saw = 2.0f * phase_[k] - 1.0f;
                saw -= polyBlep(phase_[k], inc);
                // pulso 50 %: serra − serra deslocada meio ciclo
                float p2 = phase_[k] + 0.5f;
                p2 -= std::floor(p2);
                float saw2 = 2.0f * p2 - 1.0f - polyBlep(p2, inc);
                const float pulse = saw - saw2;
                // triângulo: integrador leaky do pulso
                triState_[k] += (pulse * inc * 4.0f - triState_[k] * 0.0005f);
                const float tri = clampf(triState_[k], -1.0f, 1.0f);

                float v;
                if (wave < 0.5f) v = saw + (pulse - saw) * (wave * 2.0f);
                else v = pulse + (tri - pulse) * ((wave - 0.5f) * 2.0f);
                acc += v;
            }

            const float y = acc * norm;
            for (std::size_t c = 0; c < channels; ++c)
                out.at(c, f) = y;
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // 2^x rápido o suficiente pra afinação (erro << 1 cent na faixa útil)
    static float fastExp2(const float x) noexcept { return std::exp2(x); }

    static float polyBlep(float t, const float dt) noexcept {
        if (dt <= 0.0f) return 0.0f;
        if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float sr_ = 48000.0f;
    float phase_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float triState_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float voiceSemi_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float voiceTarget_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float glideC_ = 1.0f;
    int prevCi_ = -1;
    int prevInvN_ = -1;
    float driftCur_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float driftTgt_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 8000;
    std::uint64_t rng_ = 0xCBF29CE484222325ULL;
};

}  // namespace rasgo::modular
