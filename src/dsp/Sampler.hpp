#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/PitchShift.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SAMPLER — matéria gravada como voz (Módulo 48)
// ============================================================================
//
// O `MEMORY` (#7) é o buffer granular; o `LOOPER` (#41) o delay de linha.
// `SAMPLER` é o toca-fatias: um `trig` → um golpe de um trecho, com
// varispeed, reverse, repitch e desgaste.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/48_sampler.md`.
// Base de porte: `RASGO/NAVALHA2_JUCE` — `SlicePlayer` (de-click adaptativo,
// varispeed, reverse) de **Glerm Soares** (Navalha) / **Lúcio Araújo**
// (Navalha 2), GPL-3.0-or-later; `HeritagePitch` via `dsp/PitchShift.hpp`.
//
// - `start` (0–1)   ponto de partida dentro da fatia
// - `speed` (−1..1) varispeed bipolar: sinal·2^(|speed|·2) → ±0,25×–±4×;
//                   negativo = reverso
// - `slices` (1–16) divide o buffer gravado em N fatias iguais; `pos`
//                   (CV) escolhe qual
// - `repitch` (0–1) 0 = transposição muda a velocidade (varispeed);
//                   1 = transposição via pitch-shifter, velocidade solta
// - `wear` (0–1)    desgaste por disparo: jitter de início + redução de
//                   taxa + bit-crush — DETERMINÍSTICO (xorshift semeado
//                   no disparo)
// - `loop` (0/1)    one-shot ↔ loop da fatia
//
// GRAVAÇÃO: gate `rec` alto → grava `in` no buffer (até ~8 s). Ou o painel
// injeta um arquivo via `setBuffer()` (fora de `process()`). Sem buffer →
// silêncio. Determinístico (o `wear` semeia no disparo).
//
// `prepare()` aloca o buffer (~1,5 MB a 48 k); `process()` não aloca.

