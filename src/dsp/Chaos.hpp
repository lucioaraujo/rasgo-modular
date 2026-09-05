#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// CHAOS — campo caótico de poço duplo (Módulo 36)
// ============================================================================
//
// Fonte genuinamente CAÓTICA — não pseudo-aleatória como `DECISION`/
// `TURING` (que sorteiam), não passeio como `DRIFT` (que soma ruído
// filtrado). Dois integradores (x, y) perseguem uma força restauradora
// não-linear (x − x³) com dois poços estáveis em x≈−1/x≈+1; `drive`/
// `damping` decidem se o sistema assenta num poço, oscila entre os dois
// ou "caça" imprevisível. Sem um "chute" periódico aleatório, o sistema
// é uma EDO determinística sem forçamento: uma vez orbitando um poço,
// nunca alcança o outro (achado do próprio autor do Antitotem, ver
// comentário-fonte) — o chute é o que deixa a trajetória atravessar de
// um poço pro outro de vez em quando.
//
// Estudado de `ANTITOTEM/src/core/ChaosSources.h::ChaosField` (código
// do autor, GPLv3/AGPLv3 — compatível; ver `PESQUISA_MODULOS.md §2.3`),
// reescrito no idioma header-only zero-dep do Rasgo Modular — desvio:
// porta `reseed` (trigger externo) em vez de método chamado pelo host,
// `rate_mod` como entrada de CV (convenção do Rasgo), painel/parâmetros
// próprios.
//
// `rate` cobre de CV lenta (0,02 Hz) a textura de áudio (400 Hz) — o
// mesmo eixo que ENERGIA escala noutros módulos do Antitotem. Saída
// sempre limitada a [−1,1] (os estados internos x/y são clampados
// separadamente, mais largos, pra dar fôlego à dinâmica sem escapar pra
// um valor perigoso). Determinístico (RNG semeado). Sem alocação em
// `process()`.

namespace rasgo::modular {

class Chaos final : public Signal {
public:
    Chaos()
        : Signal(
              {{"reseed", PortKind::Control, "trig"},
               {"rate_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"rate", 0.02f, 400.0f, 4.0f, "Hz"},
               {"drive", 0.0f, 1.0f, 0.6f, ""},
               {"damping", 0.05f, 1.0f, 0.3f, ""},
               {"freeze", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CHAOS"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        // 8 -> 7 HP no passe de ergonomia 2026-09-06 (só 3 knobs + toggle
        // + 3 jacks); display cheio, knobs 2 col centradas.
        Panel p;
        p.hp = 7;
        p.add(Widget::Kind::Label, "CHAOS", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "field", "", 2.5f, 6.0f, 30.6f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 21.5f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damping", 5.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "FRZ", "freeze", 23.0f, 54.0f);
        p.add(Widget::Kind::Jack, "RSD", "in:reseed", 7.0f, 98.0f);
        p.add(Widget::Kind::Jack, "RTM", "in:rate_mod", 18.0f, 98.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 29.0f, 98.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        resetField();
        rngState_ = 0x9E3779B97F4A7C15ULL;
        prevReseed_ = 0.0f;
        kickCounter_ = 0;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float rateBase = parameterValue("rate");
        const float drive = parameterValue("drive");
        const float damping = parameterValue("damping");
        const bool freeze = parameterValue("freeze") >= 0.5f;

        const AudioBlock* reseedIn = inputs[0];
        const AudioBlock* rateModIn = inputs[1];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (reseedIn != nullptr) {
                const float r = reseedIn->at(0, frame);
                if (prevReseed_ < 0.5f && r >= 0.5f) reseedField();
                prevReseed_ = r;
            }

            if (!freeze) {
                const float rate = clampf(
                    rateBase + (rateModIn ? rateModIn->at(0, frame) : 0.0f),
                    0.02f, 400.0f);
                const float dt = clampf(rate / sampleRate_, 0.0f, 0.02f);
                const float pull = drive * 2.4f;
                const float xNext = x_ + dt * y_ * 6.0f;
                float yNext = y_ + dt
                    * (pull * (x_ - x_ * x_ * x_) - damping * 3.0f * y_);
                // chute periódico -- sem ele o sistema nunca cruza pro
                // outro poço (ver o comentário do topo)
                const std::uint32_t samplesPerCycle = static_cast<std::uint32_t>(
                    std::max(1.0f, sampleRate_ / std::max(rate, 0.02f)));
                if (++kickCounter_ >= samplesPerCycle) {
                    kickCounter_ = 0;
                    yNext += whiteNoise() * drive * 2.5f;
                }
                x_ = clampf(xNext, -2.2f, 2.2f);
                y_ = clampf(yNext, -3.5f, 3.5f);
                if (!std::isfinite(x_) || !std::isfinite(y_)) resetField();
            }

            const float v = clampf(x_, -1.0f, 1.0f);
            for (std::size_t c = 0; c < channels; ++c)
                out.at(c, frame) = v;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    // xorshift64* -> [-1,1)
    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12; rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }
    // x=0 é o equilíbrio INSTÁVEL do sistema (o topo entre os dois
    // poços) -- começar exatamente ali deixaria tudo parado até um
    // chute empurrar; 0,15 já entra inclinado pra um lado.
    void resetField() noexcept { x_ = 0.15f; y_ = 0.0f; kickCounter_ = 0; }
    void reseedField() noexcept {
        x_ = clampf(whiteNoise() * 1.6f, -2.2f, 2.2f);
        y_ = clampf(whiteNoise() * 3.5f, -3.5f, 3.5f);
        kickCounter_ = 0;
    }

    float x_ = 0.15f, y_ = 0.0f;
    std::uint32_t kickCounter_ = 0;
    std::uint64_t rngState_ = 0x9E3779B97F4A7C15ULL;
    float prevReseed_ = 0.0f;
};

}  // namespace rasgo::modular
