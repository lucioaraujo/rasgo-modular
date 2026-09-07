#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// STAGES — gerador de segmentos configuráveis (Módulo 54)
// ============================================================================
//
// O `FUNCTION` é UMA rampa deformável (Maths). O `STAGES` é N segmentos
// reconfiguráveis cuja FUNÇÃO EMERGE de como se encadeiam: rampas →
// envelope/LFO, degraus → sequência de CV, conforme `hold`/`loop` e o que
// está cabeado. Mutable Stages / Rossum Control Forge / Blukač Fractalist.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/54_stages.md`.
//
// - `segments` (2–8)          quantos degraus/rampas na volta
// - `rate` (0,02–20 Hz, +CV)  velocidade da volta (modo loop)
// - `contour` (0–1)           forma dos níveis: 0 sobe · 0,5 arco · 1 desce
// - `curve` (−1..1)           curva de transição (exp/lin/log)
// - `hold` (0–1)              0 desliza (rampa) · 1 salta e segura (degrau)
// - `tilt` (−1..1)            distorção das durações (começo ↔ fim mais longos)
// - `jitter` (0–1, desvio Rasgo) passeio lento semeado nos níveis/durações
// - `loop` (0/1)              corre livre (LFO) ↔ um disparo (envelope — gate)
//
// Desenhado por macros (gerador, não editor de breakpoints — identidade
// RASGO). `jitter=0` → determinístico puro. `process()` não aloca.

