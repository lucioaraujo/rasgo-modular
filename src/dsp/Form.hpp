#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <cmath>
#include <cstdint>

// ============================================================================
// FORM — a dinâmica da forma (Módulo 105)
// ============================================================================
//
// Proposta `dossies/PROPOSTA_forma_e_andamento.md`; "pode implementar"
// (autor, 21/11/2026). Um mixer que CONDUZ: quatro camadas entram, passam
// pelo arco da peça e saem com crescendo, diminuendo, respiração e morendo.
// Também entrega a forma como sinal (TENS, SECT, ROLE, CLMX, BRTH, END),
// para o resto do patch — e o TEMPO — seguirem: "uma forma só se ouve se
// todas as camadas a seguem" (Rasgo Synth Studio, docs/COMPOSICAO.md).
//
// O arco é uma caminhada pelos papéis INTRO · SUBIDA · CLÍMAX · QUEDA · CODA,
// com a CODA absorvente (rasgo-synth). GRAM escolhe a gramática:
//   0 ARCO (Freytag)  1 ONDAS  2 CRESCIMENTO  3 CALMARIA  4 KISHŌ (a virada
//   é um corte seco, subito piano)
// LEN centra a duração (minutos); cada seção dura uma fração sorteada dela,
// e a caminhada é que decide quando chega a coda.
//
// Ganho de cada camada em dB: −DEPTH·(1 − tensão)·SEG·peso, sempre ≤ 0 dB
// (nunca acima do nível de base: não estoura o MASTER). Peso: camada 1 (o
// essencial) 0,6 · 2: 0,85 · 3: 1 · 4 (o enfeite) 1,25. Rampas de 20 ms.
// GEST dá a chance da respiração antes do clímax (½–2 tempos calados) e do
// subito na virada. MODE: IMPOR (o arco) · SEGUIR (a tensão vem de ENV) ·
// CONTRAPOR (o arco invertido). Com CLK ligado, as trocas de seção caem no
// pulso. LOOP: depois da coda, recomeça (uma peça nova) em 2 s.

namespace rasgo::modular {

class Form final : public Signal {
public:
    enum Role { kIntro = 0, kRise, kClimax, kFall, kCoda };

