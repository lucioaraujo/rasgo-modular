// Teste isolado do MotionEngine (apps/panel/MotionEngine.hpp) — protótipo
// da camada de composição descrita em
// `dossies/ESTUDO_seed_composicao_generativa.md §3`.

#include "core/SignalGraph.hpp"
#include "panel/MotionEngine.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;
using rasgo::panel::MotionEngine;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

// módulo mínimo de teste: 1 parâmetro contínuo, sem portas — só pra
// exercitar setParameterBase()/parameterUserValue() do MotionEngine.
class Knob final : public Signal {
public:
    explicit Knob(const float def = 0.5f)
        : Signal({}, {}, {{"value", 0.0f, 1.0f, def, ""}}) {}
    std::string type() const override { return "TEST.KNOB"; }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>&) noexcept override {}
};

// módulo com uma entrada + parâmetro, pra testar aditividade com cabo
// (connectToParameter) por cima do que o MotionEngine escreve.
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

void testWalkStaysInRangeAndMoves() {
    SignalGraph g;
    const auto n = g.add(std::make_unique<Knob>());
    g.prepare(48000.0f, 1, 64);

    MotionEngine motion;
    MotionEngine::Binding b;
    b.node = n; b.paramId = "value";
    b.behavior = MotionEngine::Behavior::Walk;
    b.lo = 0.1f; b.hi = 0.9f; b.rateHz = 2.0f;   // rápido, pra convergir no teste
    b.seed = 0x9E3779B97F4A7C15ULL;
    motion.add(b);

    float lo = 2.0f, hi = -2.0f, first = -1.0f, last = -1.0f;
    for (int i = 0; i < 20000; ++i) {
        motion.tick(g, 1.0f / 1000.0f);
        const float v = g.parameterUserValue(n, "value");
        check(std::isfinite(v), "walk finito");
        check(v >= 0.1f - 1e-4f && v <= 0.9f + 1e-4f, "walk dentro da faixa");
        lo = std::min(lo, v); hi = std::max(hi, v);
        if (i == 0) first = v;
        last = v;
    }
    check(hi - lo > 0.2f, "walk visita valores bem diferentes ao longo do tempo");
    check(std::fabs(last - first) > 0.05f, "walk não fica preso no valor inicial");
}

void testOscillateSweepsBetweenBounds() {
    SignalGraph g;
    const auto n = g.add(std::make_unique<Knob>());
    g.prepare(48000.0f, 1, 64);

    MotionEngine motion;
    MotionEngine::Binding b;
    b.node = n; b.paramId = "value";
    b.behavior = MotionEngine::Behavior::Oscillate;
    b.lo = 0.2f; b.hi = 0.8f; b.rateHz = 1.0f;   // 1 Hz -> 1 ciclo/segundo
    motion.add(b);

    float lo = 2.0f, hi = -2.0f;
    const float dt = 1.0f / 2000.0f;
    for (int i = 0; i < 4000; ++i) {   // 2 segundos = 2 ciclos
        motion.tick(g, dt);
        const float v = g.parameterUserValue(n, "value");
        check(v >= 0.2f - 1e-4f && v <= 0.8f + 1e-4f, "oscillate dentro da faixa");
        lo = std::min(lo, v); hi = std::max(hi, v);
    }
    check(lo < 0.25f, "oscillate chega perto do piso");
    check(hi > 0.75f, "oscillate chega perto do teto");
}

void testAttractChasesOtherParameter() {
    SignalGraph g;
    const auto a = g.add(std::make_unique<Knob>(0.9f));
    const auto b_ = g.add(std::make_unique<Knob>(0.1f));
    g.prepare(48000.0f, 1, 64);

    MotionEngine motion;
    MotionEngine::Binding b;
    b.node = b_; b.paramId = "value";
    b.behavior = MotionEngine::Behavior::Attract;
    b.lo = 0.0f; b.hi = 1.0f;
    b.nodeB = a; b.paramIdB = "value";
    b.gain = 3.0f;   // rápido, pra convergir no teste
    motion.add(b);

    const float dt = 1.0f / 1000.0f;
    for (int i = 0; i < 5000; ++i) motion.tick(g, dt);
    const float v = g.parameterUserValue(b_, "value");
    check(std::fabs(v - 0.9f) < 0.02f, "attract converge pro alvo (outro módulo)");
}

