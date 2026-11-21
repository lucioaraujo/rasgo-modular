// Rasgo Modular como PLUGIN (VST3 e LV2; experimental, 20/11/2026).
// Incluído no fim de `RasgoModularApp.cpp` quando `RASGO_PLUGIN=1`: o
// instrumento é o MESMO MainComponent do aplicativo, sem placa de som.
//
// - O processador é dono do MainComponent: o patch continua tocando com o
//   editor fechado (o VARIA e os visores andam no timer, como no app). O
//   editor só mostra o componente.
// - Áudio: a DAW chama `processBlock`, que chama o `getNextAudioBlock` do
//   app (os sub-blocos de 256, a entrada para o SIGNAL-IN, tudo igual).
// - MIDI da DAW → SIGNAL-IN (ligue o ON e cabeie uma saída, como no app),
//   com o instante exato de cada evento no bloco.
// - Transporte da DAW → MIDI clock sintético (START, STOP, CONTINUE, song
//   position e 24 clocks por semínima na posição exata): o CLK, o RUN e o
//   RST do SIGNAL-IN seguem o play e o andamento da DAW.
// - O patch é salvo no projeto da DAW (o texto do `.rmp`).
//
// Plano em DESENVOLVIMENTO.md ("VST3 / CLAP / AU") e na auditoria 2 §7.

