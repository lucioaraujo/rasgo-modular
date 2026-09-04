#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// SWITCH — Chave sequencial (Módulo 28)
// ============================================================================
//
// O rack roteia por CABO, mas não tinha o ROTEADOR CONTROLADO — a chave que
// cicla entre fontes por clock/CV. `dir` = 0: MUX N→1 (`a`/`b`/`c`/`d` →
// `out`). `dir` = 1: DEMUX 1→N (a entrada `a` vai pra `out`/`out_b`/`out_c`/
// `out_d` conforme o passo; as não-selecionadas deslizam pra 0). O endereço
// avança no `clock` (borda ↑), zera no `reset`, ou é dado direto pela CV
// `addr` (se conectada, ela manda). No ponto de troca faz um crossfade
// (`glide`, só no mux) em vez de corte seco; um slew de 1 ms sempre suaviza
// o degrau. `step` (saída) segue a posição normalizada.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/28_switch.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Doepfer A-151/A-152 (chave sequencial / endereçada por clock ou CV);
//   - 4ms SISM / Erica Pico SEQ (slew no ponto de troca);
//   - multiplexador CD4051 (teoria: 1 de N linhas ligada à comum);
//   - `mode` de leitura do `SEQUENCE` do Rasgo (forward/pingpong/random).

namespace rasgo::modular {

class Switch final : public Signal {
public:
    Switch()
        : Signal(
              {{"a", PortKind::Audio, ""},
               {"b", PortKind::Audio, ""},
               {"c", PortKind::Audio, ""},
               {"d", PortKind::Audio, ""},
               {"clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"},
               {"addr", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""},
               {"step", PortKind::Control, ""},
               {"out_b", PortKind::Audio, ""},
               {"out_c", PortKind::Audio, ""},
               {"out_d", PortKind::Audio, ""}},
              {{"steps", 2.0f, 4.0f, 4.0f, ""},
               {"mode", 0.0f, 3.0f, 0.0f, ""},
               {"dir", 0.0f, 1.0f, 0.0f, ""},
               {"glide", 0.0f, 1.0f, 0.0f, ""},
               {"slew", 0.0f, 1.0f, 0.1f, ""}}) {}