void testMotionAdditiveWithCableModulation() {
    // a claim central do estudo: o MotionEngine escreve por
    // setParameterBase() -> não apaga uma modulação por cabo já
    // plugada no MESMO parâmetro (motor aditivo, 2026-09-04).
    SignalGraph g;
    const auto src = g.add(std::make_unique<Constant>(0.5f));
    const auto dst = g.add(std::make_unique<Sum>());
    g.connectToParameter(src, 0, dst, "base", /*depth=*/2.0f, /*offset=*/0.0f);
    g.prepare(48000.0f, 1, 64);

    MotionEngine motion;
    MotionEngine::Binding b;
    b.node = dst; b.paramId = "base";
    b.behavior = MotionEngine::Behavior::Walk;
    b.lo = -1.0f; b.hi = 1.0f; b.rateHz = 5.0f;
    motion.add(b);

    AudioBlock out(48000.0f, 1, 64);
    for (int i = 0; i < 200; ++i) {
        motion.tick(g, 1.0f / 500.0f);
        const float motionBase = g.parameterUserValue(dst, "base");
        g.process(out, dst, 0);
        // saída = base do MotionEngine + depth·fonte (0.5*2=1.0) — as
        // duas escritas convivem, nenhuma apaga a outra
        for (std::size_t f = 0; f < out.frames(); ++f)
            check(std::fabs(out.at(0, f) - (motionBase + 1.0f)) < 1e-4f,
                  "motion base + modulação por cabo somam (aditivo)");
    }
}

// ---- modo 2: a mão caótica (`inhabit`, §3.7) --------------------------

class Voice final : public Signal {
public:
    Voice()
        : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}},
                 {{"cutoff", 20.0f, 20000.0f, 800.0f, "Hz"},
                  {"resonance", 0.0f, 1.0f, 0.3f, ""},
                  {"steps", 1.0f, 16.0f, 8.0f, ""},
                  {"hold", 0.0f, 1.0f, 0.0f, ""},
                  {"blend", 0.0f, 1.0f, 0.5f, ""}}) {}
    std::string type() const override { return "TEST.VOICE"; }
    Panel panel() const override {
        Panel p; p.hp = 10;
        p.add(Widget::Kind::Knob, "CUT", "cutoff", 5.0f, 20.0f);
        p.add(Widget::Kind::Knob, "RES", "resonance", 20.0f, 20.0f);
        p.add(Widget::Kind::Knob, "STEP", "steps", 35.0f, 20.0f);
        p.add(Widget::Kind::Toggle, "HLD", "hold", 5.0f, 40.0f);
        p.add(Widget::Kind::Slider, "BLN", "blend", 20.0f, 40.0f);
        return p;
    }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>&) noexcept override {}
};

class FakeMixer final : public Signal {
public:
    FakeMixer() : Signal({}, {}, {{"pan1", -1.0f, 1.0f, 0.0f, ""}}) {}
    std::string type() const override { return "MIXER"; }
    Panel panel() const override {
        Panel p; p.hp = 6;
        p.add(Widget::Kind::Knob, "PAN", "pan1", 5.0f, 20.0f);
        return p;
    }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>&) noexcept override {}
};

void testInhabitMovesAndStaysSane() {
    SignalGraph g;
    const auto v = g.add(std::make_unique<Voice>());
    g.prepare(48000.0f, 1, 64);
    MotionEngine m;
    m.inhabit(g, 12345ULL, {v});
    EXPECT(m.fiberCount() == 5);

    float clo = 1e9f, chi = -1e9f;
    for (int i = 0; i < 6000; ++i) {
        m.tick(g, 0.033f, 0.4f);
        const float c = g.parameterUserValue(v, "cutoff");
        check(std::isfinite(c) && c >= 20.0f && c <= 20000.0f, "cutoff são");
        clo = std::min(clo, c); chi = std::max(chi, c);
    }
    check(chi - clo > 300.0f, "a mão move o cutoff ao longo do tempo");
    check(chi - clo < 12000.0f, "mas dentro de uma janela em torno do seed");
}

void testStructuralBarelyMoves() {
    SignalGraph g;
    const auto v = g.add(std::make_unique<Voice>());
    g.prepare(48000.0f, 1, 64);
    MotionEngine m;
    m.inhabit(g, 777ULL, {v});
    for (int i = 0; i < 8000; ++i) {
        m.tick(g, 0.033f, 0.5f);
        const float s = g.parameterUserValue(v, "steps");
        check(std::fabs(s - std::round(s)) < 1e-4f, "steps quantizado a inteiro");
        check(std::fabs(s - 8.0f) <= 2.0f, "steps mal se move (perto do seed)");
    }
}

