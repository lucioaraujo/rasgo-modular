#pragma once

// ============================================================================
// PatchSeed — gerador de patch por SEED (gramática probabilística de portas)
// ============================================================================
//
// NÃO é um banco de arquétipos. O seed é um passeio ALEATÓRIO PONDERADO
// pelo grafo de portas do rack: classifica toda saída/entrada por função
// musical, usa uma matriz de compatibilidade `W[fonte][destino]` (com um
// bônus "experimental" escalado por `wildness`), e sorteia cabo a cabo.
// Não há limitação de cabeamento — qualquer conexão é possível, só mais ou
// menos provável.
//
// Espaço: rack de 27 módulos, 80 entradas, 68 saídas → ~10^16 (patch de
// 6 cabos) a ~10^44 (20 cabos). Seed `uint64` = 1,8·10^19 sementes.
//
// GARANTIAS: sempre há um caminho voz → ... → MASTER → sink (audível); o
// CLOCK sempre tica; detector de ciclo liga como feedback. Do mínimo
// (3 cabos) à teia densa conforme `complexity`.
//
// (2026-09-10) NENHUM CABO SEM FUNÇÃO SONORA: o passeio e a DRIFT só
// cabeiam em módulos que já chegam à saída (`feeds`, recalculado a cada
// cabo — CRESCE conforme fontes de áudio se ligam a entradas audíveis);
// uma poda final varre os cabos de montagem que ficaram órfãos. Antes,
// ~62% dos cabos tinham destino que não chegava à saída (fiação de
// exploração pelo rack inteiro). Efeito colateral: o stream de RNG
// diverge, então TODO `RASGO_SEED=N` anterior a esta data produz um
// patch diferente. Verificado em `tests/test_seed_patch.cpp`.
//
// Genes (poucos, via SplitMix64 — stream separado por decisão): complexity,
// wildness, energy, space, motion, voiceBias, root/scale, bpm. O resto
// emerge do passeio semeado. Determinístico: mesmo número → mesmo patch.
//
// Modelado no "fator de escolha" do RASGO Synth Studio + a DERIVA do
// ANTITOTEM (o módulo `DRIFT` faz o patch evoluir sozinho). Consultados,
// não editados.
//
// Específico do painel de teste (não é regra RASGO comum).

#include "core/SignalGraph.hpp"
#include "panel/SeedGrammar.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rasgo::panel {

struct SeedIdentity {
    float complexity = 0.4f;  // 0 = mínimo (3 cabos) .. 1 = denso (~43)
    float wildness = 0.3f;    // 0 = convencional .. 1 = experimental
    float energy = 0.5f;      // alvo de dinâmica
    float space = 0.3f;       // quantidade de reverb
    float motion = 0.5f;      // quanto o DRIFT/LFO mexe a estrutura
    int voiceBias = 0;        // 0..6: osc/físico/acorde/ruído/granular/fold/feedback
    int root = 0, scale = 2;
    float bpm = 112.0f;
    int mult = 1;
    // (10/11/2026) quantas vozes o patch tem no MIXER: 1 = só a voz;
    // 2 = + contravoz (altura própria); 3 = + textura; 4 = + eco ou pulso.
    // Ver `dossies/PROPOSTA_quatro_canais_da_seed.md`.
    int ensemble = 1;
};

// A seed abre os canais nas suas casas do palco (pans fora do centro)?
// Ligado em 10/11/2026 junto com o PALCO do MIXER — o autor: "precisamos
// encontrar um meio de trabalhar o pan que não seja no centro, porém que
// não seja aleatório" (antes: centro em toda seed, pedido de 8/9/2026).
inline constexpr bool kSeedOpensStage = true;

