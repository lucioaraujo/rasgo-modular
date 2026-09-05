#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// FUNCTION — gerador de função (Módulo 1)
// ============================================================================
//
// Uma rampa que é ENVELOPE, LFO ou OSCILADOR conforme a taxa. Mesma
// matemática, escalas de tempo diferentes - função emergente (Stages),
// temporalidade orgânica (Tides). Fonte de áudio E de modulação.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/01_gerador_de_funcao.md`.
//
// Fontes ESTUDADAS (algoritmo, não código):
//   - Mutable Tides / Stages (STM32F, MIT) - gerador de inclinação
//     unificado, uni/bi, forma deformável;
//   - PolyBLEP (Välimäki & Huovilainen; Martin Finke) - banda limitada na
//     descontinuidade do dente-de-serra quando a taxa entra em áudio;
//   - EMW VC Wavetable LFO (hardware do autor) - LFO com forma controlada
//     e faixa que cruza pra áudio;
//   - random-walk correlacionado (`AQUORBIUM/biome.odt`) - o `drift`.
//
// Desvio Rasgo: `drift` - passo aleatório lento e correlacionado na taxa
// efetiva (o tempo "respira" sem nenhuma entrada). `drift = 0` -> saída
// determinística.
//
// Anti-aliasing (marco 1): a parte DENTE-DE-SERRA da forma (quando `slope`
// se afasta de 0,5) recebe PolyBLEP no wrap. A parte TRIÂNGULO tem só
// quebra de 1ª derivada e alia bem menos - polyBLAMP nos cantos fica como
// 2ª camada (anotado no dossiê §3/§7).