    std::string type() const override { return "SWITCH"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "SWITCH", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "pos", "", 2.5f, 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "STEP", "steps", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 28.0f, 26.0f);
        p.add(Widget::Kind::Knob, "GLID", "glide", 8.0f, 46.0f);
        p.add(Widget::Kind::Knob, "SLEW", "slew", 28.0f, 46.0f);
        p.add(Widget::Kind::Toggle, "DEMUX", "dir", 46.0f, 26.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 7.0f, 66.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 19.0f, 66.0f);
        p.add(Widget::Kind::Jack, "C", "in:c", 31.0f, 66.0f);
        p.add(Widget::Kind::Jack, "D", "in:d", 43.0f, 66.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 86.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 86.0f);
        p.add(Widget::Kind::Jack, "ADR", "in:addr", 31.0f, 86.0f);
        p.add(Widget::Kind::Jack, "OA", "out:out", 7.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OB", "out:out_b", 19.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OC", "out:out_c", 31.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OD", "out:out_d", 43.0f, 110.0f);
        p.add(Widget::Kind::Jack, "STP", "out:step", 55.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        pos_ = 0;
        prevPos_ = 0;
        ppDir_ = 1;
        xfade_ = 0.0f;
        out_[0] = out_[1] = out_[2] = out_[3] = 0.0f;
        prevClock_ = false;
        prevReset_ = false;
        rng_ = 0x9E3779B97F4A7C15ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const int steps = clampi(
            static_cast<int>(std::lround(parameterValue("steps"))), 2, 4);
        const int mode = clampi(
            static_cast<int>(std::lround(parameterValue("mode"))), 0, 3);
        const bool demux = parameterValue("dir") >= 0.5f;
        const float glide = clamp01(parameterValue("glide"));
        const float slew = clamp01(parameterValue("slew"));

        // decaimento do crossfade: glide 0 -> instantâneo; glide 1 -> ~50 ms
        const float xfDecay = 1.0f / (glide * glide * 0.05f * sr_ + 1.0f);
        // slew anti-clique: sempre pelo menos ~1 ms, cresce com `slew`
        const float slewTime = 0.001f + slew * slew * 0.05f;
        const float slewCoef = 1.0f - std::exp(-dt_ / slewTime);

        const AudioBlock* in[4] = {inputs[0], inputs[1], inputs[2], inputs[3]};
        const AudioBlock* clk = inputs[4];
        const AudioBlock* rst = inputs[5];
        const AudioBlock* adr = inputs[6];

        if (pos_ >= steps) pos_ = steps - 1;
        if (prevPos_ >= steps) prevPos_ = steps - 1;

        for (std::size_t f = 0; f < frames; ++f) {
            if (rst != nullptr) {
                const bool hi = rst->at(0, f) >= 0.5f;
                if (hi && !prevReset_) {
                    prevPos_ = pos_;
                    pos_ = 0;
                    ppDir_ = 1;
                    xfade_ = 1.0f;
                }
                prevReset_ = hi;
            }

            if (adr != nullptr) {
                const int want = clampi(
                    static_cast<int>(std::lround(
                        clamp01(adr->at(0, f)) * static_cast<float>(steps - 1))),
                    0, steps - 1);
                if (want != pos_) {
                    prevPos_ = pos_;
                    pos_ = want;
                    xfade_ = 1.0f;
                }
            } else if (clk != nullptr && mode != 3) {
                const bool hi = clk->at(0, f) >= 0.5f;
                if (hi && !prevClock_) {
                    prevPos_ = pos_;
                    advance(mode, steps);
                    xfade_ = 1.0f;
                }
                prevClock_ = hi;
            }

            if (demux) {
                // 1→N: `a` vai pro passo atual; as outras deslizam pra 0
                const float src = in[0] != nullptr ? in[0]->at(0, f) : 0.0f;
                for (int k = 0; k < 4; ++k) {
                    const float tgt = (k == pos_ && k < steps) ? src : 0.0f;
                    out_[k] += (tgt - out_[k]) * slewCoef;
                }
            } else {
                // N→1: mux com crossfade no ponto de troca
                const float sel = in[pos_] != nullptr ? in[pos_]->at(0, f) : 0.0f;
                const float prv =
                    in[prevPos_] != nullptr ? in[prevPos_]->at(0, f) : 0.0f;
                const float mix =
                    xfade_ > 0.0f ? sel + (prv - sel) * xfade_ : sel;
                out_[0] += (mix - out_[0]) * slewCoef;
                for (int k = 1; k < 4; ++k) out_[k] += (0.0f - out_[k]) * slewCoef;
            }
            xfade_ -= xfDecay;
            if (xfade_ < 0.0f) xfade_ = 0.0f;

            const float stepOut =
                static_cast<float>(pos_) / static_cast<float>(steps - 1);
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = out_[0];
                outputs[1].at(c, f) = stepOut;
                outputs[2].at(c, f) = out_[1];
                outputs[3].at(c, f) = out_[2];
                outputs[4].at(c, f) = out_[3];
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
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    std::uint32_t nextRng() noexcept {
        rng_ ^= rng_ >> 12;
        rng_ ^= rng_ << 25;
        rng_ ^= rng_ >> 27;
        return static_cast<std::uint32_t>((rng_ * 0x2545F4914F6CDD1DULL) >> 32);
    }

    void advance(const int mode, const int steps) noexcept {
        switch (mode) {
            case 1:  // pingpong
                pos_ += ppDir_;
                if (pos_ <= 0) { pos_ = 0; ppDir_ = 1; }
                else if (pos_ >= steps - 1) { pos_ = steps - 1; ppDir_ = -1; }
                break;
            case 2: {  // random com anti-repetição de 1
                int n = static_cast<int>(nextRng() % static_cast<std::uint32_t>(steps));
                if (n == pos_) n = (n + 1) % steps;
                pos_ = n;
                break;
            }
            default:  // forward
                pos_ = (pos_ + 1) % steps;
                break;
        }
    }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    int pos_ = 0;
    int prevPos_ = 0;
    int ppDir_ = 1;
    float xfade_ = 0.0f;
    float out_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool prevClock_ = false;
    bool prevReset_ = false;
    std::uint64_t rng_ = 0x9E3779B97F4A7C15ULL;
};

}  // namespace rasgo::modular
