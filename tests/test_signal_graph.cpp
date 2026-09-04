// Testes da fundação de áudio do Rasgo Modular: SignalGraph, a conexão
// como objeto (ruptura/cicatriz), feedback com atraso de um bloco,
// modulação saída->parâmetro, fan-in explícito e o barramento
// ControlSnapshot. Os módulos aqui são mínimos (constante, ganho, soma):
// os módulos DSP de verdade (oscilador, filtro...) entram um a um depois.

#include "core/ControlSnapshot.hpp"
#include "core/SignalGraph.hpp"

#include <clocale>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

void expectNear(const float actual, const float expected, const float tol = 1.0e-4f) {
    check(std::fabs(actual - expected) < tol, "expectNear");
}

// --- módulos mínimos de teste ------------------------------------------------

class Constant final : public Signal {
public:
    explicit Constant(const float value)
        : Signal({}, {{"out", PortKind::Audio, ""}},
                 {{"level", -10.0f, 10.0f, value, ""}}) {}
    std::string type() const override { return "TEST.CONSTANT"; }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const float level = parameterValue("level");
        for (std::size_t channel = 0; channel < outputs[0].channels(); ++channel)
            for (std::size_t frame = 0; frame < outputs[0].frames(); ++frame)
                outputs[0].at(channel, frame) = level;
    }
};

class Gain final : public Signal {
public:
    explicit Gain(const float gain = 1.0f)
        : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}},
                 {{"gain", 0.0f, 8.0f, gain, ""}}) {}
    std::string type() const override { return "TEST.GAIN"; }
    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const float gain = parameterValue("gain");
        for (std::size_t channel = 0; channel < outputs[0].channels(); ++channel)
            for (std::size_t frame = 0; frame < outputs[0].frames(); ++frame) {
                const float in = inputs[0] ? inputs[0]->at(channel, frame) : 0.0f;
                outputs[0].at(channel, frame) = in * gain;
            }
    }
};

class Sum final : public Signal {
public:
    Sum()
        : Signal({{"a", PortKind::Audio, ""}, {"b", PortKind::Audio, ""}},
                 {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "TEST.SUM"; }
    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        for (std::size_t channel = 0; channel < outputs[0].channels(); ++channel)
            for (std::size_t frame = 0; frame < outputs[0].frames(); ++frame) {
                const float a = inputs[0] ? inputs[0]->at(channel, frame) : 0.0f;
                const float b = inputs[1] ? inputs[1]->at(channel, frame) : 0.0f;
                outputs[0].at(channel, frame) = a + b;
            }
    }
};

float blockPeak(const AudioBlock& block) {
    float peak = 0.0f;
    for (std::size_t channel = 0; channel < block.channels(); ++channel)
        for (std::size_t frame = 0; frame < block.frames(); ++frame)
            peak = std::max(peak, std::fabs(block.at(channel, frame)));
    return peak;
}

// --- testes -----------------------------------------------------------------

void testControlSnapshot() {
    ControlSnapshot<4> bus;
    ControlSnapshot<4>::Frame frame;
    frame.value = {0.25f, 0.5f, 0.75f, std::nanf("")};  // um não-finito
    frame.revision = 7;
    bus.publish(frame);

    const auto read = bus.read();
    expectNear(read.value[0], 0.25f);
    expectNear(read.value[2], 0.75f);
    expectNear(read.value[3], 0.0f);  // não-finito saneado
    EXPECT(read.revision == 7);

    // Publicação/leitura concorrente: o leitor nunca vê meio snapshot.
    // O escritor sempre publica os 4 slots IGUAIS; um snapshot coerente
    // portanto tem os 4 iguais. Baseline igual antes de começar.
    {
        ControlSnapshot<4>::Frame baseline;
        baseline.value = {0.0f, 0.0f, 0.0f, 0.0f};
        bus.publish(baseline);
    }
    std::atomic<bool> stop{false};
    std::thread writer([&] {
        for (int i = 0; i < 20000 && !stop.load(); ++i) {
            ControlSnapshot<4>::Frame f;
            const float v = static_cast<float>(i % 100) / 100.0f;
            f.value = {v, v, v, v};
            f.revision = static_cast<std::uint64_t>(i);
            bus.publish(f);
        }
    });
    bool coherent = true;
    for (int i = 0; i < 20000; ++i) {
        const auto snapshot = bus.read();
        if (!(snapshot.value[0] == snapshot.value[1]
              && snapshot.value[1] == snapshot.value[2]
              && snapshot.value[2] == snapshot.value[3]))
            coherent = false;
    }
    stop.store(true);
    writer.join();
    EXPECT(coherent);
}

