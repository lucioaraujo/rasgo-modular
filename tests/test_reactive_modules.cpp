#include "panel/ModuleCatalog.hpp"
#include "panel/LearnCatalog.hpp"
#include "panel/MotionEngine.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <limits>
#include <new>

using namespace rasgo::modular;

namespace { bool trackHeap = false; std::size_t heapCalls = 0; }
#if defined(_MSC_VER)
#define HEAP_PROBE_NOINLINE __declspec(noinline)
#else
#define HEAP_PROBE_NOINLINE __attribute__((noinline))
#endif
HEAP_PROBE_NOINLINE void* operator new(std::size_t n) {
    if (trackHeap) ++heapCalls;
    if (void* p = std::malloc(n == 0 ? 1 : n)) return p;
    throw std::bad_alloc();
}
HEAP_PROBE_NOINLINE void* operator new[](std::size_t n) { return ::operator new(n); }
HEAP_PROBE_NOINLINE void operator delete(void* p) noexcept { std::free(p); }
HEAP_PROBE_NOINLINE void operator delete[](void* p) noexcept { std::free(p); }
HEAP_PROBE_NOINLINE void operator delete(void* p, std::size_t) noexcept { std::free(p); }
HEAP_PROBE_NOINLINE void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {
int failures = 0;
void check(bool good, const char* name) {
    if (!good && failures++ < 20) std::cerr << "FAIL: " << name << '\n';
}

struct Rig {
    Signal& module;
    std::vector<AudioBlock> in, out;
    std::vector<const AudioBlock*> inputs;
    Rig(Signal& m, float sr = 48000, std::size_t frames = 128, std::size_t channels = 1)
        : module(m), in(m.inputCount(), AudioBlock(sr, channels, frames)),
          out(m.outputCount(), AudioBlock(sr, channels, frames)), inputs(m.inputCount(), nullptr) {
        m.prepare(sr, frames);
    }
    void connect(std::size_t i) { inputs[i] = &in[i]; }
    void process() { module.process(inputs, out); }
};

void testBandsAndStereo() {
    constexpr double pi = 3.14159265358979323846;
    const float rates[] = {8000, 44100, 48000, 96000, 192000};
    for (float sr : rates) {
        for (int band = 0; band < 3; ++band) {
            Listen l;
            Rig r(l, sr, 128, 2); r.connect(0);
            const float hz = band == 0 ? 80 : band == 1 ? 700 : std::min(7000.0f, sr * 0.4f);
            std::array<float, 3> measured{};
            const int blocks = static_cast<int>(sr * 0.35f / 128);
            for (int b = 0; b < blocks; ++b) {
                for (std::size_t f = 0; f < 128; ++f) {
                    const float x = static_cast<float>(0.5 * std::sin(2 * pi * hz * (b * 128 + f) / sr));
                    r.in[0].at(0, f) = x;
                    r.in[0].at(1, f) = -x;
                }
                r.process();
                for (std::size_t f = 0; f < 128; ++f) {
                    check(r.out[0].at(0, f) == r.in[0].at(0, f)
                          && r.out[0].at(1, f) == r.in[0].at(1, f), "stereo THRU exact");
                    for (std::size_t p = 1; p < 7; ++p)
                        check(std::isfinite(r.out[p].at(0, f)), "finite control outputs");
                }
            }
            for (std::size_t p = 0; p < 3; ++p) measured[p] = r.out[1 + p].at(0, 127);
            check(measured[static_cast<std::size_t>(band)] > 0.2f, "band detects its tone, including antiphase stereo");
            for (int other = 0; other < 3; ++other)
                if (other != band)
                    check(measured[static_cast<std::size_t>(band)] > measured[static_cast<std::size_t>(other)] * 2.5f,
                          "dominant band rejects distant tones");
        }
    }
}

int burstHits(float floor, float rearm, bool continuous = false) {
    Listen l; l.setParameter("floor", floor); l.setParameter("rearm", rearm);
    Rig r(l); r.connect(0);
    bool previous = false; int count = 0;
    for (int b = 0; b < 375; ++b) {
        for (std::size_t f = 0; f < 128; ++f) {
            const int n = b * 128 + static_cast<int>(f);
            const int local = n % 9600;
            const float amplitude = continuous ? 0.5f : local < 1440 ? 0.04f : 0;
            r.in[0].at(0, f) = amplitude * std::sin(6.28318530718f * 80 * static_cast<float>(n) / 48000);
        }
        r.process();
        for (std::size_t f = 0; f < 128; ++f) {
            const bool hit = r.out[4].at(0, f) > 0.5f;
            if (hit && !previous) ++count;
            previous = hit;
        }
    }
    return count;
}

void testDetectorsAndReset() {
    check(burstHits(-72, 70) == 5, "five separated low attacks");
    check(burstHits(-18, 70) == 0, "floor prevents quiet attacks");
    check(burstHits(-72, 250) < burstHits(-72, 20), "rearm has a measured effect");
    check(burstHits(-54, 70, true) == 1, "sustained tone is not a repeating clock");
    Listen l;
    Rig r(l, 48000, 64, 8); r.connect(0);
    for (std::size_t c = 0; c < 8; ++c)
        for (std::size_t f = 0; f < 64; ++f)
            r.in[0].at(c, f) = f == 1 ? std::numeric_limits<float>::infinity()
                : f == 2 ? std::numeric_limits<float>::quiet_NaN() : 0.4f;
    r.process();
    for (const auto& o : r.out)
        for (std::size_t c = 0; c < 8; ++c)
            for (std::size_t f = 0; f < 64; ++f)
                check(std::isfinite(o.at(c, f)), "invalid input does not poison output");
    l.prepare(44100, 64); r.inputs[0] = nullptr; r.process();
    for (const auto& o : r.out)
        for (std::size_t f = 0; f < 64; ++f) check(o.at(0, f) == 0, "prepare resets silence");
}

std::vector<float> listenPartition(std::size_t block) {
    Listen l; Rig r(l, 48000, block); r.connect(0);
    std::vector<float> values; values.reserve(32768 * 6);
    for (std::size_t n = 0; n < 32768; n += block) {
        for (std::size_t f = 0; f < block; ++f)
            r.in[0].at(0, f) = (n + f) % 7000 < 1300
                ? 0.4f * std::sin(0.04f * static_cast<float>(n + f)) : 0;
        r.process();
        for (std::size_t f = 0; f < block; ++f)
            for (std::size_t p = 1; p < 7; ++p) values.push_back(r.out[p].at(0, f));
    }
    return values;
}

struct Phrases { std::vector<float> pitches, phases, accents; int ends = 0, gates = 0; };
Phrases phraseRun(float direction, float variance, float cadence, float space = 0,
                  float stimulus = 1, int seed = 7, std::size_t block = 128) {
    Phrase p;
    p.setParameter("length", 4); p.setParameter("direction", direction);
    p.setParameter("variation", variance); p.setParameter("cadence", cadence);
    p.setParameter("space", space); p.setParameter("seed", static_cast<float>(seed));
    Rig r(p, 48000, block);
    r.connect(0); r.connect(1); r.connect(3);
    Phrases result; bool gate = false, end = false;
    for (std::size_t n = 0; n < 8192; n += block) {
        for (std::size_t f = 0; f < block; ++f) {
            r.in[0].at(0, f) = 0.5f;
            r.in[1].at(0, f) = (n + f) % 512 == 0 ? 1.0f : 0;
            r.in[3].at(0, f) = stimulus;
        }
        r.process();
        for (std::size_t f = 0; f < block; ++f) {
            if ((n + f) % 512 == 0) {
                result.pitches.push_back(r.out[0].at(0, f));
                result.phases.push_back(r.out[4].at(0, f));
                result.accents.push_back(r.out[2].at(0, f));
            }
            const bool nowGate = r.out[1].at(0, f) >= 0.5f, nowEnd = r.out[3].at(0, f) >= 0.5f;
            if (nowGate && !gate) ++result.gates;
            if (nowEnd && !end) ++result.ends;
            gate = nowGate; end = nowEnd;
            for (const auto& o : r.out) check(std::isfinite(o.at(0, f)), "phrase remains finite");
        }
    }
    return result;
}

void testPhraseBehavior() {
    const auto up = phraseRun(1, 0, 0), down = phraseRun(-1, 0, 0);
    check(up.gates == 16 && up.ends == 4, "notes rearticulate and phrase endings count");
    for (std::size_t i = 0; i < 16; ++i) {
        check(std::fabs(up.pitches[i] - (0.5f + static_cast<float>(i % 4) / 3)) < 1e-6f, "ascending contour and anchor");
        check(std::fabs(down.pitches[i] - (0.5f - static_cast<float>(i % 4) / 3)) < 1e-6f, "descending contour");
    }
    const auto repeat = phraseRun(0, 0, 0), varied = phraseRun(0, 1, 0), resolved = phraseRun(1, 0, 1);
    for (std::size_t i = 4; i < 16; ++i) check(repeat.pitches[i] == repeat.pitches[i % 4], "motif repeats with VAR zero");
    check(repeat.pitches != varied.pitches, "variation changes actual note pitches");
    check(repeat.pitches != phraseRun(0, 0, 0, 0, 1, 99).pitches, "seed changes motif");
    for (std::size_t i = 3; i < 16; i += 4) check(resolved.pitches[i] == 0.5f, "cadence resolves to anchor");
    check(phraseRun(0, 0, 0, 1, 0).gates == 8, "space leaves first and last notes");
    check(up.accents[0] > up.accents[1], "beginning has stronger articulation");
    check(phraseRun(0, 0, 0, 0, 0).accents[0] < repeat.accents[0], "stimulus changes accents");
    check(repeat.pitches == phraseRun(0, 0, 0, 0, 1, 7, 1).pitches, "phrase invariant to block size");
    Phrase p; Rig r(p); r.process();
    check(r.out[1].at(0, 127) == 0 && r.out[4].at(0, 127) == 0, "no autonomous clock");
    r.connect(1); r.in[1].at(0, 0) = 1; r.process();
    check(r.out[1].at(0, 1) == 1, "external gesture articulates");
    r.in[1].clear(); r.connect(4); r.in[4].at(0, 0) = 1; r.process();
    check(r.out[3].at(0, 0) == 0, "return waits for a clock");
    r.in[4].clear(); r.in[1].at(0, 0) = 1; r.process();
    check(r.out[3].at(0, 0) == 1 && r.out[0].at(0, 0) == 0, "requested return concludes at anchor");
    r.in[1].clear(); r.connect(2); r.in[2].at(0, 0) = 1; r.process();
    check(r.out[1].at(0, 0) == 0 && r.out[4].at(0, 0) == 0, "reset clears articulation and position");
}

void testNoHeap() {
    Listen l; Phrase p; Rig a(l), b(p);
    a.connect(0); b.connect(1);
    b.in[1].at(0, 0) = 1;
    const auto begin = std::chrono::steady_clock::now();
    heapCalls = 0; trackHeap = true;
    for (int i = 0; i < 4000; ++i) { a.process(); b.process(); }
    trackHeap = false;
    check(heapCalls == 0, "new modules allocate no heap in process");
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
    std::cout << "LISTEN + PHRASE mono 128 frames: " << seconds / 4000 * 1e6
              << " us/block; " << seconds / (4000.0 * 128 / 48000) * 100
              << "% real-time budget; heap calls: " << heapCalls << '\n';
}

class Burst final : public Signal {
public:
    Burst() : Signal({}, {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "TEST-BURST"; }
    void prepare(float sr, std::size_t block) override {
        Signal::prepare(sr, block); n_ = 0;
    }
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& out) noexcept override {
        for (std::size_t f = 0; f < out[0].frames(); ++f, ++n_) {
            const auto local = n_ % 9600;
            out[0].at(0, f) = local < 1600 ? 0.7f
                * std::sin(6.28318530718f * 80 * static_cast<float>(local) / 48000)
                * std::exp(-static_cast<float>(local) / 600) : 0;
        }
    }
private:
    std::size_t n_ = 0;
};

void testReactiveVoice() {
    using rasgo::panel::makeModule;
    SignalGraph g;
    const auto source = g.add(std::make_unique<Burst>());
    const auto listen = g.add(makeModule("LISTEN"));
    const auto phrase = g.add(makeModule("PHRASE"));
    const auto quant = g.add(makeModule("QUANTIZER"));
    const auto osc = g.add(makeModule("OSC"));
    const auto env = g.add(makeModule("ENVELOPE"));
    g.node(phrase).setParameter("space", 0);
    g.node(quant).setParameter("range", 1);
    g.connect(source, 0, listen, 0); g.connect(listen, 4, phrase, 1);
    g.connect(listen, 1, phrase, 3); g.connect(phrase, 0, quant, 0);
    g.connect(quant, 0, osc, 0); g.connect(osc, 0, env, 0);
    g.connect(phrase, 1, env, 1);
    const auto patch = g.serialize();
    auto restored = SignalGraph::deserialize(patch, [](const std::string& t) -> std::unique_ptr<Signal> {
        if (t == "TEST-BURST") return std::make_unique<Burst>();
        return rasgo::panel::makeModule(t);
    });
    g.prepare(48000, 1, 128); restored.prepare(48000, 1, 128);
    g.setActiveOutput(env); restored.setActiveOutput(env);
    AudioBlock out(48000, 1, 128), twin(48000, 1, 128);
    double sum = 0; int notes = 0; bool lastGate = false;
    for (int b = 0; b < 400; ++b) {
        g.process(out, env, 0); restored.process(twin, env, 0);
        const auto* gate = g.lastOutput(phrase, 1);
        for (std::size_t f = 0; f < 128; ++f) {
            const bool high = gate->at(0, f) > 0.5f;
            if (high && !lastGate) ++notes;
            lastGate = high;
            check(out.at(0, f) == twin.at(0, f), "saved reactive patch renders identically");
            check(std::isfinite(out.at(0, f)), "reactive voice finite");
            sum += out.at(0, f) * out.at(0, f);
        }
    }
    const double rms = std::sqrt(sum / (400 * 128));
    check(notes >= 5 && notes <= 6, "audio attacks actually play PHRASE");
    check(rms > 0.001, "LISTEN-PHRASE-QUANTIZER-OSC-ENVELOPE is audible");
    std::cout << "reactive voice notes=" << notes << ", RMS=" << rms << '\n';
}

class ExampleOut final : public Signal {
public:
    ExampleOut() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "OUT"; }
    void process(const std::vector<const AudioBlock*>& in, std::vector<AudioBlock>& out) noexcept override {
        if (in[0]) out[0].copyFrom(*in[0]); else out[0].clear();
    }
};

