// Teste isolado do Patch Genetics (apps/panel/PatchGenetics.hpp) —
// MUTATE/EVOLVE/FREEZE, descrito em
// `dossies/ESTUDO_seed_composicao_generativa.md §4`.

#include "core/SignalGraph.hpp"
#include "panel/PatchGenetics.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace rasgo::modular;
using namespace rasgo::panel;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

// módulo de teste com 3 parâmetros: `a`/`b` livres, `mode` estrutural
// (na lista bloqueada do MUTATE).
class Multi final : public Signal {
public:
    explicit Multi(const float defA = 0.5f, const float defB = 0.0f,
                   const float defMode = 0.0f)
        : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}},
                 {{"a", 0.0f, 1.0f, defA, ""},
                  {"b", -2.0f, 2.0f, defB, ""},
                  {"mode", 0.0f, 3.0f, defMode, ""}}) {}
    std::string type() const override { return "TEST.MULTI"; }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>&) noexcept override {}
};

class Constant final : public Signal {
public:
    explicit Constant(const float v)
        : Signal({}, {{"out", PortKind::Audio, ""}}, {}), v_(v) {}
    std::string type() const override { return "TEST.CONSTANT"; }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        for (std::size_t c = 0; c < outputs[0].channels(); ++c)
            for (std::size_t f = 0; f < outputs[0].frames(); ++f)
                outputs[0].at(c, f) = v_;
    }
private:
    float v_;
};

// pra checar o valor MODULADO (não só a base): escreve o parâmetro na
// saída, como test_motion_engine.cpp faz.
class Sum final : public Signal {
public:
    Sum() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}},
                   {{"base", -4.0f, 4.0f, 0.0f, ""}}) {}
    std::string type() const override { return "TEST.SUM"; }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const float b = parameterValue("base");
        for (std::size_t c = 0; c < outputs[0].channels(); ++c)
            for (std::size_t f = 0; f < outputs[0].frames(); ++f)
                outputs[0].at(c, f) = b;
    }
};

void testUntouchedNeverMutates() {
    SignalGraph g;
    const auto isolated = g.add(std::make_unique<Multi>(0.5f, 0.5f, 1.0f));
    g.prepare(48000.0f, 1, 64);
    mutatePatch(g, 12345ULL, 1.0f);   // fraction 1.0 -> tenta tudo
    check(g.parameterUserValue(isolated, "a") == 0.5f,
          "nó sem cabo nunca muda (a)");
    check(g.parameterUserValue(isolated, "b") == 0.5f,
          "nó sem cabo nunca muda (b)");
}

void testTouchedMutatesWithinRangeAndSparesStructural() {
    SignalGraph g;
    const auto src = g.add(std::make_unique<Constant>(0.2f));
    const auto dst = g.add(std::make_unique<Multi>(0.5f, 0.0f, 2.0f));
    g.connect(src, 0, dst, 0);
    g.prepare(48000.0f, 1, 64);

    bool changedA = false, changedB = false;
    for (std::uint64_t sd = 1; sd <= 60; ++sd) {
        mutatePatch(g, sd, 1.0f);
        const float a = g.parameterUserValue(dst, "a");
        const float b = g.parameterUserValue(dst, "b");
        check(a >= 0.0f && a <= 1.0f, "a fica dentro da faixa");
        check(b >= -2.0f && b <= 2.0f, "b fica dentro da faixa");
        check(g.parameterUserValue(dst, "mode") == 2.0f,
              "mode (estrutural/bloqueado) nunca muda");
        if (a != 0.5f) changedA = true;
        if (b != 0.0f) changedB = true;
    }
    check(changedA && changedB,
          "parâmetros livres do nó cabeado variam ao longo de várias mutações");
}

void testFreezeProtects() {
    SignalGraph g;
    const auto src = g.add(std::make_unique<Constant>(0.2f));
    const auto dst = g.add(std::make_unique<Multi>(0.5f, 0.0f));
    g.connect(src, 0, dst, 0);
    g.prepare(48000.0f, 1, 64);

    const std::unordered_set<std::size_t> frozen{dst};
    for (std::uint64_t sd = 1; sd <= 40; ++sd)
        mutatePatch(g, sd, 1.0f, frozen);
    check(g.parameterUserValue(dst, "a") == 0.5f, "FREEZE protege o nó (a)");
    check(g.parameterUserValue(dst, "b") == 0.0f, "FREEZE protege o nó (b)");
}

