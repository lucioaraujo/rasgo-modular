#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

// ============================================================================
// STRING — corda por guia-de-onda (Módulo 11)
// ============================================================================
//
// A outra face do MATTER: em vez de somar modos, um laço de atraso com
// perda - o modelo de corda de Karplus-Strong estendido. Uma corda
// pinçada (ou arcada, com entrada contínua) cuja altura é o comprimento do
// laço, cujo brilho é o filtro no laço, cujo sustain é o ganho de
// realimentação. `position` = onde a corda é pinçada (filtro pente na
// excitação).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/11_string.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Karplus & Strong (1983), "Digital Synthesis of Plucked-String and
//     Drum Timbres"; Jaffe & Smith (1983), "Extensions of the
//     Karplus-Strong Plucked-String Algorithm" - afinação por all-pass
//     fracionário, decaimento dependente de frequência, posição de pinça;
//   - J. O. Smith, *Physical Audio Signal Processing* - guia-de-onda,
//     estabilidade do laço, filtro de perda;
//   - Mutable Elements / Rings (modo corda) - `position`/`brightness`/
//     `damping` como eixos tocáveis; excitação externa ou interna.
//
// Determinístico: rajada de excitação = ruído xorshift semeado em
// prepare(). Buffer alocado em prepare(); process() não aloca.

namespace rasgo::modular {

class StringVoice final : public Signal {
public:
    StringVoice()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"pluck", PortKind::Control, "trig"},
               {"freq_mod", PortKind::Control, "v/oct"},
               {"damp_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 20.0f, 4000.0f, 110.0f, "Hz"},
               {"decay", 0.0f, 1.0f, 0.7f, ""},
               {"damping", 0.0f, 1.0f, 0.4f, ""},
               {"position", 0.02f, 0.5f, 0.14f, ""},
               {"exciter", 0.0f, 1.0f, 0.6f, ""},
               {"drive", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "STRING"; }

    // Layout: 12 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "STRING", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "string", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DECAY", "decay", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damping", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "POS", "position", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "EXCIT", "exciter", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 35.0f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "PLK", "in:pluck", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:freq_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "DMP", "in:damp_mod", 41.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 5.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        bufLen_ = static_cast<std::size_t>(std::max(64.0f, sampleRate / 15.0f));
        buffer_.assign(bufLen_, 0.0f);
        writePos_ = 0;
        lpState_ = 0.0f;
        apState_ = 0.0f;
        lastApOut_ = 0.0f;
        prevPluck_ = 0.0f;
        rngState_ = 0x6A09E667F3BCC909ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float freqParam = parameterValue("freq");
        const float decay = parameterValue("decay");
        const float dampingParam = parameterValue("damping");
        const float position = parameterValue("position");
        const float exciter = parameterValue("exciter");
        const float drive = parameterValue("drive");
        const float mix = parameterValue("mix");

        const AudioBlock* in = inputs[0];
        const AudioBlock* pluck = inputs[1];
        const AudioBlock* freqMod = inputs[2];
        const AudioBlock* dampMod = inputs[3];

        float f0 = freqParam;
        if (freqMod)
            f0 *= std::exp2(freqMod->at(0, 0));
        f0 = clampf(f0, 15.0f, sampleRate_ * 0.45f);
        float damping = dampingParam;
        if (dampMod)
            damping = clampf(damping + dampMod->at(0, 0), 0.0f, 1.0f);

        // atraso total do laço = sr/f0 = D (inteiro) + frac; frac via
        // all-pass de 1ª ordem (Jaffe & Smith)
        const float loopDelay = sampleRate_ / f0;
        int intDelay = static_cast<int>(loopDelay - 0.5f);
        if (intDelay < 2) intDelay = 2;
        if (static_cast<std::size_t>(intDelay) >= bufLen_ - 2)
            intDelay = static_cast<int>(bufLen_) - 3;
        const float frac = loopDelay - static_cast<float>(intDelay);
        const float apCoeff = (1.0f - frac) / (1.0f + frac);

        // filtro de perda no laço: 1-polo LP, mais fechado com `damping`
        const float lpA = 0.05f + (1.0f - damping) * 0.9f;
        // ganho de realimentação (sustain), sempre < 1
        const float fbGain = 0.86f + decay * 0.139f;   // 0,86 .. 0,999
        const float driveGain = 1.0f + drive * 5.0f;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (pluck) {
                const float pk = pluck->at(0, frame);
                if (prevPluck_ < 0.5f && pk >= 0.5f)
                    excite(intDelay, position, exciter);
                prevPluck_ = pk;
            }

            // lê a amostra atrasada
            const std::size_t rp =
                (writePos_ + bufLen_ - static_cast<std::size_t>(intDelay))
                % bufLen_;
            const float delayed = buffer_[rp];

            // filtro de perda
            lpState_ += lpA * (delayed - lpState_);
            // all-pass fracionário (afinação fina)
            const float ap = apCoeff * lpState_ + apState_
                - apCoeff * lastApOut_;
            apState_ = lpState_;
            lastApOut_ = ap;

            // realimentação + excitação contínua (arco). O tanh no laço
            // garante estabilidade (o arco leva a um ciclo-limite, não à
            // divergência) - princípio do FILTER auto-oscilante.
            float v = fbGain * ap;
            if (in)
                v += 0.35f * in->at(0, frame);
            v = std::tanh(v * driveGain);
            buffer_[writePos_] = v;
            writePos_ = (writePos_ + 1) % bufLen_;

            const float dry = in ? in->at(0, frame) : 0.0f;
            const float outValue = mix * ap + (1.0f - mix) * dry;
            for (std::size_t channel = 0; channel < channels; ++channel)
                out.at(channel, frame) = outValue;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    // Excitação: enche o segmento do laço com ruído filtrado por um pente
    // na posição de pinça (um modo cujo nó cai em `position` não é excitado).
    void excite(const int intDelay, const float position,
                const float exciter) noexcept {
        const int combLag = std::max(
            1, static_cast<int>(position * static_cast<float>(intDelay)));
        float prev = 0.0f;
        for (int i = 0; i < intDelay; ++i) {
            const float n = whiteNoise();
            // pente simples: n - n(atrasado por combLag), aproximado com um
            // passa-baixa leve pra não estourar agudos
            const float shaped = 0.5f * (n - prev);
            prev = n;
            const float combGain =
                (i >= combLag) ? 1.0f : (static_cast<float>(i) / combLag);
            // preenche o trecho do laço que será lido a seguir
            const std::size_t idx =
                (writePos_ + bufLen_ - static_cast<std::size_t>(intDelay)
                 + static_cast<std::size_t>(i))
                % bufLen_;
            buffer_[idx] += exciter * shaped * combGain;
        }
    }

    std::vector<float> buffer_;
    std::size_t bufLen_ = 1;
    std::size_t writePos_ = 0;
    float lpState_ = 0.0f;
    float apState_ = 0.0f;
    float lastApOut_ = 0.0f;
    float prevPluck_ = 0.0f;
    std::uint64_t rngState_ = 0x6A09E667F3BCC909ULL;
};

}  // namespace rasgo::modular
