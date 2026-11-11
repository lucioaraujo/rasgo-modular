#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// HARMONY — movimento harmônico (Módulo 14)
// ============================================================================
//
// O que faz a peça MUDAR DE TOM. A cada fronteira de seção (um trigger
// externo ou um relógio interno lento), avança o centro tonal — `root` e
// `scale` — por uma de seis técnicas REAIS de movimento harmônico, cada
// uma com intervalos de raiz distintos. As saídas alimentam o `root` e o
// `scale` de um `QUANTIZER`: a melodia generativa passa a ter harmonia
// que anda, não só uma tonalidade fixa.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/14_harmony.md`.
//
// Fontes ESTUDADAS (a lógica de intervalos, não o código):
//   - `RASGO_SYNTH/rasgo-synth-core/src/sequencer/HarmonicWanderer.hpp`
//     (projeto irmão): seis técnicas documentadas, cada uma com a fonte
//     musicológica citada. Só a lógica de intervalos é portada (fato
//     musical); o código não é incluído entre projetos.
//     · Coltrane / Giant Steps (1959): raiz +4 st, ciclo de 3 centros.
//     · Substituição tritônica + ii-V-I: −7 st (quintas descendentes) ou,
//       às vezes, ±1 st (a substituição torna a condução cromática).
//     · Mediante cromática não-funcional (Jobim / bossa): ±3 ou ±4 st,
//       sem resolução implícita.
//     · Intercâmbio modal: a raiz NÃO se move, só o modo (acorde
//       emprestado da família paralela).
//     · Jazz modal (Miles Davis / "Kind of Blue"): quase sempre estático;
//       de vez em quando o passo de "So What" (½ ou 1 tom acima).
//     · Backdoor ii-V (bVII7→I): raiz +2 st.
//
// Determinístico: xorshift64* semeado em prepare(). Avança por trigger
// externo OU por relógio interno em `rate`.
//
// Movimento 6, NEO-RIEMANN (11/11/2026, lote da harmonia, aprovado): a
// tríade (tônica + Maior/menor) sofre P (paralela), R (relativa) ou L
// (Leittonwechsel) — cada uma move UMA nota por 1–2 semitons (H. Riemann;
// R. Cohn, 1998; D. Lewin). CICLO: 0 passeio livre pelo Tonnetz (nunca
// desfaz o passo anterior), 1 hexatônico (P L P L…), 2 octatônico (P R P
// R…), 3 LR (as quintas que caminham). Escala só Maior (1) e menor natural
// (2). Saída OP: qual transformação (P = ⅓, L = ⅔, R = 1).

