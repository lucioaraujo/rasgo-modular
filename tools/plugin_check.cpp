// Hospedeiro de teste do plugin (20/11/2026), sem janela: carrega o VST3,
// toca 20 s a 48 kHz em blocos de 256 (com um acorde MIDI e um transporte
// que anda), confere que a saída é finita, audível e abaixo de 0 dBFS, e
// que o estado salvo abre numa segunda instância. Não substitui o
// `pluginval` nem as DAWs; é o primeiro portão, local e na CI.
//
//   rasgo_modular_plugin_check "<caminho>/Rasgo Modular.vst3"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include <cmath>
#include <cstdlib>
#include <cstdio>

namespace {

struct Transport final : juce::AudioPlayHead {
    double ppq = 0.0, bpm = 120.0; bool playing = true;
    juce::Optional<PositionInfo> getPosition() const override {
        PositionInfo p;
        p.setIsPlaying(playing);
        p.setBpm(bpm);
        p.setPpqPosition(ppq);
        return p;
    }
};

int fail(const char* why) { std::fprintf(stderr, "FALHOU: %s\n", why); return 1; }

std::unique_ptr<juce::AudioPluginInstance> load(juce::AudioPluginFormatManager& fm,
                                                const juce::String& path, juce::String& err) {
    juce::OwnedArray<juce::PluginDescription> found;
    for (auto* f : fm.getFormats())
        if (f->fileMightContainThisPluginType(path)) f->findAllTypesForFile(found, path);
    if (found.isEmpty()) { err = "nenhum plugin no arquivo"; return nullptr; }
    return fm.createPluginInstance(*found[0], 48000.0, 256, err);
}

struct Result { double rms = 0.0; float peak = 0.0f; bool finite = true; };

Result play(juce::AudioPluginInstance& p, Transport& tr, double seconds, bool notes) {
    p.setPlayHead(&tr);
    p.prepareToPlay(48000.0, 256);
    juce::AudioBuffer<float> buf(std::max(2, p.getTotalNumOutputChannels()), 256);
    juce::MidiBuffer midi;
    Result r; double sum = 0.0; long n = 0;
    const int blocks = static_cast<int>(seconds * 48000.0 / 256.0);
    for (int b = 0; b < blocks; ++b) {
        buf.clear(); midi.clear();
        if (notes && b == 100) for (int k : {60, 64, 67}) midi.addEvent(juce::MidiMessage::noteOn(1, k, (juce::uint8) 100), 10);
        if (notes && b == 600) for (int k : {60, 64, 67}) midi.addEvent(juce::MidiMessage::noteOff(1, k), 0);
        p.processBlock(buf, midi);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 256; ++i) {
                const float v = buf.getSample(c, i);
                if (!std::isfinite(v)) r.finite = false;
                r.peak = std::max(r.peak, std::abs(v));
                sum += static_cast<double>(v) * v; ++n;
            }
        tr.ppq += 256.0 * tr.bpm / 60.0 / 48000.0;
        // o timer da interface (VARIA, visores) anda entre os blocos
        if (b % 8 == 0) juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
    }
    p.releaseResources();
    r.rms = n ? std::sqrt(sum / static_cast<double>(n)) : 0.0;
    return r;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) return fail("uso: rasgo_modular_plugin_check <plugin.vst3>");
    const juce::ScopedJuceInitialiser_GUI juce;
    juce::AudioPluginFormatManager fm;
    juce::addDefaultFormatsToManager(fm);
    juce::String err;
    auto a = load(fm, juce::String::fromUTF8(argv[1]), err);
    if (!a) { std::fprintf(stderr, "%s\n", err.toRawUTF8()); return fail("carregar o plugin"); }
    std::printf("plugin: %s (%s), %d entradas, %d saídas, MIDI %s\n",
                a->getName().toRawUTF8(), a->getPluginDescription().pluginFormatName.toRawUTF8(),
                a->getTotalNumInputChannels(), a->getTotalNumOutputChannels(),
                a->acceptsMidi() ? "sim" : "não");
    Transport tr;
    const Result r = play(*a, tr, 20.0, true);
    std::printf("20 s: RMS %.1f dBFS, pico %.1f dBFS\n",
                20.0 * std::log10(std::max(1e-12, r.rms)), 20.0 * std::log10(std::max(1e-12f, r.peak)));
    if (!r.finite) return fail("saída não finita");
    if (r.rms < 1e-4) return fail("sem som (o plugin deve soar ao ser inserido)");
    if (r.peak > 1.0f) return fail("pico acima de 0 dBFS");

    juce::MemoryBlock state;
    a->getStateInformation(state);
    if (state.getSize() < 32) return fail("estado vazio");
    if (std::getenv("RASGO_CHECK_DUMP")) {
        const juce::String s(static_cast<const char*>(state.getData()), state.getSize());
        std::printf("estado (início): %s\n", s.toRawUTF8());
    }
    auto b = load(fm, juce::String::fromUTF8(argv[1]), err);
    if (!b) return fail("segunda instância");
    // sem rodar o laço de mensagens no meio: o VARIA (timer) não anda, e o
    // patch reaberto tem de sair byte a byte igual ao salvo
    b->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    juce::MemoryBlock again;
    b->getStateInformation(again);
    // VST3: o estado vem embrulhado (base64) e nada anda entre as duas
    // chamadas — tem de ser idêntico. LV2: o estado vem em texto e os
    // timers da interface rodam na thread própria do plugin, então o VARIA
    // pode mover um knob no meio; confere-se o que define o patch — a seed,
    // os módulos, os cabos e a ordem do painel —, não os valores que o
    // VARIA move.
    const auto skeleton = [](const juce::MemoryBlock& m) {
        juce::String s(static_cast<const char*>(m.getData()), m.getSize());
        // LV2: o patch vai no preset .ttl, no base64 do JUCE ("tamanho.dados")
        if (s.contains("StateString> \"")) {
            const auto enc = s.fromFirstOccurrenceOf("StateString> \"", false, false)
                              .upToFirstOccurrenceOf("\"", false, false);
            juce::MemoryBlock raw;
            if (raw.fromBase64Encoding(enc))
                s = juce::String(static_cast<const char*>(raw.getData()), raw.getSize());
        }
        if (!s.contains("panel shown")) return juce::String();
        juce::String out;
        for (const auto& line : juce::StringArray::fromLines(s)) {
            if (line.startsWith("node "))
                out << line.upToFirstOccurrenceOf("=", false, false)
                          .upToLastOccurrenceOf(" ", false, false) << "\n";
            else if (line.isNotEmpty() && !line.containsChar('='))
                out << line << "\n";
            else if (line.startsWith("cable") || line.startsWith("connect"))
                out << line << "\n";
        }
        return out;
    };
    const juce::String k1 = skeleton(state), k2 = skeleton(again);
    const bool same = k1.isNotEmpty() ? (k1 == k2) : (again == state);
    if (!same && k1.isNotEmpty()) {
        const auto l1 = juce::StringArray::fromLines(k1), l2 = juce::StringArray::fromLines(k2);
        for (int i = 0; i < std::max(l1.size(), l2.size()); ++i)
            if (l1[i] != l2[i]) {
                std::printf("  linha %d: '%s' x '%s'\n", i, l1[i].toRawUTF8(), l2[i].toRawUTF8());
                break;
            }
    }
    std::printf("estado: %d bytes; reaberto idêntico: %s\n",
                static_cast<int>(state.getSize()), same ? "sim" : "não");
    if (!same) return fail("o estado não voltou igual");
    Transport tr2;
    const Result r2 = play(*b, tr2, 5.0, false);
    if (!r2.finite || r2.rms < 1e-4) return fail("a instância restaurada não soa");
    b.reset(); a.reset();
    std::printf("plugin check: tudo certo\n");
    return 0;
}
