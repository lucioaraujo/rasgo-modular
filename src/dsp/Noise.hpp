#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// NOISE — ruído e aleatório (Módulo 19)
// ============================================================================
//
// A fonte de ACASO CONTÍNUO. `DECISION` decide eventos discretos; `NOISE`
// dá o piso de ruído (percussão, vento, textura) E as fontes de
// modulação aleatória que todo patch modular usa: branco / rosa / brown,
// sample-and-hold e uma tensão que passeia (smooth random).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/19_ruido.md`.
//
// Fontes ESTUDADAS (algoritmo, não código):
//   - Paul Kellet "economy" pink noise filter (music-dsp, domínio
//     público) — 7 filtros de 1 polo somados ≈ −3 dB/oitava;
//   - sample-and-hold clássico (Buchla 265/266, Doepfer A-118);
//   - Buchla 266 "smooth random" — tensão que desliza entre alvos, não
//     degraus;
//   - `DECISION` do Rasgo (`shape`) — uniforme→sino pela média de N
//     uniformes = acaso estruturado.
//
// Desvio Rasgo: `spread` leva a distribuição do S&H / smooth de uniforme
// a sino. Determinístico: dois streams xorshift semeados em prepare().

namespace rasgo::modular {

class Noise final : public Signal {
public:
    Noise()
        : Signal(
              {{"trigger", PortKind::Control, "trig"},
               {"in", PortKind::Audio, ""}},
              {{"white", PortKind::Audio, ""},
               {"pink", PortKind::Audio, ""},
               {"brown", PortKind::Audio, ""},
               {"sh", PortKind::Audio, ""},
               {"smooth", PortKind::Audio, ""}},
              {{"rate", 0.01f, 2000.0f, 8.0f, "Hz"},
               {"slew", 0.0f, 1.0f, 0.3f, ""},
               {"spread", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "NOISE"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;   // 5 saídas -> precisa de largura pra espaçar os jacks
        p.add(Widget::Kind::Label, "NOISE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "noise", "", 2.5f, 8.0f, 55.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SLEW", "slew", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 38.0f, 30.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trigger", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 17.0f, 96.0f);
        p.add(Widget::Kind::Jack, "WHT", "out:white", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PNK", "out:pink", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "BRN", "out:brown", 29.0f, 116.0f);
        p.add(Widget::Kind::Jack, "S&H", "out:sh", 41.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SMTH", "out:smooth", 53.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        for (float& b : b_) b = 0.0f;
        brown_ = 0.0f;
        held_ = 0.0f;
        smooth_ = 0.0f;
        smoothTarget_ = 0.0f;
        phase_ = 0.0;
        prevTrig_ = 0.0f;
        rngWhite_ = 0x243F6A8885A308D3ULL;
        rngSH_ = 0xB7E151628AED2A6BULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& white = outputs[0];
        AudioBlock& pink = outputs[1];
        AudioBlock& brownOut = outputs[2];
        AudioBlock& shOut = outputs[3];
        AudioBlock& smoothOut = outputs[4];
        const std::size_t frames = white.frames();
        const std::size_t channels = white.channels();

        const float rate = parameterValue("rate");
        const float slew = parameterValue("slew");
        const float spread = parameterValue("spread");
        // slewCoef: slew=0 -> instantâneo (1.0); slew=1 -> ~2 s de glide
        const float slewSec = slew * slew * 2.0f;
        const float slewCoef = slewSec <= 0.0f
            ? 1.0f
            : 1.0f - std::exp(-1.0f / std::max(1.0f, slewSec * sampleRate_));
        const double dp = static_cast<double>(rate) / sampleRate_;

        const AudioBlock* trig = inputs[0];
        const AudioBlock* extIn = inputs[1];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float w = whiteSample();

            // rosa (Paul Kellet economy)
            b_[0] = 0.99886f * b_[0] + w * 0.0555179f;
            b_[1] = 0.99332f * b_[1] + w * 0.0750759f;
            b_[2] = 0.96900f * b_[2] + w * 0.1538520f;
            b_[3] = 0.86650f * b_[3] + w * 0.3104856f;
            b_[4] = 0.55000f * b_[4] + w * 0.5329522f;
            b_[5] = -0.7616f * b_[5] - w * 0.0168980f;
            const float pinkV =
                (b_[0] + b_[1] + b_[2] + b_[3] + b_[4] + b_[5] + b_[6]
                 + w * 0.5362f) * 0.11f;
            b_[6] = w * 0.115926f;

            // brown (passeio com vazamento)
            brown_ = 0.99f * brown_ + w * 0.05f;
            const float brownV = clampf(brown_ * 3.8f, -1.0f, 1.0f);

            // pulso: trigger externo OU relógio interno
            bool tick = false;
            if (trig != nullptr) {
                const float t = trig->at(0, frame);
                if (prevTrig_ < 0.5f && t >= 0.5f) tick = true;
                prevTrig_ = t;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) { phase_ -= 1.0; tick = true; }
            }
            if (tick) {
                const float d = shapedDraw(spread);
                held_ = (extIn != nullptr) ? extIn->at(0, frame) : d;
                smoothTarget_ = d;
            }
            smooth_ += (smoothTarget_ - smooth_) * slewCoef;

            for (std::size_t c = 0; c < channels; ++c) {
                white.at(c, frame) = w;
                pink.at(c, frame) = clampf(pinkV, -1.0f, 1.0f);
                brownOut.at(c, frame) = brownV;
                shOut.at(c, frame) = held_;
                smoothOut.at(c, frame) = smooth_;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float u01(std::uint64_t& s) noexcept {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;  // [-1,1)
    }
    float whiteSample() noexcept { return u01(rngWhite_); }
    // uniforme <-> sino (média de 4) conforme `spread`
    float shapedDraw(const float spread) noexcept {
        const float uni = u01(rngSH_);
        if (spread <= 1.0e-4f) return uni;
        const float bell = 0.25f * (u01(rngSH_) + u01(rngSH_)
                                    + u01(rngSH_) + u01(rngSH_));
        return uni + (bell - uni) * spread;
    }

    float b_[7] = {};
    float brown_ = 0.0f;
    float held_ = 0.0f;
    float smooth_ = 0.0f, smoothTarget_ = 0.0f;
    double phase_ = 0.0;
    float prevTrig_ = 0.0f;
    std::uint64_t rngWhite_ = 0x243F6A8885A308D3ULL;
    std::uint64_t rngSH_ = 0xB7E151628AED2A6BULL;
};

}  // namespace rasgo::modular
