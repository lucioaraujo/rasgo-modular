// Equilíbrio de volume entre seeds (`apps/panel/SeedBalance.hpp`).
//
// Existe porque o achado veio da ESCUTA, não dos testes: na sessão de
// validação de 23 set. 2026 o autor relatou seeds abrindo tão fracos que
// era preciso subir o volume das caixas para saber se havia som. Medido
// depois em 40 seeds: 52,7 LU de dispersão.
//
// Estes casos fixam as propriedades que a correção precisa ter — e a
// primeira delas é a que mais importa num instrumento reproduzível por
// seed: **não quebrar o determinismo**.

#include "core/SignalGraph.hpp"
#include "dsp/Loudness.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace rasgo::modular;

namespace {

int falhas = 0;
void check(const bool ok, const char* o_que) {
    if (!ok) { std::printf("  FALHOU: %s\n", o_que); ++falhas; }
}

constexpr float kSr = 44100.0f;
constexpr std::size_t kB = 256;

struct Patch {
    SignalGraph g;
    std::size_t sink = 0;
};

// O rack completo do instrumento, na mesma ordem dos front-ends.
Patch montar(std::uint64_t seed) {
    Patch p;
    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types)
            if (auto n = rasgo::panel::makeModule(t)) p.g.add(std::move(n));
    p.sink = p.g.add(std::make_unique<rasgo::panel::SinkOut>());
    rasgo::panel::seedPatch(p.g, seed);
    p.g.prepare(kSr, 2, kB);
    p.g.setActiveOutput(p.sink);
    return p;
}

float ganhoDoMaster(SignalGraph& g) {
    for (std::size_t i = 0; i < g.nodeCount(); ++i)
        if (g.node(i).type() == "MASTER")
            return g.node(i).parameterValue("gain");
    return 0.0f;
}

struct Medida { float lufs; double pico; };

Medida medir(Patch& p, double segundos) {
    LoudnessMeter lm; lm.prepare(kSr);
    AudioBlock o(kSr, 2, kB);
    double pico = 0.0;
    const long blocos = (long)(segundos * kSr / kB);
    for (long i = 0; i < blocos; ++i) {
        p.g.process(o, p.sink, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            const float l = o.at(0, k), r = o.at(1, k);
            lm.push(l, r);
            pico = std::max(pico, (double)std::max(std::fabs(l), std::fabs(r)));
        }
    }
    return {lm.integrated(), pico};
}

// ---- 1. determinismo -------------------------------------------------
//
// O seed É a identidade do patch no Rasgo Modular: anotar o número tem de
// bastar para reproduzir a peça. Uma correção que dependesse de estado
// acumulado, do relógio ou da ordem das chamadas destruiria isso em
// silêncio — o patch abriria diferente a cada vez e ninguém saberia por
// quê. Este é o teste mais importante do arquivo.
void testDeterminismo() {
    for (const std::uint64_t s : {1ull, 12345ull, 857388517851513341ull}) {
        auto a = montar(s), b = montar(s);
        const float ca = rasgo::panel::balanceSeedLevel(a.g, s, kSr, kB);
        const float cb = rasgo::panel::balanceSeedLevel(b.g, s, kSr, kB);
        check(ca == cb, "a mesma seed dá a MESMA correção");
        check(ganhoDoMaster(a.g) == ganhoDoMaster(b.g),
              "a mesma seed dá o mesmo ganho de MASTER");
    }
}

// ---- 2. o grafo volta ao início --------------------------------------
//
// A medição roda o grafo. Se ele não for rebobinado, o músico ouve o
// patch começando adiante do que o seed descreve — e dois racks com o
// mesmo seed soariam diferente conforme tivessem passado por aqui.
void testGrafoRebobinado() {
    auto comBalanco = montar(12345), semBalanco = montar(12345);
    rasgo::panel::balanceSeedLevel(comBalanco.g, 12345, kSr, kB);
    // força o mesmo ganho nos dois, pra comparar só o PONTO DE PARTIDA
    for (std::size_t i = 0; i < comBalanco.g.nodeCount(); ++i)
        if (comBalanco.g.node(i).type() == "MASTER")
            comBalanco.g.node(i).setParameter("gain",
                ganhoDoMaster(semBalanco.g));

    AudioBlock a(kSr, 2, kB), b(kSr, 2, kB);
    comBalanco.g.process(a, comBalanco.sink, 0);
    semBalanco.g.process(b, semBalanco.sink, 0);
    double dif = 0.0;
    for (std::size_t k = 0; k < kB; ++k)
        dif = std::max(dif, (double)std::fabs(a.at(0, k) - b.at(0, k)));
    check(dif < 1.0e-6,
          "depois de medir, o primeiro bloco é o mesmo de quem não mediu");
}