void testPlayableExample(const bool panels = false) {
    const auto path = panels
        ? std::filesystem::path(REACTIVE_EXAMPLE_PATH).parent_path() / "painel_tutoriais.rmp"
        : std::filesystem::path(REACTIVE_EXAMPLE_PATH);
    std::ifstream file(path);
    check(file.good(), "example patch exists");
    std::ostringstream text; text << file.rdbuf();
    check(text.str().size() < 8192, "example is a bounded patch, not audio");
    auto g = SignalGraph::deserialize(text.str(), [](const std::string& t) -> std::unique_ptr<Signal> {
        if (t == "OUT") return std::make_unique<ExampleOut>();
        return rasgo::panel::makeModule(t);
    });
    check(g.nodeCount() == (panels ? 12u : 11u) && g.cableCount() == (panels ? 15u : 14u),
          "example instantiates complete intended patch");
    if (panels) check(g.node(11).type() == "FILTER", "panel fixture includes the filter");
    // Estes dois arquivos demonstram a cadeia LISTEN/PHRASE, não a vista
    // RACK. Checar o alcance evita indicá-los outra vez como teste da
    // diferença TODOS ↔ SAÍDA: neles essa diferença não existe.
    const auto feeding = g.nodesFeeding(10);
    std::size_t inRack = 0, reachingOutput = 0;
    for (std::size_t id = 0; id < g.nodeCount(); ++id) {
        if (g.node(id).type() == "OUT") continue;
        ++inRack;
        if (id < feeding.size() && feeding[id]) ++reachingOutput;
    }
    check(inRack == reachingOutput, "LISTEN/PHRASE example does not demonstrate RACK filtering");
    std::cout << path.filename().string() << ": RACK TODOS=" << inRack
              << ", SAIDA=" << reachingOutput << " (mesma vista por projeto)\n";
    g.prepare(48000, 2, 128); g.setActiveOutput(10);
    AudioBlock output(48000, 2, 128);
    double sum = 0, voiceSum = 0, peak = 0;
    int notes = 0; bool lastGate = false;
    for (int b = 0; b < 3200; ++b) {
        g.process(output, 10, 0);
        const auto* gate = g.lastOutput(3, 1);
        const auto* voice = g.lastOutput(7, 0);
        for (std::size_t f = 0; f < 128; ++f) {
            const bool high = gate->at(0, f) >= 0.5f;
            if (high && !lastGate) ++notes;
            lastGate = high;
            check(std::isfinite(output.at(0, f)), "playable example finite");
            sum += output.at(0, f) * output.at(0, f);
            voiceSum += voice->at(0, f) * voice->at(0, f);
            peak = std::max(peak, static_cast<double>(std::fabs(output.at(0, f))));
        }
    }
    const double rms = std::sqrt(sum / (3200 * 128));
    check(notes >= 6, "example audio plays the phrase, not just the drum");
    check(voiceSum > 0.1 && rms > 0.001 && peak < 0.96, "example melody present and protected master output");
    std::cout << path.filename().string() << ": notes=" << notes << ", RMS=" << rms << ", peak=" << peak << '\n';
}

