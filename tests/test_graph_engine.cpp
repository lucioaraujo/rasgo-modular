#include "core/Graph.hpp"
#include "core/ArchiveStorage.hpp"
#include "TempPath.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace rasgo::modular;

namespace {

// Verificação de teste que NÃO some sob NDEBUG. `assert()` vira no-op numa
// build Release, e aí as flags `bool ...Rejected` viravam "set but not
// used" com `-Werror` - a build Release quebrava e os testes ficavam
// vazios. `check()` sempre avalia, conta a falha e o processo sai != 0.
int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define assert(x) check((x), #x)

std::unique_ptr<Module> factory(const std::string& type) {
    if (type == "SOURCE.CONSTANT")
        return std::make_unique<ConstantSource>();
    if (type == "TRANSFORM.GAIN")
        return std::make_unique<Gain>();
    if (type == "METER.VALUE")
        return std::make_unique<Meter>();
    if (type == "INPUT.CONTROL")
        return std::make_unique<ControlSource>();
    if (type == "EVENT.SOURCE")
        return std::make_unique<EventSource>();
    if (type == "EVENT.GATE")
        return std::make_unique<EventGate>();
    if (type == "TIME.CLOCK")
        return std::make_unique<Clock>();
    if (type == "DECISION.PROBABILITY")
        return std::make_unique<Probability>();
    if (type == "ROUTE.SPLIT")
        return std::make_unique<Split>();
    if (type == "MIX.SUM")
        return std::make_unique<Sum>();
    return nullptr;
}

void expectNear(const float actual, const float expected) {
    assert(std::fabs(actual - expected) < 0.0001f);
}

} // namespace

int main() {
    StreamDescriptor audioStream{true, 44100.0, 0.0, 2, 1, 4, "samples", "linear", {"L", "R"}};
    StreamDescriptor sameAudioStream{true, 44100.0, 0.25, 2, 1, 4, "samples", "linear", {"L", "R"}};
    StreamDescriptor descriptorStream{true, 100.0, 0.0, 13, 1, 8, "descriptors", "normalized", {}, StreamCategory::Descriptor};
    assert(audioStream.compatibleWith(sameAudioStream));
    assert(!audioStream.compatibleWith(descriptorStream));

    EventQueue<3> eventQueue;
    assert(eventQueue.push(Event{"late", 0.0f, 20}));
    assert(eventQueue.push(Event{"early", 0.0f, 5}));
    assert(eventQueue.push(Event{"middle", 0.0f, 10}));
    assert(eventQueue.front().type == "early");
    Event dequeued;
    assert(eventQueue.pop(dequeued) && dequeued.timestamp == 5);
    assert(eventQueue.pop(dequeued) && dequeued.timestamp == 10);
    assert(eventQueue.pop(dequeued) && dequeued.timestamp == 20);

    ControlBlock controls(2);
    controls.at(0) = 0.75f;
    controls.at(1) = -0.25f;
    assert(controls.size() == 2);
    controls.clear();
    expectNear(controls.at(0), 0.0f);

    DescriptorBlock descriptors(2, 3);
    descriptors.at(1, 2) = 0.5f;
    assert(descriptors.width() == 2 && descriptors.frames() == 3);
    expectNear(descriptors.at(1, 2), 0.5f);

    EventBlock eventBlock;
    Event blockEvent;
    assert(eventBlock.push(Event{"analysis", 0.2f, 4}));
    assert(eventBlock.size() == 1);
    assert(eventBlock.pop(blockEvent) && blockEvent.type == "analysis");

    StreamDescriptor controlStream{true, 100.0, 0.0, 1, 1, 1, "control", "normalized", {}, StreamCategory::Control};
    MultimodalGraph multimodal;
    const auto multimodalOutput = multimodal.addPort({"audio-out", PortDirection::Output,
                                                       StreamCategory::Audio, audioStream});
    const auto multimodalInput = multimodal.addPort({"audio-in", PortDirection::Input,
                                                      StreamCategory::Audio, audioStream});
    multimodal.connect(multimodalOutput, multimodalInput);
    multimodal.prepare();
    assert(multimodal.prepared());
    assert(multimodal.connectionCount() == 1);
    assert(multimodal.manifest().find("audio-out") != std::string::npos);
    MultimodalPayload payload = AudioBlock(44100.0f, 2, 4);
    assert(payloadCategory(payload) == StreamCategory::Audio);

    PayloadEnvelope envelope;
    envelope.origin = "RASGO.TEST.SOURCE";
    envelope.frameTimestamp = 128;
    envelope.logicalTime = 0.0029;
    envelope.category = StreamCategory::Audio;
    envelope.payload = payload;
    envelope.stream = audioStream;
    envelope.latencyKnown = true;
    envelope.latencySeconds = 0.001;
    envelope.provenance = {"RASGO workspace", "test-envelope", "0.1", "own"};
    assert(envelope.valid());
    assert(envelope.toJson().find("RASGO.TEST.SOURCE") != std::string::npos);
    envelope.origin = "RASGO \"quoted\" source";
    std::string archiveRecord;
    assert(ArchiveSerializer::serializeEnvelope(envelope, archiveRecord));
    assert(archiveRecord.find("rasgo-payload-envelope") != std::string::npos);
    assert(archiveRecord.find("RASGO \\\"quoted\\\" source") != std::string::npos);
    envelope.category = StreamCategory::Control;
    assert(!ArchiveSerializer::serializeEnvelope(envelope, archiveRecord));
    assert(archiveRecord.empty());
    assert(!envelope.valid());
    envelope.category = StreamCategory::Audio;
    envelope.provenance.source.clear();
    assert(!envelope.valid());
    envelope.provenance.source = "RASGO workspace";
    // `std::filesystem::temp_directory_path()` LANÇA se o diretório
    // apontado por TMPDIR não existir, e uma exceção solta num teste
    // aborta o processo sem dizer qual verificação falhou. O helper
    // devolve um caminho sempre — caindo no diretório de trabalho como
    // último recurso — e é o mesmo que os testes de WAV usam desde que a
    // CI mostrou que `/tmp` fixo não existe no Windows.
    const std::filesystem::path archiveDirectory =
        std::filesystem::path(rasgo::test::tempDir()) / "rasgo-modular-archive-test";
    std::error_code archiveError;
    std::filesystem::remove_all(archiveDirectory, archiveError);
    assert(ArchiveStorage::writeEnvelope(envelope, archiveDirectory));
    assert(std::filesystem::exists(archiveDirectory / "manifest.json"));
    assert(std::filesystem::exists(archiveDirectory / "payload.bin"));
    assert(std::filesystem::exists(archiveDirectory / "provenance.json"));
    ArchivePackage archivePackage;
    assert(ArchiveStorage::readPackage(archiveDirectory, archivePackage));
    assert(!archivePackage.payload.empty());
    {
        std::ofstream tamperedPayload(archiveDirectory / "payload.bin",
                                      std::ios::binary | std::ios::app);
        const char tamper = 'x';
        tamperedPayload.write(&tamper, 1);
    }
    assert(!ArchiveStorage::readPackage(archiveDirectory, archivePackage));
    std::filesystem::remove_all(archiveDirectory, archiveError);

    auto& payloadAudio = std::get<AudioBlock>(payload);
    payloadAudio.at(0, 0) = 0.5f;
    MultimodalPayload processedPayload = AudioBlock(44100.0f, 2, 4);
    MultimodalChain audioChain;
    audioChain.add(std::make_unique<AudioPayloadProcessor>(2.0f));
    audioChain.add(std::make_unique<AudioPayloadProcessor>(3.0f));
    audioChain.prepare(StreamCategory::Audio);
    assert(audioChain.processAt(payload, processedPayload, ProcessContext{128, 0.0029, 4}));
    expectNear(std::get<AudioBlock>(processedPayload).at(0, 0), 3.0f);
    assert(audioChain.processorCount() == 2);
    assert(audioChain.realtimeSafeBase());
    assert(audioChain.snapshot().context.frameStart == 128);
    audioChain.reset();

    MultimodalPayload wrongPayload = ControlBlock(1);
    assert(!audioChain.process(wrongPayload, processedPayload));

    bool multimodalMismatchRejected = false;
    try {
        const auto controlInput = multimodal.addPort({"control-in", PortDirection::Input,
                                                       StreamCategory::Control, controlStream});
        multimodal.connect(multimodalOutput, controlInput);
    } catch (const std::invalid_argument&) {
        multimodalMismatchRejected = true;
    }
    assert(multimodalMismatchRejected);

    bool fanInRejected = false;
    try {
        const auto secondOutput = multimodal.addPort({"audio-out-2", PortDirection::Output,
                                                      StreamCategory::Audio, audioStream});
        multimodal.connect(secondOutput, multimodalInput);
    } catch (const std::invalid_argument&) {
        fanInRejected = true;
    }
    assert(fanInRejected);

    Clock clock(10);
    clock.process();
    assert(clock.eventOutput(0).timestamp == 10);
    clock.process();
    assert(clock.eventOutput(0).timestamp == 11);

    Probability probability(7);
    probability.setParameter("probability", 1.0f);
    probability.setInput(0, Event{"pass", 1.0f, 12});
    probability.process();
    assert(probability.eventOutput(0).type == "pass");

    Split split;
    split.setInput(0, 3.0f);
    split.process();
    expectNear(split.output(0), 3.0f);
    expectNear(split.output(1), 3.0f);

    Sum sum;
    sum.setInput(0, 2.0f);
    sum.setInput(1, -0.5f);
    sum.process();
    expectNear(sum.output(0), 1.5f);

    AudioBlock audio(44100.0f, 2, 4);
    audio.at(0, 0) = 0.5f;
    audio.at(1, 3) = -0.25f;
    assert(audio.sampleRate() == 44100.0f);
    assert(audio.channels() == 2);
    assert(audio.frames() == 4);
    audio.clear();
    expectNear(audio.at(0, 0), 0.0f);
    expectNear(audio.at(1, 3), 0.0f);

    AudioBlock audioOutput(44100.0f, 2, 4);
    audio.at(0, 0) = 0.5f;
    audio.at(1, 3) = -0.25f;
    AudioGainProcessor audioGain(2.0f);
    assert(audioGain.processAudio(audio, audioOutput));
    expectNear(audioOutput.at(0, 0), 1.0f);
    expectNear(audioOutput.at(1, 3), -0.5f);
    AudioBlock incompatible(48000.0f, 2, 4);
    assert(!audioGain.processAudio(audio, incompatible));

    AudioGraph audioGraph;
    const auto firstAudioGain = audioGraph.add(std::make_unique<AudioGainProcessor>(2.0f));
    const auto secondAudioGain = audioGraph.add(std::make_unique<AudioGainProcessor>(3.0f));
    audioGraph.connect(firstAudioGain, secondAudioGain);
    audioGraph.prepare(audioStream);
    assert(audioGraph.descriptor().labels.size() == 2);
    AudioBlock graphOutput(44100.0f, 2, 4);
    assert(audioGraph.process(audio, graphOutput));
    expectNear(graphOutput.at(0, 0), 3.0f);
    expectNear(graphOutput.at(1, 3), -1.5f);

    bool nonAudioRejected = false;
    try {
        audioGraph.prepare(descriptorStream);
    } catch (const std::invalid_argument&) {
        nonAudioRejected = true;
    }
    assert(nonAudioRejected);

    Graph graph;
    const auto source = graph.add(std::make_unique<ConstantSource>(2.5f));
    const auto gain = graph.add(std::make_unique<Gain>(4.0f));
    const auto meter = graph.add(std::make_unique<Meter>());
    graph.connect(source, 0, gain, 0);
    graph.connect(gain, 0, meter, 0);
    assert(graph.nodeCount() == 3);
    assert(graph.nodeOutputDescriptor(gain, 0).name == "out");
    graph.setParameter(gain, "gain", 4.0f);
    assert(graph.nodeCount() == 3);
    assert(graph.nodeManifest(gain).find("TRANSFORM.GAIN") != std::string::npos);
    graph.process();
    expectNear(graph.output(meter, 0), 10.0f);

    const auto patch = graph.serialize();
    auto restored = Graph::deserialize(patch, factory);
    restored.process();
    expectNear(restored.output(meter, 0), 10.0f);

    bool cycleRejected = false;
    try {
        Graph cyclic;
        const auto a = cyclic.add(std::make_unique<Gain>());
        const auto b = cyclic.add(std::make_unique<Gain>());
        cyclic.connect(a, 0, b, 0);
        cyclic.connect(b, 0, a, 0);
        cyclic.process();
    } catch (const std::logic_error&) {
        cycleRejected = true;
    }
    assert(cycleRejected);

    bool incompatiblePortsRejected = false;
    try {
        Graph typed;
        const auto control = typed.add(std::make_unique<ControlSource>(1.0f));
        const auto audioGain = typed.add(std::make_unique<Gain>());
        typed.connect(control, 0, audioGain, 0);
    } catch (const std::invalid_argument&) {
        incompatiblePortsRejected = true;
    }
    assert(incompatiblePortsRejected);

    bool incompatibleStreamsRejected = false;
    try {
        auto describedSource = std::make_unique<ConstantSource>();
        auto describedGain = std::make_unique<Gain>();
        describedSource->setOutputStreamDescriptor(0, audioStream);
        describedGain->setInputStreamDescriptor(0, descriptorStream);
        Graph described;
        const auto sourceId = described.add(std::move(describedSource));
        const auto gainId = described.add(std::move(describedGain));
        described.connect(sourceId, 0, gainId, 0);
    } catch (const std::invalid_argument&) {
        incompatibleStreamsRejected = true;
    }
    assert(incompatibleStreamsRejected);

    Graph events;
    const auto eventSource = events.add(std::make_unique<EventSource>(Event{"note_on", 0.75f, 42}));
    const auto eventGate = events.add(std::make_unique<EventGate>());
    events.connect(eventSource, 0, eventGate, 0);
    events.process();
    expectNear(events.output(eventGate, 0), 1.0f);
    assert(events.eventOutput(eventSource, 0).timestamp == 42);
    assert(eventComesBefore(Event{"a", 0.0f, 41}, Event{"b", 0.0f, 42}));

    auto restoredEvents = Graph::deserialize(events.serialize(), factory);
    restoredEvents.process();
    expectNear(restoredEvents.output(eventGate, 0), 1.0f);
    assert(restoredEvents.eventOutput(eventSource, 0).timestamp == 42);

    if (g_failures == 0) {
        std::cout << "RASGO Modular graph engine tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
