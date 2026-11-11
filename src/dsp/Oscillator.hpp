#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// OSC — oscilador (Módulo 18)
// ============================================================================
//
// A voz "neutra" do subtrativo. O `FUNCTION` (Módulo 1) é gerador de
// função; `MATTER`/`STRING` são vozes de caráter forte. O `OSC` é o
// tijolo clássico: 1 V/oct preciso, VÁRIAS FORMAS ao mesmo tempo
// (seno / triângulo / dente-de-serra / pulso / sub), PWM, hard sync,
// FM linear through-zero e um sub-oscilador embutido.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/18_oscilador.md`.
//
// Fontes ESTUDADAS (algoritmo, não código):
//   - PolyBLEP (Välimäki & Huovilainen; Martin Finke) — banda limitada na
//     descontinuidade do dente-de-serra / pulso;
//   - hard sync clássico de VCO analógico (Roads, CMT) — reiniciar a fase
//     num mestre desloca o formante;
//   - through-zero FM (Buchla 259) — FM LINEAR (soma de Hz, a fase pode
//     reverter) preserva a afinação percebida; a FM de fase não;
//   - sub-oscilador por divisão (Roland Juno / Moog);
//   - `drift` — random-walk lento correlacionado (`AQUORBIUM/biome.odt`,
//     `FUNCTION` do Rasgo).
//
// Desvio Rasgo: `drift` — a afinação "respira" sem nenhuma entrada.
// `drift = 0` → saída determinística.
//
// Anti-aliasing (marco 3): PolyBLEP no wrap da serra e nas duas
// transições do pulso e do sub. O triângulo tem só quebra de 1ª
// derivada e alia bem menos — polyBLAMP fica como 2ª camada (dossiê §6).
//
// `prox` (2026-09-05) — estudado de `ANTITOTEM/src/core/
// CmosVoice.h::oscillatorProximity` (código do autor, GPLv3/AGPLv3 —
// compatível; ver `PESQUISA_MODULOS.md §2.3`): mistura CADA saída com
// uma versão dela mesma passada por um passa-baixa de 1 polo bem
// suave — uma "profundidade"/abafamento sem precisar de outro módulo.
// `prox = 0` é EXATAMENTE a saída crua (no-op determinístico — a
// mesma convenção de `drift = 0`). Desvio: aqui aplica nas 5 saídas
// simultâneas do `OSC` (o Antitotem tem 5 osciladores separados, cada
// um com sua própria PROX; o `OSC` do Rasgo é 1 oscilador com 5
// formas — 1 PROX afeta todas). `orbit` do Antitotem (LFO de pitch
// autônomo) ficou de fora: o `OSC` já tem `drift` fazendo
// exatamente esse papel — duplicar seria redundante, não uma
// contribuição nova.
//
// Lote do oscilador (11/11/2026, recomendações aprovadas pelo autor;
// dossiê §9). Três acréscimos; cada um em 0 deixa o OSC como antes:
//   - PD (phase distortion, só no SIN): a fase é dobrada antes do seno —
//     corre até 0,5 em k = 0,5 − 0,49·PD e devagar depois (técnica de
//     M. Ishibashi, conhecimento público). k nunca fica abaixo de 8
//     incrementos de fase: nota aguda com PD alto perde brilho em vez de
//     chiar.
//   - UNI (unísono, só na SAW): 7 serras desafinadas; os desvios relativos
//     e as curvas de mistura são as MEDIDAS publicadas por A. Szabo
//     ("How to Emulate the Super Saw", 2010) — números, não código; a
//     curva de espalhamento é nossa (0,15·x + 0,85·x³, suave no começo). Fases
//     iniciais fixas; ganho normalizado pela potência (o RMS não sobe).
//   - SUBD (1/2…1/16) + entrada SUBD (CV, 1 V = a faixa inteira): o SUB é a
//     nota dividida por um inteiro — um DIVISOR de verdade, travado na fase
//     do oscilador (conta as voltas, como um flip-flop), então não deriva e
//     a troca do divisor entra na próxima volta, sem estalo. Série
//     sub-harmônica (Schillinger; o Subharmonicon) — conceito. A chave SUB2
//     saiu do painel: `sub_2` ligado (patches antigos) vira SUBD 4.

