#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// PLL — oscilador de malha de fase / VCO caçador (Módulo 37)
// ============================================================================
//
// Não é um segundo `OSC` genérico — é um oscilador SOFISTICADO, com itens
// que o `OSC` (Módulo 18) ainda não tem: em vez de sincronizar duro numa
// referência (reset de fase, como `OSC.sync_enable`), este PERSEGUE a
// fase de uma referência externa — um detector de fase compara as duas e
// CURVA a própria taxa pra se aproximar, produzindo a instabilidade de
// "caça" (hunting) característica de um PLL real que não trava de vez.
// Sem referência plugada, toca livre (o oscilador base continua sendo um
// oscilador de verdade, não só um efeito de outro).
//
// Estudado de `ANTITOTEM/src/core/CmosVoice.h` — o estudo "OSC5/4046
// PLL" dentro de `CmosVoice` (código do autor, GPLv3/AGPLv3 —
// compatível; ver `PESQUISA_MODULOS.md §2.3`), generalizado e reescrito
// no idioma header-only zero-dep do Rasgo Modular:
//
//   - **`ratio`** (desvio Rasgo — o Antitotem só faz perseguição 1:1):
//     multiplica a fase-alvo antes de comparar, deixando o PLL travar em
//     sub-harmônicos/harmônicos da referência, não só na mesma nota;
//     faixa larga (0,03–8×) alcança o estudo do "OSC4" do Antitotem
//     (4093/4020, divisor bem fundo) sem precisar de outro módulo — em
//     RATIO baixo o `PLL` trava num sub-múltiplo bem lento da
//     referência, quase um divisor de clock via malha de fase;
//   - **`lock_gain`** exposto ao usuário (o Antitotem usa uma constante
//     fixa `pllLockGain=0,35`) — controla o quão FORTE persegue: baixo
//     "caça" bem devagar, alto trava rápido (mas mais instável se a
//     razão não bate exatamente);
//
// ALCANCE DE CAPTURA (medido, não hipotético): a correção é limitada a
// ±0,9 (segurança — sem isso a taxa podia inverter de sinal ou disparar).
// Isso limita matematicamente o quão longe `FREQ` pode estar do alvo
// (`REF` × `RATIO`) e ainda travar exato: correção necessária =
// `1 − alvo/FREQ`; fora de ±0,9, o `PLL` só se aproxima (caça de
// verdade, nunca trava) — sonda: `FREQ`=150 Hz perseguindo um alvo de
// 300 Hz (correção necessária = −1, fora do alcance) estabiliza perto de
// ~270 Hz, nunca exatamente 300; `FREQ`=220 Hz pro mesmo alvo (correção
// necessária ≈ −0,36, dentro do alcance) trava exato. Não é bug — é o
// mesmo "capture range" limitado de um PLL analógico de verdade.
//   - **`shape`** — morph contínuo seno↔triângulo↔serra↔quadrada (a
//     função `waveform()` do estudo, com PolyBLEP), diferente das 5
//     saídas simultâneas do `OSC` — aqui é 1 saída, forma variável;
//   - **rede de feedback selecionável** (`feedback_type`) — direto,
//     retificado, capacitivo (memória lenta), pulso, "transistor"
//     (`tanh` assimétrico) ou "refluxo" (contra a própria memória) —
//     modula a FASE (não a frequência) antes de ler a forma, como no
//     estudo; era o candidato "rede de feedback selecionável" da
//     pesquisa, adiado antes por redesenhar o `fm_amount` do `OSC` —
//     aqui é novo, sem esse conflito;
//   - **`ring`** — saída de anel: `saída × referência`, o mesmo
//     heterodino que o estudo usa entre OSC5 e OSC A;
//   - **`lock`** (saída de controle, desvio Rasgo) — 0..1, quão perto o
//     detector de fase está de zero; um módulo pode SEGUIR o próprio
//     estado de travamento (mesmo espírito do `gainReductionDb()` do
//     `MASTER`).
//
// Determinístico sem referência (RNG nenhum). Sem alocação em
// `process()`.