namespace rasgo::modular {

class Stages final : public Signal {
public:
    Stages()
        : Signal(
              {{"gate", PortKind::Control, "gate"},
               {"reset", PortKind::Control, "trig"},
               {"rate_mod", PortKind::Control, ""}},
              {{"out", PortKind::Control, ""},
               {"eoc", PortKind::Control, "gate"},
               {"step", PortKind::Control, "gate"}},
              {{"segments", 2.0f, 8.0f, 4.0f, ""},
               {"rate", 0.02f, 20.0f, 0.5f, "Hz"},
               {"contour", 0.0f, 1.0f, 0.5f, ""},
               {"curve", -1.0f, 1.0f, 0.0f, ""},
               {"hold", 0.0f, 1.0f, 0.0f, ""},
               {"tilt", -1.0f, 1.0f, 0.0f, ""},
               {"jitter", 0.0f, 1.0f, 0.0f, ""},
               {"loop", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "STAGES"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "STAGES", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "stages", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "SEGS", "segments", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CNTR", "contour", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CURVE", "curve", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "HOLD", "hold", 24.0f, 54.0f);
        p.add(Widget::Kind::Knob, "TILT", "tilt", 40.0f, 54.0f);
        p.add(Widget::Kind::Knob, "JITR", "jitter", 8.0f, 78.0f);
        p.add(Widget::Kind::Toggle, "LOOP", "loop", 26.0f, 80.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 20.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RTM", "in:rate_mod", 32.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "EOC", "out:eoc", 20.0f, 120.0f);
        p.add(Widget::Kind::Jack, "STEP", "out:step", 32.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        phase_ = 0.0;
        prevK_ = 0;
        prevGate_ = 0.0f;
        prevReset_ = 0.0f;
        gateRunning_ = 0.0f;
        eocCd_ = 0;
        stepCd_ = 0;
        rng_ = 0x57A6E5C0DE1A1C0FULL;
        for (int k = 0; k < 8; ++k) { jn_[k] = 0.0f; jn2_[k] = 0.0f; }
        segN_ = 4;
        rebuild(4, 0.5f, 0.0f, 0.0f);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& eoc = outputs[1];
        AudioBlock& stepO = outputs[2];
        const std::size_t frames = out.frames();

        const AudioBlock* gateB = inputs[0];
        const AudioBlock* resetB = inputs[1];
        const AudioBlock* rmB = inputs[2];

        const int N = static_cast<int>(std::lround(
            clampf(parameterValue("segments"), 2.0f, 8.0f)));
        const float contour = clamp01(parameterValue("contour"));
        const float curve = clampf(parameterValue("curve"), -1.0f, 1.0f);
        const float hold = clamp01(parameterValue("hold"));
        const float tilt = clampf(parameterValue("tilt"), -1.0f, 1.0f);
        const float jitter = clamp01(parameterValue("jitter"));
        const bool loop = parameterValue("loop") >= 0.5f;

        if (N != segN_ || contour != cContour_ || tilt != cTilt_
            || jitter != cJitter_)
            rebuild(N, contour, tilt, jitter);

        const float pulseLen = 0.004f * sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            const float rate = clampf(
                parameterValue("rate") + (rmB ? rmB->at(0, f) : 0.0f),
                0.001f, 20.0f);

            // ---- reset ----
            const float rs = resetB ? resetB->at(0, f) : 0.0f;
            if (rs >= 0.5f && prevReset_ < 0.5f) {
                phase_ = 0.0; prevK_ = 0;
            }
            prevReset_ = rs;

            // ---- gate (one-shot) ----
            const float gt = gateB ? gateB->at(0, f) : 0.0f;
            if (!loop) {
                if (gt >= 0.5f && prevGate_ < 0.5f) {
                    gateRunning_ = 1.0f; phase_ = 0.0; prevK_ = 0;
                }
            }
            prevGate_ = gt;

            // ---- avança a fase ----
            const double adv = static_cast<double>(rate) / sr_
                * (loop ? 1.0 : static_cast<double>(gateRunning_));
            phase_ += adv;
            bool eocPulse = false;
            if (phase_ >= 1.0) {
                phase_ -= std::floor(phase_);
                eocPulse = true;
                if (!loop) { gateRunning_ = 0.0f; phase_ = 0.9999999; }
                if (jitter > 0.0f) {
                    advanceJitter();
                    rebuild(segN_, cContour_, cTilt_, cJitter_);
                }
            }
            if (eocPulse) eocCd_ = static_cast<int>(pulseLen);

            // ---- acha o segmento ----
            double acc = 0.0;
            int k = 0;
            while (k < segN_ - 1 && acc + dur_[k] <= phase_) {
                acc += dur_[k]; ++k;
            }
            if (k != prevK_) { stepCd_ = static_cast<int>(pulseLen); }
            prevK_ = k;

            const float segFrac = dur_[k] > 1e-9
                ? static_cast<float>((phase_ - acc) / dur_[k]) : 0.0f;
            const float from = lvl_[k];
            const float to = loop
                ? lvl_[(k + 1) % segN_]
                : (k < segN_ - 1 ? lvl_[k + 1] : lvl_[k]);

            // `hold` = fração do segmento SEGURANDO `from` no fim; a rampa
            // acontece nos primeiros (1−hold). hold≥1 → segura `from` a
            // volta toda e SALTA na fronteira (degrau puro).
            const float t = hold >= 0.999f
                ? 0.0f
                : clampf(segFrac / (1.0f - hold), 0.0f, 1.0f);
            const float tc = curveShape(t, curve);
            const float o = clampf(from + (to - from) * tc, -1.0f, 1.0f);

            const float e = eocCd_ > 0 ? 1.0f : 0.0f;
            const float s = stepCd_ > 0 ? 1.0f : 0.0f;
            if (eocCd_ > 0) --eocCd_;
            if (stepCd_ > 0) --stepCd_;

            for (std::size_t c = 0; c < out.channels(); ++c) out.at(c, f) = o;
            for (std::size_t c = 0; c < eoc.channels(); ++c) eoc.at(c, f) = e;
            for (std::size_t c = 0; c < stepO.channels(); ++c)
                stepO.at(c, f) = s;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rnd11() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(static_cast<std::int32_t>(rng_ >> 32))
            / 2147483648.0f;
    }

    static float curveShape(const float t, const float c) noexcept {
        // c<0 exponencial (rápido→lento, côncava p/ baixo);
        // c>0 logarítmica (devagar→rápido, côncava p/ cima)
        if (c < -1.0e-3f)
            return 1.0f - std::pow(1.0f - t, 1.0f + (-c) * 4.0f);
        if (c > 1.0e-3f)
            return std::pow(t, 1.0f + c * 4.0f);
        return t;
    }

    void advanceJitter() noexcept {
        for (int k = 0; k < 8; ++k) {
            jn_[k] = clampf(jn_[k] + rnd11() * 0.12f, -1.0f, 1.0f);
            jn2_[k] = clampf(jn2_[k] + rnd11() * 0.12f, -1.0f, 1.0f);
        }
    }

    void rebuild(const int N, const float contour, const float tilt,
                 const float jitter) noexcept {
        segN_ = N;
        cContour_ = contour;
        cTilt_ = tilt;
        cJitter_ = jitter;
        float wsum = 0.0f;
        for (int k = 0; k < N; ++k) {
            const float u = N > 1 ? static_cast<float>(k)
                / static_cast<float>(N - 1) : 0.0f;
            const float asc = u;
            const float arch = 1.0f - std::fabs(2.0f * u - 1.0f);
            const float desc = 1.0f - u;
            float lv = contour < 0.5f
                ? asc + (arch - asc) * (contour * 2.0f)
                : arch + (desc - arch) * ((contour - 0.5f) * 2.0f);
            lv += jitter * jn_[k] * 0.5f;
            lvl_[k] = clampf(lv, -1.0f, 1.0f);

            float w = 1.0f + tilt * (2.0f * u - 1.0f)
                    + jitter * jn2_[k] * 0.5f;
            w = std::max(0.05f, w);
            dur_[k] = w;
            wsum += w;
        }
        for (int k = 0; k < N; ++k) dur_[k] /= wsum;
    }

    double phase_ = 0.0;
    int prevK_ = 0;
    float prevGate_ = 0.0f, prevReset_ = 0.0f;
    float gateRunning_ = 0.0f;
    int eocCd_ = 0, stepCd_ = 0;
    std::uint64_t rng_ = 0x57A6E5C0DE1A1C0FULL;
    float jn_[8] = {}, jn2_[8] = {};
    float lvl_[8] = {}, dur_[8] = {};
    int segN_ = 4;
    float cContour_ = 0.5f, cTilt_ = 0.0f, cJitter_ = 0.0f;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