namespace rasgo::modular {

class Oscillator final : public Signal {
public:
    Oscillator()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"fm", PortKind::Audio, ""},
               {"pwm", PortKind::Control, ""},
               {"sync", PortKind::Control, "trig"},
               {"sub_cv", PortKind::Control, ""}},
              {{"sine", PortKind::Audio, ""},
               {"tri", PortKind::Audio, ""},
               {"saw", PortKind::Audio, ""},
               {"pulse", PortKind::Audio, ""},
               {"sub", PortKind::Audio, ""}},
              {{"freq", 8.0f, 8000.0f, 110.0f, "Hz"},
               {"fine", -100.0f, 100.0f, 0.0f, "cent"},
               {"pw", 0.02f, 0.98f, 0.5f, ""},
               {"fm_amount", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""},
               {"sub_2", 0.0f, 1.0f, 0.0f, ""},
               {"sync_enable", 0.0f, 1.0f, 0.0f, ""},
               {"prox", 0.0f, 1.0f, 0.0f, ""},
               {"pd", 0.0f, 1.0f, 0.0f, ""},
               {"unison", 0.0f, 1.0f, 0.0f, ""},
               {"sub_div", 2.0f, 16.0f, 2.0f, ""}}) {}

    std::string type() const override { return "OSC"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "OSC", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "wave", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FINE", "fine", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "PW", "pw", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FM", "fm_amount", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 7.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "SYNC", "sync_enable", 35.0f, 54.0f);
        p.add(Widget::Kind::Knob, "PROX", "prox", 49.0f, 52.0f);
        p.add(Widget::Kind::Knob, "PD", "pd", 7.0f, 72.0f);
        p.add(Widget::Kind::Knob, "UNI", "unison", 21.0f, 72.0f);
        p.add(Widget::Kind::Knob, "SUBD", "sub_div", 35.0f, 72.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:pitch", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FM", "in:fm", 17.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PWM", "in:pwm", 29.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SYNC", "in:sync", 41.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SUBD", "in:sub_cv", 53.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SIN", "out:sine", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "TRI", "out:tri", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SAW", "out:saw", 29.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PLS", "out:pulse", 41.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SUB", "out:sub", 53.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        subPhase_ = 0.0;
        prevSync_ = 0.0f;
        driftState_ = 0.0f;
        driftCounter_ = 0;
        rngState_ = 0x9E3779B97F4A7C15ULL;
        smoothFreq_ = parameterValue("freq");
        smoothPw_ = parameterValue("pw");
        driftInterval_ =
            static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 40.0f));
        paramCoeff_ = std::exp(-1.0f / (0.005f * std::max(1.0f, sampleRate)));
        sineLP_ = triLP_ = sawLP_ = pulseLP_ = subLP_ = 0.0f;
        subCount_ = 0;
        subDiv_ = 2;
        for (std::size_t v = 0; v < kUni; ++v) uniPhase_[v] = kUniStart[v];
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& sineOut = outputs[0];
        AudioBlock& triOut = outputs[1];
        AudioBlock& sawOut = outputs[2];
        AudioBlock& pulseOut = outputs[3];
        AudioBlock& subOut = outputs[4];
        const std::size_t frames = sineOut.frames();
        const std::size_t channels = sineOut.channels();

        const float freqParam = parameterValue("freq");
        const float fineParam = parameterValue("fine");
        const float pwParam = parameterValue("pw");
        const float fmAmount = parameterValue("fm_amount");
        const float drift = parameterValue("drift");
        const float prox = parameterValue("prox");
        // patches de antes: a chave SUB2 ligada vira SUBD 4 (uma vez)
        if (parameterValue("sub_2") >= 0.5f) {
            setParameter("sub_div", 4.0f);
            setParameter("sub_2", 0.0f);
        }
        const float pdAmt = clampf(parameterValue("pd"), 0.0f, 1.0f);
        const float uni = clampf(parameterValue("unison"), 0.0f, 1.0f);
        const float subDivParam = parameterValue("sub_div");
        const AudioBlock* subCv = inputs.size() > 4 ? inputs[4] : nullptr;
        // UNI: espalhamento e mistura (curvas de Szabo para a mistura)
        const float spread = 0.15f * uni + 0.85f * uni * uni * uni;
        const float mixC = -0.55366f * uni + 0.99785f;
        const float mixS = -0.73764f * uni * uni + 1.2841f * uni + 0.044372f;
        const float uniNorm = 1.0f / std::sqrt(mixC * mixC + 6.0f * mixS * mixS);
        const bool syncEnabled = parameterValue("sync_enable") >= 0.5f;
        const float driftStep = 0.0006f * drift * drift;
        const float fineRatio = std::exp2(fineParam / 1200.0f);
        const double nyq = 0.45 * sampleRate_;

        const AudioBlock* pitchMod = inputs[0];
        const AudioBlock* fmIn = inputs[1];
        const AudioBlock* pwmIn = inputs[2];
        const AudioBlock* sync = inputs[3];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            smoothFreq_ = freqParam + paramCoeff_ * (smoothFreq_ - freqParam);
            smoothPw_ = pwParam + paramCoeff_ * (smoothPw_ - pwParam);

            // drift: passo aleatório lento e correlacionado na afinação
            if (++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftState_ += whiteNoise() * driftStep;
                driftState_ = clampf(driftState_, -0.045f, 0.045f);
            }

            float f = smoothFreq_ * fineRatio;
            if (pitchMod != nullptr)
                f *= std::exp2(pitchMod->at(0, frame));
            f *= std::exp2(driftState_);
            f = clampf(f, 2.0f, static_cast<float>(nyq));

            // FM linear through-zero: soma de Hz; `dp` pode ficar negativo
            const float fmSample = fmIn ? fmIn->at(0, frame) : 0.0f;
            const double dp =
                (static_cast<double>(f) + fmSample * fmAmount * 4.0 * f)
                / sampleRate_;
            const double adt = std::fabs(dp);

            // hard sync: reinicia as fases na borda de subida
            if (syncEnabled && sync != nullptr) {
                const float trig = sync->at(0, frame);
                if (prevSync_ < 0.5f && trig >= 0.5f) {
                    phase_ = 0.0;
                    subPhase_ = 0.0;
                    subCount_ = 0;
                    for (std::size_t v = 0; v < kUni; ++v) uniPhase_[v] = kUniStart[v];
                }
                prevSync_ = trig;
            }

            float pw = smoothPw_;
            if (pwmIn != nullptr) pw += pwmIn->at(0, frame);
            pw = clampf(pw, 0.02f, 0.98f);

            const float p = static_cast<float>(phase_);
            const float sp = static_cast<float>(subPhase_);

            // --- formas ---
            float sine;
            if (pdAmt <= 0.0f) {
                sine = std::sin(6.28318530717958648f * p);
            } else {
                const float kMin = std::min(0.5f, 8.0f * static_cast<float>(adt));
                const float k = std::max(kMin, 0.5f - 0.49f * pdAmt);
                const float w = p < k ? 0.5f * p / k
                                      : 0.5f + 0.5f * (p - k) / (1.0f - k);
                sine = std::sin(6.28318530717958648f * w);
            }
            const float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);

            float saw = 2.0f * p - 1.0f;
            saw -= static_cast<float>(polyBlep(phase_, adt));
            if (uni > 0.0f) {
                float sides = 0.0f;
                for (std::size_t v = 0; v < kUni; ++v) {
                    if (v == kUniCentre) continue;
                    const double ratio = 1.0 + static_cast<double>(kUniOffset[v] * spread);
                    const double vdt = adt * ratio;
                    float s = 2.0f * static_cast<float>(uniPhase_[v]) - 1.0f;
                    s -= static_cast<float>(polyBlep(uniPhase_[v], vdt));
                    sides += s;
                    uniPhase_[v] += dp * ratio;
                    uniPhase_[v] -= std::floor(uniPhase_[v]);
                }
                saw = (mixC * saw + mixS * sides) * uniNorm;
            }

            float pulse = (p < pw) ? 1.0f : -1.0f;
            pulse += static_cast<float>(polyBlep(phase_, adt));
            pulse -= static_cast<float>(
                polyBlep(wrap01(phase_ + 1.0 - static_cast<double>(pw)), adt));

            const double sdt = adt / static_cast<double>(subDiv_);
            float sub = (sp < 0.5f) ? 1.0f : -1.0f;
            sub += static_cast<float>(polyBlep(subPhase_, sdt));
            sub -= static_cast<float>(polyBlep(wrap01(subPhase_ + 0.5), sdt));

            // PROX: mistura com uma versão passada por passa-baixa de
            // 1 polo bem suave (0,06 -- a mesma constante do estudo).
            // prox=0 é EXATAMENTE a saída crua, determinístico (a
            // mesma convenção de drift=0).
            sineLP_ += (sine - sineLP_) * 0.06f;
            triLP_ += (tri - triLP_) * 0.06f;
            sawLP_ += (saw - sawLP_) * 0.06f;
            pulseLP_ += (pulse - pulseLP_) * 0.06f;
            subLP_ += (sub - subLP_) * 0.06f;
            const float sineV = sine * (1.0f - prox) + sineLP_ * prox;
            const float triV = tri * (1.0f - prox) + triLP_ * prox;
            const float sawV = saw * (1.0f - prox) + sawLP_ * prox;
            const float pulseV = pulse * (1.0f - prox) + pulseLP_ * prox;
            const float subV = sub * (1.0f - prox) + subLP_ * prox;

            for (std::size_t channel = 0; channel < channels; ++channel) {
                sineOut.at(channel, frame) = sineV;
                triOut.at(channel, frame) = triV;
                sawOut.at(channel, frame) = sawV;
                pulseOut.at(channel, frame) = pulseV;
                subOut.at(channel, frame) = subV;
            }

            phase_ += dp;
            // SUB: divisor travado na fase — conta as voltas do oscilador
            // (para a frente e, na FM through-zero, para trás)
            const double wraps = std::floor(phase_);
            phase_ -= wraps;                       // wrap [0,1), qualquer sinal
            if (wraps != 0.0) {
                subCount_ += static_cast<int>(wraps);
                // o divisor só muda na volta: sem estalo
                float dv = subDivParam;
                if (subCv != nullptr) dv += 15.0f * subCv->at(0, frame);
                subDiv_ = static_cast<int>(std::lround(clampf(dv, 2.0f, 16.0f)));
                subCount_ = ((subCount_ % subDiv_) + subDiv_) % subDiv_;
            }
            subPhase_ = (static_cast<double>(subCount_) + phase_)
                      / static_cast<double>(subDiv_);
        }
    }