namespace rasgo::modular {

class FunctionGenerator final : public Signal {
public:
    FunctionGenerator()
        : Signal(
              {{"rate_mod", PortKind::Control, "v/oct"},
               {"slope_mod", PortKind::Control, ""},
               {"sync", PortKind::Control, "trig"}},
              {{"uni", PortKind::Audio, ""}, {"bi", PortKind::Audio, ""}},
              {{"rate", 0.01f, 12000.0f, 2.0f, "Hz"},
               {"slope", 0.0f, 1.0f, 0.5f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""},
               {"sync_enable", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "FUNCTION"; }

    // Layout: 12 HP (1 HP = 4 unidades de grade em x -> 48 unidades).
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        // 8 -> 10 HP (passe de ergonomia 2026-09-06): "SLOPE"/"SYNC" (5 e
        // 4 letras) não cabiam a 12 mm num painel de 40 mm. Display cheio,
        // knobs 2 col centradas, entradas numa fileira, saídas na de baixo.
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "FUNCTION", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "shape", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 11.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SLOPE", "slope", 31.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 11.0f, 50.0f);
        p.add(Widget::Kind::Toggle, "SYNC", "sync_enable", 32.0f, 52.0f);
        p.add(Widget::Kind::Jack, "RATE", "in:rate_mod", 8.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SLOPE", "in:slope_mod", 22.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SYNC", "in:sync", 38.0f, 96.0f);
        p.add(Widget::Kind::Jack, "UNI", "out:uni", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "BI", "out:bi", 21.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        driftState_ = 0.0f;
        driftCounter_ = 0;
        rngState_ = 0x2545F4914F6CDD1DULL;
        smoothRate_ = parameterValue("rate");
        smoothSlope_ = parameterValue("slope");
        prevSync_ = 0.0f;
        driftInterval_ = static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 20.0f));
        // constante de suavização de parâmetro ~5 ms
        paramCoeff_ = std::exp(-1.0f / (0.005f * std::max(1.0f, sampleRate)));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& uni = outputs[0];
        AudioBlock& bi = outputs[1];
        const std::size_t frames = uni.frames();
        const std::size_t channels = uni.channels();

        const float rateParam = parameterValue("rate");
        const float slopeParam = parameterValue("slope");
        const float drift = parameterValue("drift");
        const bool syncEnabled = parameterValue("sync_enable") >= 0.5f;
        const float driftStep = 0.0009f * drift * drift;  // ~±½ oitava no máx

        const AudioBlock* rateMod = inputs[0];
        const AudioBlock* slopeMod = inputs[1];
        const AudioBlock* sync = inputs[2];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            // suavização de parâmetro (entre blocos e dentro do bloco)
            smoothRate_ = rateParam + paramCoeff_ * (smoothRate_ - rateParam);
            smoothSlope_ = slopeParam + paramCoeff_ * (smoothSlope_ - slopeParam);

            // modulação
            float rate = smoothRate_;
            if (rateMod != nullptr)
                rate *= std::exp2(rateMod->at(0, frame));  // 1 V/oct
            float slope = smoothSlope_;
            if (slopeMod != nullptr)
                slope += slopeMod->at(0, frame);
            slope = clampf(slope, 0.0f, 1.0f);

            // drift: passo aleatório lento e correlacionado
            if (++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftState_ += (whiteNoise() * driftStep);
                driftState_ = clampf(driftState_, -0.5f, 0.5f);
            }
            rate *= std::exp2(driftState_);
            rate = clampf(rate, 0.001f, 20000.0f);

            // sync: reinicia a fase na borda de subida
            if (syncEnabled && sync != nullptr) {
                const float trig = sync->at(0, frame);
                if (prevSync_ < 0.5f && trig >= 0.5f)
                    phase_ = 0.0;
                prevSync_ = trig;
            }

            const double dp = static_cast<double>(rate) / sampleRate_;
            const float k = clampf(slope, 0.001f, 0.999f);
            const float p = static_cast<float>(phase_);

            // --- forma ---
            // triângulo assimétrico (contínuo em valor): sobe 0->1 em [0,k),
            // desce 1->0 em [k,1). Unipolar.
            const float tri = (p < k) ? (p / k) : ((1.0f - p) / (1.0f - k));

            // dente-de-serra na direção de `slope` (com salto de valor):
            // slope>0.5 -> serra que sobe; slope<0.5 -> serra que desce.
            const float sawAmount = 2.0f * std::fabs(slope - 0.5f);  // 0 no triângulo
            const float sawDir = (slope >= 0.5f) ? 1.0f : -1.0f;
            float saw = sawDir * (2.0f * p - 1.0f);
            // PolyBLEP no wrap (salto de -2*sawDir)
            saw -= sawDir * static_cast<float>(polyBlep(phase_, dp));

            // mistura: triângulo (bipolar) <-> serra
            const float biTri = 2.0f * tri - 1.0f;
            const float biValue = (1.0f - sawAmount) * biTri + sawAmount * saw;
            const float uniValue = 0.5f * (biValue + 1.0f);

            for (std::size_t channel = 0; channel < channels; ++channel) {
                uni.at(channel, frame) = uniValue;
                bi.at(channel, frame) = biValue;
            }

            phase_ += dp;
            if (phase_ >= 1.0)
                phase_ -= 1.0;
        }
    }

private:
    static float clampf(const float value, const float lo, const float hi) noexcept {
        return value < lo ? lo : (value > hi ? hi : value);
    }

    // PolyBLEP: residual pra corrigir o salto de +1->-1 do dente-de-serra
    // no wrap (Martin Finke / Välimäki). `t` = fase [0,1), `dt` = incremento.
    static double polyBlep(double t, const double dt) noexcept {
        if (dt <= 0.0)
            return 0.0;
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0;
        }
        if (t > 1.0 - dt) {
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }
        return 0.0;
    }

    // xorshift64* -> ruído branco em [-1,1)
    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32)) / 2147483648.0f;
    }

    double phase_ = 0.0;
    float smoothRate_ = 2.0f;
    float smoothSlope_ = 0.5f;
    float paramCoeff_ = 0.0f;
    float prevSync_ = 0.0f;

    float driftState_ = 0.0f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 2400;
    std::uint64_t rngState_ = 0x2545F4914F6CDD1DULL;
};

}  // namespace rasgo::modular