void testMixerMasterExempt() {
    SignalGraph g;
    const auto v = g.add(std::make_unique<Voice>());
    const auto mx = g.add(std::make_unique<FakeMixer>());
    g.prepare(48000.0f, 1, 64);
    MotionEngine m;
    m.inhabit(g, 42ULL, {v, mx});
    EXPECT(m.fiberCount() == 5);   // só as 5 fibras do Voice, nada do MIXER
    for (int i = 0; i < 2000; ++i) m.tick(g, 0.033f, 0.5f);
    check(g.parameterUserValue(mx, "pan1") == 0.0f, "MIXER.pan1 intocado");
}

void testInhabitDeterminism() {
    auto run = [] {
        SignalGraph g;
        const auto v = g.add(std::make_unique<Voice>());
        g.prepare(48000.0f, 1, 64);
        MotionEngine m;
        m.inhabit(g, 99887766ULL, {v});
        std::vector<float> tr;
        for (int i = 0; i < 4000; ++i) {
            m.tick(g, 0.033f, 0.45f);
            tr.push_back(g.parameterUserValue(v, "cutoff"));
            tr.push_back(g.parameterUserValue(v, "resonance"));
        }
        return tr;
    };
    const auto a = run(), b = run();
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i) if (a[i] != b[i]) same = false;
    EXPECT(same);
}

void testHandEditWins() {
    // com a mão caótica LIGADA, o músico mexe num knob -> a fibra
    // re-ancora ali, não volta pro valor anterior.
    SignalGraph g;
    const auto v = g.add(std::make_unique<Voice>());
    g.prepare(48000.0f, 1, 64);
    MotionEngine m;
    m.inhabit(g, 314159ULL, {v});
    for (int i = 0; i < 200; ++i) m.tick(g, 0.033f, 0.4f);   // deixa respirar

    // o músico gira CUTOFF pra 5000 Hz (setParameterBase = mesma via do knob)
    g.setParameterBase(v, "cutoff", 5000.0f);
    float lo = 1e9f, hi = -1e9f;
    for (int i = 0; i < 400; ++i) {
        m.tick(g, 0.033f, 0.4f);   // ~13 s
        const float c = g.parameterUserValue(v, "cutoff");
        lo = std::min(lo, c); hi = std::max(hi, c);
    }
    // segue respirando em torno de 5000, NÃO voltou pros ~800 do seed
    check(lo > 2500.0f, "a fibra re-ancorou onde o músico deixou (não caiu pros ~800)");
    check(hi < 8500.0f, "e não disparou pra longe");
}

void testHotStaysGentle() {
    SignalGraph g;
    const auto v = g.add(std::make_unique<Voice>());
    g.prepare(48000.0f, 1, 64);
    MotionEngine m;
    m.inhabit(g, 5ULL, {v});
    float rlo = 2.0f, rhi = -2.0f;
    for (int i = 0; i < 8000; ++i) {
        m.tick(g, 0.033f, 0.4f);
        const float r = g.parameterUserValue(v, "resonance");
        check(r >= 0.0f && r <= 1.0f, "resonance na faixa");
        rlo = std::min(rlo, r); rhi = std::max(rhi, r);
    }
    // quente: respira, não varre — janela pequena em torno de 0,3
    check(rhi - rlo > 0.01f, "resonance ainda se move");
    check(rhi - rlo < 0.45f, "resonance não varre o range (é 'quente')");
}

void testDeterminism() {
    auto run = [](int steps) {
        SignalGraph g;
        const auto n = g.add(std::make_unique<Knob>());
        g.prepare(48000.0f, 1, 64);
        MotionEngine motion;
        MotionEngine::Binding b;
        b.node = n; b.paramId = "value";
        b.behavior = MotionEngine::Behavior::Walk;
        b.lo = 0.0f; b.hi = 1.0f; b.rateHz = 1.5f;
        b.seed = 0x2545F4914F6CDD1DULL;
        motion.add(b);
        std::vector<float> trace;
        for (int i = 0; i < steps; ++i) {
            motion.tick(g, 1.0f / 800.0f);
            trace.push_back(g.parameterUserValue(n, "value"));
        }
        return trace;
    };
    const auto a = run(3000), b = run(3000);
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i)
        if (a[i] != b[i]) same = false;
    EXPECT(same);
}

}  // namespace

int main() {
    testWalkStaysInRangeAndMoves();
    testOscillateSweepsBetweenBounds();
    testAttractChasesOtherParameter();
    testMotionAdditiveWithCableModulation();
    testDeterminism();
    testInhabitMovesAndStaysSane();
    testStructuralBarelyMoves();
    testMixerMasterExempt();
    testInhabitDeterminism();
    testHandEditWins();
    testHotStaysGentle();
    if (g_failures == 0) {
        std::cout << "RASGO Modular motion engine tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
