#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// BOUNCE (#67) — bolas que quicam: ritmo por física (2026-11-09, aprovado pelo
// autor a partir do estudo do Cardinal, dossies/ESTUDO_cardinal_catalogo_e_licencas.md §4b).
//
// Até 3 bolas caem sob gravidade e quicam no chão; a cada quique perdem
// energia (coeficiente de restituição ELAST). O intervalo entre os quiques
// encolhe por um fator igual a ELAST — o acelerando natural de uma bola
// quicando. Mecânica de livro-texto (queda livre, colisão parcialmente
// elástica). Conceito lido no JW-Modules Bouncy Balls (BSD-3) e no
// voxglitch Hazumi (GPL-3.0+); nenhum código copiado.
//
// Saídas: GT (um pulso a cada quique de qualquer bola), ACC (a força do
// último quique, 0–1, mantida até o próximo — o acento), H1–H3 (a altura de
// cada bola, 0–1, como CV contínua).
//
// Desvio Rasgo:
//  (a) WIND — a cada quique, a restituição varia por um acaso SEMEADO: o
//      ritmo nunca é exatamente o da física ideal;
//  (b) AUTO — quando as bolas param, elas são lançadas de novo, de alturas
//      sorteadas (semeadas): o módulo toca sozinho para sempre; DROP relança
//      na hora.
//
// Escalas: altura 1 = o lançamento mais alto; GRAV em "alturas por s²"
// (9,8 → uma queda de altura 1 dura ~0,45 s). A CV GRAV multiplica a
// gravidade por 2^(2·cv). process() não aloca; determinístico.
namespace rasgo::modular {

class Bounce final : public Signal {
public:
    static constexpr int kBalls = 3;

    Bounce() : Signal(
        {{"drop", PortKind::Control, "trig"},
         {"grav_cv", PortKind::Control, ""}},
        {{"gate", PortKind::Control, "gate"},
         {"accent", PortKind::Control, ""},
         {"h1", PortKind::Control, ""},
         {"h2", PortKind::Control, ""},
         {"h3", PortKind::Control, ""}},
        {{"balls", 1.0f, 3.0f, 3.0f, ""},
         {"gravity", 1.0f, 40.0f, 9.8f, ""},
         {"elastic", 0.3f, 0.95f, 0.75f, ""},
         {"spread", 0.0f, 1.0f, 0.5f, ""},
         {"wind", 0.0f, 1.0f, 0.15f, ""},
         {"auto", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "BOUNCE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "BOUNCE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "bounce", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "BALLS", "balls", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "GRAV", "gravity", 25.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ELAST", "elastic", 43.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "WIND", "wind", 25.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "AUTO", "auto", 45.3f, 56.0f);
        p.add(Widget::Kind::Jack, "DROP", "in:drop", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GRAV", "in:grav_cv", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ACC", "out:accent", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "H1", "out:h1", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "H2", "out:h2", 43.0f, 116.0f);
        p.add(Widget::Kind::Jack, "H3", "out:h3", 55.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0xB0B0BA11B0D1E067ULL;
        prevDrop_ = 0.0f;
        gateSamples_ = 0;
        accent_ = 0.0f;
        hits_ = 0;
        launch(std::clamp(parameterValue("spread"), 0.0f, 1.0f));
    }

    float height(int ball) const noexcept {
        return h_[static_cast<std::size_t>(std::clamp(ball, 0, kBalls - 1))];
    }
    long hits() const noexcept { return hits_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const AudioBlock* drop = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* gcv = inputs.size() > 1 ? inputs[1] : nullptr;

        const int balls = std::clamp(static_cast<int>(std::lround(safe(parameterValue("balls"), 3.0f))), 1, kBalls);
        const float gravK = std::clamp(safe(parameterValue("gravity"), 9.8f), 1.0f, 40.0f);
        const float elastic = std::clamp(safe(parameterValue("elastic"), 0.75f), 0.3f, 0.95f);
        const float spread = std::clamp(safe(parameterValue("spread"), 0.5f), 0.0f, 1.0f);
        const float wind = std::clamp(safe(parameterValue("wind"), 0.15f), 0.0f, 1.0f);
        const bool autoRelaunch = safe(parameterValue("auto"), 1.0f) >= 0.5f;
        const float dt = 1.0f / sr_;
        const int trigLen = std::max(1, static_cast<int>(0.010f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (drop) {
                const float d = safe(drop->at(0, f), 0.0f);
                if (prevDrop_ < 0.5f && d >= 0.5f) launch(spread);
                prevDrop_ = d;
            }
            const float cv = gcv ? std::clamp(safe(gcv->at(0, f), 0.0f), -2.0f, 2.0f) : 0.0f;
            const float g = gravK * std::exp2(2.0f * cv);
            bool moving = false;
            for (int b = 0; b < kBalls; ++b) {
                auto& h = h_[static_cast<std::size_t>(b)];
                auto& v = v_[static_cast<std::size_t>(b)];
                auto& rest = rest_[static_cast<std::size_t>(b)];
                if (b >= balls || rest) { h = 0.0f; v = 0.0f; continue; }
                v -= g * dt;
                h += v * dt;
                if (h <= 0.0f) {
                    h = 0.0f;
                    const float impact = -v;
                    // força relativa à queda de altura 1: sqrt(2g)
                    const float strength = std::clamp(impact / std::sqrt(2.0f * g), 0.0f, 1.0f);
                    float e = elastic;
                    if (wind > 0.0f) e *= 1.0f + 0.25f * wind * (2.0f * unit() - 1.0f);
                    e = std::clamp(e, 0.05f, 0.97f);
                    v = impact * e;
                    if (strength > 0.01f) {
                        gateSamples_ = trigLen;
                        accent_ = strength;
                        ++hits_;
                        if (hits_ > 1000000000L) hits_ = 0;
                    }
                    if (v * v < 2.0f * g * 1.0e-4f) { v = 0.0f; rest = true; }  // parou (pico < 1e-4)
                }
                if (!rest) moving = true;
            }
            if (!moving && autoRelaunch) launch(spread);

            const float gate = gateSamples_ > 0 ? 1.0f : 0.0f;
            if (gateSamples_ > 0) --gateSamples_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = gate;
                outputs[1].at(c, f) = accent_;
                outputs[2].at(c, f) = h_[0];
                outputs[3].at(c, f) = h_[1];
                outputs[4].at(c, f) = h_[2];
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    // Alturas: a bola 1 em 1; as outras abaixo, afastadas por SPREAD, com um
    // pouco de acaso semeado — as bolas não quicam juntas.
    void launch(float spread) noexcept {
        for (int b = 0; b < kBalls; ++b) {
            const float jitter = 0.1f * unit();
            const float hb = 1.0f - spread * (static_cast<float>(b) / static_cast<float>(kBalls)) - (b > 0 ? jitter : 0.0f);
            h_[static_cast<std::size_t>(b)] = std::clamp(hb, 0.05f, 1.0f);
            v_[static_cast<std::size_t>(b)] = 0.0f;
            rest_[static_cast<std::size_t>(b)] = false;
        }
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0xB0B0BA11B0D1E067ULL;
    float prevDrop_ = 0.0f;
    int gateSamples_ = 0;
    float accent_ = 0.0f;
    long hits_ = 0;
    std::array<float, kBalls> h_{};
    std::array<float, kBalls> v_{};
    std::array<bool, kBalls> rest_{};
};

} // namespace rasgo::modular
