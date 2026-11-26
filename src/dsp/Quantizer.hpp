#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>

// ============================================================================
// QUANTIZER — quantizador de escala (Módulo 12)
// ============================================================================
//
// Transforma CV contínua em ALTURAS de uma escala musical. A saída é em
// oitavas (1 V/oct): `pitch` alimenta o `rate_mod` de uma voz e a
// transposição fica certa. Com sample-and-hold (`trigger`), a nota só
// muda no pulso - é assim que `TURING`/`DECISION` viram melodia.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/12_quantizer.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - as tabelas de graus vêm da pesquisa registrada em
//     `RASGO_SYNTH/rasgo-synth-core/src/sequencer/Scales.hpp` (39+ escalas
//     reais, cruzadas com nomenclatura de teoria/jazz, com 3 erros de uma
//     fonte anterior corrigidos). Aqui um subconjunto curado - leve, não a
//     máquina inteira ([[feedback_generative_design_light_touch]]);
//   - quantizadores de hardware (Doepfer A-156, Intellijel Scales,
//     Marbles `t`) - histerese pra não tremular entre graus vizinhos;
//   - glide exponencial (theremin, `RASGO_SYNTH/dsp/ThereminVoice.hpp`).
//
// Determinístico (sem RNG).
//
// Lote da harmonia (11/11/2026, aprovado; dossiê §9):
//   - CRIVO = escala nº 12: as alturas são n ≡ R1 (mod M1) ∪ (ou ∩) n ≡ R2
//     (mod M2) sobre a linha dos semitons a partir da tônica — a peneira de
//     I. Xenakis (*Formalized Music*). Com M que não divide 12 a escala não
//     se repete na oitava. Os módulos e os resíduos ficam à vista (knobs).
//     Interseção vazia → cai na união (nunca fica mudo).
//   - AFIN: 12-EDO (o de antes) · Justa (5-limite) · Pitagórica · Mesotônica
//     ¼ de coma — as três medidas A PARTIR DA TÔNICA (com o HARMONY ou o TOM
//     mudando a tônica, a afinação acompanha: adaptativa) · 19-EDO · 31-EDO
//     (a grade muda: cada semitom vai ao passo mais próximo da divisão).
//     Aplicada na saída, depois da escolha da nota e antes do glide.
//
// O banco de escalas do Trioio (23/11/2026; `dossies/PROPOSTA_banco_de_escalas.md`,
// opção A): SCALE = 13 → BANCO. A escala vem do banco pelo número em `bank`
// (o id do nome), graus em cents e período qualquer. Os dados e o leitor
// ficam fora daqui (`src/escalas/`, AGPL); este módulo só recebe um
// resolvedor (`setBankResolver`). Sem resolvedor ou com id desconhecido, o
// BANCO toca a cromática (nunca mudo). No banco o AFIN e o MRPH não valem:
// os graus já são a afinação.