// ---- 3. aproxima o alvo, sem estourar --------------------------------
void testAproximaOAlvo() {
    int melhorou = 0, piorou = 0;
    const std::uint64_t seeds[] = {1, 7, 12345, 99999, 4242424242ull,
                                   857388517851513341ull};
    for (const std::uint64_t s : seeds) {
        auto sem = montar(s);
        const Medida antes = medir(sem, 8.0);
        if (antes.lufs <= LoudnessMeter::kSilence) continue;

        auto com = montar(s);
        rasgo::panel::balanceSeedLevel(com.g, s, kSr, kB);
        const Medida depois = medir(com, 8.0);

        const float dAntes = std::fabs(antes.lufs - rasgo::panel::kSeedTargetLufs);
        const float dDepois = std::fabs(depois.lufs - rasgo::panel::kSeedTargetLufs);
        if (dDepois < dAntes - 0.5f) ++melhorou;
        if (dDepois > dAntes + 0.5f) ++piorou;

        // a régua do pico: nunca deixar o material acima do teto de
        // segurança da saída por causa da correção
        check(depois.pico <= (double)rasgo::panel::SinkOut::kCeiling + 1.0e-3,
              "a correção não empurra o pico acima do teto do sink");
    }
    check(melhorou > 0, "ao menos um seed fica mais perto do alvo");
    check(piorou == 0, "nenhum seed fica mais LONGE do alvo");
}

// ---- 4. casos de borda -----------------------------------------------
void testSemMaster() {
    // patch sem MASTER é legítimo (som direto no OUT): saída limpa, zero
    SignalGraph g;
    const std::size_t sink = g.add(std::make_unique<rasgo::panel::SinkOut>());
    g.prepare(kSr, 2, kB);
    g.setActiveOutput(sink);
    const float c = rasgo::panel::balanceSeedLevel(g, 12345, kSr, kB);
    check(c == 0.0f, "sem MASTER, correção é zero e não há erro");
}

// A correção vem do SEED, não do grafo recebido — e isto é contrato, não
// acidente. A função monta o próprio rack de prova para não tocar no
// grafo do músico (ver `SeedBalance.hpp`), e a consequência é que quem
// chama **precisa** passar o seed de que aquele grafo nasceu. Passar
// outro dá uma correção que não tem relação com o que vai soar.
//
// Este caso existe para deixar o acoplamento VISÍVEL em vez de implícito:
// se alguém mudar a função para ignorar o seed, ou os chamadores para
// passar qualquer número, o teste cai aqui e não numa sessão de escuta.
void testCorrecaoVemDoSeed() {
    auto a = montar(1), b = montar(1);
    const float certa  = rasgo::panel::balanceSeedLevel(a.g, 1, kSr, kB);
    const float errada = rasgo::panel::balanceSeedLevel(b.g, 12345, kSr, kB);
    check(certa != errada,
          "seeds diferentes dão correções diferentes (a correção usa o seed)");

    // e a via direta, sem grafo nenhum, é a mesma conta
    check(rasgo::panel::seedGainCorrection(1, kSr, kB) == certa,
          "balanceSeedLevel aplica exatamente seedGainCorrection");
}

}  // namespace

int main() {
    testDeterminismo();
    testGrafoRebobinado();
    testAproximaOAlvo();
    testSemMaster();
    testCorrecaoVemDoSeed();
    if (falhas == 0) std::puts("test_seed_balance: OK");
    return falhas == 0 ? 0 : 1;
}
