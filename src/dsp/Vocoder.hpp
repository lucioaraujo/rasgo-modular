#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// VOCODER — vocoder de N bandas (Módulo 60)
// ============================================================================
//
// O canal vocoder de Homer Dudley (Bell Labs, 1938): a envoltória de
// energia de um MODULADOR (voz, fala) por banda de frequência controla o
// ganho da mesma banda numa PORTADORA (um sinal rico — serra, pad,
// ruído). A portadora "fala".
//
// O `FORMANT` ganhou um `mode` vocoder de 5 bandas (as ressonâncias de
// vogal). Este é o vocoder DEDICADO: até 20 bandas log-espaçadas de
// 80 Hz a 8 kHz → fala inteligível de banda larga, não só vocálica.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/60_vocoder.md`.
//
// - `bands`     (4–20)   quantas bandas. Mais = mais inteligível, menos
//                        "robô".
// - `shift`     (−1..1)  desloca as frequências da SÍNTESE em relação às
//                        da análise (2^(shift·1,2)) — formant shift: voz
//                        maior/menor sem mudar a fala.
// - `attack`    (0–1)    ataque dos seguidores de envelope (1–60 ms).
// - `release`   (0–1)    release (20–600 ms). Curto = staccato, longo =
//                        as consoantes borram.
// - `sibilance` (0–1)    quanto do agudo do modulador (> ~3,5 kHz) passa
//                        DIRETO — as consoantes fricativas (s, f, ch) que
//                        a análise de banda não pega bem.
// - `freeze`    (0/1)    congela as envoltórias — a portadora fica
//                        "falando a última sílaba" para sempre.
// - `mix`       (0–1)    portadora seca ↔ vocodada.
//
// Portadora livre → uma serra interna afinável por CV (`pitch`) fala
// sozinha a partir do `mod`. `process()` não aloca. Determinístico.

