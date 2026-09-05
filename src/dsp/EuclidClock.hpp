#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// CLOCK — relógio euclidiano com acento polirrítmico (Módulo 5)
// ============================================================================
//
// O tempo do patch: um clock de passo em divisões de um andamento, um
// gate EUCLIDIANO (pulsos distribuídos o mais uniformemente possível numa
// grade) e um ACENTO que nasce do AND/OR de dois divisores — polirritmia
// "de graça", como nos vpme Euclidean Circles.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/05_clock.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - vpme Euclidean Circles / QD — euclidiano + AND/OR entre divisores;
//   - ALM Pamela's PRO Workout — clock com divisões, swing, e "deriva"
//     como parâmetro (o andamento respira);
//   - Mutable Grids — ritmo como campo contínuo;
//   - E(k,n) de Toussaint / Bjorklund — aqui pela fórmula de Bresenham
//     `(i·k) mod n < k` (equivalente cíclico, O(1), sem buffer).
//
// Desvio Rasgo: `drift` — random-walk lento no andamento efetivo
// (±~10%), o mesmo princípio do Módulo 1. `drift = 0` → determinístico.
//
// Avanço: clock interno em `bpm·mult` OU, se `ext_clock` estiver
// conectado, nas bordas de subida externas (o período é estimado do
// intervalo entre bordas, pra manter `gate_len`/`swing` coerentes).
//
// `feel` (2026-09-05) — estudado de `ANTITOTEM/src/core/
// SimpleSequencer.h::ClockFeel` (código do autor, GPLv3/AGPLv3 —
// compatível; ver `PESQUISA_MODULOS.md §2.3`), reduzido: o Antitotem
// tem 8 posições incluindo `swing` — aqui ficou de fora porque o `CLOCK`
// já tem um `swing` contínuo próprio (duplicar como posição discreta só
// confundiria). `feel` escolhe o AGRUPAMENTO em vez da velocidade pura:
// `mult` continua controlando velocidade fina, `feel` multiplica por
// 1/3/5/7/9/11 (reto/tercina/quintina/septina/nonina/undecina) —
// diferente de só girar MULT até 3,0 porque fica quantizado e nomeado
// (não precisa acertar o número exato de ouvido). `glitch` é o único
// modo qualitativamente novo: cada passo sorteia um fator de tempo
// (~0,7–1,4×) — o clock deixa de ser um trem de pulsos regular.