void testRackInsertion() {
    using namespace rasgo::panel;
    const auto typeOf = [](const std::string& t) { return t; };
    const auto insert = [&](std::vector<std::string>& order, const std::string& type) {
        const auto original = order;
        const auto at = moduleInsertionIndex(order, type, typeOf);
        check(at <= order.size(), "insertion index is in bounds");
        order.insert(order.begin() + static_cast<std::ptrdiff_t>(at), type);
        auto withoutNew = order;
        withoutNew.erase(withoutNew.begin() + static_cast<std::ptrdiff_t>(at));
        check(withoutNew == original, "insertion never moves existing modules");
    };
    std::vector<std::string> full, legacy;
    for (const auto& group : moduleCatalog()) {
        auto sorted = group.types;
        sortFamilyForDisplay(sorted);
        for (const char* t : sorted) {
            full.emplace_back(t);
            if (std::string(t) != "LISTEN" && std::string(t) != "PHRASE")
                legacy.emplace_back(t);
        }
    }
    for (bool listenFirst : {false, true}) {
        auto order = legacy;
        insert(order, listenFirst ? "LISTEN" : "PHRASE");
        insert(order, listenFirst ? "PHRASE" : "LISTEN");
        check(order == full, "new modules join their family regardless of insertion order");
    }
    // Every type, not just the new two, must follow the same rule.
    std::vector<std::string> fromEmpty;
    for (auto i = full.rbegin(); i != full.rend(); ++i) insert(fromEmpty, *i);
    check(fromEmpty == full, "reverse insertion reconstructs all 60 catalogue positions");
    for (const auto& type : full) {
        auto order = full;
        const auto first = std::find(order.begin(), order.end(), type) - order.begin();
        insert(order, type);
        check(order[static_cast<std::size_t>(first) + 1] == type,
              "duplicate joins the existing type");
        insert(order, type);
        check(order[static_cast<std::size_t>(first) + 2] == type,
              "third instance follows the last duplicate");
    }
    std::vector<std::string> custom{"MASTER", "QUANTIZER", "OSC", "HARMONY", "SCOPE"};
    insert(custom, "PHRASE");
    check(custom == std::vector<std::string>{"MASTER", "PHRASE", "QUANTIZER", "OSC", "HARMONY", "SCOPE"},
          "manual rack uses a same-family anchor without sorting neighbours");
    insert(custom, "LISTEN");
    check(custom.front() == "LISTEN", "OUT insertion retains MIXER and MASTER as tail-ranked types");
    std::vector<std::string> absentFamily{"OSC", "SPACE", "MASTER"};
    insert(absentFamily, "PHRASE");
    check(absentFamily == std::vector<std::string>{"OSC", "PHRASE", "SPACE", "MASTER"},
          "missing family is anchored before the next family");
    std::vector<std::string> finalFamily{"OSC", "PHRASE"};
    insert(finalFamily, "LISTEN");
    check(finalFamily.back() == "LISTEN", "missing final family appends safely");
    insert(finalFamily, "UNKNOWN-TYPE");
    check(finalFamily.back() == "UNKNOWN-TYPE", "unknown type has no guessed family");

    // Actual node IDs differ from display positions. Computing the position
    // must not touch graph serialization (parameters, cables, RNG/node order).
    SignalGraph graph;
    std::vector<std::size_t> shown;
    for (const char* t : {"QUANTIZER", "MASTER", "PHRASE", "LISTEN"}) {
        const auto id = graph.add(makeModule(t));
        const auto before = graph.serialize();
        const auto at = moduleInsertionIndex(shown, std::string(t),
            [&](std::size_t node) { return graph.node(node).type(); });
        shown.insert(shown.begin() + static_cast<std::ptrdiff_t>(at), id);
        check(graph.serialize() == before, "display insertion leaves DSP graph untouched");
    }
    check(shown == std::vector<std::size_t>{2, 0, 3, 1}, "rack orders node IDs, not DSP instantiation");
    auto restored = SignalGraph::deserialize(graph.serialize(), makeModule);
    check(moduleInsertionIndex(shown, std::string("PHRASE"),
              [&](std::size_t node) { return restored.node(node).type(); }) == 1,
          "same insertion rule works after graph round-trip");
    std::cout << "rack insertion: families, duplicates, manual order and graph preservation OK\n";
}