void testEvolveChangesOverSteps() {
    SignalGraph g;
    const auto src = g.add(std::make_unique<Constant>(0.2f));
    const auto dst = g.add(std::make_unique<Multi>(0.5f, 0.0f));
    g.connect(src, 0, dst, 0);
    g.prepare(48000.0f, 1, 64);

    evolvePatch(g, 4242ULL, /*steps=*/12, /*fractionPerStep=*/0.5f);
    const float a = g.parameterUserValue(dst, "a");
    const float b = g.parameterUserValue(dst, "b");
    check(a >= 0.0f && a <= 1.0f && b >= -2.0f && b <= 2.0f,
          "evolve: resultado final dentro da faixa");
    check(a != 0.5f || b != 0.0f, "evolve mudou algo em 12 passos");
}

void testDeterminism() {
    auto build = []() {
        SignalGraph g;
        const auto src = g.add(std::make_unique<Constant>(0.3f));
        const auto dst = g.add(std::make_unique<Multi>());
        g.connect(src, 0, dst, 0);
        g.prepare(48000.0f, 1, 64);
        return std::pair<SignalGraph, std::size_t>(std::move(g), dst);
    };
    auto pa = build();
    auto pb = build();
    mutatePatch(pa.first, 777ULL, 0.8f);
    mutatePatch(pb.first, 777ULL, 0.8f);
    check(pa.first.parameterUserValue(pa.second, "a")
              == pb.first.parameterUserValue(pb.second, "a"),
          "mesmo seed -> mesma mutação (a)");
    check(pa.first.parameterUserValue(pa.second, "b")
              == pb.first.parameterUserValue(pb.second, "b"),
          "mesmo seed -> mesma mutação (b)");
}

void testMutateAdditiveWithCableModulation() {
    // a claim central: MUTATE escreve por setParameterBase() -> não
    // apaga uma modulação por cabo já plugada no mesmo parâmetro.
    SignalGraph g;
    const auto carrier = g.add(std::make_unique<Constant>(0.1f));
    const auto dst = g.add(std::make_unique<Sum>());
    g.connect(carrier, 0, dst, 0);   // cabo de áudio comum -> "tocado"
    const auto modSrc = g.add(std::make_unique<Constant>(0.25f));
    g.connectToParameter(modSrc, 0, dst, "base", /*depth=*/2.0f, /*offset=*/0.0f);
    g.prepare(48000.0f, 1, 64);

    mutatePatch(g, 555ULL, 1.0f);
    const float base = g.parameterUserValue(dst, "base");

    AudioBlock out(48000.0f, 1, 64);
    g.process(out, dst, 0);
    // saída = base (mutada) + depth*fonte (2*0.25=0.5) — as duas
    // escritas convivem
    for (std::size_t f = 0; f < out.frames(); ++f)
        check(std::fabs(out.at(0, f) - (base + 0.5f)) < 1e-4f,
              "base mutada + modulação por cabo somam (aditivo)");
}

void testCrossFraction1MatchesDonorExactly() {
    SignalGraph target, donor;
    const auto tSrc = target.add(std::make_unique<Constant>(0.1f));
    const auto tDst = target.add(std::make_unique<Multi>(0.5f, 0.0f, 2.0f));
    target.connect(tSrc, 0, tDst, 0);
    target.prepare(48000.0f, 1, 64);

    donor.add(std::make_unique<Multi>(0.9f, -1.5f, 2.0f));
    donor.prepare(48000.0f, 1, 64);

    crossPatch(target, donor, 111ULL, 1.0f);
    check(target.parameterUserValue(tDst, "a") == 0.9f,
          "fraction=1: 'a' vira exatamente o valor do doador");
    check(target.parameterUserValue(tDst, "b") == -1.5f,
          "fraction=1: 'b' vira exatamente o valor do doador");
    check(target.parameterUserValue(tDst, "mode") == 2.0f,
          "mode (bloqueado) nunca cruza, mesmo com fraction=1");
}

void testCrossRespectsFrozenAndUntouched() {
    SignalGraph target, donor;
    const auto tSrc = target.add(std::make_unique<Constant>(0.1f));
    const auto touched = target.add(std::make_unique<Multi>(0.5f, 0.0f));
    target.connect(tSrc, 0, touched, 0);
    const auto isolated = target.add(std::make_unique<Multi>(0.3f, 0.3f));
    const auto frozenNode = target.add(std::make_unique<Multi>(0.4f, 0.4f));
    target.connect(tSrc, 0, frozenNode, 0);
    target.prepare(48000.0f, 1, 64);

    donor.add(std::make_unique<Multi>(0.99f, 1.9f));
    donor.prepare(48000.0f, 1, 64);

    const std::unordered_set<std::size_t> frozen{frozenNode};
    crossPatch(target, donor, 222ULL, 1.0f, frozen);
    check(target.parameterUserValue(touched, "a") == 0.99f,
          "nó tocado (sem freeze) cruza de verdade");
    check(target.parameterUserValue(isolated, "a") == 0.3f,
          "nó sem cabo nunca cruza (mesma regra do MUTATE)");
    check(target.parameterUserValue(frozenNode, "a") == 0.4f,
          "FREEZE protege do CROSS igual protege do MUTATE");
}