namespace rasgo::modular {

class EuclidClock final : public Signal {
public:
    EuclidClock()
        : Signal(
              {{"ext_clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"},
               {"bpm_mod", PortKind::Control, ""}},
              {{"clock", PortKind::Control, "gate"},
               {"euclid", PortKind::Control, "gate"},
               {"accent", PortKind::Control, "gate"}},
              {{"bpm", 20.0f, 300.0f, 120.0f, "BPM"},
               {"mult", 0.25f, 8.0f, 2.0f, "x"},
               {"length", 1.0f, 32.0f, 8.0f, ""},
               {"fill", 0.0f, 32.0f, 4.0f, ""},
               {"rotate", 0.0f, 31.0f, 0.0f, ""},
               {"swing", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""},
               {"gate_len", 0.05f, 0.95f, 0.5f, ""},
               {"accent_a", 1.0f, 16.0f, 4.0f, ""},
               {"accent_b", 1.0f, 16.0f, 3.0f, ""},
               {"accent_mode", 0.0f, 1.0f, 0.0f, ""},
               {"feel", 0.0f, 6.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CLOCK"; }

    // Layout: 16 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "CLOCK", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "pattern", "", 2.5f, 6.0f, 76.3f);
        p.add(Widget::Kind::Knob, "BPM", "bpm", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MULT", "mult", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LEN", "length", 37.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FILL", "fill", 52.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ROT", "rotate", 67.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SWING", "swing", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 22.0f, 52.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 37.0f, 52.0f);
        p.add(Widget::Kind::Knob, "ACC-A", "accent_a", 52.0f, 52.0f);
        p.add(Widget::Kind::Knob, "ACC-B", "accent_b", 67.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "AND", "accent_mode", 7.0f, 76.0f);
        p.add(Widget::Kind::Knob, "FEEL", "feel", 22.0f, 76.0f);
        p.add(Widget::Kind::Jack, "EXT", "in:ext_clock", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "BPM", "in:bpm_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "CLK", "out:clock", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "EUC", "out:euclid", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ACC", "out:accent", 29.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        stepCounter_ = 0;
        prevExt_ = 0.0f;
        prevReset_ = 0.0f;
        extSamples_ = 0.0f;
        extPeriod_ = std::max(1.0f, sampleRate / 4.0f);
        extSeen_ = false;
        phaseInStep_ = 0.0f;
        driftState_ = 0.0f;
        driftCounter_ = 0;
        driftInterval_ =
            static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 20.0f));
        rngState_ = 0xD1B54A32D192ED03ULL;
        glitchFactor_ = 1.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& clockOut = outputs[0];
        AudioBlock& euclidOut = outputs[1];
        AudioBlock& accentOut = outputs[2];
        const std::size_t frames = clockOut.frames();
        const std::size_t channels = clockOut.channels();

        const float bpm = parameterValue("bpm");
        const float mult = parameterValue("mult");
        const int length = clampi(
            static_cast<int>(std::lround(parameterValue("length"))), 1, 32);
        const int fill = clampi(
            static_cast<int>(std::lround(parameterValue("fill"))), 0, 32);
        const int rotate = clampi(
            static_cast<int>(std::lround(parameterValue("rotate"))), 0, 31);
        const float swing = parameterValue("swing");
        const float drift = parameterValue("drift");
        const float gateLen = parameterValue("gate_len");
        const int accentA = clampi(
            static_cast<int>(std::lround(parameterValue("accent_a"))), 1, 16);
        const int accentB = clampi(
            static_cast<int>(std::lround(parameterValue("accent_b"))), 1, 16);
        const bool accentAnd = parameterValue("accent_mode") >= 0.5f;
        const float driftStep = 0.02f * drift * drift;
        const int feelIdx = clampi(
            static_cast<int>(std::lround(parameterValue("feel"))), 0, 6);
        // reto/tercina/quintina/septina/nonina/undecina/glitch (razão 1,
        // o glitch usa o fator por passo em vez de multiplicar aqui)
        static constexpr float kTupletRatio[7] = {1, 3, 5, 7, 9, 11, 1};
        const float tupletRatio = kTupletRatio[feelIdx];
        const bool glitch = feelIdx == 6;

        const AudioBlock* ext = inputs[0];
        const AudioBlock* reset = inputs[1];
        const AudioBlock* bpmMod = inputs[2];
        const bool externalClock = (ext != nullptr);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            // deriva orgânica no andamento
            if (++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftState_ += whiteNoise() * driftStep;
                driftState_ = clampf(driftState_, -0.12f, 0.12f);
            }

            if (reset != nullptr) {
                const float r = reset->at(0, frame);
                if (prevReset_ < 0.5f && r >= 0.5f) {
                    stepCounter_ = 0;
                    phase_ = 0.0;
                    extSamples_ = 0.0f;
                }
                prevReset_ = r;
            }

            bool stepped = false;
            if (externalClock) {
                const float e = ext->at(0, frame);
                if (prevExt_ < 0.5f && e >= 0.5f) {
                    if (extSeen_ && extSamples_ > 1.0f)
                        extPeriod_ = extSamples_;
                    extSamples_ = 0.0f;
                    extSeen_ = true;
                    advanceStep(glitch);
                    stepped = true;
                }
                prevExt_ = e;
                extSamples_ += 1.0f;
                phaseInStep_ = clampf(extSamples_ / extPeriod_, 0.0f, 0.99999f);
            } else {
                const float effBpm =
                    clampf(bpm + (bpmMod ? bpmMod->at(0, frame) : 0.0f),
                           1.0f, 1000.0f);
                const float glitchMul = glitch ? glitchFactor_ : 1.0f;
                const float stepHz = effBpm / 60.0f * mult * tupletRatio
                    * (1.0f + driftState_) * glitchMul;
                phase_ += static_cast<double>(std::max(0.0001f, stepHz))
                    / sampleRate_;
                if (phase_ >= 1.0) {
                    phase_ -= 1.0;
                    advanceStep(glitch);
                    stepped = true;
                }
                phaseInStep_ = static_cast<float>(phase_);
            }
            (void)stepped;

            const int i = stepCounter_ % length;
            const bool euOnset = euclidOnset(i, length, fill, rotate);
            const bool acOnset = accentOnset(stepCounter_, accentA, accentB,
                                             accentAnd);

            // swing atrasa os passos ímpares dentro da própria fatia
            const float swOff = (i % 2 == 1) ? swing * 0.45f : 0.0f;
            const bool win = phaseInStep_ >= swOff
                && phaseInStep_ < swOff + gateLen;

            const float clk = (phaseInStep_ < gateLen) ? 1.0f : 0.0f;
            const float eu = (euOnset && win) ? 1.0f : 0.0f;
            const float ac = (acOnset && win) ? 1.0f : 0.0f;

            for (std::size_t channel = 0; channel < channels; ++channel) {
                clockOut.at(channel, frame) = clk;
                euclidOut.at(channel, frame) = eu;
                accentOut.at(channel, frame) = ac;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    void advanceStep(const bool glitch) noexcept {
        ++stepCounter_;
        if (stepCounter_ >= 1000000)
            stepCounter_ = stepCounter_ % 720720;  // ~LCM(1..16), preserva fase
        // só sorteia quando `feel` = glitch -- nunca consome o stream do
        // RNG compartilhado com `drift` fora desse modo (senão até um
        // patch com FEEL sempre reto teria `drift` deslocado)
        if (glitch) glitchFactor_ = 0.7f + 0.7f * (0.5f + 0.5f * whiteNoise());
    }

    // E(k,n) pela fórmula de Bresenham: onset em i sse (i·k) mod n < k.
    // Equivale ciclicamente ao Bjorklund pros casos musicais e é O(1).
    static bool euclidOnset(const int index, const int length, const int fill,
                            const int rotate) noexcept {
        if (fill <= 0)
            return false;
        if (fill >= length)
            return true;
        const long long j = (static_cast<long long>(index) + rotate) % length;
        const long long a = (j * fill) % length;
        return a < fill;
    }

    static bool accentOnset(const int step, const int a, const int b,
                            const bool useAnd) noexcept {
        const bool hitA = (step % a) == 0;
        const bool hitB = (step % b) == 0;
        return useAnd ? (hitA && hitB) : (hitA || hitB);
    }

    // xorshift64* -> ruído branco [-1,1)
    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    double phase_ = 0.0;
    int stepCounter_ = 0;
    float phaseInStep_ = 0.0f;

    float prevExt_ = 0.0f;
    float prevReset_ = 0.0f;
    float extSamples_ = 0.0f;
    float extPeriod_ = 12000.0f;
    bool extSeen_ = false;

    float driftState_ = 0.0f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 2400;
    std::uint64_t rngState_ = 0xD1B54A32D192ED03ULL;
    float glitchFactor_ = 1.0f;
};

}  // namespace rasgo::modular