namespace rasgo::modular {

class Pll final : public Signal {
public:
    Pll()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"fm", PortKind::Audio, ""},
               {"ref", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""},
               {"ring", PortKind::Audio, ""},
               {"lock", PortKind::Control, ""}},
              {{"freq", 8.0f, 8000.0f, 220.0f, "Hz"},
               {"fine", -100.0f, 100.0f, 0.0f, "cent"},
               {"shape", 0.0f, 3.0f, 1.0f, ""},
               {"ratio", 0.03f, 8.0f, 1.0f, "x"},
               {"lock_gain", 0.0f, 1.0f, 0.35f, ""},
               {"fm_amount", 0.0f, 1.0f, 0.0f, ""},
               {"feedback_amount", 0.0f, 1.0f, 0.0f, ""},
               {"feedback_type", 0.0f, 5.0f, 0.0f, ""}}) {}

    std::string type() const override { return "PLL"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "PLL", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "lock", "", 2.5f, 6.0f, 76.3f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FINE", "fine", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SHAPE", "shape", 37.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATIO", "ratio", 52.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LOCK", "lock_gain", 67.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FM", "fm_amount", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback_amount", 22.0f, 52.0f);
        p.add(Widget::Kind::Knob, "FTYP", "feedback_type", 37.0f, 52.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:pitch", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FM", "in:fm", 17.0f, 96.0f);
        p.add(Widget::Kind::Jack, "REF", "in:ref", 29.0f, 96.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 41.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RING", "out:ring", 53.0f, 96.0f);
        p.add(Widget::Kind::Jack, "LOCK", "out:lock", 65.0f, 96.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        prevOut_ = 0.0f;
        capacitor_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& ring = outputs[1];
        AudioBlock& lock = outputs[2];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float freqParam = parameterValue("freq");
        const float fineParam = parameterValue("fine");
        const float shapeParam = parameterValue("shape");
        const float ratio = parameterValue("ratio");
        const float lockGain = parameterValue("lock_gain");
        const float fmAmount = parameterValue("fm_amount");
        const float fbAmount = parameterValue("feedback_amount");
        const int fbType = clampi(
            static_cast<int>(std::lround(parameterValue("feedback_type"))), 0, 5);
        const float fineRatio = std::exp2(fineParam / 1200.0f);
        const double nyq = 0.45 * sampleRate_;

        const AudioBlock* pitchMod = inputs[0];
        const AudioBlock* fmIn = inputs[1];
        const AudioBlock* refIn = inputs[2];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float f = freqParam * fineRatio;
            if (pitchMod != nullptr)
                f *= std::exp2(pitchMod->at(0, frame));
            f = clampf(f, 2.0f, static_cast<float>(nyq));

            const float fmSample = fmIn ? fmIn->at(0, frame) : 0.0f;
            const float refSample = refIn ? refIn->at(0, frame) : 0.0f;
            const bool hasRef = refIn != nullptr;

            // detector de fase: a referência (esperada como serra
            // bipolar -1..1, ex.: OSC.saw) vira uma fase 0..1; RATIO
            // multiplica antes de comparar, pra travar em razões além
            // de 1:1
            float correction = 0.0f;
            float lockAmount = 0.0f;
            if (hasRef) {
                const float refPhase01 = wrap01f(refSample * 0.5f + 0.5f);
                const float expected = wrap01f(refPhase01 * ratio);
                // convenção: erro = PRÓPRIA fase menos a esperada (igual
                // ao estudo: `phaseE - phaseA`) -- se a própria fase está
                // ADIANTADA (erro > 0), a correção FREIA (rate·(1−erro));
                // se está ATRASADA (erro < 0), ACELERA. Inverter o sinal
                // aqui puxa pro lado errado (achado por teste: a taxa
                // efetiva se afastava da referência em vez de travar).
                float phaseError = static_cast<float>(phase_) - expected;
                phaseError -= std::round(phaseError);  // [-0,5, 0,5)
                correction = clampf(phaseError * lockGain * 3.0f, -0.9f, 0.9f);
                lockAmount = clampf(1.0f - std::fabs(phaseError) * 2.0f, 0.0f, 1.0f);
            }

            const double dpBase =
                (static_cast<double>(f) + fmSample * fmAmount * 4.0 * f)
                / sampleRate_;
            const double dp = dpBase * (1.0 - static_cast<double>(correction));
            const double adt = std::fabs(dp);

            // feedback modula a FASE (não a frequência) só na leitura da
            // forma, sem entrar no acumulador -- mesmo desenho do estudo
            const float fb = feedbackSample(fbType);
            const float phaseArg =
                wrap01f(static_cast<float>(phase_) + fb * fbAmount * 0.11f);
            const float wave = shapeWave(phaseArg, static_cast<float>(adt),
                                         shapeParam);

            prevOut_ = wave;
            capacitor_ += (prevOut_ - capacitor_) * 0.002f;

            const float ringV = clampf(wave * refSample * 2.1f, -1.0f, 1.0f);

            for (std::size_t c = 0; c < channels; ++c) {
                out.at(c, frame) = wave;
                ring.at(c, frame) = ringV;
                lock.at(c, frame) = lockAmount;
            }

            phase_ += dp;
            phase_ -= std::floor(phase_);
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float wrap01f(float t) noexcept {
        t -= std::floor(t);
        return t;
    }

    static float polyBlep(float t, const float dt) noexcept {
        if (dt <= 0.0f || dt >= 0.5f) return 0.0f;
        if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    // morph contínuo seno -> triângulo -> serra -> quadrada, com
    // PolyBLEP na serra/quadrada
    static float shapeWave(float phase, const float dt, const float shape) noexcept {
        phase -= std::floor(phase);
        const float sine = std::sin(6.28318530717958648f * phase);
        const float triangle = 1.0f - 4.0f * std::fabs(phase - 0.5f);
        float saw = 2.0f * phase - 1.0f - polyBlep(phase, dt);
        const float width = 0.5f;
        float square = phase < width ? 1.0f : -1.0f;
        square += polyBlep(phase, dt);
        float edge = phase + 1.0f - width;
        edge -= std::floor(edge);
        square -= polyBlep(edge, dt);
        const float position = clampf(shape, 0.0f, 3.0f);
        const int segment = static_cast<int>(position);
        const float blend = position - static_cast<float>(segment);
        const float next = segment == 0 ? triangle : (segment == 1 ? saw : square);
        const float current = segment == 0 ? sine : (segment == 1 ? triangle : saw);
        return current + (next - current) * blend;
    }

    // rede de feedback -- estudada de `CmosVoice::feedbackSample()`,
    // reduzida a 1 tipo por vez (o estudo soma vários simultâneos; aqui
    // é discreto/selecionável, mais simples de usar e de testar)
    float feedbackSample(const int type) const noexcept {
        switch (type) {
        case 0: return prevOut_;                                       // direto
        case 1: return std::fabs(prevOut_) * 2.0f - 1.0f;              // retificado
        case 2: return capacitor_;                                     // capacitivo
        case 3: return prevOut_ >= 0.0f ? 1.0f : -1.0f;                // pulso
        case 4: return std::tanh(prevOut_ * 2.2f + capacitor_ * 0.42f); // "transistor"
        default: return prevOut_ - capacitor_ * 0.82f;                 // refluxo
        }
    }

    double phase_ = 0.0;
    float prevOut_ = 0.0f;
    float capacitor_ = 0.0f;
};

}  // namespace rasgo::modular