namespace rasgo::modular {

class Vocoder final : public Signal {
public:
    Vocoder()
        : Signal(
              {{"carrier", PortKind::Audio, ""},
               {"mod", PortKind::Audio, ""},
               {"pitch", PortKind::Control, "v/oct"}},
              {{"out", PortKind::Audio, ""}},
              {{"bands", 4.0f, 20.0f, 16.0f, ""},
               {"shift", -1.0f, 1.0f, 0.0f, ""},
               {"attack", 0.0f, 1.0f, 0.15f, ""},
               {"release", 0.0f, 1.0f, 0.35f, ""},
               {"sibilance", 0.0f, 1.0f, 0.35f, ""},
               {"freeze", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "VOCODER"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "VOCODER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "vocoder", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "BANDS", "bands", 9.0f, 32.0f);
        p.add(Widget::Kind::Knob, "SHIFT", "shift", 27.0f, 32.0f);
        p.add(Widget::Kind::Knob, "SIBIL", "sibilance", 45.0f, 32.0f);
        p.add(Widget::Kind::Knob, "ATK", "attack", 9.0f, 56.0f);
        p.add(Widget::Kind::Knob, "REL", "release", 27.0f, 56.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 45.0f, 56.0f);
        p.add(Widget::Kind::Toggle, "FRZ", "freeze", 9.0f, 80.0f);
        p.add(Widget::Kind::Jack, "CAR", "in:carrier", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "MOD", "in:mod", 22.0f, 100.0f);
        p.add(Widget::Kind::Jack, "PIT", "in:pitch", 36.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        for (int k = 0; k < kMax; ++k) {
            anaBp_[k] = Svf{};
            synBp_[k] = Svf{};
            env_[k] = 0.0f;
        }
        sibHp_ = Svf{};
        carPh_ = 0.0;
        rng_ = 0x70CDE12A9F0CADE1ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();

        const AudioBlock* carB = inputs[0];
        const AudioBlock* modB = inputs[1];
        const AudioBlock* pitchB = inputs[2];

        const int N = static_cast<int>(std::lround(
            clampf(parameterValue("bands"), 4.0f, static_cast<float>(kMax))));
        const float shift = clampf(parameterValue("shift"), -1.0f, 1.0f);
        const float atkP = clamp01(parameterValue("attack"));
        const float relP = clamp01(parameterValue("release"));
        const float sib = clamp01(parameterValue("sibilance"));
        const bool freeze = parameterValue("freeze") >= 0.5f;
        const float mix = clamp01(parameterValue("mix"));

        const float atkC = 1.0f - std::exp(
            -1.0f / ((0.001f + atkP * atkP * 0.06f) * sr_));
        const float relC = 1.0f - std::exp(
            -1.0f / ((0.02f + relP * relP * 0.58f) * sr_));
        const float synMul = std::exp2(shift * 1.2f);

        // frequências das bandas: log de 80 Hz a 8 kHz
        const float f0 = 80.0f, f1 = std::min(8000.0f, sr_ * 0.45f);
        const float sibHz = std::min(3500.0f, sr_ * 0.4f);

        for (std::size_t f = 0; f < frames; ++f) {
            const float carIn = carB ? carB->at(0, f) : 0.0f;
            const float m = modB ? modB->at(0, f) : 0.0f;

            // portadora interna se nada cabeado: serra afinável
            float car = carIn;
            if (!carB) {
                const float pc = pitchB ? pitchB->at(0, f) : 0.0f;
                const float cf = clampf(110.0f * std::exp2(pc), 20.0f,
                                        sr_ * 0.45f);
                carPh_ += static_cast<double>(cf) / sr_;
                carPh_ -= std::floor(carPh_);
                car = 0.8f * (2.0f * static_cast<float>(carPh_) - 1.0f)
                    + 0.12f * (rng01() - 0.5f);   // um fio de ruído (voz)
            }

            // largura de banda ~ constante em oitavas: `bwOct` por banda
            const float bwOct = 6.64f / static_cast<float>(N);   // oitavas
            const float q = 0.5f * (std::pow(2.0f, bwOct) - 1.0f)
                          / std::pow(2.0f, bwOct * 0.5f);         // ≈ 1/Q

            float wet = 0.0f;
            for (int k = 0; k < N; ++k) {
                const float t = static_cast<float>(k)
                              / static_cast<float>(N - 1);
                const float fb = f0 * std::pow(f1 / f0, t);

                // análise: energia do modulador nessa banda. O `·q`
                // normaliza o ganho ressonante do SVF pra ~unitário.
                const float ab = anaBp_[k].runBand(m, fb, q, sr_) * q;
                const float amag = std::fabs(ab);
                if (!freeze)
                    env_[k] += (amag - env_[k])
                             * (amag > env_[k] ? atkC : relC);
                // expansão pra baixo (~2:1): segue a envoltória do
                // modulador mais fielmente — o piso de ruído não "abre"
                // a banda (as consoantes ficam, o chiado não)
                const float e = env_[k];
                const float ge = e * clampf(e * 7.0f, 0.05f, 1.0f);

                // síntese: a mesma banda da portadora, deslocada por SHIFT
                const float sf = clampf(fb * synMul, 20.0f, sr_ * 0.47f);
                const float sb = synBp_[k].runBand(car, sf, q, sr_) * q;
                wet += sb * ge;
            }
            wet *= 42.0f;   // makeup do banco (compensa o ·q duplo)

            // sibilância: agudo do modulador direto (as fricativas)
            if (sib > 0.0f) {
                const float sh = sibHp_.runHigh(m, sibHz, 0.5f, sr_);
                wet += sh * sib * 0.9f;
            }

            wet = std::tanh(wet * 0.8f);
            const float y = carIn * (1.0f - mix) + wet * mix;
            for (std::size_t c = 0; c < out.channels(); ++c) out.at(c, f) = y;
        }
    }

private:
    static constexpr int kMax = 20;

    struct Svf {
        float ic1eq = 0.0f, ic2eq = 0.0f;
        float runBand(const float x, const float fc, const float k,
                      const float sr) noexcept {
            const float g = std::tan(3.14159265f * clampf(fc, 10.0f,
                                     sr * 0.49f) / sr);
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float v1 = a1 * (ic1eq + g * (x - ic2eq));
            const float v2 = ic2eq + g * v1;
            ic1eq = 2.0f * v1 - ic1eq;
            ic2eq = 2.0f * v2 - ic2eq;
            return v1;   // bandpass
        }
        float runHigh(const float x, const float fc, const float k,
                      const float sr) noexcept {
            const float g = std::tan(3.14159265f * clampf(fc, 10.0f,
                                     sr * 0.49f) / sr);
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float v1 = a1 * (ic1eq + g * (x - ic2eq));
            const float v2 = ic2eq + g * v1;
            ic1eq = 2.0f * v1 - ic1eq;
            ic2eq = 2.0f * v2 - ic2eq;
            return x - k * v1 - v2;   // highpass
        }
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rng01() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(rng_ >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    Svf anaBp_[kMax];
    Svf synBp_[kMax];
    Svf sibHp_;
    float env_[kMax] = {};
    double carPh_ = 0.0;
    std::uint64_t rng_ = 0x70CDE12A9F0CADE1ULL;
};

}  // namespace rasgo::modular