    Form()
        : Signal(
              {{"in1", PortKind::Audio, ""}, {"in2", PortKind::Audio, ""},
               {"in3", PortKind::Audio, ""}, {"in4", PortKind::Audio, ""},
               {"clock", PortKind::Control, "trig"},
               {"next", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"},
               {"env", PortKind::Control, ""}},
              {{"out1", PortKind::Audio, ""}, {"out2", PortKind::Audio, ""},
               {"out3", PortKind::Audio, ""}, {"out4", PortKind::Audio, ""},
               {"mix", PortKind::Audio, ""},
               {"tension", PortKind::Control, ""},
               {"section", PortKind::Control, "trig"},
               {"role", PortKind::Control, ""},
               {"climax", PortKind::Control, "trig"},
               {"breath", PortKind::Control, "gate"},
               {"end", PortKind::Control, "gate"}},
              {{"grammar", 0.0f, 4.0f, 0.0f, ""},
               {"length", 1.0f, 20.0f, 4.0f, "min"},
               {"depth", 3.0f, 24.0f, 11.0f, "dB"},
               {"gesture", 0.0f, 1.0f, 0.5f, ""},
               {"follow1", 0.0f, 1.0f, 1.0f, ""},
               {"follow2", 0.0f, 1.0f, 1.0f, ""},
               {"follow3", 0.0f, 1.0f, 1.0f, ""},
               {"follow4", 0.0f, 1.0f, 1.0f, ""},
               {"mode", 0.0f, 2.0f, 0.0f, ""},
               {"loop", 0.0f, 1.0f, 1.0f, ""},
               {"seed", 1.0f, 65535.0f, 1.0f, ""}}) {}

    std::string type() const override { return "FORM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "FORM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "arc", "", 2.5f, 6.0f, 76.0f);
        p.add(Widget::Kind::Knob, "GRAM", "grammar", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LEN", "length", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DEPTH", "depth", 37.0f, 30.0f);
        p.add(Widget::Kind::Knob, "GEST", "gesture", 52.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 67.0f, 30.0f);
        for (int k = 0; k < 4; ++k) {
            const std::string n = std::to_string(k + 1);
            p.add(Widget::Kind::Knob, "SEG" + n, "follow" + n, 7.0f + 15.0f * static_cast<float>(k), 50.0f);
        }
        p.add(Widget::Kind::Knob, "SEED", "seed", 67.0f, 50.0f);
        p.add(Widget::Kind::Toggle, "LOOP", "loop", 56.0f, 99.0f);
        for (int k = 0; k < 4; ++k) {
            const std::string n = std::to_string(k + 1);
            p.add(Widget::Kind::Jack, "IN" + n, "in:in" + n, 5.0f + 12.0f * static_cast<float>(k), 70.0f);
            p.add(Widget::Kind::Jack, "O" + n, "out:out" + n, 5.0f + 12.0f * static_cast<float>(k), 86.0f);
        }
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 53.0f, 70.0f);
        p.add(Widget::Kind::Jack, "NEXT", "in:next", 65.0f, 70.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 77.0f, 70.0f);
        p.add(Widget::Kind::Jack, "ENV", "in:env", 53.0f, 86.0f);
        p.add(Widget::Kind::Jack, "MIX", "out:mix", 77.0f, 86.0f);
        p.add(Widget::Kind::Jack, "TENS", "out:tension", 5.0f, 114.0f);
        p.add(Widget::Kind::Jack, "SECT", "out:section", 17.0f, 114.0f);
        p.add(Widget::Kind::Jack, "ROLE", "out:role", 29.0f, 114.0f);
        p.add(Widget::Kind::Jack, "CLMX", "out:climax", 41.0f, 114.0f);
        p.add(Widget::Kind::Jack, "BRTH", "out:breath", 53.0f, 114.0f);
        p.add(Widget::Kind::Jack, "END", "out:end", 65.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        rampCoef_ = 1.0f - std::exp(-1.0f / (0.020f * sr_));
        envCoef_ = 1.0f - std::exp(-1.0f / (0.5f * sr_));
        for (auto& g : gain_) g = 1.0f;
        seededWith_ = -1;
        prevClock_ = prevNext_ = prevReset_ = 0.0f;
        envSm_ = 0.0f;
        clockPeriod_ = 0.5 * sr_;
        sinceClock_ = 0.0;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const int seed = static_cast<int>(parameterValue("seed") + 0.5f);
        if (seed != seededWith_) { seededWith_ = seed; piece_ = 0; startPiece(); }
        const float depth = clampf(parameterValue("depth"), 3.0f, 24.0f);
        const int mode = clampi(static_cast<int>(std::lround(parameterValue("mode"))), 0, 2);
        float follow[4];
        for (int k = 0; k < 4; ++k) follow[k] = clampf(parameterValue(kFollow[k]), 0.0f, 1.0f);
        static constexpr float kWeight[4] = {0.6f, 0.85f, 1.0f, 1.25f};
        const AudioBlock* clk = inputs[4];
        const AudioBlock* next = inputs[5];
        const AudioBlock* rst = inputs[6];
        const AudioBlock* env = inputs[7];

        for (std::size_t f = 0; f < frames; ++f) {
            bool beat = false;
            if (clk) {
                const float c = clk->at(0, f);
                if (prevClock_ < 0.5f && c >= 0.5f) {
                    beat = true;
                    if (sinceClock_ > 1.0) clockPeriod_ = sinceClock_;
                    sinceClock_ = 0.0;
                }
                prevClock_ = c;
                sinceClock_ += 1.0;
            }
            if (rst) {
                const float r = rst->at(0, f);
                if (prevReset_ < 0.5f && r >= 0.5f) startPiece();
                prevReset_ = r;
            }
            bool forceNext = false;
            if (next) {
                const float n = next->at(0, f);
                if (prevNext_ < 0.5f && n >= 0.5f) forceNext = true;
                prevNext_ = n;
            }
            stepArc(beat, clk != nullptr, forceNext);

            // a tensão: o arco, ou a energia de fora, ou o arco invertido
            float tension = arcTension();
            if (env) {
                envSm_ += (clampf(env->at(0, f), 0.0f, 1.0f) - envSm_) * envCoef_;
                if (mode == 1) tension = envSm_;
            }
            if (mode == 2) tension = 1.0f - tension;
            if (ended_) tension = 0.0f;
            tension_ = tension;

            const bool silent = breathing_ || ended_;
            float mix = 0.0f;
            for (int k = 0; k < 4; ++k) {
                const float db = -depth * (1.0f - tension) * follow[k] * kWeight[k];
                const float target = silent ? 0.0f : std::pow(10.0f, std::min(0.0f, db) / 20.0f);
                if (subito_ > 0) gain_[k] = target;     // corte seco (KISHŌ)
                else gain_[k] += (target - gain_[k]) * rampCoef_;
                const AudioBlock* in = inputs[static_cast<std::size_t>(k)];
                float x = in ? in->at(0, f) : 0.0f;
                if (!std::isfinite(x)) x = 0.0f;
                y_[k] = x * gain_[k];
                mix += y_[k];
            }
            if (subito_ > 0) --subito_;
            const float m = std::tanh(mix);
            for (std::size_t c = 0; c < channels; ++c) {
                for (int k = 0; k < 4; ++k) outputs[static_cast<std::size_t>(k)].at(c, f) = y_[k];
                outputs[4].at(c, f) = m;
                outputs[5].at(c, f) = tension_;
                outputs[6].at(c, f) = sectTrig_ > 0 ? 1.0f : 0.0f;
                outputs[7].at(c, f) = static_cast<float>(role_) / 4.0f;
                outputs[8].at(c, f) = climaxTrig_ > 0 ? 1.0f : 0.0f;
                outputs[9].at(c, f) = breathing_ ? 1.0f : 0.0f;
                outputs[10].at(c, f) = ended_ ? 1.0f : 0.0f;
            }
            if (sectTrig_ > 0) --sectTrig_;
            if (climaxTrig_ > 0) --climaxTrig_;
        }
    }

