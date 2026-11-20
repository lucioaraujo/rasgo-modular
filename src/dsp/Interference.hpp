#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// INTERFERENCE (#94) — franjas de interferência (ideia óptica, 9/11/2026,
// aprovada pelo autor: "aprovo todos"; dossiê `dossies/94_interference.md`).
//
// A experiência de Young (1801): ondas de fontes coerentes se somam onde
// chegam em fase (franja clara) e se cancelam onde chegam em oposição
// (franja escura). Um ponto de observação corre pela tela: entre as fontes 1
// e 2, a diferença de caminho cresce a SPEED franjas por segundo; a terceira
// fonte (amplitude THIRD) acrescenta outra diferença, RATIO vezes a
// primeira. A intensidade I = |1 + e^{iθ} + THIRD·e^{iθ(1+RATIO)}|²,
// normalizada a 0–1, sai em INT; cada franja clara acima de THR dá um pulso
// em FRNG, cada franja escura, em DARK. Só com duas fontes (THIRD 0), as
// franjas são regulares; com a terceira, elas se agrupam e se apagam em
// ciclos — o MOIRÉ, a envoltória lenta, que sai em MOIRE. Devagar, é ritmo;
// com SPEED na faixa de áudio, INT vira timbre.
//
// Território próprio do Rasgo: o Cardinal não tem módulos ópticos.
//
// Desvio Rasgo: COHER — a coerência parcial. Com COHER baixo, a fase das
// fontes passeia ao acaso (luz de lâmpada, não de laser) e as franjas se
// apagam aos poucos: o padrão nítido vira névoa e volta.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Interference final : public Signal {
public:
    Interference() : Signal(
        {{"speed_cv", PortKind::Control, "v/oct"},
         {"ratio_cv", PortKind::Control, ""},
         {"reset", PortKind::Control, "trig"}},
        {{"int", PortKind::Control, ""},
         {"fringe", PortKind::Control, "gate"},
         {"dark", PortKind::Control, "gate"},
         {"moire", PortKind::Control, ""}},
        {{"speed", 0.05f, 2000.0f, 2.0f, "Hz"},
         {"ratio", 1.0f, 2.0f, 1.08f, ""},
         {"third", 0.0f, 1.0f, 0.0f, ""},
         {"threshold", 0.3f, 0.98f, 0.8f, ""},
         {"coherence", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "INTERFERENCE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "INTERFERENCE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "interference", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATIO", "ratio", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "THIRD", "third", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "THR", "threshold", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "COHER", "coherence", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "SPD", "in:speed_cv", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RATIO", "in:ratio_cv", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "INT", "out:int", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "FRNG", "out:fringe", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "DARK", "out:dark", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "MOIRE", "out:moire", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearState();
    }

    // A intensidade normalizada (0–1) na posição x da tela (em franjas das
    // fontes 1–2), com os desvios de fase p2 e p3 da coerência parcial —
    // também para os testes e o mostrador.
    static float intensity(double x, float ratio, float third, double p2 = 0.0, double p3 = 0.0) noexcept {
        const double th = 6.283185307179586 * x;
        const double re = 1.0 + std::cos(th + p2) + third * std::cos(th * (1.0 + ratio) + p3);
        const double im = std::sin(th + p2) + third * std::sin(th * (1.0 + ratio) + p3);
        const double peak = (2.0 + third) * (2.0 + third);
        return static_cast<float>((re * re + im * im) / peak);
    }
    double position() const noexcept { return x_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* scv = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* rcv = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* rst = inputs.size() > 2 ? inputs[2] : nullptr;

        const float speedK = std::clamp(safe(parameterValue("speed"), 2.0f), 0.05f, 2000.0f);
        const float ratioK = std::clamp(safe(parameterValue("ratio"), 1.08f), 1.0f, 2.0f);
        const float third = std::clamp(safe(parameterValue("third"), 0.0f), 0.0f, 1.0f);
        const float thr = std::clamp(safe(parameterValue("threshold"), 0.8f), 0.3f, 0.98f);
        const float coher = std::clamp(safe(parameterValue("coherence"), 1.0f), 0.0f, 1.0f);
        // o passeio de fase da coerência parcial: em COHER 0, a fase se
        // descorrelaciona em ~1 s (variância ~3 rad² por segundo); em 1, nada
        const double walk = (1.0 - coher) * (1.0 - coher) * 0.015 * std::sqrt(48000.0 / sr_);
        // a envoltória: um passa-baixa de ~0,2 s
        const float moireK = 1.0f - std::exp(-1.0f / (0.2f * sr_));
        const int gateLen = std::max(1, static_cast<int>(0.004f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (rst) {
                const float rv = safe(rst->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && rv >= 0.5f) clearState();
                prevReset_ = rv;
            }
            float speed = speedK;
            if (scv) speed *= std::exp2(std::clamp(safe(scv->at(0, f), 0.0f), -6.0f, 6.0f));
            speed = std::clamp(speed, 0.01f, 0.4f * sr_);
            float ratio = ratioK;
            if (rcv) ratio = std::clamp(ratio + 0.5f * safe(rcv->at(0, f), 0.0f), 1.0f, 2.5f);
            x_ += static_cast<double>(speed) / sr_;
            if (x_ > 1.0e6) x_ -= 1.0e6;   // a tela é longa, mas não infinita
            if (walk > 0.0) {
                p2_ += walk * (2.0 * static_cast<double>(unit()) - 1.0);
                p3_ += walk * (2.0 * static_cast<double>(unit()) - 1.0);
            }
            const float in = intensity(x_, ratio, third, p2_, p3_);
            moire_ += (in - moire_) * moireK;
            // franjas: o pico claro (subida acima de THR, uma vez por franja) e o vale escuro
            if (!bright_ && in >= thr) { bright_ = true; fringeLeft_ = gateLen; }
            else if (bright_ && in < thr * 0.8f) bright_ = false;
            if (!darkNow_ && in <= 1.0f - thr) { darkNow_ = true; darkLeft_ = gateLen; }
            else if (darkNow_ && in > (1.0f - thr) + 0.1f) darkNow_ = false;
            const float fg = fringeLeft_ > 0 ? 1.0f : 0.0f;
            const float dg = darkLeft_ > 0 ? 1.0f : 0.0f;
            if (fringeLeft_ > 0) --fringeLeft_;
            if (darkLeft_ > 0) --darkLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = in;
                outputs[1].at(c, f) = fg;
                outputs[2].at(c, f) = dg;
                outputs[3].at(c, f) = moire_;
            }
        }
    }

private:
    void clearState() noexcept {
        x_ = 0.0; p2_ = p3_ = 0.0;
        rng_ = 0x1A7E5F00DULL;
        moire_ = 0.0f;
        bright_ = darkNow_ = false;
        fringeLeft_ = darkLeft_ = 0;
        prevReset_ = 0.0f;
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    double x_ = 0.0, p2_ = 0.0, p3_ = 0.0;
    std::uint64_t rng_ = 0x1A7E5F00DULL;
    float moire_ = 0.0f, prevReset_ = 0.0f;
    bool bright_ = false, darkNow_ = false;
    int fringeLeft_ = 0, darkLeft_ = 0;
};

} // namespace rasgo::modular
