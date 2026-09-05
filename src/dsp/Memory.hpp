#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

// ============================================================================
// MEMORY — buffer granular com congelamento (Módulo 7)
// ============================================================================
//
// O patch escuta a si mesmo. Um buffer circular grava o que entra; `freeze`
// para a gravação e o buffer vira uma textura fixa; grãos janelados releem
// esse material em outra posição, outra altura, outro tempo. É a cicatriz
// do `Cable` levada a módulo: reter não é apagar, é ter de onde tocar.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/07_memory.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Mutable Clouds (STM32F4, MIT) - buffer + textura granular + freeze;
//     position/size/density/texture/pitch/blend/feedback;
//   - Instruō arbhar - captura por onset do próprio mix, autoescuta;
//   - granular clássico (Roads, "Microsound") - grão = janela curta com
//     envelope, nuvem = muitos grãos assíncronos;
//   - a ruptura/cicatriz do `Cable` (Atlas §37) - o precedente Rasgo.
//
// Determinístico: xorshift64* semeado em prepare(). Buffer alocado em
// prepare() (fora do áudio); process() não aloca.

namespace rasgo::modular {

class Memory final : public Signal {
public:
    Memory()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"position_mod", PortKind::Control, ""},
               {"pitch_mod", PortKind::Control, "v/oct"},
               {"freeze_gate", PortKind::Control, "gate"}},
              {{"out", PortKind::Audio, ""}},
              {{"grain", 0.005f, 0.5f, 0.08f, "s"},
               {"density", 0.1f, 120.0f, 20.0f, "Hz"},
               {"position", 0.0f, 1.0f, 0.2f, ""},
               {"spray", 0.0f, 1.0f, 0.1f, ""},
               {"pitch", -24.0f, 24.0f, 0.0f, "st"},
               {"feedback", 0.0f, 0.95f, 0.0f, ""},
               {"blend", 0.0f, 1.0f, 1.0f, ""},
               {"freeze", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "MEMORY"; }

    // Layout: 14 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "MEMORY", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "buffer", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "GRAIN", "grain", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DENS", "density", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "POS", "position", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRAY", "spray", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "PITCH", "pitch", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "BLEND", "blend", 35.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "HOLD", "freeze", 49.0f, 54.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "POS", "in:position_mod", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "FRZ", "in:freeze_gate", 41.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 5.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        bufLen_ = static_cast<std::size_t>(std::max(1.0f, sampleRate * 3.0f));
        buffer_.assign(bufLen_, 0.0f);
        writePos_ = 0;
        grainClock_ = 0.0f;
        lastOut_ = 0.0f;
        rngState_ = 0x27D4EB2F165667C5ULL;
        for (auto& g : grains_)
            g = Grain{};
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float grainSec = parameterValue("grain");
        const float density = parameterValue("density");
        const float positionParam = parameterValue("position");
        const float spray = parameterValue("spray");
        const float pitchParam = parameterValue("pitch");
        const float feedback = parameterValue("feedback");
        const float blend = parameterValue("blend");
        const bool freezeParam = parameterValue("freeze") >= 0.5f;

        const AudioBlock* in = inputs[0];
        const AudioBlock* posMod = inputs[1];
        const AudioBlock* pitchMod = inputs[2];
        const AudioBlock* freezeGate = inputs[3];

        const float grainInc = 1.0f
            / std::max(1.0f, grainSec * sampleRate_);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const bool frozen = freezeParam
                || (freezeGate && freezeGate->at(0, frame) >= 0.5f);
            const float dry = in ? in->at(0, frame) : 0.0f;

            if (!frozen) {
                float w = dry + feedback * lastOut_;
                w = w > 1.5f ? 1.5f : (w < -1.5f ? -1.5f : w);
                buffer_[writePos_] = w;
                writePos_ = (writePos_ + 1) % bufLen_;
            }

            // agenda grãos novos
            grainClock_ += density / sampleRate_;
            if (grainClock_ >= 1.0f) {
                grainClock_ -= 1.0f;
                spawnGrain(grainInc, positionParam
                           + (posMod ? posMod->at(0, frame) : 0.0f),
                           spray,
                           pitchParam
                           + (pitchMod ? 12.0f * pitchMod->at(0, frame) : 0.0f));
            }

            // soma os grãos ativos
            float wet = 0.0f;
            for (auto& g : grains_) {
                if (!g.active)
                    continue;
                const float window = 0.5f - 0.5f * std::cos(6.2831853f * g.phase);
                const std::size_t i0 = static_cast<std::size_t>(g.readPos) % bufLen_;
                const std::size_t i1 = (i0 + 1) % bufLen_;
                const float frac = g.readPos - std::floor(g.readPos);
                const float sample =
                    buffer_[i0] * (1.0f - frac) + buffer_[i1] * frac;
                wet += sample * window;

                g.readPos += g.speed;
                if (g.readPos >= static_cast<float>(bufLen_))
                    g.readPos -= static_cast<float>(bufLen_);
                g.phase += g.phaseInc;
                if (g.phase >= 1.0f)
                    g.active = false;
            }
            wet *= 0.6f;  // compensação de sobreposição

            const float mixed = blend * wet + (1.0f - blend) * dry;
            lastOut_ = mixed;
            for (std::size_t channel = 0; channel < channels; ++channel)
                out.at(channel, frame) = mixed;
        }
    }

private:
    struct Grain {
        bool active = false;
        float readPos = 0.0f;
        float speed = 1.0f;
        float phase = 0.0f;
        float phaseInc = 0.0f;
    };

    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    void spawnGrain(const float grainInc, const float position,
                    const float spray, const float pitchSemis) noexcept {
        Grain* slot = nullptr;
        for (auto& g : grains_)
            if (!g.active) {
                slot = &g;
                break;
            }
        if (slot == nullptr)
            return;  // pool cheio - grão descartado (comportamento de nuvem)

        const float pos = position < 0.0f ? 0.0f : (position > 1.0f ? 1.0f : position);
        const float back =
            (0.03f + pos * 0.9f + spray * uniform01() * 0.4f)
            * static_cast<float>(bufLen_);
        float start = static_cast<float>(writePos_) - back;
        while (start < 0.0f)
            start += static_cast<float>(bufLen_);

        slot->active = true;
        slot->readPos = start;
        slot->speed = std::exp2(pitchSemis / 12.0f);
        slot->phase = 0.0f;
        slot->phaseInc = grainInc;
    }

    std::vector<float> buffer_;
    std::size_t bufLen_ = 1;
    std::size_t writePos_ = 0;
    float grainClock_ = 0.0f;
    float lastOut_ = 0.0f;
    std::uint64_t rngState_ = 0x27D4EB2F165667C5ULL;

    static constexpr int kGrains = 16;
    Grain grains_[kGrains];
};

}  // namespace rasgo::modular