    // para o visor e os testes
    int role() const noexcept { return role_; }
    float tension() const noexcept { return tension_; }
    bool ended() const noexcept { return ended_; }

private:
    static constexpr const char* kFollow[4] = {"follow1", "follow2", "follow3", "follow4"};

    // a tensão-alvo de cada papel e de onde ela parte
    static float targetOf(const int role) noexcept {
        static constexpr float t[5] = {0.15f, 0.75f, 1.0f, 0.3f, 0.0f};
        return t[role];
    }

    void startPiece() noexcept {
        ++piece_;
        rng_ = 0x853C49E6748FEA9BULL ^ (static_cast<std::uint64_t>(seededWith_) * 0x9E3779B97F4A7C15ULL)
             ^ (static_cast<std::uint64_t>(piece_) * 0xD1B54A32D192ED03ULL);
        grammar_ = clampi(static_cast<int>(std::lround(parameterValue("grammar"))), 0, 4);
        lengthSec_ = clampf(parameterValue("length"), 1.0f, 20.0f) * 60.0;
        elapsed_ = 0.0;
        waves_ = 0;
        ended_ = false;
        endHold_ = 0.0;
        breathing_ = false;
        startTension_ = 0.0f;
        enterRole(kIntro);
    }

    // quanto da peça cada papel ocupa, por gramática (frações de LEN)
    double sectionSeconds(const int role) noexcept {
        static constexpr double share[5][5] = {
            {0.15, 0.30, 0.15, 0.20, 0.20},   // ARCO
            {0.10, 0.15, 0.10, 0.12, 0.15},   // ONDAS (repete subida-clímax-queda)
            {0.10, 0.18, 0.12, 0.05, 0.15},   // CRESCIMENTO (várias subidas)
            {0.10, 0.10, 0.18, 0.22, 0.20},   // CALMARIA (agitação, calma, agitação)
            {0.20, 0.25, 0.25, 0.00, 0.30},   // KISHŌ (ki, shō, ten, ketsu)
        };
        const double s = share[grammar_][role];
        return std::max(2.0, lengthSec_ * s * (0.75 + 0.5 * uni()));
    }

    void enterRole(const int role) noexcept {
        startTension_ = tension_;   // cada seção parte de onde a anterior parou
        role_ = role;
        sectionLen_ = sectionSeconds(role);
        inSection_ = 0.0;
        sectTrig_ = static_cast<int>(0.005f * sr_);
        breathPlanned_ = false;
        if (role == kClimax) climaxTrig_ = static_cast<int>(0.005f * sr_);
        // KISHŌ: a virada (ten) é um corte seco — subito piano
        if (grammar_ == 4 && role == kClimax && uni() < 0.5 + 0.5 * gestureP()) {
            subito_ = static_cast<int>(0.002f * sr_) + 1;
            kishoTen_ = true;
        } else {
            kishoTen_ = false;
        }
        // a respiração: um silêncio composto logo antes do clímax
        if (role == kRise && uni() < gestureP()) {
            breathPlanned_ = true;
            breathLen_ = std::min(sectionLen_ * 0.2, (0.5 + 1.5 * uni()) * 0.5);   // ½–2 tempos (s)
        }
    }

