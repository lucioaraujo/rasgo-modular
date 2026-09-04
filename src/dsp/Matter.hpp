#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// MATTER — ressoador modal (Módulo 9)
// ============================================================================
//
// O som de um objeto: um banco de modos ressonantes afinados, excitados
// por um sinal de entrada (um clique, ruído, a voz) ou por um golpe
// interno. As RAZÕES entre os modos são o material - harmônicas = corda;
// esticadas = sino/metal. A POSIÇÃO da excitação decide quais modos
// recebem energia (tocar uma corda no nó de um harmônico não o excita).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/09_matter.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Mutable Rings / Elements (STM32F4, MIT) - ressoador modal + corda;
//     structure/brightness/damping/position como eixos do material;
//   - síntese modal clássica (Adrien, Cook "Real Sound Synthesis") -
//     soma de osciladores amortecidos = resposta impulsiva de um corpo;
//   - rigidez de corda / inarmonicidade (Fletcher & Rossing) - os modos
//     de uma corda real não são múltiplos exatos;
//   - ressoador de 2 polos (Smith, "Physical Audio Signal Processing").
//
// Determinístico: golpe interno = rajada de ruído xorshift semeada em
// prepare(). Coeficientes recalculados por bloco (24 modos, barato).

namespace rasgo::modular {

class Matter final : public Signal {
public:
    Matter()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"strike", PortKind::Control, "trig"},
               {"freq_mod", PortKind::Control, "v/oct"},
               {"struct_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 20.0f, 8000.0f, 110.0f, "Hz"},
               {"structure", 0.0f, 1.0f, 0.0f, ""},
               {"brightness", 0.0f, 1.0f, 0.6f, ""},
               {"damping", 0.0f, 1.0f, 0.35f, ""},
               {"position", 0.02f, 0.5f, 0.18f, ""},
               {"exciter", 0.0f, 1.0f, 0.5f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "MATTER"; }

    // Layout: 14 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "MATTER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "modes", "", 2.5f, 8.0f, 55.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "STRC", "structure", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "BRITE", "brightness", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damping", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "POS", "position", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "EXCIT", "exciter", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 35.0f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "HIT", "in:strike", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:freq_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "STR", "in:struct_mod", 41.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 5.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        for (auto& m : modes_)
            m = Mode{};
        prevStrike_ = 0.0f;
        burst_ = 0.0f;
        rngState_ = 0x3C6EF372FE94F82BULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float freqParam = parameterValue("freq");
        const float brightness = parameterValue("brightness");
        const float damping = parameterValue("damping");
        const float position = parameterValue("position");
        const float exciter = parameterValue("exciter");
        const float mix = parameterValue("mix");

        const AudioBlock* in = inputs[0];
        const AudioBlock* strike = inputs[1];
        const AudioBlock* freqMod = inputs[2];
        const AudioBlock* structMod = inputs[3];

        // frequência e estrutura efetivas (uma vez por bloco)
        float f0 = freqParam;
        if (freqMod)
            f0 *= std::exp2(freqMod->at(0, 0));
        f0 = clampf(f0, 10.0f, sampleRate_ * 0.48f);
        float structure = parameterValue("structure");
        if (structMod)
            structure = clampf(structure + structMod->at(0, 0), 0.0f, 1.0f);

        updateModes(f0, structure, brightness, damping, position);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            // golpe interno: rajada de ruído curta na borda de subida
            if (strike) {
                const float s = strike->at(0, frame);
                if (prevStrike_ < 0.5f && s >= 0.5f)
                    burst_ = 1.0f;
                prevStrike_ = s;
            }
            float excitation = in ? in->at(0, frame) : 0.0f;
            if (burst_ > 0.0001f) {
                excitation += exciter * 6.0f * whiteNoise() * burst_;
                burst_ *= 0.9985f;  // rajada ~ 10 ms
                if (burst_ < 0.0002f)
                    burst_ = 0.0f;
            }

            float y = 0.0f;
            for (auto& m : modes_) {
                if (m.amp <= 0.0f)
                    continue;
                const float v = m.a1 * m.z1 - m.a2 * m.z2 + m.b0 * excitation;
                m.z2 = m.z1;
                m.z1 = v;
                y += v * m.amp;
            }
            y = softLimit(y * 3.0f);

            const float dry = excitation;
            const float outValue = mix * y + (1.0f - mix) * dry;
            for (std::size_t channel = 0; channel < channels; ++channel)
                out.at(channel, frame) = outValue;
        }
    }

private:
    static constexpr int kModes = 24;
    struct Mode {
        float a1 = 0.0f, a2 = 0.0f, b0 = 0.0f, amp = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float softLimit(const float x) noexcept {
        return std::tanh(x);
    }

    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    void updateModes(const float f0, const float structure,
                     const float brightness, const float damping,
                     const float position) noexcept {
        const float nyq = sampleRate_ * 0.49f;
        float ampNorm = 0.0f;
        const float brightExp = 2.0f - brightness * 2.0f;  // 2 (escuro) .. 0
        const float t60Base = 0.05f + (1.0f - damping) * 4.0f;

        for (int i = 0; i < kModes; ++i) {
            const float n = static_cast<float>(i + 1);
            // razão: harmônica -> progressivamente esticada (rigidez)
            const float ratio = n * (1.0f + structure * 0.055f * static_cast<float>(i));
            const float f = f0 * ratio;
            Mode& m = modes_[i];
            if (f >= nyq || f <= 0.0f) {
                m.amp = 0.0f;
                m.a1 = m.a2 = m.b0 = 0.0f;
                continue;
            }
            const float w = 6.2831853f * f / sampleRate_;
            const float t60 = t60Base / (1.0f + static_cast<float>(i) * 0.35f);
            float r = std::exp(-6.9077553f / (t60 * sampleRate_));  // ln(1000)
            r = r > 0.99995f ? 0.99995f : r;
            m.a1 = 2.0f * r * std::cos(w);
            m.a2 = r * r;
            // amplitude alvo do modo: brilho (rolloff) x posição de excitação
            const float posWeight =
                std::fabs(std::sin(n * 3.14159265f * position));
            const float targetAmp = std::pow(n, -brightExp) * posWeight;
            // b0 escala a resposta impulsiva ~proporcional a targetAmp
            // (sin(w) compensa o ganho natural do ressoador a baixa freq)
            m.b0 = targetAmp * std::sin(w);
            m.amp = 1.0f;
            if (targetAmp > ampNorm)
                ampNorm = targetAmp;
        }
        if (ampNorm > 0.0f)
            for (auto& m : modes_)
                m.b0 /= ampNorm;  // normaliza pelo modo mais forte
    }

    Mode modes_[kModes];
    float prevStrike_ = 0.0f;
    float burst_ = 0.0f;
    std::uint64_t rngState_ = 0x3C6EF372FE94F82BULL;
};

}  // namespace rasgo::modular
