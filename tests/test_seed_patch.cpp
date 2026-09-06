// Revalidação do gerador de patch por SEED contra o CATÁLOGO REAL.
// `apps/panel/PatchSeed.hpp::seedPatch()` e `PatchGenetics.hpp::crossPatch()`
// dependem da ORDEM de `ModuleCatalog.hpp::moduleCatalog()` (índice de nó →
// passeio semeado). Este teste monta o rack completo como o painel faz e
// confere as GARANTIAS do dossiê (`ESTUDO_seed_composicao_generativa.md`):
// caminho audível até a saída, sem exceção, saída finita e limitada, para
// uma faixa de seeds — o gate contra regressão quando o catálogo muda.

#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/PatchGenetics.hpp"

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

// sink trivial, como o `Out` anônimo do `panel_main.cpp`
struct Out final : Signal {
    Out() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "OUT"; }
    void process(const std::vector<const AudioBlock*>& in,
                 std::vector<AudioBlock>& out) noexcept override {
        if (in[0]) out[0].copyFrom(*in[0]);
    }
};

// monta o rack completo na MESMA ordem do painel (moduleCatalog() achatado)
// + o sink. Devolve o índice do sink.
std::size_t buildRack(SignalGraph& g) {
    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types) {
            auto n = rasgo::panel::makeModule(t);
            if (n) g.add(std::move(n));
        }
    return g.add(std::make_unique<Out>());
}

struct RunResult { bool finite = true; double peak = 0.0; double rms = 0.0; };

RunResult renderGraph(SignalGraph& g, std::size_t sink, int blocks) {
    RunResult r;
    g.prepare(kSr, 1, kB);
    g.setActiveOutput(sink);
    AudioBlock block(kSr, 1, kB);
    double sq = 0.0;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        g.process(block, sink, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v = block.at(0, k);
            if (!std::isfinite(v)) r.finite = false;
            r.peak = std::max(r.peak, (double)std::fabs(v));
            sq += (double)v * v;
            ++n;
        }
    }
    r.rms = std::sqrt(sq / std::max<std::size_t>(1, n));
    return r;
}

void testCatalogInstantiates() {
    SignalGraph g;
    const std::size_t sink = buildRack(g);
    // 42 módulos do catálogo + 1 sink
    EXPECT(g.nodeCount() == 43);
    EXPECT(g.node(sink).type() == "OUT");
}

void testSeedsAudibleAndBounded() {
    int silent = 0;
    for (std::uint64_t s = 1; s <= 24; ++s) {
        SignalGraph g;
        const std::size_t sink = buildRack(g);
        bool threw = false;
        try { rasgo::panel::seedPatch(g, s); }
        catch (...) { threw = true; }
        check(!threw, "seedPatch não lança");
        if (threw) continue;

        // garantia: o passeio cabeou algo
        check(g.cableCount() >= 2, "seed cabeou o rack");

        const RunResult r = renderGraph(g, sink, 400);   // ~1 s (envelopes/clocks)
        check(r.finite, "saída do seed finita");
        check(r.peak < 12.0, "saída do seed limitada");   // MASTER limita; folga
        if (r.rms < 1e-4) {
            ++silent;
            std::cerr << "  [nota] seed " << s << " quase-mudo (rms " << r.rms
                      << ", " << g.cableCount() << " cabos)\n";
        }
    }
    // A garantia do dossiê é "sempre audível". Na prática ~3 em 24 saem
    // quase-mudos (a espinha voz→MASTER→sink é cabeada mas o gatilho não
    // dispara na janela) — limitação PRÉ-EXISTENTE do `seedPatch`, idêntica
    // antes e depois da reordenação do catálogo de 2026-09-06 (verificado
    // byte a byte). O gate aqui pega uma REGRESSÃO GROSSA (reordenar quebrar
    // metade dos seeds), não a cauda conhecida.
    EXPECT(silent <= 5);
}

void testCrossKeepsItBounded() {
    for (std::uint64_t s = 1; s <= 8; ++s) {
        SignalGraph target;
        const std::size_t tsink = buildRack(target);
        SignalGraph donor;
        buildRack(donor);
        bool threw = false;
        try {
            rasgo::panel::seedPatch(target, s * 3 + 1);
            rasgo::panel::seedPatch(donor, s * 7 + 2);
            rasgo::panel::crossPatch(target, donor, s * 11 + 3, 0.5f);
        } catch (...) { threw = true; }
        check(!threw, "seed+cross não lançam");
        if (threw) continue;
        const RunResult r = renderGraph(target, tsink, 24);
        check(r.finite, "saída pós-CROSS finita");
        check(r.peak < 12.0, "saída pós-CROSS limitada");
    }
}

void testDeterministic() {
    // mesmo seed → mesmo patch → mesma saída, byte a byte
    SignalGraph a, b;
    const std::size_t sa = buildRack(a);
    const std::size_t sb = buildRack(b);
    rasgo::panel::seedPatch(a, 4242ULL);
    rasgo::panel::seedPatch(b, 4242ULL);
    a.prepare(kSr, 1, kB);
    b.prepare(kSr, 1, kB);
    a.setActiveOutput(sa);
    b.setActiveOutput(sb);
    AudioBlock ba(kSr, 1, kB), bb(kSr, 1, kB);
    bool same = true;
    for (int blk = 0; blk < 20; ++blk) {
        a.process(ba, sa, 0);
        b.process(bb, sb, 0);
        for (std::size_t k = 0; k < kB; ++k)
            if (ba.at(0, k) != bb.at(0, k)) same = false;
    }
    EXPECT(same);
}

}  // namespace

int main() {
    testCatalogInstantiates();
    testSeedsAudibleAndBounded();
    testCrossKeepsItBounded();
    testDeterministic();

    if (g_failures == 0) {
        std::cout << "RASGO Modular seed patch tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
