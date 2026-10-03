#pragma once

#include "core/SignalGraph.hpp"

#include <string>
#include <vector>

// ============================================================================
// SeedGrammar — a gramática de portas do Seed, nomeada e exposta
// ============================================================================
//
// A pergunta que motivou isto: "vamos avançar na gramática explícita do
// Seed" (`ESTUDO_seed_composicao_generativa.md §1.1` — `SOURCE→TRANSFORM→
// CONTROL→MODULATE→FEEDBACK→OUTPUT`). Lendo `PatchSeed.hpp::seedPatch()`
// inteiro antes de escrever qualquer coisa: **a gramática já existe** —
// só estava anônima, embutida em enums/lambdas locais dentro de uma
// função de ~500 linhas, sem nome exposto nem documentação própria.
//
// Este arquivo é um REFACTOR comprovado, não uma reescrita: a
// classificação de portas e a matriz de compatibilidade foram movidas
// pra cá byte a byte (mesma lógica, mesmos números, mesma ordem) — NADA
// aqui consome RNG, então mover isto não muda UMA AMOSTRA do que
// nenhum `RASGO_SEED=N` já produzia. A "caminhada" que de fato sorteia
// cabo a cabo (`seedPatch()`, a parte que chama `rnd()`) continua
// exatamente onde estava, inalterada — só passou a LER a classificação
// e a matriz daqui em vez de tê-las embutidas.
//
// Por que não virou literalmente as 6 categorias da conversa: a
// gramática do Rasgo classifica PORTA, não módulo inteiro — granularidade
// mais fina (ex.: `NOISE.smooth` e `NOISE.white` são a mesma família de
// módulo mas classes de porta DIFERENTES, porque uma é CV lenta e a
// outra é ruído de áudio). Reorganizar pra 6 categorias por MÓDULO seria
// uma REGRESSÃO de precisão — e mudaria o que todo seed já existente
// produz, sem necessidade técnica. Mapeamento, pra quem vem do
// vocabulário da conversa:
//
// | Classe do Rasgo (porta) | ~ papel da conversa      | Exemplos |
// |---|---|---|
// | `SeedSrc::Voice`  | SOURCE (a voz, áudio)        | `OSC.saw`, `CHORD.out` |
// | `SeedSrc::Bus`    | SOURCE/TRANSFORM (já processado) | `FILTER.all`, `SPACE.out` |
// | `SeedSrc::Slow`   | MODULATE (CV lenta/estruturada) | `FUNCTION.out`, `DRIFT.a` |
// | `SeedSrc::Rand`   | MODULATE (CV crua/rápida)    | `NOISE.sh`, `SH.out` |
// | `SeedSrc::Gate`   | CONTROL (timing/evento)      | `CLOCK.euclid`, `DECISION.gate` |
// | `SeedSrc::Pitch`  | CONTROL (altura)             | `QUANTIZER.pitch`, `TURING.cv` |
// | `SeedDst::Audio`/`Fm` | TRANSFORM (entrada de sinal) | `FILTER.in`, `OSC.fm` |
// | `SeedDst::Mod`    | MODULATE (destino)           | `*_mod`, `cv`, `sweep`, `amount` |
// | `SeedDst::Gate`/`Pitch` | CONTROL (destino)      | `gate`, `trigger`, `pitch` |
//
// `FEEDBACK` não é uma classe de porta — é uma PROPRIEDADE do cabo
// (`reaches()`/`Cable::isFeedback()`, continua em `PatchSeed.hpp`, é
// sobre TOPOLOGIA, não sobre classificação de porta). `OUTPUT` é o
// `sink`/`MASTER`, fixo por convenção do rack, não sorteado por esta
// gramática.
//
// Específico do painel de teste (não é regra RASGO comum).