    float gestureP() const noexcept { return clampf(parameterValue("gesture"), 0.0f, 1.0f); }

    int nextRole() noexcept {
        const bool late = elapsed_ > lengthSec_ * 0.8;
        switch (grammar_) {
        case 1:   // ONDAS
            if (role_ == kFall) { ++waves_; return (late || waves_ >= 4 || uni() < 0.3) ? kCoda : kRise; }
            break;
        case 2:   // CRESCIMENTO: subidas seguidas antes do clímax
            if (role_ == kRise && !late && uni() < 0.55) return kRise;
            if (role_ == kClimax) return kCoda;
            break;
        case 3:   // CALMARIA entre duas agitações
            if (role_ == kIntro) return kClimax;
            if (role_ == kFall) { ++waves_; return waves_ >= 2 || late ? kCoda : kClimax; }
            break;
        case 4:   // KISHŌ: ki → shō → ten → ketsu
            if (role_ == kClimax) return kCoda;
            break;
        default: break;
        }
        static constexpr int line[5] = {kRise, kClimax, kFall, kCoda, kCoda};
        return line[role_];
    }

    void stepArc(const bool beat, const bool hasClock, const bool forceNext) noexcept {
        const double dt = 1.0 / sr_;
        if (ended_) {
            endHold_ += dt;
            if (parameterValue("loop") >= 0.5f && endHold_ >= 2.0) startPiece();
            return;
        }
        elapsed_ += dt;
        inSection_ += dt;
        // a respiração cai no fim da subida
        breathing_ = breathPlanned_ && role_ == kRise && inSection_ >= sectionLen_ - breathLen_;
        bool due = forceNext || inSection_ >= sectionLen_;
        // com relógio, a troca espera o próximo pulso (até um período)
        if (due && hasClock && !forceNext && !beat && inSection_ < sectionLen_ + clockPeriod_ / sr_)
            due = false;
        if (!due) return;
        if (role_ == kCoda) { ended_ = true; breathing_ = false; return; }
        breathing_ = false;
        enterRole(nextRole());
    }

    // a tensão dentro da seção: do ponto de partida ao alvo do papel (o
    // clímax chega já no alto; a coda desce até o silêncio: morendo)
    float arcTension() const noexcept {
        const float x = static_cast<float>(sectionLen_ > 0.0 ? std::min(1.0, inSection_ / sectionLen_) : 1.0);
        if (role_ == kClimax) return kishoTen_ ? 0.35f : 1.0f;
        if (role_ == kIntro) return targetOf(kIntro) * x;
        const float a = startTension_, b = targetOf(role_);
        const float s = x * x * (3.0f - 2.0f * x);   // suave nas pontas
        return a + (b - a) * s;
    }

    double uni() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<double>((x >> 11) & ((1ULL << 53) - 1)) / static_cast<double>(1ULL << 53);
    }
    static int clampi(const int v, const int lo, const int hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
    static float clampf(const float v, const float lo, const float hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    float sr_ = 48000.0f, rampCoef_ = 0.001f, envCoef_ = 0.0001f;
    std::array<float, 4> gain_{{1.0f, 1.0f, 1.0f, 1.0f}};
    std::array<float, 4> y_{};
    std::uint64_t rng_ = 1;
    int seededWith_ = -1, piece_ = 0, grammar_ = 0, role_ = kIntro, waves_ = 0;
    double lengthSec_ = 240.0, elapsed_ = 0.0, sectionLen_ = 10.0, inSection_ = 0.0;
    double endHold_ = 0.0, breathLen_ = 0.5, clockPeriod_ = 24000.0, sinceClock_ = 0.0;
    float startTension_ = 0.0f, tension_ = 0.0f, envSm_ = 0.0f;
    float prevClock_ = 0.0f, prevNext_ = 0.0f, prevReset_ = 0.0f;
    bool ended_ = false, breathing_ = false, breathPlanned_ = false, kishoTen_ = false;
    int sectTrig_ = 0, climaxTrig_ = 0, subito_ = 0;
};

}  // namespace rasgo::modular
