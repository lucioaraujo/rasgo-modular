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
// (3 cabos) à teia densa (~43 no passeio + espinha ≈ 60) conforme
// `complexity`.
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
};

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
    return id;
}

// ============================================================================

inline void seedPatch(rasgo::modular::SignalGraph& g, std::uint64_t seed) {
    using namespace rasgo::modular;
    const SeedIdentity id = seedIdentity(seed);
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

    // parâmetros + excitação da voz
    if (std::string(vc.mod) == "OSC") {
        // registro variado: ~70% grave, ~30% médio. Teto do médio em
        // 340 Hz — com +2 oitavas do QUANTIZER a nota mais alta chega a
        // ~1,4 kHz (musical), não a ~5 kHz.
        setP(voiceNode, "freq",
             chance(0.7f) ? rng(38.0f, 175.0f) : rng(175.0f, 340.0f));
        setP(voiceNode, "pw", rng(0.25f, 0.75f));   // não vira trem de picos
        setP(voiceNode, "drift", rng(0.0f, 0.4f));
        setP(voiceNode, "sub_2", chance(0.4f) ? 1.0f : 0.0f);
        if (pIn(voiceNode, "pitch") >= 0)
            connectByName("QUANTIZER", "pitch", "OSC", "pitch");
    } else if (std::string(vc.mod) == "CHORD") {
        setP(voiceNode, "freq", rng(50.0f, 180.0f));
        setP(voiceNode, "chord", rng(0.0f, 1.0f));
        setP(voiceNode, "voices", static_cast<float>(2 + rnd() % 3));
        setP(voiceNode, "detune", rng(0.0f, 0.5f));
        setP(voiceNode, "wave", rng(0.0f, 1.0f));
        connectByName("QUANTIZER", "pitch", "CHORD", "pitch");
    } else if (std::string(vc.mod) == "MATTER" || std::string(vc.mod) == "STRING") {
        setP(voiceNode, "freq", rng(60.0f, 260.0f));
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
        if (hasT("OSC")) { setT("OSC", "freq", rng(50.0f, 200.0f));
            connectByName("QUANTIZER", "pitch", "OSC", "pitch");
            connectByName("OSC", "saw", "MEMORY", "in"); }
    } else if (std::string(vc.mod) == "SHAPE") {
        setP(voiceNode, "fold", rng(0.2f, 0.9f));
        setP(voiceNode, "symmetry", rng(-0.8f, 0.8f));
        setP(voiceNode, "sat", rng(0.0f, 0.5f));
        setP(voiceNode, "level", 0.9f);
        if (hasT("OSC")) { setT("OSC", "freq", rng(50.0f, 180.0f));
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
    const char* procs[5] = {"FILTER", "SHAPE", "LPG", "PARAMETRIC", "SPACE"};
    const int nProc = static_cast<int>(std::lround(id.complexity * 3.0f))
        + (chance(0.4f) ? 1 : 0);
    for (int pi = 0; pi < nProc; ++pi) {
        const char* pr = procs[rnd() % 5];
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
        const bool brightRaw = bodyNode == voiceNode
            && (vt == "NOISE" || vt == "OSC" || vt == "SHAPE"
                || vt == "FILTER");
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
    setT("MIXER", "gain1", rng(-10.0f, -2.0f));
    setT("MIXER", "pan1", rng(-0.4f, 0.4f));
    if (hasT("MIXER")) {
        const int ci = pIn(firstT("MIXER"), "ch1"), so = pOut(bodyNode, bodyOut);
        if (ci >= 0 && so >= 0)
            connect({bodyNode, static_cast<std::size_t>(so)},
                    {firstT("MIXER"), static_cast<std::size_t>(ci)});
    }
    if (hasT("MASTER") && hasT("MIXER")) {
        setT("MASTER", "width", rng(0.85f, 1.5f));
        // fixo (não sorteado) — 50% do slider (faixa -60..+12 -> -24 dB) em
        // TODO seed, sem exceção; o autor sobe na mão a partir daí.
        // (era 30% / -38,4 dB; mudado a pedido do autor em 2026-09-05.)
        setT("MASTER", "gain", -24.0f);
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

    // ================ PASSEIO PONDERADO (o coração) ================
    // teto por passeio: 3 → ~43 tentativas (era ×25 = ~28, com 42 módulos;
    // com 56 há mais portas livres, o rack aguenta mais relação).
    const int nCables = 3 + static_cast<int>(std::lround(id.complexity * 40.0f));
    for (int c = 0; c < nCables; ++c) {
        // 1) escolhe uma classe de destino, depois um destino livre nela
        float dw[5] = {3.0f, 1.5f, 6.0f, 4.0f, 1.5f};
        // mais cedo prioriza gate/pitch/audio; mais tarde, mod
        if (c < 4) { dw[SEED_DST_GATE] = 5.0f; dw[SEED_DST_PITCH] = 3.0f; dw[SEED_DST_AUDIO] = 4.0f; }
        float dtot = 0.0f; for (float x : dw) dtot += x;
        float dr = f01() * dtot; int dc = 0;
        for (; dc < 4; ++dc) { if (dr < dw[dc]) break; dr -= dw[dc]; }
        // destino livre aleatório
        Port d{0, 0}; bool okD = false;
        for (int tries = 0; tries < 12 && !okD; ++tries) {
            if (dst[dc].empty()) break;
            const Port cand = dst[dc][rnd() % dst[dc].size()];
            // MIXER/MASTER não são destino do passeio — as camadas
            // extras entram pela seção controlada abaixo (senão o passeio
            // joga fonte crua direto no barramento de saída)
            if (typ(cand.node) == "MIXER" || typ(cand.node) == "MASTER")
                continue;
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
    if (hasT("MIXER")) {
        std::size_t mx = firstT("MIXER");
        const char* chs[2] = {"ch2", "ch3"};
        const int nLayers = chance(0.5f) ? (chance(0.35f) ? 2 : 1) : 0;
        int used = 0;
        for (int sc = 0; sc < 6 && used < nLayers; ++sc)
            for (const Port& p : src[sc]) {
                if (used >= nLayers) break;
                if (p.node == bodyNode || !isProcessedBus(p.node)) continue;
                if (reaches(p.node, sink)) continue;      // já chega à saída
                const int ci = pIn(mx, chs[used]);
                if (ci < 0 || (ci < (int)inUsed[mx].size() && inUsed[mx][ci])) continue;
                if (connect(p, {mx, static_cast<std::size_t>(ci)})) {
                    setP(mx, (std::string("gain") + std::to_string(used + 2)).c_str(),
                         rng(-24.0f, -11.0f));
                    setP(mx, (std::string("pan") + std::to_string(used + 2)).c_str(),
                         rng(-0.5f, 0.5f));
                    ++used;
                }
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
        for (int i = 0; i < nDrift && i < 4; ++i) {
            const PT& tp = tgts[rnd() % (sizeof(tgts) / sizeof(tgts[0]))];
            if (!hasT(tp.mod)) continue;
            const int so = pOut(dn, outs[i]);
            if (so < 0) continue;
            try {
                g.connectToParameter(dn, static_cast<std::size_t>(so),
                                     firstT(tp.mod), tp.par,
                                     tp.depth * (0.3f + id.motion), tp.off, true);
            } catch (...) {}
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
                || nm == "fm_amount")
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