namespace rasgo::panel {

// mesma ordem/valores de antes (eram `S_VOICE.. S_NONE` locais) — a
// ORDEM importa: é o índice em `src[]`/`W[][]`.
enum SeedSrc { SEED_SRC_VOICE, SEED_SRC_BUS, SEED_SRC_SLOW, SEED_SRC_RAND,
              SEED_SRC_GATE, SEED_SRC_PITCH, SEED_SRC_NONE };
enum SeedDst { SEED_DST_AUDIO, SEED_DST_FM, SEED_DST_MOD, SEED_DST_GATE,
              SEED_DST_PITCH, SEED_DST_NONE };

struct SeedPort { std::size_t node, port; };

inline const char* seedSrcName(const int s) noexcept {
    static const char* names[] = {"VOICE", "BUS", "SLOW", "RAND", "GATE", "PITCH", "NONE"};
    return (s >= 0 && s < 7) ? names[s] : "?";
}
inline const char* seedDstName(const int d) noexcept {
    static const char* names[] = {"AUDIO", "FM", "MOD", "GATE", "PITCH", "NONE"};
    return (d >= 0 && d < 6) ? names[d] : "?";
}

// nomes de PORTA que valem como "gate"/evento em qualquer módulo (nem
// todo `unit=="trig"` tem esse nome — alguns módulos usam nomes de
// domínio, tipo `strike`/`pluck` do MATTER/STRING).
inline bool seedIsGateName(const std::string& n) {
    return n == "gate" || n == "strike" || n == "pluck" || n == "trigger"
        || n == "clock" || n == "advance" || n == "sync" || n == "reset"
        || n == "ext_clock" || n == "a" || n == "b";
}

// classifica TODA porta do grafo (puro — sem RNG, sem efeito colateral
// no grafo). `outSrc[6]`/`outDst[5]` recebem as portas de cada classe,
// na ordem em que os nós/portas aparecem no grafo (determinístico).
inline void seedClassifyPorts(rasgo::modular::SignalGraph& g,
                              std::vector<SeedPort> outSrc[6],
                              std::vector<SeedPort> outDst[5]) {
    using namespace rasgo::modular;
    const std::size_t N = g.nodeCount();
    for (std::size_t n = 0; n < N; ++n) {
        Signal& node = g.node(n);
        const std::string t = node.type();
        const bool clockish = t == "CLOCK" || t == "LOGIC";
        const bool voiceMod = t == "OSC" || t == "CHORD" || t == "MATTER"
            || t == "STRING" || t == "MEMORY" || t == "SHAPE";
        for (std::size_t p = 0; p < node.outputCount(); ++p) {
            const auto& d = node.outputDescriptor(p);
            const std::string nm = d.name;
            int c = SEED_SRC_NONE;
            if (d.kind == PortKind::Audio) {
                if (t == "NOISE" && (nm == "smooth" || nm == "sh")) c = SEED_SRC_RAND;
                else if (voiceMod || t == "NOISE") c = SEED_SRC_VOICE;
                else if (t == "QUANTIZER" && (nm == "pitch" || nm == "semitone")) c = SEED_SRC_PITCH;
                else if (t == "TURING" || (t == "SEQUENCE" && nm == "pitch")) c = SEED_SRC_PITCH;
                else c = SEED_SRC_BUS;
            } else {  // Control
                if (clockish || d.unit == "gate" || d.unit == "trig"
                    || nm == "euclid" || nm == "accent" || nm == "pulse"
                    || nm == "eos" || nm == "event" || nm == "change"
                    || (t == "DECISION" && nm == "gate")
                    || (t == "QUANTIZER" && nm == "gate")) c = SEED_SRC_GATE;
                else if (t == "FUNCTION" || t == "DRIFT" || t == "HARMONY"
                         || (t == "ENVELOPE" && nm == "env")) c = SEED_SRC_SLOW;
                else c = SEED_SRC_RAND;
            }
            if (c != SEED_SRC_NONE) outSrc[c].push_back({n, p});
        }
        // O CLOCK é a espinha de tempo — o `seedPatch` cabeia as SAÍDAS
        // dele (euclid/clock) explicitamente. Deixar o passeio semeado
        // cabear um trigger qualquer em `CLOCK.reset`/`ext`/`bpm_mod`
        // ESTOLA o relógio (reset a cada amostra → `phase_` nunca anda →
        // tudo que depende do euclid fica mudo). Achado 2026-09-06 (seed
        // 18). As entradas do CLOCK ficam FORA dos destinos do passeio.
        if (t == "CLOCK") continue;

        for (std::size_t p = 0; p < node.inputCount(); ++p) {
            const auto& d = node.inputDescriptor(p);
            const std::string nm = d.name;
            // Portas acrescentadas DEPOIS da v0.1.2 ficam fora do passeio:
            // entrar no sorteio mudaria os patches de seeds antigos, e o
            // mesmo número tem de dar sempre o mesmo patch.
            if (t == "QUANTIZER" && (nm == "root_cv" || nm == "scale_cv")) continue;
            int c = SEED_DST_NONE;
            if (d.kind == PortKind::Audio) c = (nm == "fm") ? SEED_DST_FM : SEED_DST_AUDIO;
            else if (nm == "pitch" || nm == "freq_mod" || nm == "transpose") c = SEED_DST_PITCH;
            else if (seedIsGateName(nm) || d.unit == "trig" || d.unit == "gate") c = SEED_DST_GATE;
            else c = SEED_DST_MOD;   // *_mod, pwm, cv, sweep, amount, etc.
            if (c != SEED_DST_NONE) outDst[c].push_back({n, p});
        }
    }
}

// matriz de compatibilidade fonte×destino, com bônus experimental
// escalado por `wildness` (0..1) — mesmos números de `PatchSeed.hpp`.
inline void seedCompatibilityMatrix(const float wildness, float W[6][5]) {
    const float wild = wildness;
    const float m[6][5] = {
        //        AUDIO  FM              MOD              GATE  PITCH
        /*VOICE*/{6.0f,  0.6f + wild * 4.0f, 1.0f + wild * 4.0f, 0.0f, 0.0f},
        /*BUS  */{4.0f,  0.2f + wild * 2.5f, 1.0f + wild * 2.0f, 0.0f, 0.0f},
        /*SLOW */{0.4f + wild * 1.5f, 1.0f, 10.0f, 0.4f, 1.2f + wild},
        /*RAND */{0.4f + wild * 2.0f, 1.5f, 7.0f, 1.2f, 3.5f},
        /*GATE */{0.0f, 0.0f, 1.5f, 10.0f, wild * 3.0f},
        /*PITCH*/{0.0f, 1.0f, 1.2f, 0.0f, 8.0f},
    };
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 5; ++j)
            W[i][j] = m[i][j];
}

}  // namespace rasgo::panel