void testChainAndFanIn() {
    SignalGraph graph;
    const auto a = graph.add(std::make_unique<Constant>(0.5f));
    const auto b = graph.add(std::make_unique<Constant>(0.2f));
    const auto mix = graph.add(std::make_unique<Sum>());
    const auto out = graph.add(std::make_unique<Gain>(2.0f));
    graph.connect(a, 0, mix, 0);
    graph.connect(b, 0, mix, 1);
    graph.connect(mix, 0, out, 0);

    // fan-in é explícito: uma segunda conexão na mesma porta é recusada.
    bool rejected = false;
    try {
        graph.connect(a, 0, mix, 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    EXPECT(rejected);

    graph.prepare(48000.0f, 2, 64);
    AudioBlock output(48000.0f, 2, 64);
    graph.process(output, out, 0);
    expectNear(output.at(0, 0), (0.5f + 0.2f) * 2.0f);
}

void testDisconnectAndReconnect() {
    // a superfície de cabeamento (painel) precisa desligar e religar
    // portas de entrada ao vivo.
    SignalGraph graph;
    const auto a = graph.add(std::make_unique<Constant>(0.5f));
    const auto b = graph.add(std::make_unique<Constant>(0.2f));
    const auto out = graph.add(std::make_unique<Gain>(1.0f));
    graph.connect(a, 0, out, 0);
    graph.prepare(48000.0f, 1, 32);
    AudioBlock o(48000.0f, 1, 32);
    graph.process(o, out, 0);
    expectNear(o.at(0, 0), 0.5f);

    // desliga e religa a outra fonte na mesma porta
    EXPECT(graph.disconnect(out, 0));
    EXPECT(!graph.disconnect(out, 0));   // já livre
    graph.connect(b, 0, out, 0);         // agora aceita (porta livre)
    graph.prepare(48000.0f, 1, 32);
    graph.process(o, out, 0);
    expectNear(o.at(0, 0), 0.2f);

    // porta livre -> sem conexão -> silêncio
    EXPECT(graph.disconnect(out, 0));
    graph.prepare(48000.0f, 1, 32);
    graph.process(o, out, 0);
    expectNear(o.at(0, 0), 0.0f);
}

void testConnectionRupture() {
    SignalGraph graph;
    const auto source = graph.add(std::make_unique<Constant>(1.0f));
    const auto sink = graph.add(std::make_unique<Gain>(1.0f));
    auto& cable = graph.connect(source, 0, sink, 0);
    graph.prepare(48000.0f, 1, 64);

    AudioBlock output(48000.0f, 1, 64);
    graph.process(output, sink, 0);
    expectNear(output.at(0, 0), 1.0f);  // intacto

    cable.rupture();
    graph.process(output, sink, 0);
    const float firstScar = blockPeak(output);
    EXPECT(firstScar > 0.5f);   // a cicatriz ainda soa
    EXPECT(firstScar <= 1.0f);

    // ~0,5 s depois a cicatriz está mais fraca, mas ainda audível.
    for (int block = 0; block < 375; ++block)
        graph.process(output, sink, 0);
    const float midScar = blockPeak(output);
    EXPECT(midScar < firstScar);
    EXPECT(midScar > 1.0e-3f);

    // ~3 s: decaiu ao silêncio absoluto.
    for (int block = 0; block < 2200; ++block)
        graph.process(output, sink, 0);
    EXPECT(blockPeak(output) < 1.0e-6f);

    cable.reconnect();
    graph.process(output, sink, 0);
    expectNear(output.at(0, 0), 1.0f);  // reconectado
}

void testFeedback() {
    // out = in + 0.5 * out(bloco anterior). Sem a marca de feedback isto
    // seria um ciclo e prepare() lançaria.
    SignalGraph graph;
    const auto input = graph.add(std::make_unique<Constant>(0.1f));
    const auto mix = graph.add(std::make_unique<Sum>());
    const auto tap = graph.add(std::make_unique<Gain>(0.5f));
    graph.connect(input, 0, mix, 0);
    graph.connect(tap, 0, mix, 1);
    graph.connect(mix, 0, tap, 0, /*feedback=*/true);
    graph.prepare(48000.0f, 1, 64);

    AudioBlock output(48000.0f, 1, 64);
    float value = 0.0f;
    for (int block = 0; block < 64; ++block) {
        graph.process(output, mix, 0);
        value = 0.1f + 0.5f * value;  // recorrência esperada
    }
    expectNear(output.at(0, 0), value, 1.0e-3f);
    expectNear(value, 0.2f, 1.0e-2f);  // ponto fixo 0.1/(1-0.5)

    // ciclo sem feedback é rejeitado
    SignalGraph bad;
    const auto x = bad.add(std::make_unique<Gain>());
    const auto y = bad.add(std::make_unique<Gain>());
    bad.connect(x, 0, y, 0);
    bad.connect(y, 0, x, 0);
    bool threw = false;
    try {
        bad.prepare(48000.0f, 1, 64);
    } catch (const std::logic_error&) {
        threw = true;
    }
    EXPECT(threw);
}

void testCableConductance() {
    // conductance = 1 -> passa sempre; conductance baixa -> passa às vezes,
    // com dropouts; conductance = 0 -> silêncio.
    SignalGraph graph;
    const auto src = graph.add(std::make_unique<Constant>(1.0f));
    const auto sink = graph.add(std::make_unique<Gain>(1.0f));
    auto& cable = graph.connect(src, 0, sink, 0);
    graph.prepare(48000.0f, 1, 64);
    AudioBlock out(48000.0f, 1, 64);

    graph.process(out, sink, 0);
    expectNear(out.at(0, 0), 1.0f);  // default conduz sempre

    cable.setConductance(0.0f);
    for (int b = 0; b < 200; ++b) graph.process(out, sink, 0);
    EXPECT(blockPeak(out) < 1.0e-3f);  // nunca conduz -> silêncio (após o fade de ~30 ms)

    cable.setConductance(0.5f);
    int conductingBlocks = 0;
    for (int b = 0; b < 400; ++b) {
        graph.process(out, sink, 0);
        if (blockPeak(out) > 0.5f) ++conductingBlocks;
    }
    // ~metade dos blocos conduz (folga grande, é estocástico)
    EXPECT(conductingBlocks > 80 && conductingBlocks < 320);

    // determinismo: mesma seed (nós/portas), mesma sequência
    SignalGraph g2;
    const auto s2 = g2.add(std::make_unique<Constant>(1.0f));
    const auto k2 = g2.add(std::make_unique<Gain>(1.0f));
    auto& c2 = g2.connect(s2, 0, k2, 0);
    c2.setConductance(0.5f);
    g2.prepare(48000.0f, 1, 64);
    AudioBlock o2(48000.0f, 1, 64);
    graph.prepare(48000.0f, 1, 64);  // re-prepara -> re-seeda igual
    cable.setConductance(0.5f);
    bool sameSequence = true;
    for (int b = 0; b < 200; ++b) {
        graph.process(out, sink, 0);
        g2.process(o2, k2, 0);
        if ((blockPeak(out) > 0.5f) != (blockPeak(o2) > 0.5f)) sameSequence = false;
    }
    EXPECT(sameSequence);
}

void testParameterModulation() {
    // Uma constante 0.5 modula o ganho de um Gain (depth 4) -> ganho ~2.0.
    SignalGraph graph;
    const auto carrier = graph.add(std::make_unique<Constant>(1.0f));
    const auto modulator = graph.add(std::make_unique<Constant>(0.5f));
    const auto vca = graph.add(std::make_unique<Gain>(0.0f));
    graph.connect(carrier, 0, vca, 0);
    graph.connectToParameter(modulator, 0, vca, "gain", /*depth=*/4.0f);
    graph.prepare(48000.0f, 1, 64);

    AudioBlock output(48000.0f, 1, 64);
    graph.process(output, vca, 0);
    expectNear(output.at(0, 0), 2.0f);  // 1.0 * (0 + 4 * 0.5)
}

std::unique_ptr<Signal> testFactory(const std::string& type) {
    if (type == "TEST.CONSTANT") return std::make_unique<Constant>(0.0f);
    if (type == "TEST.GAIN") return std::make_unique<Gain>();
    if (type == "TEST.SUM") return std::make_unique<Sum>();
    return nullptr;
}

void testAdditiveParameterBase() {
    // A modulação é ADITIVA sobre a base (o valor do knob): o valor
    // efetivo é base + offset + depth*fonte. Girar o knob (setParameterBase)
    // não é apagado pela modulação a cada bloco, e o texto do patch guarda
    // a base, não o valor instantâneo já modulado.
    SignalGraph graph;
    const auto carrier = graph.add(std::make_unique<Constant>(1.0f));
    const auto modulator = graph.add(std::make_unique<Constant>(0.5f));
    const auto vca = graph.add(std::make_unique<Gain>(0.0f));
    graph.connect(carrier, 0, vca, 0);
    graph.connectToParameter(modulator, 0, vca, "gain", /*depth=*/2.0f);

    // o "knob" vai para 1.0 depois de já haver modulação ligada
    graph.setParameterBase(vca, "gain", 1.0f);
    EXPECT(graph.parameterUserValue(vca, "gain") == 1.0f);

    graph.prepare(48000.0f, 1, 64);
    AudioBlock output(48000.0f, 1, 64);
    graph.process(output, vca, 0);
    expectNear(output.at(0, 0), 2.0f);   // 1.0 * (base 1.0 + 2.0*0.5)

    // depois de processar, a base do knob continua legível e estável
    EXPECT(graph.parameterUserValue(vca, "gain") == 1.0f);
    graph.process(output, vca, 0);
    expectNear(output.at(0, 0), 2.0f);   // estável, bloco após bloco

    const std::string patch = graph.serialize();
    EXPECT(patch.find("gain=1") != std::string::npos);
    SignalGraph restored = SignalGraph::deserialize(patch, testFactory);
    EXPECT(restored.serialize() == patch);          // idempotente após process
    restored.prepare(48000.0f, 1, 64);
    AudioBlock ro(48000.0f, 1, 64);
    restored.process(ro, vca, 0);
    expectNear(ro.at(0, 0), 2.0f);
}

void testSerialization() {
    SignalGraph graph;
    const auto a = graph.add(std::make_unique<Constant>(0.5f));
    const auto b = graph.add(std::make_unique<Constant>(0.2f));
    const auto mix = graph.add(std::make_unique<Sum>());
    const auto out = graph.add(std::make_unique<Gain>(2.0f));
    graph.connect(a, 0, mix, 0).setConductance(1.0f);
    graph.connect(b, 0, mix, 1, false, 0.75f);
    graph.connect(mix, 0, out, 0);
    graph.connectToParameter(a, 0, out, "gain", 3.0f, 0.1f);

    const std::string patch = graph.serialize();
    EXPECT(patch.find("rasgo-modular-patch 1") == 0);
    EXPECT(patch.find("cable 1:0 -> 2:1 gain=0.75") != std::string::npos);
    EXPECT(patch.find("mod 0:0 -> 3:gain") != std::string::npos);

    SignalGraph restored = SignalGraph::deserialize(patch, testFactory);
    EXPECT(restored.nodeCount() == 4);
    EXPECT(restored.cableCount() == 3);

    // round-trip textual estável (antes de processar - process() modula
    // parâmetros, o que é comportamento correto mas muda o texto).
    EXPECT(restored.serialize() == patch);

    graph.prepare(48000.0f, 1, 64);
    restored.prepare(48000.0f, 1, 64);
    AudioBlock o1(48000.0f, 1, 64), o2(48000.0f, 1, 64);
    graph.process(o1, out, 0);
    restored.process(o2, out, 0);
    expectNear(o1.at(0, 0), o2.at(0, 0));
}

void testSerializationLocaleIndependent() {
    // Regressão: um front-end que chama setlocale(LC_ALL, "") num sistema
    // com vírgula decimal (pt_BR, fr_FR, de_DE...) NÃO pode fazer o
    // deserialize truncar "3.75" em 3 (std::stof segue o LC_NUMERIC do C).
    // Sintoma real: todo parâmetro fracionário caía pro mínimo a cada
    // salvar/carregar → o painel abria "sem som" (envelopes de 1 ms).
    const char* saved = std::setlocale(LC_ALL, nullptr);
    const std::string prev = saved ? saved : "C";
    if (!std::setlocale(LC_ALL, "fr_FR.UTF-8")
        && !std::setlocale(LC_ALL, "de_DE.UTF-8")
        && !std::setlocale(LC_ALL, "pt_BR.UTF-8")) {
        std::cout << "  (locale de vírgula indisponível — teste pulado)\n";
        return;
    }

    SignalGraph graph;
    const auto k = graph.add(std::make_unique<Constant>(0.0f));
    const auto g = graph.add(std::make_unique<Gain>(3.75f));   // fracionário
    graph.connect(k, 0, g, 0);
    const std::string patch = graph.serialize();
    EXPECT(patch.find("gain=3.75") != std::string::npos);      // escreve ponto

    SignalGraph restored = SignalGraph::deserialize(patch, testFactory);
    expectNear(restored.node(g).parameterValue("gain"), 3.75f);
    EXPECT(restored.serialize() == patch);                     // idempotente

    std::setlocale(LC_ALL, prev.c_str());
}

void testCableRelation() {
    // A relação É o processo (Warps, Atlas §39): a conexão lê um segundo
    // sinal (`companion`, do bloco anterior) e o combina com o que atravessa.
    auto run = [](const Relation rel, const float amount) {
        SignalGraph graph;
        const auto x = graph.add(std::make_unique<Constant>(0.5f));
        const auto y = graph.add(std::make_unique<Constant>(0.4f));
        const auto sink = graph.add(std::make_unique<Gain>(1.0f));
        graph.connect(x, 0, sink, 0).setRelation(rel, y, 0, amount);
        graph.prepare(48000.0f, 1, 64);
        AudioBlock out(48000.0f, 1, 64);
        graph.process(out, sink, 0);  // companion ainda zero
        graph.process(out, sink, 0);  // companion = y do bloco anterior
        return out.at(0, 0);
    };

    expectNear(run(Relation::None, 1.0f), 0.5f);
    expectNear(run(Relation::RingMod, 1.0f), 0.5f * 0.4f);              // 0.2
    expectNear(run(Relation::RingMod, 0.5f), 0.5f * 0.5f + 0.2f * 0.5f);  // seco/molhado
    expectNear(run(Relation::Difference, 1.0f), 0.5f - 0.4f);          // 0.1
    // Fold: 0.5*(1 + 3*0.4) = 1.1 -> dobra -> 2 - 1.1 = 0.9
    expectNear(run(Relation::Fold, 1.0f), 0.9f);

    // round-trip textual com a relação embutida
    SignalGraph g;
    const auto a = g.add(std::make_unique<Constant>(0.5f));
    const auto b = g.add(std::make_unique<Constant>(0.3f));
    const auto k = g.add(std::make_unique<Gain>(1.0f));
    g.connect(a, 0, k, 0).setRelation(Relation::RingMod, b, 0, 0.75f);
    const std::string patch = g.serialize();
    EXPECT(patch.find("relation=ring companion=1:0 amount=0.75") != std::string::npos);
    SignalGraph restored = SignalGraph::deserialize(patch, testFactory);
    EXPECT(restored.serialize() == patch);
    EXPECT(restored.cable(0).hasRelation());
    EXPECT(restored.cable(0).relation() == Relation::RingMod);
}

void testConnectionMatrix() {
    // A matriz é uma VISÃO do grafo: linhas = saídas, colunas = destinos
    // (entradas + parâmetros), célula = Cable / ParameterLink / vazio.
    SignalGraph graph;
    const auto a = graph.add(std::make_unique<Constant>(0.5f));
    const auto g = graph.add(std::make_unique<Gain>(1.0f));
    graph.connect(a, 0, g, 0).setConductance(0.5f);
    graph.connectToParameter(a, 0, g, "gain", 2.0f);

    const auto rows = graph.matrixSources();
    const auto cols = graph.matrixSlots();
    EXPECT(rows.size() == 2);   // Constant.out + Gain.out
    // Gain: 1 entrada (in) + 1 parâmetro (gain); Constant: 0 entradas + 1
    // parâmetro (level)
    EXPECT(cols.size() == 3);

    int cables = 0, mods = 0, empties = 0;
    for (const auto& src : rows)
        for (const auto& slot : cols) {
            const auto cell = graph.matrixCell(src, slot);
            if (cell.kind == SignalGraph::MatrixCell::Kind::Cable) ++cables;
            else if (cell.kind == SignalGraph::MatrixCell::Kind::Modulation) ++mods;
            else ++empties;
        }
    EXPECT(cables == 1);
    EXPECT(mods == 1);
    EXPECT(empties == static_cast<int>(rows.size() * cols.size()) - 2);

    // o texto marca condução<1 com 'c' e modulação com 'm'
    const std::string text = graph.matrixToText();
    EXPECT(text.find('c') != std::string::npos);
    EXPECT(text.find('m') != std::string::npos);
}

void testConstellationCoupling() {
    // acoplamento: perto = forte, longe = quase mudo. Não muda a topologia.
    EXPECT(SignalGraph::couplingFromDistance(0.0f, 1.0f) == 1.0f);
    EXPECT(SignalGraph::couplingFromDistance(5.0f, 1.0f) < 0.001f);
    EXPECT(SignalGraph::couplingFromDistance(2.0f, 0.0f) == 1.0f);  // raio 0 = desativado

    SignalGraph graph;
    const auto src = graph.add(std::make_unique<Constant>(1.0f));
    const auto sink = graph.add(std::make_unique<Gain>(1.0f));
    graph.connect(src, 0, sink, 0);
    graph.setNodePosition(src, 0.0f, 0.0f);
    graph.setNodePosition(sink, 0.0f, 0.0f);  // coincidentes
    graph.applyConstellation(1.0f);
    graph.prepare(48000.0f, 1, 64);
    AudioBlock out(48000.0f, 1, 64);
    graph.process(out, sink, 0);
    expectNear(out.at(0, 0), 1.0f);  // acoplamento pleno

    graph.setNodePosition(sink, 4.0f, 0.0f);  // longe
    graph.applyConstellation(1.0f);
    for (int b = 0; b < 20; ++b) graph.process(out, sink, 0);
    EXPECT(blockPeak(out) < 0.01f);  // praticamente desconectado pela distância

    graph.clearConstellation();
    for (int b = 0; b < 20; ++b) graph.process(out, sink, 0);
    expectNear(out.at(0, 0), 1.0f);  // volta ao neutro
}

void testSemanticBus() {
    // conectar por SIGNIFICADO: um nó contribui pra uma qualidade, um
    // parâmetro de outro nó segue essa qualidade - sem cabo explícito.
    SignalGraph graph;
    const auto meter = graph.add(std::make_unique<Constant>(0.8f));
    const auto carrier = graph.add(std::make_unique<Constant>(1.0f));
    const auto vca = graph.add(std::make_unique<Gain>(0.0f));
    graph.connect(carrier, 0, vca, 0);
    graph.contributeQuality(meter, 0, SignalGraph::Quality::Energy, 1.0f);
    graph.followQuality(SignalGraph::Quality::Energy, vca, "gain", 2.0f);
    graph.prepare(48000.0f, 1, 64);

    AudioBlock out(48000.0f, 1, 64);
    graph.process(out, vca, 0);  // 1º bloco: qualidade ainda 0 -> gain 0
    graph.process(out, vca, 0);  // 2º: energia = 0,8 -> gain = 1,6
    expectNear(graph.qualityValue(SignalGraph::Quality::Energy), 0.8f);
    expectNear(out.at(0, 0), 1.6f);

    EXPECT(std::string(SignalGraph::qualityName(SignalGraph::Quality::Brightness))
           == "brightness");

    // duas contribuições na mesma qualidade -> média
    SignalGraph g2;
    const auto a = g2.add(std::make_unique<Constant>(0.4f));
    const auto b = g2.add(std::make_unique<Constant>(0.6f));
    const auto k = g2.add(std::make_unique<Gain>(1.0f));
    const auto c = g2.add(std::make_unique<Constant>(1.0f));
    g2.connect(c, 0, k, 0);
    g2.contributeQuality(a, 0, SignalGraph::Quality::Tension, 1.0f);
    g2.contributeQuality(b, 0, SignalGraph::Quality::Tension, 1.0f);
    g2.followQuality(SignalGraph::Quality::Tension, k, "gain", 1.0f);
    g2.prepare(48000.0f, 1, 64);
    AudioBlock o2(48000.0f, 1, 64);
    g2.process(o2, k, 0);
    g2.process(o2, k, 0);
    expectNear(g2.qualityValue(SignalGraph::Quality::Tension), 0.5f);  // (0,4+0,6)/2

    // round-trip textual das 3 camadas: posições + qin + qout
    SignalGraph g3;
    const auto s3 = g3.add(std::make_unique<Constant>(0.7f));
    const auto k3 = g3.add(std::make_unique<Gain>(1.0f));
    const auto c3 = g3.add(std::make_unique<Constant>(1.0f));
    g3.connect(c3, 0, k3, 0);
    g3.setNodePosition(s3, 1.5f, -2.0f);
    g3.contributeQuality(s3, 0, SignalGraph::Quality::Motion, 1.5f, 0.1f);
    g3.followQuality(SignalGraph::Quality::Motion, k3, "gain", 2.0f, 0.3f);
    const std::string patch3 = g3.serialize();
    EXPECT(patch3.find("pos 0 1.5 -2") != std::string::npos);
    EXPECT(patch3.find("qin 0:0 motion weight=1.5 bias=0.1") != std::string::npos);
    EXPECT(patch3.find("qout motion -> 1:gain depth=2 offset=0.3") != std::string::npos);
    SignalGraph r3 = SignalGraph::deserialize(patch3, testFactory);
    EXPECT(r3.serialize() == patch3);
    const auto p = r3.nodePosition(0);
    expectNear(p.first, 1.5f);
    expectNear(p.second, -2.0f);
}

}  // namespace

int main() {
    testControlSnapshot();
    testChainAndFanIn();
    testDisconnectAndReconnect();
    testConnectionRupture();
    testFeedback();
    testCableConductance();
    testParameterModulation();
    testAdditiveParameterBase();
    testCableRelation();
    testConnectionMatrix();
    testConstellationCoupling();
    testSemanticBus();
    testSerialization();
    testSerializationLocaleIndependent();

    if (g_failures == 0) {
        std::cout << "RASGO Modular signal graph tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