namespace rasgo::modular {

class Harmony final : public Signal {
public:
    Harmony()
        : Signal(
              {{"advance", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"}},
              {{"root", PortKind::Audio, ""},     // semitom/12, pra QUANTIZER.root (depth 12)
               {"scale", PortKind::Audio, ""},    // índice/11,   pra QUANTIZER.scale (depth 11)
               {"change", PortKind::Control, "gate"},
               {"op", PortKind::Control, ""}},
              {{"movement", 0.0f, 6.0f, 4.0f, ""},   // 4 = jazz modal (default calmo); 6 = neo-Riemann
               {"rate", 0.005f, 2.0f, 0.06f, "Hz"},
               {"root_start", 0.0f, 11.0f, 2.0f, "st"},
               {"scale_lo", 1.0f, 10.0f, 1.0f, ""},   // menor índice de escala sorteável
               {"scale_hi", 1.0f, 10.0f, 6.0f, ""},   // maior índice de escala sorteável
               {"hold", 0.0f, 1.0f, 0.0f, ""},
               {"nr_cycle", 0.0f, 3.0f, 0.0f, ""}}) {}

    std::string type() const override { return "HARMONY"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        // passe de ergonomia 2026-09-06: display cheio; saída "SCALE"
        // (5 letras) agora com espaço; entradas realinhadas.
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "HARMONY", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "keycenter", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "MOVE", "movement", 6.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ROOT", "root_start", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "S-LO", "scale_lo", 6.0f, 54.0f);
        p.add(Widget::Kind::Knob, "S-HI", "scale_hi", 22.0f, 54.0f);
        p.add(Widget::Kind::Knob, "HOLD", "hold", 38.0f, 54.0f);
        p.add(Widget::Kind::Knob, "CICLO", "nr_cycle", 6.0f, 76.0f);
        p.add(Widget::Kind::Jack, "ADV", "in:advance", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 20.0f, 100.0f);
        p.add(Widget::Kind::Jack, "ROOT", "out:root", 8.0f, 118.0f);
        p.add(Widget::Kind::Jack, "SCALE", "out:scale", 22.0f, 118.0f);
        p.add(Widget::Kind::Jack, "CHG", "out:change", 36.0f, 118.0f);
        p.add(Widget::Kind::Jack, "OP", "out:op", 36.0f, 100.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        prevAdvance_ = 0.0f;
        prevReset_ = 0.0f;
        rngState_ = 0xA24BAED4963EE407ULL;
        root_ = static_cast<int>(std::lround(parameterValue("root_start"))) % 12;
        if (root_ < 0) root_ += 12;
        scale_ = clampi(static_cast<int>(std::lround(parameterValue("scale_lo"))),
                        1, 10);
        coltraneStep_ = 0;
        nrLast_ = -1;
        nrStep_ = 0;
        op_ = 0.0f;
        changeCountdown_ = 0;
        gateSamples_ = static_cast<int>(0.02f * sampleRate);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& rootOut = outputs[0];
        AudioBlock& scaleOut = outputs[1];
        AudioBlock& changeOut = outputs[2];
        const std::size_t frames = rootOut.frames();
        const std::size_t channels = rootOut.channels();
        AudioBlock* opOut = outputs.size() > 3 ? &outputs[3] : nullptr;

        const int movement = clampi(
            static_cast<int>(std::lround(parameterValue("movement"))), 0, 6);
        const int nrCycle = clampi(
            static_cast<int>(std::lround(parameterValue("nr_cycle"))), 0, 3);
        const float rate = parameterValue("rate");
        const int scLo = clampi(
            static_cast<int>(std::lround(parameterValue("scale_lo"))), 1, 10);
        const int scHi = clampi(
            static_cast<int>(std::lround(parameterValue("scale_hi"))), scLo, 10);
        const float hold = parameterValue("hold");

        const AudioBlock* advance = inputs[0];
        const AudioBlock* reset = inputs[1];
        const bool externalClock = (advance != nullptr);
        const double dp = static_cast<double>(rate) / sampleRate_;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (reset) {
                const float r = reset->at(0, frame);
                if (prevReset_ < 0.5f && r >= 0.5f) {
                    root_ = static_cast<int>(
                                std::lround(parameterValue("root_start"))) % 12;
                    if (root_ < 0) root_ += 12;
                    scale_ = scLo;
                    coltraneStep_ = 0;
                    nrLast_ = -1;
                    nrStep_ = 0;
                }
                prevReset_ = r;
            }

            bool step = false;
            if (externalClock) {
                const float a = advance->at(0, frame);
                if (prevAdvance_ < 0.5f && a >= 0.5f)
                    step = true;
                prevAdvance_ = a;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) {
                    phase_ -= 1.0;
                    step = true;
                }
            }

            if (step && !(hold > 0.0f && uniform01() < hold)) {
                const int oldRoot = root_;
                const int oldScale = scale_;
                if (movement == 6) neoRiemann(nrCycle);
                else advanceKeyCenter(movement, scLo, scHi);
                if (root_ != oldRoot || scale_ != oldScale)
                    changeCountdown_ = gateSamples_;
            }

            float chg = 0.0f;
            if (changeCountdown_ > 0) {
                chg = 1.0f;
                --changeCountdown_;
            }

            const float rootN = static_cast<float>(root_) / 12.0f;
            const float scaleN = static_cast<float>(scale_) / 11.0f;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                rootOut.at(channel, frame) = rootN;
                scaleOut.at(channel, frame) = scaleN;
                changeOut.at(channel, frame) = chg;
                if (opOut) opOut->at(channel, frame) = op_;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    int pickScale(const int lo, const int hi) noexcept {
        if (hi <= lo) return lo;
        return lo + static_cast<int>(uniform01() * static_cast<float>(hi - lo + 1))
            % (hi - lo + 1);
    }

    void moveRoot(const int semis) noexcept {
        root_ = ((root_ + semis) % 12 + 12) % 12;
    }

    // 0 Coltrane · 1 TritoneSub · 2 ChromaticMediant · 3 ModalInterchange
    // · 4 ModalJazz · 5 BackdoorIiV
    void advanceKeyCenter(const int movement, const int scLo,
                          const int scHi) noexcept {
        switch (movement) {
        case 0:  // Coltrane: +4 st, ciclo de 3
            moveRoot(4);
            coltraneStep_ = (coltraneStep_ + 1) % 3;
            scale_ = pickScale(scLo, scHi);
            break;
        case 1:  // TritoneSub + ii-V-I
            if (uniform01() < 0.4f)
                moveRoot(uniform01() < 0.5f ? 1 : -1);
            else
                moveRoot(-7);
            scale_ = pickScale(scLo, scHi);
            break;
        case 2:  // Mediante cromática não-funcional
            moveRoot((uniform01() < 0.5f ? 3 : 4)
                     * (uniform01() < 0.5f ? 1 : -1));
            scale_ = pickScale(scLo, scHi);
            break;
        case 3:  // Intercâmbio modal: raiz fixa, só o modo muda
            scale_ = pickScale(scLo, scHi);
            break;
        case 5:  // Backdoor ii-V: +2 st
            moveRoot(2);
            scale_ = pickScale(scLo, scHi);
            break;
        case 4:
        default:  // Jazz modal: quase sempre estático
            if (uniform01() < 0.7f)
                return;
            moveRoot(uniform01() < 0.5f ? 1 : 2);  // passo de "So What"
            break;
        }
    }

    // P / L / R sobre (root_, Maior = 1 | menor = 2)
    void neoRiemann(const int cycle) noexcept {
        if (scale_ != 1 && scale_ != 2) scale_ = 1;
        int op;   // 0 P · 1 L · 2 R
        switch (cycle) {
        case 1: op = (nrStep_ % 2 == 0) ? 0 : 1; break;   // hexatônico
        case 2: op = (nrStep_ % 2 == 0) ? 0 : 2; break;   // octatônico
        case 3: op = (nrStep_ % 2 == 0) ? 1 : 2; break;   // LR
        default:
            // passeio livre: nunca repete o último (cada uma é involução)
            if (nrLast_ < 0) {
                op = static_cast<int>(uniform01() * 3.0f) % 3;
            } else {
                op = static_cast<int>(uniform01() * 2.0f) % 2;
                if (op >= nrLast_) ++op;
            }
            break;
        }
        ++nrStep_;
        nrLast_ = op;
        const bool major = scale_ == 1;
        switch (op) {
        case 0: break;                              // P: mesma tônica
        case 1: moveRoot(major ? 4 : -4); break;    // L: Dó → mi; mi → Dó
        default: moveRoot(major ? -3 : 3); break;   // R: Dó → lá; lá → Dó
        }
        scale_ = major ? 2 : 1;                     // as três trocam o modo
        op_ = static_cast<float>(op + 1) / 3.0f;
    }

    int nrLast_ = -1;
    int nrStep_ = 0;
    float op_ = 0.0f;

    double phase_ = 0.0;
    float prevAdvance_ = 0.0f;
    float prevReset_ = 0.0f;
    std::uint64_t rngState_ = 0xA24BAED4963EE407ULL;

    int root_ = 2;
    int scale_ = 1;
    int coltraneStep_ = 0;
    int changeCountdown_ = 0;
    int gateSamples_ = 960;
};

}  // namespace rasgo::modular