private:
    static float clampf(const float value, const float lo, const float hi) noexcept {
        return value < lo ? lo : (value > hi ? hi : value);
    }
    static double wrap01(double t) noexcept { return t - std::floor(t); }

    // PolyBLEP: residual pra corrigir o salto do dente-de-serra / pulso na
    // descontinuidade (Martin Finke / Välimäki). `t` = fase [0,1), `dt` =
    // |incremento|. Coeficiente 1 = salto de 2 (formas normalizadas).
    static double polyBlep(double t, const double dt) noexcept {
        if (dt <= 0.0 || dt >= 0.5)
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
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    // UNI: 7 vozes; desvios relativos medidos por Szabo (2010)
    static constexpr std::size_t kUni = 7, kUniCentre = 3;
    static constexpr float kUniOffset[kUni] = {-0.11002313f, -0.06288439f, -0.01952356f, 0.0f,
                                               0.01991221f, 0.06216538f, 0.10745242f};
    static constexpr double kUniStart[kUni] = {0.13, 0.71, 0.42, 0.0, 0.88, 0.27, 0.56};
    double uniPhase_[kUni] = {0.13, 0.71, 0.42, 0.0, 0.88, 0.27, 0.56};
    int subCount_ = 0;
    int subDiv_ = 2;

    double phase_ = 0.0;
    double subPhase_ = 0.0;
    float prevSync_ = 0.0f;
    float smoothFreq_ = 110.0f;
    float smoothPw_ = 0.5f;
    float paramCoeff_ = 0.0f;

    float driftState_ = 0.0f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 1200;
    std::uint64_t rngState_ = 0x9E3779B97F4A7C15ULL;

    float sineLP_ = 0.0f, triLP_ = 0.0f, sawLP_ = 0.0f, pulseLP_ = 0.0f,
        subLP_ = 0.0f;
};

}  // namespace rasgo::modular