void testCrossNoMatchingTypeLeavesTargetUntouched() {
    SignalGraph target, donor;
    const auto tSrc = target.add(std::make_unique<Constant>(0.1f));
    const auto tDst = target.add(std::make_unique<Multi>(0.5f, 0.0f));
    target.connect(tSrc, 0, tDst, 0);
    target.prepare(48000.0f, 1, 64);

    // doador SEM nenhum TEST.MULTI -- so' outro tipo
    donor.add(std::make_unique<Constant>(0.7f));
    donor.prepare(48000.0f, 1, 64);

    crossPatch(target, donor, 333ULL, 1.0f);
    check(target.parameterUserValue(tDst, "a") == 0.5f,
          "sem tipo correspondente no doador, o no' do alvo fica intocado");
    check(target.parameterUserValue(tDst, "b") == 0.0f,
          "sem tipo correspondente no doador, o no' do alvo fica intocado (b)");
}

void testCrossMultipleOccurrencesWrapAroundSingleDonor() {
    SignalGraph target, donor;
    const auto tSrc = target.add(std::make_unique<Constant>(0.1f));
    const auto m1 = target.add(std::make_unique<Multi>(0.1f, 0.1f));
    const auto m2 = target.add(std::make_unique<Multi>(0.2f, 0.2f));
    target.connect(tSrc, 0, m1, 0);
    target.connect(tSrc, 0, m2, 0);
    target.prepare(48000.0f, 1, 64);

    // so' 1 TEST.MULTI no doador -- os 2 do alvo devem casar com ele
    // (modulo do tamanho da lista do doador)
    donor.add(std::make_unique<Multi>(0.77f, 0.77f));
    donor.prepare(48000.0f, 1, 64);

    crossPatch(target, donor, 444ULL, 1.0f);
    check(target.parameterUserValue(m1, "a") == 0.77f,
          "1a ocorrencia do alvo casa com o unico doador");
    check(target.parameterUserValue(m2, "a") == 0.77f,
          "2a ocorrencia do alvo tambem casa (modulo o tamanho do doador)");
}

void testCrossDeterminism() {
    auto build = []() {
        SignalGraph g;
        const auto src = g.add(std::make_unique<Constant>(0.1f));
        const auto dst = g.add(std::make_unique<Multi>(0.2f, 0.2f));
        g.connect(src, 0, dst, 0);
        g.prepare(48000.0f, 1, 64);
        return std::pair<SignalGraph, std::size_t>(std::move(g), dst);
    };
    auto ta = build(), tb = build();
    SignalGraph donorA, donorB;
    donorA.add(std::make_unique<Multi>(0.8f, -0.5f));
    donorB.add(std::make_unique<Multi>(0.8f, -0.5f));
    donorA.prepare(48000.0f, 1, 64);
    donorB.prepare(48000.0f, 1, 64);

    crossPatch(ta.first, donorA, 999ULL, 0.6f);
    crossPatch(tb.first, donorB, 999ULL, 0.6f);
    check(ta.first.parameterUserValue(ta.second, "a")
              == tb.first.parameterUserValue(tb.second, "a"),
          "mesmo seed -> mesmo cruzamento (a)");
    check(ta.first.parameterUserValue(ta.second, "b")
              == tb.first.parameterUserValue(tb.second, "b"),
          "mesmo seed -> mesmo cruzamento (b)");
}

}  // namespace

int main() {
    testUntouchedNeverMutates();
    testTouchedMutatesWithinRangeAndSparesStructural();
    testFreezeProtects();
    testEvolveChangesOverSteps();
    testDeterminism();
    testMutateAdditiveWithCableModulation();
    testCrossFraction1MatchesDonorExactly();
    testCrossRespectsFrozenAndUntouched();
    testCrossNoMatchingTypeLeavesTargetUntouched();
    testCrossMultipleOccurrencesWrapAroundSingleDonor();
    testCrossDeterminism();
    if (g_failures == 0) {
        std::cout << "RASGO Modular patch genetics tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