namespace rasgo::modular {

class Quantizer final : public Signal {
public:
    // Subconjunto curado das tabelas pesquisadas (graus em semitons desde a
    // tônica). A ordem define o índice do parâmetro `scale`.
    struct ScaleDef {
        const char* name;
        std::array<int, 12> degrees;
        int length;
    };
    static const std::array<ScaleDef, 12>& scales() {
        static const std::array<ScaleDef, 12> s{{
            {"Chromatic", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 12},
            {"Major", {0, 2, 4, 5, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Aeolian", {0, 2, 3, 5, 7, 8, 10, 10, 10, 10, 10, 10}, 7},
            {"Dorian", {0, 2, 3, 5, 7, 9, 10, 10, 10, 10, 10, 10}, 7},
            {"Phrygian Dominant", {0, 1, 4, 5, 7, 8, 10, 10, 10, 10, 10, 10}, 7},
            {"Lydian", {0, 2, 4, 6, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Melodic Minor", {0, 2, 3, 5, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Minor Pentatonic", {0, 3, 5, 7, 10, 10, 10, 10, 10, 10, 10, 10}, 5},
            {"Major Pentatonic", {0, 2, 4, 7, 9, 9, 9, 9, 9, 9, 9, 9}, 5},
            {"Hirajoshi", {0, 2, 3, 7, 8, 8, 8, 8, 8, 8, 8, 8}, 5},
            {"Whole Tone", {0, 2, 4, 6, 8, 10, 10, 10, 10, 10, 10, 10}, 6},
            {"Octave", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1},
        }};
        return s;
    }

    Quantizer()
        : Signal(
              {{"cv", PortKind::Audio, ""},
               {"transpose", PortKind::Control, "v/oct"},
               {"trigger", PortKind::Control, "trig"},
               // tônica e escala por cabo (v0.1.3): substituem os knobs
               // ROOT e SCALE quando ligadas. Escala das saídas do HARMONY
               // (raiz/12, índice/11). No FIM da lista para não mudar o
               // índice das portas nos patches salvos.
               {"root_cv", PortKind::Control, ""},
               {"scale_cv", PortKind::Control, ""},
               // MORPH por cabo (21/11/2026): soma ao knob MRPH, 0–1
               {"morph_cv", PortKind::Control, ""}},
              {{"pitch", PortKind::Audio, "v/oct"},
               {"gate", PortKind::Control, "gate"},
               {"semitone", PortKind::Audio, ""}},
              {{"scale", 0.0f, 13.0f, 1.0f, ""},
               {"root", 0.0f, 11.0f, 0.0f, "st"},
               {"range", 1.0f, 6.0f, 2.0f, "oct"},
               {"glide", 0.0f, 1.0f, 0.0f, ""},
               {"hysteresis", 0.0f, 1.0f, 0.3f, ""},
               {"tuning", 0.0f, 5.0f, 0.0f, ""},
               {"sieve_m1", 2.0f, 13.0f, 3.0f, ""},
               {"sieve_r1", 0.0f, 12.0f, 0.0f, ""},
               {"sieve_m2", 2.0f, 13.0f, 4.0f, ""},
               {"sieve_r2", 0.0f, 12.0f, 1.0f, ""},
               {"sieve_and", 0.0f, 1.0f, 0.0f, ""},
               // A máquina de escalas (21/11/2026; do Trioio, ESTUDO §4.4,
               // aprovado em 9/11): a escala B e a interpolação A → B. No
               // FIM: patches iguais; MRPH 0 = o QUANTIZER de antes.
               {"scale_b", 0.0f, 11.0f, 2.0f, ""},
               {"morph", 0.0f, 1.0f, 0.0f, ""},
               // o banco (23/11/2026): o id (24 bits, exato no float) da
               // escala escolhida no navegador; sem knob. No FIM da lista.
               {"bank", 0.0f, 16777215.0f, 0.0f, ""}}) {}

    static constexpr int kBankIndex = 13;   // SCALE = 13 → o BANCO

    // uma escala do banco, como o resolvedor a entrega (memória imutável)
    struct BankScale { const float* cents = nullptr; int count = 0; float period = 1200.0f; };
    using BankResolver = bool (*)(std::uint32_t id, BankScale& out);
    static void setBankResolver(const BankResolver r) noexcept { resolver().store(r); }

    static constexpr int kSieveIndex = 12;   // SCALE = 12 → o CRIVO

    // o crivo: n (semitons desde a tônica) pertence?
    struct Sieve { int m1, r1, m2, r2; bool both; };
    static bool inSieve(const int n, const Sieve& s) noexcept {
        const bool a = ((n - s.r1) % s.m1 + s.m1) % s.m1 == 0;
        const bool b = ((n - s.r2) % s.m2 + s.m2) % s.m2 == 0;
        return s.both ? (a && b) : (a || b);
    }
    static bool sieveEmpty(const Sieve& s) noexcept {
        for (int n = 0; n < s.m1 * s.m2; ++n) if (inSieve(n, s)) return false;
        return true;
    }

    // desvio (em semitons) da afinação `tuning` para um semitom `s` que está
    // `rel` semitons acima da tônica (rel = s − tônica)
    static float tuningOffset(const int tuning, const int s, const int rel) noexcept {
        // cents de cada grau desde a tônica, menos o 12-EDO (de tabelas
        // públicas: 5-limite justo; pitagórica por quintas; mesotônica ¼ coma)
        static constexpr float kJust[12] = {0.0f, 11.73f, 3.91f, 15.64f, -13.69f, -1.96f,
                                            -9.78f, 1.96f, 13.69f, -15.64f, 17.60f, -11.73f};
        static constexpr float kPyth[12] = {0.0f, -9.78f, 3.91f, -5.87f, 7.82f, -1.96f,
                                            11.73f, 1.96f, -7.82f, 5.87f, -3.91f, 9.78f};
        // mesotônica: bemóis em 2ª m, 3ª m, 6ª m, 7ª m (as das escalas
        // menores e modais) e o trítono como 4ª aumentada
        static constexpr float kMean[12] = {0.0f, 17.11f, -6.84f, 10.26f, -13.69f, 3.42f,
                                            -20.53f, -3.42f, 13.69f, -10.26f, 6.84f, -17.11f};
        const int deg = ((rel % 12) + 12) % 12;
        switch (tuning) {
        case 1: return kJust[deg] / 100.0f;
        case 2: return kPyth[deg] / 100.0f;
        case 3: return kMean[deg] / 100.0f;
        case 4:
        case 5: {
            const float n = tuning == 4 ? 19.0f : 31.0f;
            // a grade nova ancorada na TÔNICA (rel), não no Dó
            (void)s;
            const float rf = static_cast<float>(rel);
            return std::round(rf * n / 12.0f) * 12.0f / n - rf;
        }
        default: return 0.0f;
        }
    }

    std::string type() const override { return "QUANTIZER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        // Revisão de painel (autor, 26/11/2026): colunas pelo centro, knobs e
        // jacks nas mesmas colunas, saídas embaixo no centro (G1–G4).
        p.hp = 14;
        p.add(Widget::Kind::Label, "QUANTIZER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "scale", "", 2.5f, 6.0f, 66.12f);
        p.add(Widget::Kind::Knob, "SCALE", "scale", 10.56f, 28.0f);
        p.add(Widget::Kind::Knob, "ROOT", "root", 24.56f, 28.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 38.56f, 28.0f);
        p.add(Widget::Kind::Knob, "AFIN", "tuning", 52.56f, 28.0f);
        p.add(Widget::Kind::Knob, "GLIDE", "glide", 10.56f, 46.0f);
        p.add(Widget::Kind::Knob, "HYST", "hysteresis", 24.56f, 46.0f);
        p.add(Widget::Kind::Knob, "B", "scale_b", 38.56f, 46.0f);
        p.add(Widget::Kind::Knob, "MRPH", "morph", 52.56f, 46.0f);
        p.add(Widget::Kind::Knob, "M1", "sieve_m1", 10.56f, 64.0f);
        p.add(Widget::Kind::Knob, "R1", "sieve_r1", 24.56f, 64.0f);
        p.add(Widget::Kind::Knob, "M2", "sieve_m2", 38.56f, 64.0f);
        p.add(Widget::Kind::Knob, "R2", "sieve_r2", 52.56f, 64.0f);
        p.add(Widget::Kind::Toggle, "AND", "sieve_and", 33.56f, 74.0f, 8.0f);
        p.add(Widget::Kind::Jack, "CV", "in:cv", 7.56f, 92.0f);
        p.add(Widget::Kind::Jack, "TRSP", "in:transpose", 18.76f, 92.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trigger", 29.96f, 92.0f);
        p.add(Widget::Kind::Jack, "ROOT", "in:root_cv", 41.16f, 92.0f);
        p.add(Widget::Kind::Jack, "SCL", "in:scale_cv", 52.36f, 92.0f);
        p.add(Widget::Kind::Jack, "MRPH", "in:morph_cv", 63.56f, 92.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 21.56f, 110.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 35.56f, 110.0f);
        p.add(Widget::Kind::Jack, "ST", "out:semitone", 49.56f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        heldSemi_ = 0.0f;
        glideSemi_ = 0.0f;
        prevTrigger_ = 0.0f;
        gateCountdown_ = 0;
        initialized_ = false;
        bankId_ = 0xFFFFFFFFu;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& pitchOut = outputs[0];
        AudioBlock& gateOut = outputs[1];
        AudioBlock& semiOut = outputs[2];
        const std::size_t frames = pitchOut.frames();
        const std::size_t channels = pitchOut.channels();

        // ROOT/SCL ligados mandam no lugar dos knobs, lidos uma vez por
        // bloco (o HARMONY troca de centro em segundos, não em amostras)
        const AudioBlock* rootCv = inputs.size() > 3 ? inputs[3] : nullptr;
        const AudioBlock* scaleCv = inputs.size() > 4 ? inputs[4] : nullptr;
        // a CV de escala (formato do HARMONY/TOM, índice/11) chega a 0–11; o
        // CRIVO (12) é só pelo knob
        const int scaleIdx = scaleCv
            ? clampi(static_cast<int>(std::lround(scaleCv->at(0, 0) * 11.0f)), 0, 11)
            : clampi(static_cast<int>(std::lround(parameterValue("scale"))), 0, kBankIndex);
        const bool useSieve = scaleIdx == kSieveIndex;
        bool useBank = false;
        if (scaleIdx == kBankIndex) {
            const auto id = static_cast<std::uint32_t>(std::lround(parameterValue("bank")));
            if (id != bankId_ || bankCount_ == 0) {   // quando a escolha muda (ou o banco chega depois)
                bankId_ = id;
                bankCount_ = 0;
                BankScale b;
                const BankResolver r = resolver().load();
                if (r != nullptr && r(id, b) && b.count > 0 && b.period > 1.0f) {
                    bankCount_ = b.count < kBankMax ? b.count : kBankMax;
                    for (int i = 0; i < bankCount_; ++i)
                        bankDeg_[static_cast<std::size_t>(i)] = b.cents[i] / 100.0f;
                    bankPeriod_ = b.period / 100.0f;
                }
            }
            useBank = bankCount_ > 0;
        }
        Sieve sv{
            clampi(static_cast<int>(std::lround(parameterValue("sieve_m1"))), 2, 13),
            clampi(static_cast<int>(std::lround(parameterValue("sieve_r1"))), 0, 12),
            clampi(static_cast<int>(std::lround(parameterValue("sieve_m2"))), 2, 13),
            clampi(static_cast<int>(std::lround(parameterValue("sieve_r2"))), 0, 12),
            parameterValue("sieve_and") >= 0.5f};
        if (useSieve && sv.both && sieveEmpty(sv)) sv.both = false;   // nunca mudo
        const int tuning = clampi(static_cast<int>(std::lround(parameterValue("tuning"))), 0, 5);
        const int root = rootCv
            ? ((static_cast<int>(std::lround(rootCv->at(0, 0) * 12.0f)) % 12) + 12) % 12
            : clampi(static_cast<int>(std::lround(parameterValue("root"))), 0, 11);
        const float range = parameterValue("range");
        const float glide = parameterValue("glide");
        const float hysteresis = parameterValue("hysteresis");
        const ScaleDef& sc = scales()[static_cast<std::size_t>(
            useSieve || scaleIdx >= kBankIndex ? 0 : scaleIdx)];
        // a interpolação entre a escala A (SCALE) e a B, lida por bloco
        const AudioBlock* morphCv = inputs.size() > 5 ? inputs[5] : nullptr;
        float morph = parameterValue("morph") + (morphCv ? morphCv->at(0, 0) : 0.0f);
        morph = morph < 0.0f ? 0.0f : (morph > 1.0f ? 1.0f : morph);
        const bool useMorph = !useSieve && scaleIdx < kBankIndex && morph > 0.0f;
        if (useMorph) {
            const int bIdx = clampi(static_cast<int>(std::lround(parameterValue("scale_b"))), 0, 11);
            morphCount_ = morphDegrees(sc, scales()[static_cast<std::size_t>(bIdx)], morph, morphDeg_);
        }

        const AudioBlock* cv = inputs[0];
        const AudioBlock* transpose = inputs[1];
        const AudioBlock* trigger = inputs[2];
        const bool sampleHold = (trigger != nullptr);

        const float glideCoeff = glide <= 0.0f
            ? 1.0f
            : 1.0f - std::exp(-1.0f
                              / std::max(1.0f, glide * 0.2f * sampleRate_));
        const int gateSamples = static_cast<int>(0.005f * sampleRate_);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float in = cv ? cv->at(0, frame) : 0.0f;
            const float trsp = transpose ? transpose->at(0, frame) : 0.0f;
            // CV [-1,1] -> semitons; transpose em oitavas
            const float wantSemi = in * range * 12.0f + trsp * 12.0f;

            bool update = !sampleHold;
            if (sampleHold) {
                const float t = trigger->at(0, frame);
                if (prevTrigger_ < 0.5f && t >= 0.5f)
                    update = true;
                prevTrigger_ = t;
            }

            if (update) {
                const float snapped = useBank ? snapPeriod(wantSemi, bankDeg_.data(), bankCount_, bankPeriod_, root)
                                    : useSieve ? snapSieve(wantSemi, sv, root)
                                    : useMorph ? snapDegrees(wantSemi, morphDeg_.data(), morphCount_, root)
                                               : snap(wantSemi, sc, root);
                if (!initialized_) {
                    gateCountdown_ = gateSamples;
                    heldSemi_ = snapped;
                    initialized_ = true;
                } else if (snapped != heldSemi_) {
                    // A banda-morta mede quanto a ENTRADA passou da
                    // fronteira entre a nota presa e a candidata.
                    //
                    // A versão anterior comparava `|snapped - heldSemi_|`
                    // com a banda — duas saídas JÁ QUANTIZADAS — e por isso
                    // não fazia nada: `snap` devolve semitons inteiros, a
                    // diferença entre duas notas é no mínimo o menor passo
                    // da escala, e a banda máxima (0,5·12/graus) é MENOR
                    // que esse passo em 11 das 12 escalas. O parâmetro
                    // existia, era lido, e nunca bloqueava uma troca. A
                    // auditoria de 25 set. 2026 o listou como "sem prova"
                    // justamente por isso; medido em
                    // `test_quantizer.cpp`: 39 trocas com a banda em 0 e
                    // 39 com a banda em 1.
                    //
                    // Medir a entrada é o que histerese significa: segurar
                    // a nota atual até o sinal andar ALÉM da fronteira, e
                    // não julgar pela distância entre as notas.
                    // A banda é uma fração do passo LOCAL — a distância
                    // entre a nota presa e a candidata — e não do passo
                    // MÉDIO da escala (`12/graus`), que era o que a versão
                    // anterior usava.
                    //
                    // A diferença importa porque as escalas são desiguais.
                    // Na maior, o passo médio é 1,71 semitom, mas E→F e
                    // B→C são de 1 semitom: uma banda medida pelo médio
                    // chegaria a 0,857 e travaria justamente esses dois
                    // intervalos, deixando dois graus da escala
                    // inalcançáveis. Na cromática o médio é 1, a banda
                    // máxima batia exatamente em meio passo, e a histerese
                    // no máximo travava TODA melodia cromática.
                    //
                    // 0,45 é o teto, não 0,5, e a margem é o ponto: quando
                    // a entrada cai EM CIMA de um grau, ela está a meio
                    // passo da fronteira, e 0,45·passo < 0,50·passo garante
                    // que um movimento deliberado sempre passa. O que fica
                    // barrado é só o tremor perto da fronteira, que é o que
                    // o parâmetro promete.
                    const float passo = std::fabs(snapped - heldSemi_);
                    const float deadband = hysteresis * 0.45f * passo;
                    const float fronteira = 0.5f * (heldSemi_ + snapped);
                    if (std::fabs(wantSemi - fronteira) > deadband) {
                        gateCountdown_ = gateSamples;
                        heldSemi_ = snapped;
                    }
                }
            }

            // glide até o alvo
            const int heldInt = static_cast<int>(std::lround(heldSemi_));
            const float target = useBank ? heldSemi_
                                         : heldSemi_ + tuningOffset(tuning, heldInt, heldInt - root);
            if (glideCoeff >= 1.0f)
                glideSemi_ = target;
            else
                glideSemi_ += (target - glideSemi_) * glideCoeff;

            float gate = 0.0f;
            if (gateCountdown_ > 0) {
                gate = 1.0f;
                --gateCountdown_;
            }

            const float pitchOct = glideSemi_ / 12.0f;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                pitchOut.at(channel, frame) = pitchOct;
                gateOut.at(channel, frame) = gate;
                semiOut.at(channel, frame) = glideSemi_ / 24.0f;  // ~[-1,1]
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // acha o pitch da escala (em qualquer oitava) mais próximo de `semi`
    static float snap(const float semi, const ScaleDef& sc,
                      const int root) noexcept {
        const int baseOct = static_cast<int>(std::floor(semi / 12.0f));
        float best = 0.0f;
        float bestDist = 1.0e9f;
        for (int oct = baseOct - 1; oct <= baseOct + 1; ++oct) {
            for (int d = 0; d < sc.length; ++d) {
                const float cand =
                    static_cast<float>(oct * 12 + sc.degrees[static_cast<std::size_t>(d)] + root);
                const float dist = std::fabs(cand - semi);
                if (dist < bestDist) {
                    bestDist = dist;
                    best = cand;
                }
            }
        }
        return best;
    }

    // A máquina de escalas: os graus de A deslizam até o grau mais próximo
    // de B (a distância circular na oitava), em linha reta nos cents — no
    // meio do caminho aparecem alturas entre as duas escalas; os graus que
    // só B tem entram a partir da metade. m = 0 → A; m = 1 → B (graus de A
    // que caem no mesmo grau de B viram um só). Sem alocação.
    static int morphDegrees(const ScaleDef& a, const ScaleDef& b, const float m,
                            std::array<float, 24>& out) noexcept {
        int n = 0;
        bool used[12] = {};
        for (int i = 0; i < a.length; ++i) {
            const float da = static_cast<float>(a.degrees[static_cast<std::size_t>(i)]);
            float best = 0.0f, bestD = 1e9f; int bestJ = 0;
            for (int j = 0; j < b.length; ++j) {
                const float db = static_cast<float>(b.degrees[static_cast<std::size_t>(j)]);
                for (const float wrap : {-12.0f, 0.0f, 12.0f}) {
                    const float d = std::fabs(db + wrap - da);
                    if (d < bestD) { bestD = d; best = db + wrap; bestJ = j; }
                }
            }
            used[bestJ] = true;
            out[static_cast<std::size_t>(n++)] = da + m * (best - da);
        }
        if (m >= 0.5f)
            for (int j = 0; j < b.length && n < 24; ++j)
                if (!used[j]) out[static_cast<std::size_t>(n++)] =
                    static_cast<float>(b.degrees[static_cast<std::size_t>(j)]);
        return n;
    }
    static float snapDegrees(const float semi, const float* deg, const int count,
                             const int root) noexcept {
        const int baseOct = static_cast<int>(std::floor(semi / 12.0f));
        float best = 0.0f, bestDist = 1.0e9f;
        for (int oct = baseOct - 1; oct <= baseOct + 1; ++oct)
            for (int d = 0; d < count; ++d) {
                const float cand = static_cast<float>(oct * 12 + root) + deg[d];
                const float dist = std::fabs(cand - semi);
                if (dist < bestDist) { bestDist = dist; best = cand; }
            }
        return best;
    }

    // o grau do banco mais próximo de `semi`: graus em semitons (fração) num
    // período qualquer (não oitavante também), a partir da tônica
    static float snapPeriod(const float semi, const float* deg, const int count,
                            const float period, const int root) noexcept {
        const float rel = semi - static_cast<float>(root);
        const int base = static_cast<int>(std::floor(rel / period));
        float best = 0.0f, bestDist = 1.0e9f;
        for (int k = base - 1; k <= base + 1; ++k)
            for (int d = 0; d < count; ++d) {
                const float cand = static_cast<float>(root) + static_cast<float>(k) * period + deg[d];
                const float dist = std::fabs(cand - semi);
                if (dist < bestDist) { bestDist = dist; best = cand; }
            }
        return best;
    }
    static std::atomic<BankResolver>& resolver() noexcept {
        static std::atomic<BankResolver> r{nullptr};
        return r;
    }

    // o semitom do crivo (a partir da tônica) mais próximo de `semi`
    static float snapSieve(const float semi, const Sieve& s, const int root) noexcept {
        const int c = static_cast<int>(std::lround(semi));
        for (int d = 0; d <= 40; ++d) {
            // empate: o de baixo primeiro (determinístico)
            for (const int cand : {c - d, c + d})
                if (inSieve(cand - root, s)) return static_cast<float>(cand);
        }
        return static_cast<float>(root);
    }

    std::array<float, 24> morphDeg_{};
    int morphCount_ = 0;
    static constexpr int kBankMax = 128;   // o banco vai até 100 graus
    std::array<float, kBankMax> bankDeg_{};
    int bankCount_ = 0;
    float bankPeriod_ = 12.0f;
    std::uint32_t bankId_ = 0xFFFFFFFFu;   // força a primeira leitura
    float heldSemi_ = 0.0f;
    float glideSemi_ = 0.0f;
    float prevTrigger_ = 0.0f;
    int gateCountdown_ = 0;
    bool initialized_ = false;
};

}  // namespace rasgo::modular