namespace {

// A interface (o MainComponent, os timers do VARIA e dos visores) vive na
// thread de mensagens do JUCE. No LV2 ela é uma thread PRÓPRIA do plugin,
// separada da do hospedeiro (achado do teste, 20/11/2026: o estado
// carregava atrasado e a instância era destruída no meio da carga).
// Criar, carregar e destruir o patch passam por aqui: na thread de
// mensagens, esperando terminar.
template <class F>
void onMessageThread(F&& f) {
    auto* mm = juce::MessageManager::getInstance();
    // Quem já segura a trava da thread de mensagens roda aqui mesmo: a
    // thread de mensagens está parada esperando essa trava, e pedir a ela
    // seria esperar para sempre (achado no scanner VST3 do Ardour,
    // 21/11/2026: o wrapper do JUCE destrói o processador com a trava).
    if (mm->isThisTheMessageThread() || mm->currentThreadHasLockedMessageManager()) {
        f();
        return;
    }
    mm->callFunctionOnMessageThread(
        [](void* p) -> void* { (*static_cast<std::remove_reference_t<F>*>(p))(); return nullptr; },
        &f);
}

class RasgoModularProcessor final : public juce::AudioProcessor {
public:
    // A entrada estéreo é OPCIONAL no VST3 (instrumento; a DAW liga se
    // quiser mandar som ao SIGNAL-IN). No LV2 as portas são fixas no .ttl:
    // com entrada declarada, o Ardour o lista como efeito e não como
    // instrumento (21/11/2026), e uma entrada declarada mas desligada fazia o
    // cliente do JUCE cair. Lá ele é só instrumento, sem entrada de áudio.
    static BusesProperties buses() {
        const bool lv2 = juce::PluginHostType::getPluginLoadedAs()
                         == juce::AudioProcessor::wrapperType_LV2;
        if (lv2)
            return BusesProperties()
                .withOutput("Output", juce::AudioChannelSet::stereo(), true);
        return BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), false)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true);
    }
    RasgoModularProcessor() : AudioProcessor(buses()) {
        // Parâmetros para a DAW automatizar (21/11/2026): poucos e claros —
        // o VARIA (ligado e intensidade) e o ganho do MASTER. A DAW escreve;
        // a interface aplica no timer dela, só quando o valor muda (o slider
        // e o knob na tela continuam livres até a automação andar).
        addParameter(variaOn_ = new juce::AudioParameterBool(
            juce::ParameterID("varia_on", 1), "VARIA", true));
        addParameter(varia_ = new juce::AudioParameterFloat(
            juce::ParameterID("varia", 1), "VARIA amount",
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
        addParameter(master_ = new juce::AudioParameterFloat(
            juce::ParameterID("master_db", 1), "Master (dB)",
            juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), -24.0f));
        onMessageThread([this] {
            main_ = std::make_unique<MainComponent>();
            main_->onTimerHook = [this] { applyHostParameters(); };
        });
    }
    ~RasgoModularProcessor() override {
        onMessageThread([this] { main_.reset(); });
    }

    const juce::String getName() const override { return "Rasgo Modular"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    bool isBusesLayoutSupported(const BusesLayout& layout) const override {
        if (layout.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;
        if (layout.inputBuses.isEmpty()) return true;   // LV2: sem entrada
        const auto in = layout.getMainInputChannelSet();
        return in.isDisabled() || in == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(const double sampleRate, const int block) override {
        sr_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        samplePos_ = 0;
        wasPlaying_ = false;
        nextTick_ = 0;
        main_->prepareToPlay(block, sr_);
    }
    void releaseResources() override { main_->releaseResources(); }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override {
        const juce::ScopedNoDenormals noDenormals;
        const int n = buffer.getNumSamples();
        const double t0 = static_cast<double>(samplePos_) / sr_;
        main_->setHostTime(t0);
        for (const auto meta : midi) {
            const auto* raw = meta.data;
            const int size = meta.numBytes;
            if (size < 1 || raw[0] < 0x80) continue;
            main_->pushHostMidi(raw[0], size > 1 ? raw[1] : 0, size > 2 ? raw[2] : 0,
                                t0 + meta.samplePosition / sr_);
        }
        transportClock(n, t0);
        // sem entrada de áudio, o buffer não traz nada que deva ser lido
        if (getTotalNumInputChannels() == 0) buffer.clear();
        for (int c = 2; c < buffer.getNumChannels(); ++c) buffer.clear(c, 0, n);
        const juce::AudioSourceChannelInfo info(&buffer, 0, n);
        main_->getNextAudioBlock(info);
        midi.clear();
        samplePos_ += n;
    }

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    // O estado: o patch (o texto do .rmp) e, numa primeira linha própria, o
    // tamanho da janela que o músico deixou (21/11/2026). Estados sem essa
    // linha abrem igual.
    static constexpr const char* kEditorTag = "rasgo-plugin-editor ";
    void getStateInformation(juce::MemoryBlock& dest) override {
        std::string text;
        if (editorW > 0 && editorH > 0)
            text = kEditorTag + std::to_string(editorW) + " " + std::to_string(editorH) + "\n";
        text += main_->patchText();
        dest.append(text.data(), text.size());
    }
    void setStateInformation(const void* data, const int size) override {
        if (data == nullptr || size <= 0) return;
        std::string text(static_cast<const char*>(data), static_cast<std::size_t>(size));
        if (text.rfind(kEditorTag, 0) == 0) {
            const auto eol = text.find('\n');
            int w = 0, h = 0;
            if (std::sscanf(text.c_str() + std::strlen(kEditorTag), "%d %d", &w, &h) == 2
                && w >= 960 && h >= 600 && w <= 8192 && h <= 8192) {
                editorW = w; editorH = h;
            }
            text.erase(0, eol == std::string::npos ? text.size() : eol + 1);
        }
        onMessageThread([this, &text] { main_->loadPatchText(text); });
    }

    MainComponent& mainComponent() { return *main_; }

    // no timer da interface: aplica o que a DAW mudou desde a última vez
    void applyHostParameters() {
        const bool on = variaOn_->get();
        const float amount = varia_->get(), db = master_->get();
        if (on != lastOn_ || std::fabs(amount - lastAmount_) > 1e-4f) {
            main_->hostSetVaria(on, amount);
            lastOn_ = on; lastAmount_ = amount;
        }
        if (std::fabs(db - lastDb_) > 1e-3f) {
            main_->hostSetMasterDb(db);
            lastDb_ = db;
        }
    }
    int editorW = 0, editorH = 0;   // 0 = ainda não escolhido: 80% da tela

private:
    // O transporte da DAW como MIDI clock: 24 clocks por semínima, cada um
    // no instante exato dentro do bloco. Play do começo = START; play do
    // meio = song position + CONTINUE; um salto (loop) = song position.
    void transportClock(const int n, const double t0) {
        auto* ph = getPlayHead();
        if (ph == nullptr) return;
        const auto pos = ph->getPosition();
        if (!pos) return;
        const bool playing = pos->getIsPlaying();
        const auto ppqOpt = pos->getPpqPosition();
        const auto bpmOpt = pos->getBpm();
        if (!ppqOpt || !bpmOpt || *bpmOpt <= 0.0) return;
        const double ppq = *ppqOpt;
        const double perSample = *bpmOpt / 60.0 / sr_;     // semínimas por amostra
        const auto songPosition = [&](const double q) {
            const long spp = std::max(0L, static_cast<long>(std::floor(q * 4.0 + 1e-9)));
            main_->pushHostMidi(0xF2, static_cast<std::uint8_t>(spp & 0x7F),
                                static_cast<std::uint8_t>((spp >> 7) & 0x7F), t0);
            nextTick_ = spp * 6;
        };
        if (playing && !wasPlaying_) {
            if (ppq < 1e-6) { main_->pushHostMidi(0xFA, 0, 0, t0); nextTick_ = 0; }
            else { songPosition(ppq); main_->pushHostMidi(0xFB, 0, 0, t0); }
        } else if (!playing && wasPlaying_) {
            main_->pushHostMidi(0xFC, 0, 0, t0);
        }
        wasPlaying_ = playing;
        if (!playing) return;
        if (std::abs(ppq * 24.0 - static_cast<double>(nextTick_)) > 3.0)
            songPosition(ppq);                                   // loop / salto
        const double end = ppq + n * perSample;
        while (static_cast<double>(nextTick_) / 24.0 < end) {
            double at = (static_cast<double>(nextTick_) / 24.0 - ppq) / perSample;
            if (at < 0.0) at = 0.0;
            main_->pushHostMidi(0xF8, 0, 0, t0 + at / sr_);
            ++nextTick_;
        }
    }

    std::unique_ptr<MainComponent> main_;
    juce::AudioParameterBool* variaOn_ = nullptr;
    juce::AudioParameterFloat* varia_ = nullptr;
    juce::AudioParameterFloat* master_ = nullptr;
    // começam iguais aos padrões: a primeira passada não mexe no patch
    bool lastOn_ = true;
    float lastAmount_ = 1.0f, lastDb_ = -24.0f;
    double sr_ = 48000.0;
    std::int64_t samplePos_ = 0;
    bool wasPlaying_ = false;
    long nextTick_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasgoModularProcessor)
};

class RasgoModularEditor final : public juce::AudioProcessorEditor {
public:
    explicit RasgoModularEditor(RasgoModularProcessor& p)
        : AudioProcessorEditor(p), proc_(p) {
        addAndMakeVisible(proc_.mainComponent());
        setResizable(true, true);
        setResizeLimits(960, 600, 8192, 8192);
        // o tamanho que o músico deixou; na primeira vez, 80% da área útil
        // da tela principal (o app abre em ~88%, mas aqui a DAW está junto)
        int w = proc_.editorW, h = proc_.editorH;
        if (w <= 0 || h <= 0) {
            w = 1280; h = 800;
            if (const auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) {
                const auto area = d->userBounds;   // userArea é obsoleto
                w = juce::jlimit(960, 8192, juce::roundToInt(area.getWidth() * 0.8f));
                h = juce::jlimit(600, 8192, juce::roundToInt(area.getHeight() * 0.8f));
            }
        }
        setSize(w, h);
    }
    ~RasgoModularEditor() override {
        // o componente é do processador: só sai da janela
        removeChildComponent(&proc_.mainComponent());
    }
    void resized() override {
        proc_.mainComponent().setBounds(getLocalBounds());
        proc_.editorW = getWidth();
        proc_.editorH = getHeight();
    }
private:
    RasgoModularProcessor& proc_;
};

juce::AudioProcessorEditor* RasgoModularProcessor::createEditor() {
    return new RasgoModularEditor(*this);
}

}  // namespace

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new RasgoModularProcessor();
}
