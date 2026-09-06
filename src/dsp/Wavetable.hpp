#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// WAVETABLE — oscilador de tabela procedural (Módulo 40)
// ============================================================================
//
// O `OSC` é subtrativo (5 formas fixas). `WAVETABLE` é o eixo de FORMA
// varrido por CV: 16 quadros gerados no `prepare()` por uma receita
// espectral fixa (serra → quadrada → formante → seno — SEM arquivo de
// dados, decisão do autor 2026-09-06), band-limited em 10 mip-maps pra o
// antialiasing seguir a afinação.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/40_wavetable.md`.
//
// - `pos` (0–1, + CV) varre o eixo de forma;
// - `warp` (0–1) = distorção de fase Casio CZ / "WAVE CUT" do EMW WAVE-6;
// - `freq`/`fine`/`pitch` (1 V/oct)/`fm` (linear) = afinação, padrão OSC;
// - `drift` (0–1) = varredura lenta autônoma de `pos`. drift=0 + sem
//   captura → determinístico.
//
// CAPTURA AO VIVO (desvio Rasgo): `capture` (áudio) + `grab` (trigger).
// Na borda de `grab`, 1024 amostras de `capture` viram o quadro no topo
// de `pos` (crossfade de ~0,9 a 1,0). Standalone = procedural; com
// `AUDIO-IN`/`OSC`/… no `capture` = tabela viva. O quadro capturado não é
// band-limited (aliasing em afinação alta = caráter aceito).
//
// `prepare()` aloca ~1 MB e gera as tabelas; `process()` não aloca.

namespace rasgo::modular {

class Wavetable final : public Signal {
public:
    static constexpr int kFrames = 16;
    static constexpr int kLen = 1024;       // amostras por ciclo (potência de 2)
    static constexpr int kMips = 10;        // maxH = 512, 256, ... 1
    static constexpr int kMaxH = kLen / 2;  // 512