inline std::uint64_t seedBits(std::uint64_t seed, std::uint64_t stream) {
    std::uint64_t z = seed + 0x9E3779B97F4A7C15ULL * (stream + 1ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
inline float seedUnit(std::uint64_t seed, std::uint64_t stream) {
    return static_cast<float>(seedBits(seed, stream) >> 40) / 16777216.0f;
}

inline SeedIdentity seedIdentity(std::uint64_t seed) {
    SeedIdentity id;
    const std::uint64_t s = seed ? seed : 0x9E3779B97F4A7C15ULL;
    // complexity: curva pra dar bastante patch simples E bastante complexo
    const float cx = seedUnit(s, 1);
    id.complexity = cx * cx * (0.6f + 0.4f * seedUnit(s, 2));
    // minimalista mas AINDA VARIADO — o hard-clamp pra 0,02 fazia ~15% dos
    // seeds colapsarem num punhado de esqueletos quase idênticos (nProc=0,
    // nCables=3); agora fica 0,05..0,20, então nProc/nCables ainda variam.
    if (seedUnit(s, 3) < 0.13f)
        id.complexity = 0.05f + 0.15f * seedUnit(s, 15);
    if (seedUnit(s, 3) > 0.9f) id.complexity = 0.8f + 0.2f * cx;  // 10% máximo
    id.wildness = seedUnit(s, 4) * seedUnit(s, 4);
    if (seedUnit(s, 5) < 0.12f) id.wildness = 0.7f + 0.3f * seedUnit(s, 6);
    id.energy = 0.35f + 0.6f * seedUnit(s, 7);
    id.space = seedUnit(s, 8);
    id.motion = seedUnit(s, 9);
    id.voiceBias = static_cast<int>(seedBits(s, 10) % 7);
    id.root = static_cast<int>(seedBits(s, 11) % 12);
    id.scale = 1 + static_cast<int>(seedBits(s, 12) % 6);
    const float t = seedUnit(s, 13);
    id.bpm = 46.0f + t * t * 110.0f;   // 46..156, enviesado pra devagar
    id.mult = seedUnit(s, 14) < 0.18f ? 2 : (seedUnit(s, 14) > 0.93f ? 4 : 1);
    // conjunto: fluxo próprio (16), ligado à complexidade — patch simples
    // tende a poucas vozes, denso a muitas (média ~20/30/30/20 %)
    {
        static constexpr float cum[3][3] = {
            {0.35f, 0.70f, 0.90f},   // simples: 35/35/20/10 %
            {0.15f, 0.45f, 0.75f},   // média:   15/30/30/25 %
            {0.05f, 0.25f, 0.60f}};  // densa:    5/20/35/40 %
        const int band = id.complexity < 0.25f ? 0 : (id.complexity < 0.55f ? 1 : 2);
        const float u = seedUnit(s, 16);
        id.ensemble = u < cum[band][0] ? 1 : u < cum[band][1] ? 2 : u < cum[band][2] ? 3 : 4;
    }
    return id;
}

// ============================================================================

// `forceEnsemble` (1..4) só para teste: força o tamanho do conjunto (a
// régua de qualidade roda também com 4 vozes, o pior caso de soma).
//
// `makeCopy` (11/11/2026, instâncias — decisão do autor: "gostaria que já
// abrissem seeds com os módulos repetidos"): fábrica de módulos. Com ela, a
// seed garante um PAR FIXO de cópias no rack (uma segunda instância de
// cada tipo de `kSeedCopyPool`, criada só se ainda não houver, sempre na
// mesma ordem) e cada seed decide se USA a cópia. Rack novo e rack usado
// ficam com a mesma estrutura — a mesma seed dá o mesmo patch. Sem fábrica
// (painel X11, CRUZA), nada muda: uma instância de cada, como antes.
using SeedModuleFactory =
    std::function<std::unique_ptr<rasgo::modular::Signal>(const std::string&)>;
inline const char* const kSeedCopyPool[] = {"OSC", "VCA", "ENVELOPE", "LPG", "STRING", "MATTER"};

inline void seedPatch(rasgo::modular::SignalGraph& g, std::uint64_t seed,
                      int forceEnsemble = 0, const SeedModuleFactory& makeCopy = {}) {
    using namespace rasgo::modular;
    if (makeCopy)
        for (const char* t : kSeedCopyPool) {
            int n = 0;
            for (std::size_t i = 0; i < g.nodeCount(); ++i)
                if (g.node(i).type() == t) ++n;
            if (n == 1)
                if (auto m = makeCopy(t)) g.add(std::move(m));
        }
    SeedIdentity id = seedIdentity(seed);
    if (forceEnsemble >= 1 && forceEnsemble <= 4) id.ensemble = forceEnsemble;
    std::uint64_t rs = seed ? seed : 0x9E3779B97F4A7C15ULL;
    auto rnd = [&rs]() -> std::uint64_t {
        rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs;
    };
    auto f01 = [&] { return static_cast<float>((rnd() >> 40)) / 16777216.0f; };
    auto rng = [&](float a, float b) { return a + f01() * (b - a); };
    auto chance = [&](float p) { return f01() < p; };
    auto pick4 = [&](int a, int b, int c, int d) {
        const int v[4] = {a, b, c, d}; return v[rnd() % 4];
    };
    auto pickS5 = [&](const char* a, const char* b, const char* c,
                      const char* d, const char* e) {
        const char* v[5] = {a, b, c, d, e}; return v[rnd() % 5];
    };

    const std::size_t N = g.nodeCount();
    std::size_t sink = 0; bool haveSink = false;
    for (std::size_t i = 0; i < N; ++i)
        if (g.node(i).type() == "OUT") { sink = i; haveSink = true; }

    auto typ = [&](std::size_t n) { return g.node(n).type(); };
    auto hasT = [&](const char* t) {
        for (std::size_t i = 0; i < N; ++i) if (typ(i) == t) return true;
        return false;
    };
    auto firstT = [&](const char* t) -> std::size_t {
        for (std::size_t i = 0; i < N; ++i) if (typ(i) == t) return i;
        return 0;
    };
    auto setP = [&](std::size_t n, const char* p, float v) {
        try { g.node(n).setParameter(p, v); } catch (...) {}
    };
    auto setT = [&](const char* t, const char* p, float v) {
        if (hasT(t)) setP(firstT(t), p, v);
    };
    // a segunda instância de um tipo (a cópia do par), ou N se não há
    auto copyOf = [&](const char* t) -> std::size_t {
        bool seen = false;
        for (std::size_t i = 0; i < N; ++i)
            if (typ(i) == t) { if (seen) return i; seen = true; }
        return N;
    };

    // ---- classificação de portas -----------------------------------
    // Movida pra `SeedGrammar.hpp` (nomeada, documentada, mapeada pro
    // vocabulário SOURCE/TRANSFORM/CONTROL/MODULATE/FEEDBACK/OUTPUT da
    // conversa de origem — ver o comentário do topo daquele arquivo).
    // Refactor comprovado: mesma lógica, byte a byte, nenhum seed muda.
    using Port = SeedPort;
    std::vector<Port> src[6];      // por classe SeedSrc
    std::vector<Port> dst[5];      // por classe SeedDst
    std::vector<std::vector<char>> inUsed(N);
    std::vector<char> spine(N, 0);
    for (std::size_t n = 0; n < N; ++n)
        inUsed[n].assign(g.node(n).inputCount(), 0);
    seedClassifyPorts(g, src, dst);

    // ---- matriz de compatibilidade + bônus experimental -----------
    float W[6][5];
    seedCompatibilityMatrix(id.wildness, W);

    auto pOut = [&](std::size_t n, const char* nm) -> int {
        Signal& node = g.node(n);
        for (std::size_t i = 0; i < node.outputCount(); ++i)
            if (node.outputDescriptor(i).name == nm) return static_cast<int>(i);
        return -1;
    };
    auto pIn = [&](std::size_t n, const char* nm) -> int {
        Signal& node = g.node(n);
        for (std::size_t i = 0; i < node.inputCount(); ++i)
            if (node.inputDescriptor(i).name == nm) return static_cast<int>(i);
        return -1;
    };
    // caminho a -> ... -> b pelos cabos não-feedback?
    auto reaches = [&](std::size_t a, std::size_t b) -> bool {
        std::vector<char> seen(N, 0);
        std::vector<std::size_t> st{a};
        while (!st.empty()) {
            const std::size_t x = st.back(); st.pop_back();
            if (x == b) return true;
            if (x < N) { if (seen[x]) continue; seen[x] = 1; }
            for (std::size_t i = 0; i < g.cableCount(); ++i) {
                const auto& c = g.cable(i);
                if (!c.isFeedback() && c.source().node == x)
                    st.push_back(c.target().node);
            }
        }
        return false;
    };
    auto connect = [&](Port s, Port d) -> bool {
        if (d.node < N && d.port < inUsed[d.node].size()
            && inUsed[d.node][d.port]) return false;
        const bool fb = reaches(d.node, s.node);
        try {
            g.connect(s.node, s.port, d.node, d.port, fb);
        } catch (const std::logic_error&) {
            try { g.connect(s.node, s.port, d.node, d.port, true); }
            catch (...) { return false; }
        } catch (...) { return false; }
        if (d.node < N && d.port < inUsed[d.node].size()) inUsed[d.node][d.port] = 1;
        return true;
    };
    auto connectByName = [&](const char* st, const char* sp,
                             const char* dt, const char* dp) -> bool {
        if (!hasT(st) || !hasT(dt)) return false;
        const int so = pOut(firstT(st), sp), di = pIn(firstT(dt), dp);
        if (so < 0 || di < 0) return false;
        return connect({firstT(st), static_cast<std::size_t>(so)},
                       {firstT(dt), static_cast<std::size_t>(di)});
    };
    // liga por NÓ (para as cópias, que `connectByName` não alcança)
    auto connectIds = [&](std::size_t sn, const char* sp, std::size_t dn,
                          const char* dp) -> bool {
        if (sn >= N || dn >= N) return false;
        const int so = pOut(sn, sp), di = pIn(dn, dp);
        if (so < 0 || di < 0) return false;
        return connect({sn, static_cast<std::size_t>(so)},
                       {dn, static_cast<std::size_t>(di)});
    };

    // ---- rack de fábrica: a seed é a identidade do patch ---------
    // (9/11/2026) Toda seed parte do rack de FÁBRICA — cada parâmetro de
    // cada módulo volta ao seu valor padrão antes de compor. Antes, a seed
    // só redefinia o que ela mesma sorteava, e o resto passava de uma seed
    // para a outra: em 40 seeds clicadas em sequência, 39 saíam diferentes
    // da mesma seed num rack novo, e o `out_gain` do MIXER (posto pelo
    // equilíbrio de volume) subia de clique em clique até +12 dB.
    // Num rack novo isto não muda nada (os valores já são os padrões),
    // então nenhuma seed muda de som; só deixa de herdar o clique anterior.
    // Ficam de fora só as escolhas do MÚSICO que nenhuma seed sorteia: o
    // MUTE do MASTER (proteção dos ouvidos) e o ganho/monitor da entrada
    // de áudio (SIGNAL-IN gain/listen, ajuste do dispositivo).
    // `test_seed_identity.cpp` fixa o invariante.
    // As modulações de parâmetro (as da DRIFT, abaixo) também são do patch
    // anterior: sem limpar, elas se somavam clique a clique — o patch salvo
    // do autor tinha dezenas empilhadas no cutoff e na ressonância.
    g.clearModulations();
    for (std::size_t n = 0; n < N; ++n) {
        Signal& node = g.node(n);
        const std::string t = node.type();
        for (const auto& pr : node.parameters()) {
            const std::string& pid = pr.descriptor.id;
            if (t == "MASTER" && pid == "mute") continue;
            if (t == "SIGNAL-IN" && (pid == "gain" || pid == "listen")) continue;
            if (pr.value != pr.descriptor.defaultValue)
                setP(n, pid.c_str(), pr.descriptor.defaultValue);
        }
    }

    // ---- limpa tudo ---------------------------------------------
    for (std::size_t i = g.cableCount(); i-- > 0;) {
        const auto tg = g.cable(i).target();
        g.disconnect(tg.node, tg.port);
    }

    // ================ CLOCK sempre ================================
    setT("CLOCK", "bpm", id.bpm);
    setT("CLOCK", "mult", static_cast<float>(id.mult));
    setT("CLOCK", "fill", rng(4.0f, 15.0f));
    setT("CLOCK", "length", 16.0f);
    setT("CLOCK", "rotate", static_cast<float>(rnd() % 5 == 0 ? (rnd() % 5) : 0));
    setT("CLOCK", "swing", chance(0.4f) ? rng(0.0f, 0.2f) : 0.0f);
    setT("CLOCK", "drift", rng(0.0f, 0.2f));
    setT("CLOCK", "gate_len", rng(0.2f, 0.7f));

    // ================ ESPINHA (garante audível) ===================
    // fonte de altura
    setT("QUANTIZER", "scale", static_cast<float>(id.scale));
    setT("QUANTIZER", "root", static_cast<float>(id.root));
    // faixa da melodia: 1..2 oitavas. Era 1..3,5 — 3,5 oitavas a partir
    // de uma base de ~300–500 Hz levava o OSC a ~5–6 kHz nas notas altas:
    // o "assobio de oscilador" que atormentava.
    setT("QUANTIZER", "range", rng(1.0f, 2.0f));
    setT("QUANTIZER", "glide", chance(0.4f) ? rng(0.0f, 0.25f) : 0.0f);
    connectByName("CLOCK", "euclid", "QUANTIZER", "trigger");
    bool pitched = false;
    if (hasT("SEQUENCE") && chance(0.6f)) {
        setT("SEQUENCE", "mode", static_cast<float>(rnd() % 5));
        setT("SEQUENCE", "length", static_cast<float>(pick4(3, 4, 6, 8)));
        connectByName("CLOCK", "clock", "SEQUENCE", "clock");
        pitched = connectByName("SEQUENCE", "pitch", "QUANTIZER", "cv");
    }
    if (!pitched && hasT("TURING")) {
        setT("TURING", "lock", rng(0.2f, 0.85f));
        setT("TURING", "length", static_cast<float>(pick4(6, 8, 12, 16)));
        connectByName("CLOCK", "clock", "TURING", "clock");
        pitched = connectByName("TURING", "cv", "QUANTIZER", "cv");
    }
    (void)pitched;

    // voz — segundo o voiceBias (com folga pra outras)
    struct VC { const char* mod; const char* out; };
    const VC voiceOpts[7][3] = {
        {{"OSC", "saw"}, {"OSC", "pulse"}, {"OSC", "tri"}},
        {{"MATTER", "out"}, {"STRING", "out"}, {"OSC", "tri"}},
        {{"CHORD", "out"}, {"CHORD", "out"}, {"OSC", "saw"}},
        // ruído SEMPRE rosa/marrom — branco cru como voz é chiado de
        // banda cheia (~55% da energia acima de 10 kHz), o "hiper agudo
        // que se repete" reportado pelo autor; e sempre passa por um LP
        // depois (garantia mais abaixo).
        {{"NOISE", "pink"}, {"NOISE", "brown"}, {"NOISE", "pink"}},
        {{"MEMORY", "out"}, {"OSC", "tri"}, {"NOISE", "pink"}},
        {{"SHAPE", "out"}, {"OSC", "tri"}, {"OSC", "sub"}},
        {{"FILTER", "all"}, {"OSC", "saw"}, {"MATTER", "out"}},   // feedback filter
    };
    VC vc = voiceOpts[id.voiceBias][rnd() % 3];
    if (!hasT(vc.mod)) vc = {"OSC", "saw"};
    std::size_t voiceNode = firstT(vc.mod);
    const char* voiceOutNm = vc.out;
    float voiceHz = 0.0f;   // base de altura da voz (0 = sem altura: ruído)

    // parâmetros + excitação da voz
    if (std::string(vc.mod) == "OSC") {
        // registro variado: ~70% grave, ~30% médio. Teto do médio em
        // 340 Hz — com +2 oitavas do QUANTIZER a nota mais alta chega a
        // ~1,4 kHz (musical), não a ~5 kHz.
        voiceHz = chance(0.7f) ? rng(38.0f, 175.0f) : rng(175.0f, 340.0f);
        setP(voiceNode, "freq", voiceHz);
        setP(voiceNode, "pw", rng(0.25f, 0.75f));   // não vira trem de picos
        setP(voiceNode, "drift", rng(0.0f, 0.4f));
        setP(voiceNode, "sub_div", chance(0.4f) ? 4.0f : 2.0f);   // = a antiga chave SUB2
        if (pIn(voiceNode, "pitch") >= 0)
            connectByName("QUANTIZER", "pitch", "OSC", "pitch");
    } else if (std::string(vc.mod) == "CHORD") {
        voiceHz = rng(50.0f, 180.0f);
        setP(voiceNode, "freq", voiceHz);
        setP(voiceNode, "chord", rng(0.0f, 1.0f));
        setP(voiceNode, "voices", static_cast<float>(2 + rnd() % 3));
        setP(voiceNode, "detune", rng(0.0f, 0.5f));
        setP(voiceNode, "wave", rng(0.0f, 1.0f));
        connectByName("QUANTIZER", "pitch", "CHORD", "pitch");
    } else if (std::string(vc.mod) == "MATTER" || std::string(vc.mod) == "STRING") {
        voiceHz = rng(60.0f, 260.0f);
        setP(voiceNode, "freq", voiceHz);
        setP(voiceNode, "exciter", rng(0.4f, 0.95f));
        setP(voiceNode, "damping", rng(0.05f, 0.6f));
        if (std::string(vc.mod) == "MATTER")
            setP(voiceNode, "structure", rng(0.0f, 0.9f));
        else setP(voiceNode, "decay", rng(0.55f, 0.97f));
        if (pIn(voiceNode, "freq_mod") >= 0)
            connectByName("QUANTIZER", "pitch", vc.mod, "freq_mod");
        connectByName("CLOCK", "euclid", vc.mod, vc.mod == std::string("MATTER")
            ? "strike" : "pluck");
        // fio de arco pra nunca ficar mudo
        if (hasT("NOISE")) {
            setT("NOISE", "rate", rng(30.0f, 300.0f));
            const int bi = pIn(voiceNode, "in");
            if (bi >= 0) {
                // via VCA canal 2 atenuado
                if (hasT("VCA")) {
                    setT("VCA", "level2", rng(0.06f, 0.16f));
                    connectByName("NOISE", "brown", "VCA", "in2");
                    connect({firstT("VCA"), static_cast<std::size_t>(pOut(firstT("VCA"), "out2"))},
                            {voiceNode, static_cast<std::size_t>(bi)});
                }
            }
        }
    } else if (std::string(vc.mod) == "NOISE") {
        setP(voiceNode, "rate", rng(1.0f, 500.0f));   // taxa alta = chiado
        setP(voiceNode, "spread", rng(0.0f, 0.7f));
        setP(voiceNode, "slew", rng(0.0f, 0.6f));
    } else if (std::string(vc.mod) == "MEMORY") {
        setP(voiceNode, "grain", rng(0.03f, 0.35f));
        setP(voiceNode, "density", rng(4.0f, 40.0f));
        setP(voiceNode, "spray", rng(0.0f, 0.5f));
        setP(voiceNode, "feedback", rng(0.0f, 0.7f));
        setP(voiceNode, "blend", rng(0.5f, 1.0f));
        if (hasT("OSC")) { voiceHz = rng(50.0f, 200.0f); setT("OSC", "freq", voiceHz);
            connectByName("QUANTIZER", "pitch", "OSC", "pitch");
            connectByName("OSC", "saw", "MEMORY", "in"); }
    } else if (std::string(vc.mod) == "SHAPE") {
        setP(voiceNode, "fold", rng(0.2f, 0.9f));
        setP(voiceNode, "symmetry", rng(-0.8f, 0.8f));
        setP(voiceNode, "sat", rng(0.0f, 0.5f));
        setP(voiceNode, "level", 0.9f);
        if (hasT("OSC")) { voiceHz = rng(50.0f, 180.0f); setT("OSC", "freq", voiceHz);
            connectByName("QUANTIZER", "pitch", "OSC", "pitch");
            connectByName("OSC", "tri", "SHAPE", "in");
            if (chance(0.5f)) connectByName("OSC", "sub", "SHAPE", "mod"); }
    } else if (std::string(vc.mod) == "FILTER") {   // feedback / auto-osc
        // ressonante e com CARÁTER, sem ser um seno puro perfurante fixo
        // em ~2,6 kHz (era `resonance 0.88..0.99` + cutoff travado em
        // 2600 — o "hiper agudo que se repete"). Base de corte variada e
        // geralmente mais baixa; a melodia do QUANTIZER move junto.
        setP(voiceNode, "resonance", rng(0.55f, 0.9f));
        setP(voiceNode, "drive", rng(0.1f, 0.6f));
        if (hasT("NOISE")) { setT("NOISE", "rate", rng(0.5f, 8.0f));
            connectByName("NOISE", "brown", "FILTER", "in"); }
        const float fcBase = rng(110.0f, 900.0f);
        const float fcDepth = rng(200.0f, 1800.0f);
        try { g.connectToParameter(firstT("QUANTIZER"),
            static_cast<std::size_t>(pOut(firstT("QUANTIZER"), "pitch")),
            firstT("FILTER"), "cutoff", fcDepth, fcBase, true); } catch (...) {}
    }

    // a VOZ é sacrossanta: o passeio não mexe nas entradas dela (senão
    // FM/sync/mod aleatório emudece o timbre). Sobram 79 outras entradas.
    spine[voiceNode] = 1;
    for (std::size_t p = 0; p < g.node(voiceNode).inputCount(); ++p)
        inUsed[voiceNode][p] = 1;

    // cadeia de processamento: 0-3 bus processors em série (por complexity)
    std::size_t bodyNode = voiceNode;
    const char* bodyOut = voiceOutNm;

    // ---- PAR DE OSC (11/11/2026, instâncias) ---------------------------
    // Às vezes a voz OSC ganha a cópia do OSC: levemente desafinada (a
    // espessura do batimento) ou num intervalo (oitava, quinta) — somadas
    // no VCA de cópia antes da cadeia. Por que vários VCOs: o autor
    // perguntou em 10/11 ("porque muitos synth possuem vários vcos?").
    bool rawPair = false;
    if (makeCopy && std::string(vc.mod) == "OSC" && voiceHz > 0.0f
        && chance(0.25f + 0.3f * id.complexity)) {
        const std::size_t o2 = copyOf("OSC"), mx2 = copyOf("VCA");
        if (o2 < N && mx2 < N) {
            float factor;
            if (chance(0.6f)) {                         // espessura
                const float cents = rng(3.0f, 12.0f) * (chance(0.5f) ? 1.0f : -1.0f);
                factor = std::exp2(cents / 1200.0f);
            } else {                                    // intervalo
                const float v[3] = {0.5f, 2.0f, 1.5f};
                factor = v[rnd() % 3];
            }
            setP(o2, "freq", std::clamp(voiceHz * factor, 20.0f, 700.0f));
            setP(o2, "pw", rng(0.25f, 0.75f));
            setP(o2, "drift", rng(0.0f, 0.3f));
            setP(mx2, "level1", 0.6f);
            setP(mx2, "level2", factor > 0.9f && factor < 1.1f ? 0.6f : 0.4f);
            const bool ok = connectIds(firstT("QUANTIZER"), "pitch", o2, "pitch")
                && connectIds(voiceNode, voiceOutNm, mx2, "in1")
                && connectIds(o2, voiceOutNm, mx2, "in2");
            if (ok) {
                bodyNode = mx2; bodyOut = "sum"; rawPair = true;
                spine[o2] = 1; spine[mx2] = 1;
            }
        }
    }
    // DYNAMICS desde 9/11 (decisão do autor; as seeds anteriores mudaram);
    // SPRING e PRISM também em 9/11 — processadores de áudio só entram no
    // patch pela cadeia; o passeio quase nunca os alcança (2,7 % das seeds).
    const char* procs[8] = {"FILTER", "SHAPE", "LPG", "PARAMETRIC", "SPACE", "DYNAMICS",
                            "SPRING", "PRISM"};
    const int nProc = static_cast<int>(std::lround(id.complexity * 3.0f))
        + (chance(0.4f) ? 1 : 0);
    for (int pi = 0; pi < nProc; ++pi) {
        const char* pr = procs[rnd() % 8];
        if (!hasT(pr) || firstT(pr) == bodyNode) continue;
        const int ai = pIn(firstT(pr), "in");
        const int so = pOut(bodyNode, bodyOut);
        if (ai < 0 || so < 0) continue;
        if (!connect({bodyNode, static_cast<std::size_t>(so)},
                     {firstT(pr), static_cast<std::size_t>(ai)})) continue;
        bodyNode = firstT(pr);
        spine[bodyNode] = 1;
        if (std::string(pr) == "FILTER") {
            setP(bodyNode, "cutoff", rng(150.0f, 6000.0f));
            setP(bodyNode, "resonance", rng(0.05f, 0.85f));
            setP(bodyNode, "spread", rng(0.0f, 0.5f));
            setP(bodyNode, "drive", chance(0.3f) ? rng(0.1f, 0.6f) : 0.0f);
            bodyOut = pickS5("all", "all", "low", "center", "high");
        } else if (std::string(pr) == "SHAPE") {
            setP(bodyNode, "fold", rng(0.1f, 0.8f));
            setP(bodyNode, "symmetry", rng(-0.6f, 0.6f));
            setP(bodyNode, "wrap", chance(0.3f) ? rng(0.0f, 0.6f) : 0.0f);
            setP(bodyNode, "sat", rng(0.0f, 0.5f));
            setP(bodyNode, "level", 0.9f);
            bodyOut = "out";
        } else if (std::string(pr) == "LPG") {
            setP(bodyNode, "mode", rng(0.0f, 1.0f));
            setP(bodyNode, "response", rng(0.05f, 0.6f));
            setP(bodyNode, "resonance", rng(0.0f, 0.6f));
            setP(bodyNode, "offset", chance(0.5f) ? rng(0.0f, 0.3f) : 0.0f);
            connectByName("CLOCK", "euclid", "LPG", "strike");
            bodyOut = "out";
        } else if (std::string(pr) == "PARAMETRIC") {
            setP(bodyNode, "type2", 3.0f);
            setP(bodyNode, "freq2", rng(180.0f, 2200.0f));
            // realce contido: Q até 2,5 e ganho até +6 dB. Q 5 + +9 dB
            // era um pico-agulha ressonante que "assobiava" em cada nota.
            setP(bodyNode, "gain2", rng(-6.0f, 6.0f));
            setP(bodyNode, "q2", rng(0.6f, 2.5f));
            setP(bodyNode, "mix", 1.0f);
            bodyOut = "out";
        } else if (std::string(pr) == "DYNAMICS") {
            // compressão musical, nunca um susto: limiar −30..−12 dB,
            // ratio 2–6, ataque 2–30 ms, release 60–400 ms; o GAIN devolve
            // só parte do que a curva tira (nunca mais que ela abaixa).
            const float thr = rng(-30.0f, -12.0f);
            const float ratio = rng(2.0f, 6.0f);
            setP(bodyNode, "thresh", thr);
            setP(bodyNode, "ratio", ratio);
            setP(bodyNode, "attack", rng(2.0f, 30.0f));
            setP(bodyNode, "release", rng(60.0f, 400.0f));
            setP(bodyNode, "knee", rng(3.0f, 10.0f));
            setP(bodyNode, "makeup", std::min(12.0f, -thr * (1.0f - 1.0f / ratio) * 0.35f));
            setP(bodyNode, "detect", chance(0.5f) ? 1.0f : 0.0f);
            setP(bodyNode, "breath", chance(0.35f) ? rng(0.3f, 1.0f) : 0.0f);
            setP(bodyNode, "drift", chance(0.3f) ? rng(0.1f, 0.6f) : 0.0f);
            // às vezes o bumbo abaixa a voz: o pad que respira
            if (hasT("DRUM") && chance(0.35f)
                && connectByName("DRUM", "out", "DYNAMICS", "side"))
                setP(bodyNode, "side_mix", 1.0f);
            else
                setP(bodyNode, "side_mix", 0.0f);
            bodyOut = "out";
        } else if (std::string(pr) == "SPRING") {
            // a mola como espaço: cauda média, gotejamento presente,
            // mistura contida — nunca o "boing" no máximo por sorteio
            setP(bodyNode, "decay", rng(0.6f, 0.92f));
            setP(bodyNode, "tension", rng(0.2f, 0.8f));
            setP(bodyNode, "damp", rng(0.2f, 0.7f));
            setP(bodyNode, "drip", rng(0.3f, 0.8f));
            setP(bodyNode, "mix", rng(0.2f, 0.5f));
            bodyOut = "out";
        } else if (std::string(pr) == "PRISM") {
            // dispersão audível sem desfazer a nota: atraso por banda
            // até 250 ms, realimentação contida
            setP(bodyNode, "disp", rng(20.0f, 250.0f));
            setP(bodyNode, "curve", rng(-1.0f, 1.0f));
            setP(bodyNode, "spread", rng(0.3f, 0.9f));
            setP(bodyNode, "feedback", rng(0.0f, 0.5f));
            setP(bodyNode, "mix", rng(0.3f, 0.7f));
            bodyOut = "out";
        } else {  // SPACE
            setP(bodyNode, "time", rng(0.02f, 1.0f));
            setP(bodyNode, "feedback", rng(0.1f, 0.8f));
            setP(bodyNode, "diffusion", rng(0.3f, 0.95f));
            setP(bodyNode, "mix", rng(0.3f, 0.8f));
            bodyOut = "out";
        }
    }

    // REDE DE SEGURANÇA — voz brilhante que NÃO passou por nenhum
    // processador (nProc=0, comum nos seeds minimalistas) ganha UM
    // passa-baixa musical à força. Sem isto, um ruído rosa cru ou uma
    // serra crua vão direto pro MIXER — o "hiper agudo" que se repetia.
    {
        const std::string vt = typ(voiceNode);
        const bool brightRaw = rawPair || (bodyNode == voiceNode
            && (vt == "NOISE" || vt == "OSC" || vt == "SHAPE"
                || vt == "FILTER"));
        if (brightRaw && hasT("FILTER") && firstT("FILTER") != voiceNode) {
            const std::size_t flt = firstT("FILTER");
            const int ai = pIn(flt, "in");
            const int so = pOut(bodyNode, bodyOut);
            if (ai >= 0 && so >= 0
                && connect({bodyNode, static_cast<std::size_t>(so)},
                           {flt, static_cast<std::size_t>(ai)})) {
                setP(flt, "cutoff", rng(400.0f, 3200.0f));
                setP(flt, "resonance", rng(0.05f, 0.4f));
                bodyNode = flt;
                bodyOut = "all";
                spine[flt] = 1;
            }
        }
    }

    // ---- ENVELOPE DO FILTRO (11/11/2026, instâncias) -------------------
    // Quando o FILTER está na cadeia da voz, às vezes a cópia do ENVELOPE o
    // abre e fecha a cada nota — o contorno de filtro separado do de
    // volume, a gramática do baixo (pesquisa do grave, 10/11).
    if (makeCopy && hasT("FILTER") && firstT("FILTER") != voiceNode
        && spine[firstT("FILTER")] && chance(0.3f + 0.3f * id.complexity)) {
        const std::size_t e2 = copyOf("ENVELOPE"), flt = firstT("FILTER");
        if (e2 < N) {
            setP(e2, "mode", 1.0f);
            setP(e2, "attack", rng(0.002f, 0.02f));
            setP(e2, "decay", rng(0.08f, 0.6f));
            setP(e2, "sustain", 0.0f);
            setP(e2, "release", rng(0.05f, 0.3f));
            setP(e2, "level", 1.0f);
            if (connectIds(firstT("CLOCK"), "euclid", e2, "gate")
                && connectIds(e2, "env", flt, "cutoff_mod")) {
                // 1–2 oitavas de abertura (ganho do cabo) a partir de um
                // corte mais baixo: o envelope é quem abre
                g.cable(g.cableCount() - 1).setGain(rng(1.0f, 2.0f));
                setP(flt, "cutoff", rng(150.0f, 700.0f));
                spine[e2] = 1;
            }
        }
    }

    // dinâmica: ENVELOPE gatilhado (se ainda não passou por LPG e a voz
    // não é um ressoador com decay próprio)
    const bool bodyIsLpg = typ(bodyNode) == "LPG";
    const bool voiceRings = typ(voiceNode) == "MATTER" || typ(voiceNode) == "STRING";
    if (hasT("ENVELOPE") && !bodyIsLpg && (!voiceRings || chance(0.4f))) {
        std::size_t e = firstT("ENVELOPE");
        setP(e, "mode", chance(0.5f) ? 0.0f : 1.0f);
        setP(e, "attack", rng(0.002f, 0.4f));
        setP(e, "decay", rng(0.05f, 1.6f));
        setP(e, "sustain", chance(0.45f) ? rng(0.1f, 0.7f) : 0.0f);
        setP(e, "release", rng(0.03f, 1.4f));
        setP(e, "curve", rng(0.2f, 0.9f));
        const int ai = pIn(e, "in"), so = pOut(bodyNode, bodyOut);
        if (ai >= 0 && so >= 0
            && connect({bodyNode, static_cast<std::size_t>(so)},
                       {e, static_cast<std::size_t>(ai)})) {
            connectByName("CLOCK", "euclid", "ENVELOPE", "gate");
            bodyNode = e; bodyOut = "out"; spine[e] = 1;
        }
    }

    for (const char* st : {"CLOCK", "QUANTIZER", "SEQUENCE", "TURING",
                           "MIXER", "MASTER", "DRIFT", "HARMONY"})
        if (hasT(st)) spine[firstT(st)] = 1;

    // ================ MIXER <- corpo + soltos ; MASTER -> sink =====
    // canal da voz mira ~−6 dBFS de barramento — folga pra as camadas,
    // os cabos do passeio e a modulação. Era rng(-2, +5): +5 dB mandava
    // o barramento pra +2..+8 dBFS e o MASTER limitava demais (o "clipe").
    const float gain1 = rng(-10.0f, -2.0f);
    setT("MIXER", "gain1", gain1);
    // pan SEMPRE no centro em todo seed (pedido do autor, 2026-09-08) — ele
    // abre no palco à mão se quiser. Era rng(-0.4, 0.4).
    setT("MIXER", "pan1", 0.0f);
    if (hasT("MIXER")) {
        const int ci = pIn(firstT("MIXER"), "ch1"), so = pOut(bodyNode, bodyOut);
        if (ci >= 0 && so >= 0)
            connect({bodyNode, static_cast<std::size_t>(so)},
                    {firstT("MIXER"), static_cast<std::size_t>(ci)});
    }
    if (hasT("MASTER") && hasT("MIXER")) {
        setT("MASTER", "width", rng(0.85f, 1.5f));
        // fixo (não sorteado) — 75% do slider (faixa -60..+12 -> -6 dB) em
        // TODO seed, sem exceção; o autor ajusta a partir daí. O limitador
        // true-peak + body-guard do MASTER (ligados por default) seguram o
        // teto em -1 dBFS. (30% / -38,4 dB até 2026-09-05; 50% / -24 dB
        // até 2026-09-10; a pedido do autor.)
        setT("MASTER", "gain", -6.0f);
        // O `out_gain` do MIXER volta a 0 dB em TODA seed (9/11/2026). O
        // equilíbrio de volume (`SeedBalance.hpp`) põe ali o que não coube
        // no MASTER; como nada o zerava, ele só subia de clique em clique e
        // travava em +12 dB — o patch do autor estava 12 LU acima do alvo
        // ("percebi que o som estava alto"). 0 é o valor do rack de prova
        // da medição, então rack vivo e prova voltam a coincidir.
        setT("MIXER", "out_gain", 0.0f);
        connectByName("MIXER", "out", "MASTER", "in");
        if (haveSink) {
            const int mo = pOut(firstT("MASTER"), "out");
            if (mo >= 0) {
                g.disconnect(sink, 0);
                try { g.connect(firstT("MASTER"), static_cast<std::size_t>(mo), sink, 0); }
                catch (...) {}
            }
        }
    }

    // ================ CONJUNTO: os quatro canais (10/11/2026) =========
    // Um papel por canal (`dossies/PROPOSTA_quatro_canais_da_seed.md`):
    //   ch1 voz (acima) · ch2 contravoz · ch3 textura (depois do passeio)
    //   · ch4 eco ou pulso. Quantos soam é o gene `ensemble`.
    // Pedido do autor: "mais seeds utilizando os quatro canais, sobretudo
    // para resultados polifônicos e mais espacialidades de pan"; condição:
    // nenhum problema técnico de qualidade de som (`test_seed_quality`).
    // Antes: 48 % das seeds com 1 canal, 0 % com 4, e nenhuma camada com
    // altura própria.
    const bool wantStage = kSeedOpensStage;
    const float side = chance(0.5f) ? 1.0f : -1.0f;   // lado da contravoz
    auto toChannel = [&](std::size_t n, const char* out, int ch,
                         float gainDb, float home) -> bool {
        if (!hasT("MIXER")) return false;
        const std::size_t mx = firstT("MIXER");
        const std::string c = std::to_string(ch);
        const int ci = pIn(mx, ("ch" + c).c_str()), so = pOut(n, out);
        if (ci < 0 || so < 0) return false;
        if (!connect({n, static_cast<std::size_t>(so)}, {mx, static_cast<std::size_t>(ci)}))
            return false;
        setP(mx, ("gain" + c).c_str(), std::max(-30.0f, gainDb));
        setP(mx, ("pan" + c).c_str(), wantStage ? home : 0.0f);
        return true;
    };
    auto freeNode = [&](const char* t) { return hasT(t) && !spine[firstT(t)]; };
    int voices = 1;

    // ---- ch2 CONTRAVOZ: segunda linha com altura própria -----------
    // As notas vêm da voz: ARP (raiz = a nota da voz → acorde em torno da
    // melodia) ou MARKOV (aprende as transições da voz e improvisa na
    // mesma língua). Num relógio próprio, mais lento quando há mais vozes.
    // Registro: uma oitava acima ou abaixo da voz, para não se mascararem;
    // e nunca acima de 330 Hz de base (com a melodia e o arpejo por cima,
    // a nota mais alta fica em ~2,6 kHz, não no assobio).
    if (id.ensemble >= 2 && hasT("QUANTIZER")) {
        const bool up = voiceHz <= 0.0f || voiceHz * 2.0f <= 330.0f;
        // ARP abaixo só com a voz alta o bastante para duas oitavas
        // abaixo (o arpejo sobe até uma oitava a partir da raiz)
        const bool arpOk = freeNode("ARP") && (up || voiceHz >= 220.0f);
        const bool mkOk = freeNode("MARKOV") && (up || voiceHz >= 110.0f);
        const bool useArp = arpOk && (!mkOk || chance(0.5f));
        float singerHz = voiceHz <= 0.0f ? rng(90.0f, 220.0f)
                       : up ? voiceHz * 2.0f
                       : voiceHz * (useArp ? 0.25f : 0.5f);
        singerHz = std::clamp(singerHz, 55.0f, 330.0f);

        // quem canta: STRING/MATTER (golpe próprio, corpo e decaimento)
        // ou um oscilador → LPG (o LPG é o envelope e o filtro)
        struct SG { const char* mod; const char* pitchIn; const char* trigIn; bool lpg; };
        // (11/11/2026, instâncias) a contravoz também pode cantar num OSC só
        // dela, e usa a CÓPIA de STRING/MATTER/LPG/OSC quando a voz já
        // ocupa a primeira instância — "a contravoz também é importante"
        const SG opts[7] = {{"STRING", "freq_mod", "pluck", false},
                            {"MATTER", "freq_mod", "strike", false},
                            {"PULSAR", "pitch", nullptr, true},
                            {"WAVETABLE", "pitch", nullptr, true},
                            {"ADDITIVE", "pitch", nullptr, true},
                            {"OPERATOR", "pitch", nullptr, true},
                            {"OSC", "pitch", nullptr, true}};
        // o nó livre de um tipo: a primeira instância, ou a cópia
        auto nodeFor = [&](const char* t) -> std::size_t {
            if (hasT(t) && !spine[firstT(t)] && firstT(t) != voiceNode) return firstT(t);
            const std::size_t c = makeCopy ? copyOf(t) : N;
            return (c < N && !spine[c]) ? c : N;
        };
        const SG* sg = nullptr;
        std::size_t sn = N, lpN = N;
        for (int tries = 0; tries < 10 && !sg; ++tries) {
            const SG& o = opts[rnd() % 7];
            const std::size_t cand = nodeFor(o.mod);
            if (cand >= N) continue;
            const std::size_t lp = o.lpg ? nodeFor("LPG") : N;
            if (o.lpg && lp >= N) continue;
            sg = &o; sn = cand; lpN = lp;
        }
        const char* gen = useArp ? "ARP" : "MARKOV";
        if (sg && (arpOk || mkOk)) {
            const std::size_t gn = firstT(gen);
            // relógio: CLOCK dividido pela LOGIC; a contravoz não passa
            // de ~4 notas/s (2,5 com 3–4 vozes)
            const float clockHz = id.bpm / 60.0f * static_cast<float>(id.mult);
            const float maxHz = id.ensemble >= 3 ? 2.5f : 4.0f;
            int div = id.ensemble >= 3 ? pick4(2, 3, 4, 4) : pick4(1, 2, 2, 3);
            div = std::clamp(std::max(div, static_cast<int>(std::ceil(clockHz / maxHz))), 1, 32);
            bool clocked = false;
            if (freeNode("LOGIC")) {
                setT("LOGIC", "divide", static_cast<float>(div));
                setT("LOGIC", "gate_len", rng(0.3f, 0.7f));
                clocked = connectByName("CLOCK", "clock", "LOGIC", "clock")
                       && connectByName("LOGIC", "div", gen, "clock");
                if (clocked) spine[firstT("LOGIC")] = 1;
            }
            setP(gn, "rate", std::clamp(clockHz / static_cast<float>(div), 0.2f, maxHz));
            setP(gn, "gate_len", rng(0.3f, 0.75f));
            if (useArp) {
                setP(gn, "chord", static_cast<float>(rnd() % 10));
                setP(gn, "inversion", static_cast<float>(rnd() % 3));
                setP(gn, "octaves", 1.0f);
                setP(gn, "mode", static_cast<float>(rnd() % 8));
                setP(gn, "skip", chance(0.4f) ? rng(0.1f, 0.35f) : 0.0f);
                connectByName("QUANTIZER", "pitch", "ARP", "root");
            } else {
                setP(gn, "temp", rng(0.5f, 1.4f));
                setP(gn, "memory", rng(0.5f, 0.95f));
                setP(gn, "learn", 1.0f);
                setP(gn, "octave", 0.0f);
                connectByName("QUANTIZER", "pitch", "MARKOV", "learn_pitch");
                connectByName("QUANTIZER", "gate", "MARKOV", "learn_gate");
            }
            spine[gn] = 1;

            std::size_t outNode = sn; const char* outNm = "out";
            setP(sn, "freq", singerHz);
            bool sung = connectIds(gn, "pitch", sn, sg->pitchIn);
            if (!sg->lpg) {
                if (std::string(sg->mod) == "STRING") {
                    setP(sn, "decay", rng(0.5f, 0.9f));
                    setP(sn, "damping", rng(0.2f, 0.65f));
                } else {
                    setP(sn, "structure", rng(0.0f, 0.8f));
                    setP(sn, "brightness", rng(0.25f, 0.6f));
                    setP(sn, "damping", rng(0.25f, 0.65f));
                }
                setP(sn, "exciter", rng(0.4f, 0.85f));
                sung = sung && connectIds(gn, "gate", sn, sg->trigIn);
            } else {
                const std::size_t lp = lpN;
                setP(lp, "mode", rng(0.3f, 1.0f));
                setP(lp, "response", rng(0.1f, 0.45f));
                setP(lp, "resonance", rng(0.0f, 0.35f));
                setP(lp, "offset", 0.0f);
                const std::string m = sg->mod;
                if (m == "PULSAR") { setP(sn, "formant", rng(0.8f, 3.0f)); setP(sn, "level", 0.7f); }
                if (m == "OPERATOR") setP(sn, "index", rng(0.05f, 0.4f));
                if (m == "ADDITIVE") setP(sn, "tilt", rng(0.2f, 0.6f));
                if (m == "WAVETABLE") setP(sn, "pos", rng(0.0f, 1.0f));
                const char* sOut = m == "OSC" ? (chance(0.5f) ? "saw" : "tri") : "out";
                sung = sung && connectIds(sn, sOut, lp, "in")
                            && connectIds(gn, "gate", lp, "strike");
                if (sung) { spine[lp] = 1; outNode = lp; }
            }
            if (sung && toChannel(outNode, outNm, 2, gain1 - rng(4.0f, 8.0f), 0.30f * side)) {
                spine[sn] = 1;
                ++voices;
            }
        }
    }

    // ---- ch4 ECO (o corpo da voz num lugar) ou PULSO (percussão) -------
    if (id.ensemble >= 4) {
        bool done = false;
        if (chance(0.6f)) {
            struct EC { const char* mod; const char* wet; };
            const EC ecs[5] = {{"SPRING", "wet"}, {"PRISM", "wet"}, {"SPACE", "wet"},
                               {"LOOPER", "wet"}, {"HALL", "l"}};
            // um espaço que já está na cadeia da voz dá a sua cauda
            // separada; senão, um espaço livre ouve o corpo da voz
            for (const EC& e : ecs)
                if (!done && hasT(e.mod) && spine[firstT(e.mod)] && firstT(e.mod) != voiceNode
                    && toChannel(firstT(e.mod), e.wet, 4, gain1 - rng(3.0f, 8.0f), -0.40f * side))
                    done = true;
            for (int tries = 0; tries < 6 && !done; ++tries) {
                const EC& e = ecs[rnd() % 5];
                if (!freeNode(e.mod)) continue;
                const std::size_t en = firstT(e.mod);
                const int ai = pIn(en, "in"), so = pOut(bodyNode, bodyOut);
                if (ai < 0 || so < 0) continue;
                if (!connect({bodyNode, static_cast<std::size_t>(so)}, {en, static_cast<std::size_t>(ai)}))
                    continue;
                const std::string m = e.mod;
                if (m == "SPRING") { setP(en, "decay", rng(0.6f, 0.9f)); setP(en, "damp", rng(0.3f, 0.7f));
                                     setP(en, "drip", rng(0.2f, 0.6f)); }
                if (m == "PRISM") { setP(en, "disp", rng(40.0f, 250.0f)); setP(en, "feedback", rng(0.0f, 0.4f)); }
                if (m == "SPACE") { setP(en, "time", rng(0.15f, 0.9f)); setP(en, "feedback", rng(0.2f, 0.6f));
                                    setP(en, "diffusion", rng(0.5f, 0.95f)); }
                if (m == "LOOPER") { setP(en, "time", rng(0.2f, 1.2f)); setP(en, "feedback", rng(0.2f, 0.6f));
                                     setP(en, "age", rng(0.2f, 0.6f)); }
                if (m == "HALL") { setP(en, "size", rng(0.4f, 0.9f)); setP(en, "decay", rng(0.4f, 0.8f));
                                   setP(en, "damp", rng(0.3f, 0.7f)); setP(en, "mix", 1.0f); }
                spine[en] = 1;
                done = toChannel(en, e.wet, 4, gain1 - rng(3.0f, 8.0f), -0.40f * side);
            }
        }
        if (!done && freeNode("DRUM")) {
            const std::size_t dn = firstT("DRUM");
            setP(dn, "tone", rng(40.0f, 140.0f));
            setP(dn, "bend", rng(0.2f, 0.7f));
            setP(dn, "decay", rng(0.15f, 0.55f));
            setP(dn, "snap", rng(0.05f, 0.4f));
            setP(dn, "drive", chance(0.3f) ? rng(0.0f, 0.3f) : 0.0f);
            if (connectByName("CLOCK", chance(0.5f) ? "accent" : "euclid", "DRUM", "gate")) {
                spine[dn] = 1;
                done = toChannel(dn, "out", 4, rng(-20.0f, -12.0f), -0.40f * side);
            }
        }
        if (done) ++voices;
    }

    // ================ PASSEIO PONDERADO (o coração) ================
    // teto por passeio: mínimo 3 (inalterado); topo ×75 — o autor pediu
    // patches bem densos pra testar vários módulos de uma vez (2026-09-08,
    // era ×40). O laço já se auto-limita (12 tentativas por cabo, só porta
    // livre), então pedir 75 não põe 75 — põe o que couber.
    const int nCables = 3 + static_cast<int>(std::lround(id.complexity * 75.0f));
    // O passeio só cabeia em módulos que JÁ CHEGAM À SAÍDA (`feeds`) —
    // assim todo cabo tem função sonora. `feeds` CRESCE durante o passeio:
    // ligar `OSC.saw → SHAPE.mod` (SHAPE audível) faz o OSC e o que o
    // alimenta passarem a chegar à saída, abrindo novos destinos válidos.
    // Recalculado a cada cabo (O(nós+cabos), microssegundos).
    std::vector<char> feeds = haveSink ? g.nodesFeeding(sink)
                                       : std::vector<char>(N, 1);
    for (int c = 0; c < nCables; ++c) {
        if (haveSink) feeds = g.nodesFeeding(sink);
        // 1) escolhe uma classe de destino, depois um destino livre nela
        float dw[5] = {3.0f, 1.5f, 6.0f, 4.0f, 1.5f};
        // mais cedo prioriza gate/pitch/audio; mais tarde, mod
        if (c < 4) { dw[SEED_DST_GATE] = 5.0f; dw[SEED_DST_PITCH] = 3.0f; dw[SEED_DST_AUDIO] = 4.0f; }
        float dtot = 0.0f; for (float x : dw) dtot += x;
        float dr = f01() * dtot; int dc = 0;
        for (; dc < 4; ++dc) { if (dr < dw[dc]) break; dr -= dw[dc]; }
        // destino livre aleatório — SÓ em módulo que chega à saída
        Port d{0, 0}; bool okD = false;
        for (int tries = 0; tries < 16 && !okD; ++tries) {
            if (dst[dc].empty()) break;
            const Port cand = dst[dc][rnd() % dst[dc].size()];
            // MIXER/MASTER não são destino do passeio — as camadas
            // extras entram pela seção controlada abaixo (senão o passeio
            // joga fonte crua direto no barramento de saída)
            if (typ(cand.node) == "MIXER" || typ(cand.node) == "MASTER")
                continue;
            if (cand.node < feeds.size() && !feeds[cand.node])
                continue;   // destino não soa — cabo seria sem função
            if (cand.node < N && cand.port < inUsed[cand.node].size()
                && !inUsed[cand.node][cand.port]) { d = cand; okD = true; }
        }
        if (!okD) continue;
        // 2) escolhe a fonte ponderada por W[src][dc]
        float sw[6];
        for (int sc = 0; sc < 6; ++sc)
            sw[sc] = src[sc].empty() ? 0.0f : W[sc][dc];
        float stot = 0.0f; for (float x : sw) stot += x;
        if (stot <= 0.0f) continue;
        float sr2 = f01() * stot; int sc = 0;
        for (; sc < 5; ++sc) { if (sr2 < sw[sc]) break; sr2 -= sw[sc]; }
        const Port sp = src[sc][rnd() % src[sc].size()];
        if (sp.node == d.node) continue;
        // uma cópia (2ª instância) só entra no passeio se a seed já a pôs
        // num papel — senão o rack ganharia cópias por acaso (11/11/2026)
        if (copyOf(typ(sp.node).c_str()) == sp.node && !spine[sp.node]) continue;
        connect(sp, d);
    }

    // ================ camadas soltas -> mixer ch2/3 ==============
    // No máx 2 camadas extras, BEM baixas, e SÓ de fontes JÁ PROCESSADAS
    // — a saída de um FILTER/WASP/LPG/SPACE/PARAMETRIC. Um oscilador CRU
    // (OSC/PLL/CHORD/NOISE/FUNCTION/CHAOS) direto no mixer é um
    // drone/assobio brigando com a voz — o "som do PLL no canal 2"
    // reportado pelo autor (o `seedPatch` cabeava `PLL.out → MIXER.ch2` e
    // o passe genérico jogava o `PLL.freq` pra alguns kHz).
    auto isProcessedBus = [&](std::size_t n) {
        const std::string t = typ(n);
        return t == "FILTER" || t == "WASP" || t == "LPG"
            || t == "SPACE" || t == "PARAMETRIC";
    };
    // (10/11/2026) A textura é o papel do canal 3: entra com 3 ou mais
    // vozes no conjunto (antes: ch2/ch3 em metade das seeds). Com 4 vozes,
    // mais baixa — mais vozes, menos espaço para cada uma.
    if (hasT("MIXER") && id.ensemble >= 3) {
        bool layered = false;
        const float tex = id.ensemble >= 4 ? rng(-26.0f, -14.0f) : rng(-24.0f, -11.0f);
        for (int sc = 0; sc < 6 && !layered; ++sc)
            for (const Port& p : src[sc]) {
                if (layered) break;
                if (p.node == bodyNode || !isProcessedBus(p.node)) continue;
                if (copyOf(typ(p.node).c_str()) == p.node) continue;   // cópia: não
                if (reaches(p.node, sink)) continue;      // já chega à saída
                if (toChannel(p.node, g.node(p.node).outputDescriptor(p.port).name.c_str(),
                              3, tex, -0.45f * side))
                    layered = true;
            }
        if (layered) ++voices;
    }

    // ================ PALCO: o trajeto da seed (10/11/2026) ============
    // Com 2+ vozes no MIXER, a seed escolhe um trajeto (RODA, CÂNONE,
    // GESTO ou PÊNDULO) e o móbile do MIXER equilibra pelo peso do som.
    // Com 1 voz, MANUAL e centro (não há o que equilibrar).
    if (hasT("MIXER")) {
        const std::size_t mx = firstT("MIXER");
        int mode = 0;
        if (voices >= 2) {
            const float u = f01();
            mode = u < 0.30f ? 1 : u < 0.55f ? 2 : u < 0.85f ? 3 : 4;
            if (mode == 4 && !freeNode("PENDULUM")) mode = 1;
        }
        setP(mx, "stage", static_cast<float>(mode));
        setP(mx, "stage_rate", rng(0.15f, 0.65f));
        setP(mx, "stage_width", rng(0.4f, 0.9f));
        if (mode == 0) setP(mx, "pan1", 0.0f);
        if (mode == 3 && hasT("ARP") && spine[firstT("ARP")])
            connectByName("ARP", "eos", "MIXER", "phrase");   // gesto por frase
        if (mode == 4) {
            const std::size_t pn = firstT("PENDULUM");
            setP(pn, "damp", rng(0.0f, 0.2f));
            setP(pn, "speed", rng(0.1f, 0.35f));   // período ~6–20 s: o palco anda, não treme
            const bool a = connectByName("PENDULUM", "x1", "MIXER", "pan2");
            const bool b = connectByName("PENDULUM", "x2", "MIXER", chance(0.5f) ? "pan3" : "pan4");
            if (a || b) spine[pn] = 1;
            else setP(mx, "stage", 1.0f);
        }
    }

    // ================ DRIFT: o patch evolui sozinho ===============
    if (hasT("DRIFT") && id.motion > 0.1f) {
        std::size_t dn = firstT("DRIFT");
        setP(dn, "rate", 0.006f + id.motion * id.motion * 0.15f);
        setP(dn, "depth", 0.25f + id.motion * 0.6f);
        setP(dn, "momentum", rng(0.3f, 0.95f));
        setP(dn, "stride", rng(0.1f, 0.9f));
        setP(dn, "bias", rng(-0.3f, 0.3f));
        if (chance(0.5f)) connectByName("CLOCK", "euclid", "DRIFT", "advance");
        // pluga a/b/c/d em parâmetros estruturais aleatórios
        struct PT { const char* mod; const char* par; float depth; float off; };
        const PT tgts[] = {
            {"FILTER", "cutoff", 2200.0f, 700.0f},
            {"FILTER", "resonance", 0.28f, 0.35f},   // não deriva pra auto-osc
            {"SPACE", "mix", 0.4f, 0.4f},
            {"SPACE", "feedback", 0.4f, 0.45f},
            {"SHAPE", "fold", 0.45f, 0.35f},
            {"OSC", "pw", 0.4f, 0.5f},
            {"MEMORY", "position", 0.5f, 0.5f},
            {"CHORD", "detune", 0.35f, 0.3f},
            {"MATTER", "structure", 0.5f, 0.4f},
            {"QUANTIZER", "range", 0.7f, 1.6f},   // melodia não sobe demais
        };
        const int nDrift = 2 + static_cast<int>(std::lround(id.motion * 2.0f));
        const char* outs[4] = {"a", "b", "c", "d"};
        const std::vector<char> dfeeds = haveSink ? g.nodesFeeding(sink)
                                                  : std::vector<char>(N, 1);
        for (int i = 0; i < nDrift && i < 4; ++i) {
            const PT& tp = tgts[rnd() % (sizeof(tgts) / sizeof(tgts[0]))];
            if (!hasT(tp.mod)) continue;
            if (firstT(tp.mod) < dfeeds.size() && !dfeeds[firstT(tp.mod)])
                continue;   // DRIFT num módulo mudo = deriva sem função
            const int so = pOut(dn, outs[i]);
            if (so < 0) continue;
            try {
                g.connectToParameter(dn, static_cast<std::size_t>(so),
                                     firstT(tp.mod), tp.par,
                                     tp.depth * (0.3f + id.motion), tp.off, true);
            } catch (...) {}
        }
    }

    // ================ poda: nenhum cabo sem função sonora =========
    // O passeio e a DRIFT já são restritos ao subgrafo audível; isto
    // varre os cabos de MONTAGEM que ficaram órfãos — ex.: quando a voz
    // é ruído e nenhum oscilador consome a altura, `CLOCK→QUANTIZER` e
    // `TURING→QUANTIZER` não têm pra onde ir. Remover um cabo pode secar
    // outro (era o único elo audível de um ramo), então repete até
    // estabilizar (limitado pelo nº de cabos).
    if (haveSink) {
        for (std::size_t guard = 0; guard <= g.cableCount() + 1; ++guard) {
            const std::vector<char> pf = g.nodesFeeding(sink);
            bool cut = false;
            for (std::size_t i = g.cableCount(); i-- > 0;) {
                const auto& cb = g.cable(i);
                if (cb.target().node < pf.size() && !pf[cb.target().node]) {
                    g.disconnect(cb.target().node, cb.target().port);
                    cut = true;
                    break;
                }
            }
            if (!cut) break;
        }
    }

    // ================ parâmetros: todo módulo cabeado, faixa ampla =
    std::vector<char> touched(N, 0);
    for (std::size_t i = 0; i < g.cableCount(); ++i) {
        touched[g.cable(i).source().node] = 1;
        touched[g.cable(i).target().node] = 1;
    }
    for (std::size_t n = 0; n < N; ++n) {
        if (!touched[n] || spine[n]) continue;   // espinha fica intocada
        Signal& node = g.node(n);
        for (const auto& pr : node.parameters()) {
            const auto& dsc = pr.descriptor;
            const std::string nm = dsc.id;
            // pula estruturais / o que emudece fácil
            if (nm == "bpm" || nm == "mult" || nm == "length" || nm == "scale"
                || nm == "root" || nm == "mode" || nm == "sub_2"
                || nm == "sync_enable" || nm == "dc_block" || nm == "limit"
                || nm == "voices" || nm == "freeze" || nm == "mono"
                || nm == "gain" || nm == "output" || nm == "out_gain"
                || nm == "fm_amount" || nm == "listen"
                // depois da v0.1.7 — fora do sorteio (seeds antigas iguais)
                || nm == "pit_keep" || nm == "pit_oct"
                // lote do oscilador (11/11/2026): fora do sorteio genérico
                || nm == "pd" || nm == "unison" || nm == "sub_div"
                // GAIN do DYNAMICS: nível, como os outros ganhos
                || nm == "makeup")
                continue;
            if (f01() > 0.55f) continue;
            const float lo = dsc.minimum, hi = dsc.maximum;
            float v;
            // toggles: 0/1
            if (lo == 0.0f && hi == 1.0f && (nm.find("enable") != std::string::npos
                || nm == "hold" || nm == "accent_mode")) v = chance(0.5f) ? 1.0f : 0.0f;
            // parâmetros que geram agudo/aspereza forte: faixa contida
            // (sorteio de faixa CHEIA levava a auto-oscilação / folding
            // travado / EQ +24 dB — o viés de agudo reportado). Ainda
            // varre bastante, só não vai pro extremo perigoso.
            else if (nm == "resonance") v = rng(lo, lo + 0.72f * (hi - lo));
            else if (nm == "grit" || nm == "fold")
                v = rng(lo, lo + 0.6f * (hi - lo));
            else if (nm == "drive") v = rng(lo, lo + 0.65f * (hi - lo));
            else if (nm.rfind("gain", 0) == 0 && hi >= 12.0f)
                v = rng(std::max(lo, -9.0f), std::min(hi, 6.0f));
            else if (nm == "cutoff")
                v = rng(std::max(lo, 90.0f), std::min(hi, 8000.0f));
            // frequência de um oscilador NÃO-espinha cabeado como camada:
            // faixa musical, não sorteio até 8–12 kHz (era o "assobio de
            // oscilador"). `freq2..4` do PARAMETRIC são bandas de EQ,
            // outra coisa — só `freq` puro e `rate` de faixa larga.
            else if (nm == "freq")
                v = rng(std::max(lo, 40.0f), std::min(hi, 1200.0f));
            else if (nm == "rate" && hi > 200.0f)
                v = rng(lo, std::min(hi, 220.0f));
            else v = rng(lo, hi);
            setP(n, nm.c_str(), v);
        }
    }
}

}  // namespace rasgo::panel
