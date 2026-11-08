#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// DYNAMICS (#63) — compressor e gate com sidechain (2026-11-08, aprovado pelo
// autor a partir do estudo do Cardinal, dossies/ESTUDO_cardinal_catalogo_e_licencas.md §5.1).
//
// Formulação pública de manual — D. Giannoulis, M. Massberg, J. D. Reiss,
// "Digital Dynamic Range Compressor Design — A Tutorial and Analysis",
// JAES 60(6), 2012: detector (pico ou RMS) → curva estática em dB com joelho
// → balística no domínio do ganho (ataque quando a redução aumenta, release
// quando diminui) → ganho de compensação. Conceito lido no Bogaudio PRESSOR,
// no ZamComp e no CVfunk Pressed Duck; nenhum código copiado.
//
// Desvio Rasgo:
//  (a) `env` (o que o detector ouve) e `gr` (quanto ele abaixa) saem como
//      sinais de controle — o compressor vira um ouvido que move o patch;
//  (b) `breath` — o release segue o intervalo entre os golpes que ele ouve:
//      a compressão respira no andamento da música (0 = release fixo);
//  (c) `drift` — a cada golpe, ataque e release variam por um acaso SEMEADO
//      (o "bombeamento" nunca é mecânico; drift=0 → determinístico e fixo).
//
// Entradas: `in` (áudio, mono ou estéreo — detecção ligada entre canais),
// `side` (qualquer sinal; `side_mix` decide quanto o detector ouve dele) e
// `thr` (CV somada ao limiar, ±24 dB). Sem `in` o módulo fica calado (como o
// VCA), mas com só `side` ligado `env` e `gr` seguem se movendo: gerador de
// modulação a partir de qualquer sinal. `process()` não aloca.
namespace rasgo::modular {

class Dynamics final : public Signal {
public:
    Dynamics() : Signal(
        {{"in", PortKind::Audio, ""},
         {"side", PortKind::Audio, ""},
         {"thr", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""},
         {"env", PortKind::Control, ""},
         {"gr", PortKind::Control, ""}},
        {{"thresh", -48.0f, 0.0f, -18.0f, "dB"},
         {"ratio", 1.0f, 20.0f, 4.0f, ""},
         {"attack", 0.1f, 200.0f, 10.0f, "ms"},
         {"release", 5.0f, 2000.0f, 150.0f, "ms"},
         {"knee", 0.0f, 12.0f, 6.0f, "dB"},
         {"makeup", 0.0f, 24.0f, 0.0f, "dB"},
         {"side_mix", 0.0f, 1.0f, 1.0f, ""},
         {"detect", 0.0f, 1.0f, 0.0f, ""},
         {"mode", 0.0f, 1.0f, 0.0f, ""},
         {"breath", 0.0f, 1.0f, 0.0f, ""},
         {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "DYNAMICS"; }

    Panel panel() const override {
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "DYNAMICS", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Knob, "THRES", "thresh", 9.0f, 14.0f);
        p.add(Widget::Kind::Knob, "RATIO", "ratio", 33.0f, 14.0f);
        p.add(Widget::Kind::Knob, "KNEE", "knee", 57.0f, 14.0f);
        p.add(Widget::Kind::Knob, "ATK", "attack", 9.0f, 34.0f);
        p.add(Widget::Kind::Knob, "REL", "release", 33.0f, 34.0f);
        p.add(Widget::Kind::Knob, "GAIN", "makeup", 57.0f, 34.0f);
        p.add(Widget::Kind::Knob, "SIDE", "side_mix", 9.0f, 54.0f);
        p.add(Widget::Kind::Knob, "BRTH", "breath", 33.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 57.0f, 54.0f);
        p.add(Widget::Kind::Toggle, "RMS", "detect", 9.0f, 70.0f);
        p.add(Widget::Kind::Toggle, "GATE", "mode", 33.0f, 70.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 94.0f);
        p.add(Widget::Kind::Jack, "SIDE", "in:side", 30.0f, 94.0f);
        p.add(Widget::Kind::Jack, "THR", "in:thr", 52.0f, 94.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ENV", "out:env", 30.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GR", "out:gr", 52.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rmsCoef_ = coefficient(0.010f);
        meanSquare_ = 0.0;
        grDb_ = 0.0f;
        envDb_ = -120.0f;
        above_ = false;
        sinceOnset_ = 0;
        interval_ = 0.0f;
        attackMul_ = releaseMul_ = 1.0f;
        rng_ = 0xD1A4A51C5EED0063ULL;
    }

    // A curva estática (exposta para os testes): a redução de ganho em dB
    // (≤ 0) para um nível de detector em dB.
    static float staticGainDb(float xDb, float thresholdDb, float ratio, float kneeDb,
                              bool gate) noexcept {
        const float w = std::max(0.0f, kneeDb);
        const float d = xDb - thresholdDb;
        if (!gate) {
            const float slope = ratio >= 19.5f ? 1.0f : 1.0f - 1.0f / std::max(1.0f, ratio);
            if (2.0f * d < -w) return 0.0f;
            if (w > 0.0f && 2.0f * std::fabs(d) <= w) {
                const float k = d + w * 0.5f;
                return -slope * k * k / (2.0f * w);
            }
            return -slope * d;
        }
        // Gate = expansor para baixo: abaixo do limiar, cada dB a menos vira
        // (ratio) dB a menos na saída; até −80 dB.
        const float expand = std::max(1.0f, ratio) - 1.0f;
        float g;
        if (2.0f * d > w) g = 0.0f;
        else if (w > 0.0f && 2.0f * std::fabs(d) <= w) {
            const float k = w * 0.5f - d;
            g = -expand * k * k / (2.0f * w);
        } else g = expand * d;
        return std::max(-80.0f, g);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 3) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* side = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* thr = inputs.size() > 2 ? inputs[2] : nullptr;

        const float thresholdKnob = safe(parameterValue("thresh"), -18.0f);
        const float ratio = std::clamp(safe(parameterValue("ratio"), 4.0f), 1.0f, 20.0f);
        const float attackMs = std::clamp(safe(parameterValue("attack"), 10.0f), 0.1f, 200.0f);
        const float releaseMs = std::clamp(safe(parameterValue("release"), 150.0f), 5.0f, 2000.0f);
        const float knee = std::clamp(safe(parameterValue("knee"), 6.0f), 0.0f, 12.0f);
        const float makeup = std::pow(10.0f, std::clamp(safe(parameterValue("makeup"), 0.0f), 0.0f, 24.0f) / 20.0f);
        const float sideMix = side ? std::clamp(safe(parameterValue("side_mix"), 1.0f), 0.0f, 1.0f) : 0.0f;
        const bool rms = safe(parameterValue("detect"), 0.0f) >= 0.5f;
        const bool gate = safe(parameterValue("mode"), 0.0f) >= 0.5f;
        const float breath = std::clamp(safe(parameterValue("breath"), 0.0f), 0.0f, 1.0f);
        const float drift = std::clamp(safe(parameterValue("drift"), 0.0f), 0.0f, 1.0f);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const int minGap = static_cast<int>(0.030f * sr_);
        for (std::size_t f = 0; f < frames; ++f) {
            // Detector: o máximo entre canais (detecção ligada), da mistura
            // entrada ↔ sidechain.
            float peak = 0.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                const float x = read(in, c, f);
                const float s = read(side, c, f);
                const float det = (1.0f - sideMix) * x + sideMix * s;
                peak = std::max(peak, std::fabs(det));
            }
            float level = peak;
            if (rms) {
                meanSquare_ += (static_cast<double>(peak) * peak - meanSquare_) * rmsCoef_;
                level = static_cast<float>(std::sqrt(std::max(0.0, meanSquare_)) * 1.41421356);
            }
            const float levelDb = 20.0f * std::log10(std::max(level, 1.0e-6f));
            float thresholdDb = thresholdKnob;
            if (thr) thresholdDb += 24.0f * std::clamp(safe(thr->at(0, f), 0.0f), -1.0f, 1.0f);
            thresholdDb = std::clamp(thresholdDb, -72.0f, 6.0f);

            // Golpes: o detector cruza o limiar para cima depois de ≥ 30 ms
            // abaixo. Medem o intervalo (breath) e sorteiam o caráter (drift).
            if (sinceOnset_ < (1 << 30)) ++sinceOnset_;
            const bool nowAbove = levelDb > thresholdDb;
            if (nowAbove && !above_ && sinceOnset_ > minGap) {
                const float seconds = static_cast<float>(sinceOnset_) / sr_;
                interval_ = interval_ <= 0.0f ? seconds : interval_ + (seconds - interval_) * 0.35f;
                sinceOnset_ = 0;
                if (drift > 0.0f) {
                    attackMul_ = std::pow(2.0f, noise() * drift);
                    releaseMul_ = std::pow(2.0f, noise() * drift);
                } else {
                    attackMul_ = releaseMul_ = 1.0f;
                }
            }
            above_ = nowAbove;

            float releaseSeconds = releaseMs * 0.001f;
            if (breath > 0.0f && interval_ > 0.0f) {
                const float follow = std::clamp(interval_ * 0.6f, 0.02f, 2.0f);
                releaseSeconds += (follow - releaseSeconds) * breath;
            }
            const float aCoef = coefficient(attackMs * 0.001f * attackMul_);
            const float rCoef = coefficient(releaseSeconds * releaseMul_);

            const float target = staticGainDb(levelDb, thresholdDb, ratio, knee, gate);
            grDb_ += (target - grDb_) * (target < grDb_ ? aCoef : rCoef);
            if (!std::isfinite(grDb_)) grDb_ = 0.0f;
            const float gain = std::pow(10.0f, grDb_ / 20.0f) * makeup;

            // ENV: o nível que o detector ouve, com a mesma balística (no modo
            // pico o nível instantâneo segue cada meia-onda — não serve de
            // modulação).
            envDb_ += (levelDb - envDb_) * (levelDb > envDb_ ? aCoef : rCoef);
            if (!std::isfinite(envDb_)) envDb_ = -120.0f;
            const float envOut = std::clamp((envDb_ + 60.0f) / 60.0f, 0.0f, 1.0f);
            const float grOut = std::clamp(-grDb_ / 24.0f, 0.0f, 1.0f);
            for (std::size_t c = 0; c < channels; ++c) {
                const float y = read(in, c, f) * gain;
                outputs[0].at(c, f) = std::isfinite(y) ? std::clamp(y, -4.0f, 4.0f) : 0.0f;
                outputs[1].at(c, f) = envOut;
                outputs[2].at(c, f) = grOut;
            }
        }
    }

    float gainReductionDb() const noexcept { return grDb_; }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    static float read(const AudioBlock* block, std::size_t c, std::size_t f) noexcept {
        if (block == nullptr || block->channels() == 0) return 0.0f;
        const float v = block->at(std::min(c, block->channels() - 1), f);
        return std::isfinite(v) ? v : 0.0f;
    }
    float coefficient(float seconds) const noexcept {
        return 1.0f - std::exp(-1.0f / (sr_ * std::max(seconds, 1.0e-5f)));
    }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32)) / 2147483648.0f;
    }

    float sr_ = 48000.0f, rmsCoef_ = 0.002f;
    double meanSquare_ = 0.0;
    float grDb_ = 0.0f, envDb_ = -120.0f;
    bool above_ = false;
    int sinceOnset_ = 0;
    float interval_ = 0.0f, attackMul_ = 1.0f, releaseMul_ = 1.0f;
    std::uint64_t rng_ = 0xD1A4A51C5EED0063ULL;
};

} // namespace rasgo::modular