    Wavetable()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"pos", PortKind::Control, ""},
               {"fm", PortKind::Audio, ""},
               {"capture", PortKind::Audio, ""},
               {"grab", PortKind::Control, "trig"}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 8.0f, 8000.0f, 110.0f, "Hz"},
               {"fine", -100.0f, 100.0f, 0.0f, "cent"},
               {"pos", 0.0f, 1.0f, 0.0f, ""},
               {"warp", 0.0f, 1.0f, 0.0f, ""},
               {"fm_amount", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "WAVETABLE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "WAVETABLE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "wave", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FINE", "fine", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "POS", "pos", 37.0f, 30.0f);
        p.add(Widget::Kind::Knob, "WARP", "warp", 52.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FM", "fm_amount", 7.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 22.0f, 54.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:pitch", 8.0f, 94.0f);
        p.add(Widget::Kind::Jack, "POS", "in:pos", 22.0f, 94.0f);
        p.add(Widget::Kind::Jack, "FM", "in:fm", 36.0f, 94.0f);
        p.add(Widget::Kind::Jack, "CAP", "in:capture", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GRAB", "in:grab", 22.0f, 116.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 40.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        dt_ = 1.0f / std::max(1.0f, sampleRate);
        phase_ = 0.0f;
        prevGrab_ = 0.0f;
        capArmed_ = false;
        capFill_ = 0;
        capValid_ = false;
        driftPos_ = 0.0f;
        driftTgt_ = 0.0f;
        driftCount_ = 0;
        driftInterval_ =
            static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 30.0f));
        rng_ = 0x123456789ABCDEFULL;

        capBuf_.assign(kLen, 0.0f);
        buildTables();
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float freq = parameterValue("freq");
        const float fineOct = parameterValue("fine") / 1200.0f;
        const float posKnob = clamp01(parameterValue("pos"));
        const float warp = clamp01(parameterValue("warp"));
        const float fmAmt = clamp01(parameterValue("fm_amount"));
        const float drift = clamp01(parameterValue("drift"));
        const float nyq = 0.5f / dt_;

        const AudioBlock* pitchIn = inputs[0];
        const AudioBlock* posIn = inputs[1];
        const AudioBlock* fmIn = inputs[2];
        const AudioBlock* capIn = inputs[3];
        const AudioBlock* grabIn = inputs[4];
        const float driftAmp = 0.12f * drift;

        for (std::size_t f = 0; f < frames; ++f) {
            // ---- captura ----
            const float grabV = grabIn ? grabIn->at(0, f) : 0.0f;
            if (capIn && prevGrab_ < 0.5f && grabV >= 0.5f) {
                capArmed_ = true;
                capFill_ = 0;
            }
            prevGrab_ = grabV;
            if (capArmed_) {
                capBuf_[static_cast<std::size_t>(capFill_)] =
                    capIn ? capIn->at(0, f) : 0.0f;
                if (++capFill_ >= kLen) {
                    finishCapture();
                    capArmed_ = false;
                    capValid_ = true;
                }
            }

            // ---- deriva de pos ----
            if (drift > 0.0f && ++driftCount_ >= driftInterval_) {
                driftCount_ = 0;
                driftTgt_ = noise() * driftAmp;
            }
            driftPos_ += (driftTgt_ - driftPos_) * 0.0015f;

            // ---- afinação ----
            const float pOct = pitchIn ? pitchIn->at(0, f) : 0.0f;
            float f0 = freq * std::exp2(fineOct + pOct);
            if (fmIn) f0 *= (1.0f + fmIn->at(0, f) * fmAmt * 4.0f);
            f0 = clampf(f0, 0.01f, nyq * 0.9f);

            phase_ += f0 * dt_;
            phase_ -= std::floor(phase_);
            const float wp = warpPhase(phase_, warp);

            // ---- mip: harmônica máxima do mip · f0 < Nyquist ----
            const float mipF = std::log2(std::max(1.0e-6f,
                static_cast<float>(kLen) * f0 * dt_));
            int mip = static_cast<int>(std::ceil(mipF));
            if (mip < 0) mip = 0;
            if (mip > kMips - 1) mip = kMips - 1;

            // ---- posição no eixo de forma ----
            const float posCv = posIn ? posIn->at(0, f) : 0.0f;
            const float posEff = clamp01(posKnob + posCv + driftPos_);
            const int maxSlot = capValid_ ? kFrames : kFrames - 1;
            float slot = posEff * static_cast<float>(maxSlot);
            int s0 = static_cast<int>(slot);
            if (s0 > maxSlot - 1) s0 = maxSlot - 1;
            const float sf = slot - static_cast<float>(s0);

            const float a = readFrame(s0, mip, wp);
            const float b = readFrame(s0 + 1, mip, wp);
            const float y = a + (b - a) * sf;

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // xorshift* → [−1,1)
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32)) / 2147483648.0f;
    }

    // distorção de fase Casio CZ / WAVE CUT
    static float warpPhase(const float ph, const float warp) noexcept {
        if (warp <= 0.0f) return ph;
        const float k = 0.5f - 0.45f * warp;
        if (ph < k) return ph * (0.5f / k);
        return 0.5f + (ph - k) * (0.5f / (1.0f - k));
    }

    float readTable(const std::vector<float>& t, const float ph) const noexcept {
        const float x = ph * static_cast<float>(kLen);
        int i = static_cast<int>(x);
        const float fr = x - static_cast<float>(i);
        i &= (kLen - 1);
        const int j = (i + 1) & (kLen - 1);
        return t[static_cast<std::size_t>(i)] * (1.0f - fr)
             + t[static_cast<std::size_t>(j)] * fr;
    }

    float readFrame(int idx, const int mip, const float ph) const noexcept {
        if (idx >= kFrames) return readTable(capBuf_, ph);
        if (idx < 0) idx = 0;
        return readTable(tbl_[static_cast<std::size_t>(idx * kMips + mip)], ph);
    }

    // amplitude da harmônica `h` (1..kMaxH) no quadro `pos01` (0..1)
    static float recipe(const int h, const float pos01) noexcept {
        const float seg = pos01 * 3.0f;
        const int a = static_cast<int>(seg);
        const float t = seg - static_cast<float>(a);
        auto arche = [&](const int which) -> float {
            switch (which) {
            case 0:  // serra
                return 1.0f / static_cast<float>(h);
            case 1:  // quadrada (ímpares)
                return (h & 1) ? 1.0f / static_cast<float>(h) : 0.0f;
            case 2: {  // formante — 3 sinos gaussianos
                const float hf = static_cast<float>(h);
                auto bell = [&](float c) {
                    const float d = (hf - c) / 1.6f;
                    return std::exp(-d * d);
                };
                return 0.9f * bell(3.0f) + 0.7f * bell(8.0f) + 0.5f * bell(14.0f);
            }
            default:  // seno
                return h == 1 ? 1.0f : 0.0f;
            }
        };
        const int lo = a < 3 ? a : 3;
        const int hi = lo + 1 < 3 ? lo + 1 : 3;
        return arche(lo) * (1.0f - t) + arche(hi) * t;
    }

    void buildTables() {
        // LUT de seno indexada por (h·s) mod kLen — geração exata e rápida
        std::vector<float> sinLUT(kLen);
        for (int i = 0; i < kLen; ++i)
            sinLUT[static_cast<std::size_t>(i)] =
                std::sin(6.28318530718f * static_cast<float>(i)
                         / static_cast<float>(kLen));

        tbl_.assign(static_cast<std::size_t>(kFrames * kMips),
                    std::vector<float>(kLen, 0.0f));

        std::vector<float> amp(static_cast<std::size_t>(kMaxH + 1), 0.0f);
        for (int fr = 0; fr < kFrames; ++fr) {
            const float pos01 =
                static_cast<float>(fr) / static_cast<float>(kFrames - 1);
            float norm = 0.0f;
            for (int h = 1; h <= kMaxH; ++h) {
                amp[static_cast<std::size_t>(h)] = recipe(h, pos01);
                norm += std::fabs(amp[static_cast<std::size_t>(h)]);
            }
            const float g = norm > 1.0e-6f ? 0.9f / norm : 0.0f;

            for (int s = 0; s < kLen; ++s) {
                float acc = 0.0f;
                int mip = kMips - 1;              // começa com 1 harmônica
                int thresh = 1;                   // maxH do mip corrente (1,2,4,…,512)
                for (int h = 1; h <= kMaxH; ++h) {
                    acc += amp[static_cast<std::size_t>(h)] * g
                         * sinLUT[static_cast<std::size_t>((h * s) & (kLen - 1))];
                    if (h == thresh) {
                        tbl_[static_cast<std::size_t>(fr * kMips + mip)]
                            [static_cast<std::size_t>(s)] = acc;
                        --mip;
                        thresh <<= 1;
                    }
                }
                // mips que sobraram (se kMaxH não é 2^(kMips-1)) recebem o total
                while (mip >= 0) {
                    tbl_[static_cast<std::size_t>(fr * kMips + mip)]
                        [static_cast<std::size_t>(s)] = acc;
                    --mip;
                }
            }
        }
    }

    void finishCapture() noexcept {
        // remove DC + normaliza o pico pra ~0,9
        float mean = 0.0f;
        for (float v : capBuf_) mean += v;
        mean /= static_cast<float>(kLen);
        float peak = 0.0f;
        for (float& v : capBuf_) {
            v -= mean;
            peak = std::max(peak, std::fabs(v));
        }
        if (peak < 1.0e-4f) {   // silêncio: não amplifica ruído de fundo
            for (float& v : capBuf_) v = 0.0f;
            return;
        }
        const float g = 0.9f / peak;
        for (float& v : capBuf_) v *= g;
    }

    // tbl_[fr*kMips + mip] = tabela de kLen amostras
    std::vector<std::vector<float>> tbl_;
    std::vector<float> capBuf_;

    float dt_ = 1.0f / 48000.0f;
    float phase_ = 0.0f;
    float prevGrab_ = 0.0f;
    bool capArmed_ = false;
    int capFill_ = 0;
    bool capValid_ = false;

    float driftPos_ = 0.0f, driftTgt_ = 0.0f;
    std::uint32_t driftCount_ = 0, driftInterval_ = 1600;
    std::uint64_t rng_ = 0x1234567889ABCDEFULL;
};

}  // namespace rasgo::modular