namespace rasgo::modular {

class Sampler final : public Signal {
public:
    Sampler()
        : Signal(
              {{"trig", PortKind::Control, "trig"},
               {"in", PortKind::Audio, ""},
               {"rec", PortKind::Control, "gate"},
               {"pos", PortKind::Control, ""},
               {"pitch", PortKind::Control, "v/oct"}},
              {{"out", PortKind::Audio, ""}},
              {{"start", 0.0f, 1.0f, 0.0f, ""},
               {"speed", -1.0f, 1.0f, 0.5f, ""},
               {"slices", 1.0f, 16.0f, 1.0f, ""},
               {"repitch", 0.0f, 1.0f, 0.0f, ""},
               {"wear", 0.0f, 1.0f, 0.0f, ""},
               {"loop", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "SAMPLER"; }

    // chamado do painel (thread de UI), NUNCA de process(): substitui o
    // buffer por um arquivo carregado. `srcRate` = taxa do arquivo.
    void setBuffer(std::vector<float> mono, const float srcRate) {
        loaded_ = std::move(mono);
        loadedRate_ = srcRate > 0.0f ? srcRate : 48000.0f;
        recLen_ = 0;   // o gravado dá lugar ao arquivo
    }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "SAMPLER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "wave", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "START", "start", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SLICE", "slices", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "REPIT", "repitch", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "WEAR", "wear", 7.0f, 50.0f);
        p.add(Widget::Kind::Toggle, "LOOP", "loop", 26.0f, 50.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trig", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 22.0f, 92.0f);
        p.add(Widget::Kind::Jack, "REC", "in:rec", 34.0f, 92.0f);
        p.add(Widget::Kind::Jack, "POS", "in:pos", 46.0f, 92.0f);
        p.add(Widget::Kind::Jack, "PIT", "in:pitch", 58.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        bufLen_ = static_cast<std::size_t>(8.0f * sr_) + 4;
        rec_.assign(bufLen_, 0.0f);
        recWrite_ = 0;
        recLen_ = 0;
        recording_ = false;
        prevRec_ = 0.0f;
        prevTrig_ = 0.0f;
        playing_ = false;
        playPos_ = 0.0;
        rendered_ = 0;
        total_ = 1;
        rng_ = 0xA5A5A5A5DEADBEEFULL;
        shifter_.prepare(sr_);
        holdCount_ = 0;
        held_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* trigIn = inputs[0];
        const AudioBlock* audIn = inputs[1];
        const AudioBlock* recIn = inputs[2];
        const AudioBlock* posIn = inputs[3];
        const AudioBlock* pitchIn = inputs[4];

        const float startP = clamp01(parameterValue("start"));
        const float speedP = clampf(parameterValue("speed"), -1.0f, 1.0f);
        const int nSlices = static_cast<int>(std::lround(
            clampf(parameterValue("slices"), 1.0f, 16.0f)));
        const float repitch = clamp01(parameterValue("repitch"));
        const float wear = clamp01(parameterValue("wear"));
        const bool loop = parameterValue("loop") >= 0.5f;

        // buffer ativo: arquivo carregado tem prioridade sobre o gravado
        const bool useLoaded = !loaded_.empty();
        const std::vector<float>& buf = useLoaded ? loaded_ : rec_;
        const std::size_t effLen = useLoaded ? loaded_.size() : recLen_;
        const float rateRatio = useLoaded ? (loadedRate_ / sr_) : 1.0f;

        for (std::size_t f = 0; f < frames; ++f) {
            // ---- gravação ----
            const float recG = recIn ? recIn->at(0, f) : 0.0f;
            if (!useLoaded) {
                if (recG >= 0.5f && prevRec_ < 0.5f) {
                    recording_ = true;
                    recWrite_ = 0;
                }
                if (recording_) {
                    rec_[recWrite_] = audIn ? audIn->at(0, f) : 0.0f;
                    if (++recWrite_ >= bufLen_) {
                        recording_ = false;
                        recLen_ = bufLen_;
                    }
                }
                if (recG < 0.5f && prevRec_ >= 0.5f && recording_) {
                    recording_ = false;
                    recLen_ = recWrite_;
                }
            }
            prevRec_ = recG;

            // ---- disparo ----
            const float tg = trigIn ? trigIn->at(0, f) : 0.0f;
            if (tg >= 0.5f && prevTrig_ < 0.5f && effLen >= 8) {
                startVoice(startP, speedP, nSlices, repitch, wear,
                           posIn ? posIn->at(0, f) : 0.0f,
                           pitchIn ? pitchIn->at(0, f) : 0.0f,
                           effLen, rateRatio);
            }
            prevTrig_ = tg;

            // ---- síntese da voz ----
            float y = 0.0f;
            if (playing_ && effLen >= 8) {
                float raw = readBuf(buf, playPos_, sliceLo_, sliceHi_);

                // wear: redução de taxa (hold) + bit-crush
                if (crushHold_ > 1) {
                    if (holdCount_ == 0) held_ = raw;
                    raw = held_;
                    if (++holdCount_ >= crushHold_) holdCount_ = 0;
                } else {
                    holdCount_ = 0;
                }
                if (crushStep_ > 0.0f)
                    raw = std::round(raw / crushStep_) * crushStep_;

                float voiced = raw;
                if (repitch > 0.0f) {
                    const float sh = shifter_.process(raw);
                    voiced = raw * (1.0f - repitch) + sh * repitch;
                }

                if (envUp_ < attackS_) ++envUp_;
                const float env = declick(loop);
                y = voiced * env;

                playPos_ += reverse_ ? -inc_ : inc_;
                ++rendered_;

                const bool past = reverse_ ? (playPos_ <= sliceLo_)
                                           : (playPos_ >= sliceHi_ - 1.0);
                if (past) {
                    if (loop)
                        playPos_ = reverse_ ? (sliceHi_ - 1.0) : sliceLo_;
                    else
                        playing_ = false;
                } else if (!loop && rendered_ >= total_) {
                    playing_ = false;
                }
            }

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    std::uint64_t xn() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return rng_;
    }
    float rnd01() noexcept {
        return static_cast<float>(xn() >> 40) / 16777216.0f;
    }

    void startVoice(const float startP, const float speedP, const int nSlices,
                    const float repitch, const float wear, const float posCv,
                    const float pitchCv, const std::size_t effLen,
                    const float rateRatio) noexcept {
        const double len = static_cast<double>(effLen);
        int idx = static_cast<int>(clamp01(posCv) * static_cast<float>(nSlices));
        if (idx >= nSlices) idx = nSlices - 1;
        const double sliceLen = len / static_cast<double>(nSlices);
        sliceLo_ = static_cast<double>(idx) * sliceLen;
        sliceHi_ = sliceLo_ + sliceLen;

        // wear (semeado neste disparo)
        double startJit = 0.0;
        crushHold_ = 1;
        crushStep_ = 0.0f;
        if (wear > 0.0f) {
            startJit = static_cast<double>((rnd01() * 2.0f - 1.0f)
                                           * wear * 0.02f) * sliceLen;
            crushHold_ = 1 + static_cast<int>(wear * wear * 12.0f * rnd01());
            const float bits = 16.0f - wear * wear * 12.0f;
            crushStep_ = std::pow(2.0f, -(bits - 1.0f));
        }

        reverse_ = speedP < 0.0f;
        const float mag = std::pow(2.0f, std::fabs(speedP) * 2.0f);   // 0,25..4
        float pitchFactor = 1.0f;
        if (repitch < 1.0f) pitchFactor = std::exp2(pitchCv * (1.0f - repitch));
        inc_ = static_cast<double>(mag * pitchFactor * rateRatio);
        if (inc_ < 1.0e-4) inc_ = 1.0e-4;

        if (repitch > 0.0f) {
            shifter_.reset();
            shifter_.setRatio(std::exp2(static_cast<double>(pitchCv * repitch)));
        }

        playPos_ = reverse_ ? (sliceHi_ - 1.0 - startJit)
                            : (sliceLo_ + startP * sliceLen + startJit);
        playPos_ = std::min(std::max(playPos_, sliceLo_), sliceHi_ - 1.0);

        total_ = static_cast<std::size_t>((sliceHi_ - sliceLo_) / inc_) + 1;
        const double durS = static_cast<double>(total_) / sr_;
        const double fade = std::min(std::max(durS * 0.24, 0.0005), 0.005);
        attackS_ = std::max<std::size_t>(1,
            static_cast<std::size_t>(fade * sr_));
        releaseS_ = attackS_;
        rendered_ = 0;
        envUp_ = 0;
        holdCount_ = 0;
        playing_ = true;
    }

    float declick(const bool loop) const noexcept {
        const float atk = std::min(1.0f,
            static_cast<float>(envUp_) / static_cast<float>(attackS_));
        if (loop) return atk;   // no loop, sem release (o ponto de loop é
                                // uma limitação anotada — pendência)
        const std::size_t after = (total_ > rendered_) ? total_ - rendered_ : 0;
        const float rel = std::min(1.0f,
            static_cast<float>(after) / static_cast<float>(releaseS_));
        return std::min(atk, rel);
    }

    static float readBuf(const std::vector<float>& b, const double pos,
                         const double lo, const double hi) noexcept {
        double p = pos;
        if (p < lo) p = lo;
        if (p > hi - 1.0) p = hi - 1.0;
        const std::size_t i0 = static_cast<std::size_t>(p);
        std::size_t i1 = i0 + 1;
        if (i1 >= b.size()) i1 = b.size() - 1;
        const float fr = static_cast<float>(p - static_cast<double>(i0));
        return b[i0] * (1.0f - fr) + b[i1] * fr;
    }

    std::vector<float> rec_;
    std::vector<float> loaded_;
    float loadedRate_ = 48000.0f;
    std::size_t bufLen_ = 1;
    std::size_t recWrite_ = 0;
    std::size_t recLen_ = 0;
    bool recording_ = false;
    float prevRec_ = 0.0f, prevTrig_ = 0.0f;

    bool playing_ = false;
    bool reverse_ = false;
    double playPos_ = 0.0, sliceLo_ = 0.0, sliceHi_ = 1.0, inc_ = 1.0;
    std::size_t rendered_ = 0, total_ = 1, attackS_ = 1, releaseS_ = 1, envUp_ = 0;
    int crushHold_ = 1, holdCount_ = 0;
    float crushStep_ = 0.0f, held_ = 0.0f;

    DelayPitchShifter shifter_;
    std::uint64_t rng_ = 0xA5A5A5A5DEADBEEFULL;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
