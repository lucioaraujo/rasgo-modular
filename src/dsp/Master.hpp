#pragma once
#include <vector>
#include <array>

#include "core/SignalGraph.hpp"
#include "dsp/OutputStage.hpp"

#include <cmath>

// ============================================================================
// MASTER — barramento de saída estéreo (Módulo 17)
// ============================================================================
//
// O último nó antes das caixas. Largura estéreo (mid/side), soma mono,
// bloqueio de DC, limitador suave de segurança e ganho de master. Saída
// estéreo + uma saída de controle `level` (pico com decaimento) pra um
// VU.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/17_master.md`.
//
// Fontes ESTUDADAS (prática de estúdio / DSP público):
//   - matriz mid/side: mid = (L+R)/2, side = (L−R)/2; `width` escala o
//     side (0 = mono, 1 = normal, 2 = largo);
//   - bloqueio de DC: passa-alta de 1 polo em ~5 Hz (`y = x − x₁ + R·y₁`);
//   - limitador suave: `tanh` com makeup, transparente abaixo do teto.
//
// Determinístico (sem RNG). Sem alocação.

namespace rasgo::modular {

class Master final : public Signal {
public:
    Master()
        : Signal(
              {{"in", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""},
               {"level", PortKind::Control, ""}},
              {{"gain", -60.0f, 12.0f, -24.0f, "dB"},  // 50% do slider (n=(v-lo)/(hi-lo)); pedido do autor 2026-09-05 (era 30% = −38,4)
               {"width", 0.0f, 2.0f, 1.0f, ""},
               {"mono", 0.0f, 1.0f, 0.0f, ""},
               {"dc_block", 0.0f, 1.0f, 1.0f, ""},
               {"limit", 0.0f, 1.0f, 1.0f, ""},
               // silêncio da saída — rampa de ~8 ms pra não estalar
               // (pedido do autor 2026-09-05). 0 = toca · 1 = mudo.
               {"mute", 0.0f, 1.0f, 0.0f, ""},
               // governador de corpo (ver `OutputStage.hpp §4`): só age em
               // agudo alto + sustentado + concentrado em ~2,5–5 kHz. 0 = off.
               {"body_guard", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "MASTER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "MASTER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "vu", "", 2.5f, 6.0f, 35.6f);
        p.add(Widget::Kind::Slider, "GAIN", "gain", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "WIDTH", "width", 26.0f, 28.0f);
        p.add(Widget::Kind::Toggle, "MUTE", "mute", 24.0f, 42.0f);
        p.add(Widget::Kind::Toggle, "MONO", "mono", 24.0f, 54.0f);
        p.add(Widget::Kind::Toggle, "DC", "dc_block", 24.0f, 66.0f);
        p.add(Widget::Kind::Toggle, "LIMIT", "limit", 24.0f, 78.0f);
        p.add(Widget::Kind::Knob, "BODY", "body_guard", 9.0f, 78.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 104.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 17.0f, 104.0f);
        p.add(Widget::Kind::Jack, "VU", "out:level", 29.0f, 104.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        peak_ = 0.0f;
        muteGain_ = parameterValue("mute") >= 0.5f ? 0.0f : 1.0f;
        // rampa de ~8 ms (1 polo) — silêncio sem estalo
        muteCoef_ = std::exp(-1.0f / (0.008f * std::max(1.0f, sampleRate)));
        // GAIN e WIDTH também precisam de rampa. Eram lidos uma vez por
        // bloco e aplicados como constante: trocar o valor entre blocos
        // punha um DEGRAU na onda na fronteira — clique audível. Não é
        // caso raro: o VARIA mexe nos parâmetros a cada 33 ms e o arrasto
        // de um fader gera uma troca por evento de mouse. Achado pela
        // bateria de `tests/test_output_excellence.cpp` (§5 do
        // `SAIDA_AUDIO_COMUM.md`: "automação rápida de ganho, pan, width,
        // mute e bypass sem clique"), 2026-09-15.
        //
        // 8 ms, o mesmo do MUTE: rápido o bastante pra o gesto parecer
        // imediato, lento o bastante pra não ser degrau.
        paramCoef_ = muteCoef_;
        gainSm_ = dbToGain(parameterValue("gain"));
        widthSm_ = parameterValue("width");
        // proteção de saída de excelência (look-ahead + teto suave) —
        // padrão RASGO, ver `src/dsp/OutputStage.hpp`
        out_.prepare(sampleRate, -1.0f /*teto dBFS*/, 3.0f /*look-ahead ms*/,
                     120.0f /*release ms*/);
        // pico decai ~300 ms
        peakDecay_ = std::exp(-1.0f / (0.3f * std::max(1.0f, sampleRate)));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& level = outputs[1];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();
        preTapFrames_ = std::min(frames, preTap_[0].size());

        const AudioBlock* in = inputs[0];
        const float gainTarget = dbToGain(parameterValue("gain"));
        const float widthTarget = parameterValue("width");
        const bool mono = parameterValue("mono") >= 0.5f;
        const bool dcBlock = parameterValue("dc_block") >= 0.5f;
        const bool limit = parameterValue("limit") >= 0.5f;
        const float bodyGuard = parameterValue("body_guard");
        const float muteTarget = parameterValue("mute") >= 0.5f ? 0.0f : 1.0f;
        const std::size_t inCh = in ? in->channels() : 0;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float l = in ? in->at(0, frame) : 0.0f;
            float r = (in && inCh >= 2) ? in->at(1, frame) : l;

            // rampa por amostra de GAIN e WIDTH (ver `prepare`)
            gainSm_  += (gainTarget  - gainSm_)  * (1.0f - paramCoef_);
            widthSm_ += (widthTarget - widthSm_) * (1.0f - paramCoef_);

            // largura mid/side
            const float mid = 0.5f * (l + r);
            float side = 0.5f * (l - r) * widthSm_;
            l = mid + side;
            r = mid - side;
            if (mono) { l = mid; r = mid; }

            l *= gainSm_;
            r *= gainSm_;

            // MUTE — rampa suave no fader, antes da proteção de saída
            muteGain_ += (muteTarget - muteGain_) * (1.0f - muteCoef_);
            l *= muteGain_;
            r *= muteGain_;

            // TAP `pre-safety` — o par DEPOIS do master criativo (ganho,
            // largura, mono, mute) e ANTES da proteção de saída.
            // `SAIDA_AUDIO_COMUM.md §3` pede taps de gravação nomeados
            // (`pre-master-criativo`, `pre-safety`, `post-safety`) e o §4
            // diz por quê: gravar só depois do limitador faz o limitador
            // ESCONDER a dinâmica que se queria examinar. Aqui é só uma
            // cópia — não muda uma amostra do que sai.
            if (frame < preTap_[0].size()) {
                preTap_[0][frame] = l;
                preTap_[1][frame] = r;
            }

            // proteção de saída: finitude + (DC) + guarda ultrassônica +
            // governador de corpo + limitador look-ahead + teto suave
            out_.process(l, r, dcBlock, limit, bodyGuard);

            const float mag = std::fabs(l) > std::fabs(r)
                ? std::fabs(l) : std::fabs(r);
            peak_ = mag > peak_ ? mag : peak_ * peakDecay_;

            if (channels >= 2) {
                out.at(0, frame) = l;
                out.at(1, frame) = r;
                for (std::size_t ch = 2; ch < channels; ++ch)
                    out.at(ch, frame) = 0.5f * (l + r);
            } else {
                out.at(0, frame) = 0.5f * (l + r);
            }
            for (std::size_t ch = 0; ch < level.channels(); ++ch)
                level.at(ch, frame) = peak_ > 1.0f ? 1.0f : peak_;
        }
    }

    // ---- tap `pre-safety` ------------------------------------------
    // O último bloco ANTES da proteção de saída. Leia de fora do
    // `process()` (o painel e o app leem do thread de áudio, logo depois
    // de processar o grafo, sob o mesmo lock).
    const float* preSafety(const std::size_t channel) const noexcept {
        return preTap_[channel < 2 ? channel : 0].data();
    }
    std::size_t preSafetyFrames() const noexcept { return preTapFrames_; }

    // telemetria da proteção (leitura não RT-crítica; p/ um medidor de GR
    // no painel ou pra um módulo SEGUIR a própria redução de ganho)
    float gainReductionDb() const noexcept { return out_.gainReductionDb(); }
    // quanto o governador de corpo está atenuando o agudo (dB de shelf)
    float bodyGuardDb() const noexcept { return out_.bodyGuardDb(); }

private:
    static float dbToGain(const float db) noexcept {
        return db <= -60.0f ? 0.0f : std::pow(10.0f, db / 20.0f);
    }

    OutputStage out_;
    float peak_ = 0.0f;
    float peakDecay_ = 0.9999f;
    float muteGain_ = 1.0f;
    float muteCoef_ = 0.0f;
    // pré-alocado no tamanho máximo de bloco do motor: o `process` não
    // aloca nem redimensiona nada
    std::array<std::vector<float>, 2> preTap_{
        std::vector<float>(AudioBlock::maxFrames, 0.0f),
        std::vector<float>(AudioBlock::maxFrames, 0.0f)};
    std::size_t preTapFrames_ = 0;
    float paramCoef_ = 0.0f;      // rampa de GAIN/WIDTH (ver `prepare`)
    float gainSm_ = 1.0f;
    float widthSm_ = 1.0f;
};

}  // namespace rasgo::modular