void testCatalogAndPersistence() {
    using namespace rasgo::panel;
    std::size_t all = 0, original = 0;
    for (const auto& g : moduleCatalog()) all += g.types.size();
    for (const auto& g : seedModuleCatalog()) {
        original += g.types.size();
        for (const char* t : g.types)
            check(std::string(t) != "PHRASE" && std::string(t) != "LISTEN", "published seed rack preserved");
    }
    check(all == original + 2, "both new modules visible in catalogue");
    for (const char* t : {"LISTEN", "PHRASE"}) {
        auto m = makeModule(t); check(m && m->type() == t, "factory constructs new modules");
        for (Lang lang : {Lang::pt, Lang::en, Lang::fr, Lang::es}) {
            check(lookupLearnModule(t, lang) != nullptr, "module help in four languages");
            for (const auto& w : m->panel().widgets)
                if (!w.bind.empty()) {
                    const auto* e = lookupLearn(t, w.bind, lang);
                    check(e && !e->quick.empty(), "all new controls and ports documented");
                }
        }
    }
    SignalGraph g;
    const auto l = g.add(makeModule("LISTEN")), p = g.add(makeModule("PHRASE"));
    g.node(p).setParameter("seed", 4321); g.node(l).setParameter("low_cut", 120);
    g.connect(l, 4, p, 1); g.connect(l, 1, p, 3);
    const auto text = g.serialize();
    auto restored = SignalGraph::deserialize(text, makeModule);
    check(restored.serialize() == text, "patch ports and parameters round-trip");
    check(restored.node(0).parameterValue("low_cut") == 120
          && restored.node(1).parameterValue("seed") == 4321, "saved controls restored");
    MotionEngine motion;
    motion.inhabit(g, 123, {l, p});
    for (int i = 0; i < 600; ++i) motion.tick(g, 1.0f / 30.0f);
    check(g.node(l).parameterValue("low_cut") == 120
          && g.node(l).parameterValue("high_cut") == 2000
          && g.node(l).parameterValue("sens") == 0.5f,
          "VARIA preserves analysis calibration");
    check(g.node(p).parameterValue("seed") == 4321, "VARIA preserves motif identity");
    std::cout << "catalogue: " << all << " types; published seed rack: " << original << '\n';
}
} // namespace

int main() {
    testBandsAndStereo(); testDetectorsAndReset(); testPhraseBehavior();
    check(listenPartition(1) == listenPartition(256), "listen invariant to block partition");
    testNoHeap(); testReactiveVoice(); testPlayableExample(); testRackInsertion(); testCatalogAndPersistence();
    testPlayableExample(true);
    if (failures) { std::cerr << failures << " failures\n"; return 1; }
    std::cout << "reactive modules: OK\n";
}
