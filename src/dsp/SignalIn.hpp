#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SIGNAL-IN — adaptador de entrada: áudio + MIDI + CV (Módulo 49)
// ============================================================================
//
// Evolução do `AUDIO-IN` (#35) — decisão do autor 2026-09-06: em vez de
// módulos `MIDI-IN` e `CV-IN` separados, UM adaptador. Contraparte de
// ENTRADA do `NOTE-OUT`. Identidade RASGO: nó adaptador OPCIONAL, nunca
// dependência — o painel soa sozinho sem ele; `rasgo_modular_core` não
// sabe o que é ALSA nem ALSA-seq.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/49_signal_in.md`.
//
// Dois anéis SPSC lock-free, alimentados de threads externas
// (`apps/panel/`):
//   - ÁUDIO: `pushSamples(interleavedLR, frames)` — igual ao `AUDIO-IN`.
//   - MIDI:  `pushMidi(status, d1, d2)` — eventos empacotados; `process()`
//            drena e resolve uma voz MONOFÔNICA (last-note, com pilha).
//
// Saídas: `out` (áudio L — nome preservado pra compat de patch salvo),
// `r` (áudio R), `pitch` (1 V/oct, nota 60 = 0 V, + pitch-bend), `gate`,
// `vel`, `cc` (o CC nº `cc_num`). Sem nada alimentando → tudo em silêncio,
// determinístico (testável sem hardware).
//
// MIDI CLOCK (20/11/2026, pedido do autor: "funcione com outros
// softwares"): `clk` (pulsos na divisão DIV: semínima, colcheia,
// semicolcheia ou os 24 por semínima crus), `run` (gate: a DAW tocando) e
// `rst` (trigger no START, para o reset do CLOCK). CLK → EXT do CLOCK e
// RST → RST do CLOCK: o patch segue o andamento e o "um" da DAW ou da
// máquina. Os eventos chegam por bloco (o período da placa, 5–20 ms); os
// pulsos NÃO herdam esse tremor: um DLL (o filtro de tempo do JACK, ver
// `systemMessage`) dá o instante limpo de cada clock, e o pulso sai nele
// mais um atraso pequeno e constante. Song Position Pointer (0xF2)
// reposiciona a contagem. Só START/CONTINUE/STOP mandam no RUN; uma fonte
// que só manda clocks (sem transporte) também toca.
//
// COMPAT: `makeModule("AUDIO-IN")` também constrói este módulo (alias);
// `type()` devolve "SIGNAL-IN", então re-salvar um `.rmp` antigo migra.

namespace rasgo::modular {

class SignalIn final : public Signal {
public:
    SignalIn()
        : Signal({},
                 {{"out", PortKind::Audio, ""},      // áudio L (compat: era "out")
                  {"r", PortKind::Audio, ""},        // áudio R
                  {"pitch", PortKind::Control, "v/oct"},
                  {"gate", PortKind::Control, "gate"},
                  {"vel", PortKind::Control, ""},
                  {"cc", PortKind::Control, ""},
                  // MIDI clock (20/11/2026) — no FIM: patches antigos iguais
                  {"clk", PortKind::Control, "gate"},
                  {"run", PortKind::Control, "gate"},
                  {"rst", PortKind::Control, "trig"}},
                 {{"gain", 0.0f, 2.0f, 1.0f, ""},
                  {"bend", 0.0f, 24.0f, 2.0f, "st"},
                  {"cc_num", 0.0f, 127.0f, 1.0f, ""},
                  // ON (v0.1.3): a entrada de fora só entra com ele ligado.
                  // Nasce DESLIGADO — o app não abre microfone nem MIDI sem
                  // um gesto de quem toca (pedido do autor, 3 out. 2026).
                  // No FIM da lista e fora de todo sorteio (seed, VARIA,
                  // MUDA): o mesmo seed segue dando o mesmo patch.
                  {"listen", 0.0f, 1.0f, 0.0f, ""},
                  // DIV do MIDI clock (20/11/2026): 0 = semínima, 1 =
                  // colcheia (casa com o MULT 2 padrão do CLOCK), 2 =
                  // semicolcheia, 3 = 24 por semínima. Também fora de todo
                  // sorteio: escolha de quem liga a DAW.
                  {"clk_div", 0.0f, 3.0f, 1.0f, ""}}) {}

    std::string type() const override { return "SIGNAL-IN"; }

    Panel panel() const override {
        Panel p;
        // Revisão de painel (autor, 26/11/2026): colunas pelo centro, knobs e jacks
        // nas mesmas colunas, rótulos com folga; mais estreito onde sobrava (G1–G4).
        p.hp = 8;
        p.add(Widget::Kind::Label, "SIGNAL-IN", "", 2.0f, 2.0f);
        p.add(Widget::Kind::Display, "in", "", 2.5f, 6.0f, 35.64f);
        p.add(Widget::Kind::Knob, "GAIN", "gain", 5.32f, 30.0f);
        p.add(Widget::Kind::Knob, "BEND", "bend", 16.32f, 30.0f);
        p.add(Widget::Kind::Knob, "DIV", "clk_div", 27.32f, 30.0f);
        p.add(Widget::Kind::Toggle, "ON", "listen", 7.32f, 54.0f);
        p.add(Widget::Kind::Knob, "CC#", "cc_num", 16.32f, 50.0f);
        p.add(Widget::Kind::Jack, "L", "out:out", 9.32f, 78.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 20.32f, 78.0f);
        p.add(Widget::Kind::Jack, "CLK", "out:clk", 31.32f, 78.0f);
        p.add(Widget::Kind::Jack, "1V/O", "out:pitch", 9.32f, 94.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 20.32f, 94.0f);
        p.add(Widget::Kind::Jack, "RUN", "out:run", 31.32f, 94.0f);
        p.add(Widget::Kind::Jack, "VEL", "out:vel", 9.32f, 110.0f);
        p.add(Widget::Kind::Jack, "CC", "out:cc", 20.32f, 110.0f);
        p.add(Widget::Kind::Jack, "RST", "out:rst", 31.32f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        std::size_t cap = 1;
        while (cap < static_cast<std::size_t>(sampleRate) + 1) cap <<= 1;
        ring_.assign(cap * 2, 0.0f);
        mask_ = cap - 1;
        writeIdx_.store(0, std::memory_order_relaxed);
        readIdx_ = 0;

        midiWrite_.store(0, std::memory_order_relaxed);
        midiRead_ = 0;
        for (auto& e : midi_) e = 0;

        noteCount_ = 0;
        curNote_ = 60;
        gateHigh_ = false;
        curVel_ = 0.0f;
        ccVal_ = 0.0f;
        bendNorm_ = 0.0f;
        gateRamp_ = 0.0f;
        rampCoef_ = 1.0f - std::exp(-1.0f / (0.001f * std::max(1.0f, sampleRate)));
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        resetClock();
        transportSeen_ = false;
    }

    // Andamento medido do MIDI clock (0 = sem clock). Para o visor e os testes.
    float midiBpm() const noexcept {
        return clkCount_ >= 2 ? 60.0f * sr_ / (clkPeriod_ * 24.0f) : 0.0f;
    }
    bool midiRunning() const noexcept { return running(); }

    // ---- PRODUTOR ÁUDIO (thread de captura ALSA) ----
    void pushSamples(const float* interleavedLR, const std::size_t frames) noexcept {
        if (ring_.empty()) return;
        const std::size_t cap = mask_ + 1;
        const std::size_t w = writeIdx_.load(std::memory_order_relaxed);
        const std::size_t n = frames > cap ? cap : frames;
        const std::size_t start = frames > cap ? frames - cap : 0;
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t slot = (w + i) & mask_;
            ring_[slot * 2] = interleavedLR[(start + i) * 2];
            ring_[slot * 2 + 1] = interleavedLR[(start + i) * 2 + 1];
        }
        writeIdx_.store(w + n, std::memory_order_release);
    }

    // ---- PRODUTOR MIDI (thread ALSA-seq) ----
    // status/d1/d2 = bytes MIDI crus (canal já mascarado ou não — só o
    // nibble alto importa aqui). Nunca bloqueia; anel de 1024 eventos.
    // `offsetSamples` (20/11/2026): quando o evento aconteceu, em amostras
    // relativas ao começo do PRÓXIMO `process()` (negativo = no passado).
    // O app calcula do carimbo de tempo do MIDI; sem carimbo, 0 (o começo
    // do bloco). Só o MIDI clock usa.
    void pushMidi(const std::uint8_t status, const std::uint8_t d1,
                  const std::uint8_t d2, const float offsetSamples = 0.0f) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_relaxed);
        midiOff_[w & kMidiMask] = std::isfinite(offsetSamples) ? offsetSamples : 0.0f;
        midi_[w & kMidiMask] = (static_cast<std::uint32_t>(status) << 16)
                             | (static_cast<std::uint32_t>(d1) << 8)
                             | static_cast<std::uint32_t>(d2);
        midiWrite_.store(w + 1, std::memory_order_release);
    }

    // ---- CONSUMIDOR (thread de áudio do grafo) ----
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& oL = outputs[0];
        AudioBlock& oR = outputs[1];
        AudioBlock& oPitch = outputs[2];
        AudioBlock& oGate = outputs[3];
        AudioBlock& oVel = outputs[4];
        AudioBlock& oCc = outputs[5];
        AudioBlock* oClk = outputs.size() > 6 ? &outputs[6] : nullptr;
        AudioBlock* oRun = outputs.size() > 7 ? &outputs[7] : nullptr;
        AudioBlock* oRst = outputs.size() > 8 ? &outputs[8] : nullptr;
        const std::size_t frames = oL.frames();
        const std::size_t channels = oL.channels();

        if (parameterValue("listen") < 0.5f) {
            // desligado: nada de fora entra. O MIDI pendente é descartado
            // para não disparar notas velhas ao ligar.
            drainMidi(static_cast<int>(parameterValue("cc_num") + 0.5f));
            curNote_ = 60; gateHigh_ = false; gateRamp_ = 0.0f;
            resetClock(); transportSeen_ = false;
            for (auto& o : outputs) o.clear();
            blockStart_ += static_cast<double>(frames);
            return;
        }
        const float gain = parameterValue("gain");
        const float bendSt = parameterValue("bend");
        const int ccNum = static_cast<int>(parameterValue("cc_num") + 0.5f);

        // drena todos os eventos MIDI pendentes (uma vez por bloco — a
        // granularidade de bloco é suficiente pra controle)
        drainMidi(ccNum);
        static constexpr int kDiv[4] = {24, 12, 6, 1};
        const int div = kDiv[std::min(3, std::max(0,
            static_cast<int>(parameterValue("clk_div") + 0.5f)))];

        const float pitchV = static_cast<float>(curNote_ - 60) / 12.0f
                           + bendNorm_ * bendSt / 12.0f;
        const float gateTarget = gateHigh_ ? 1.0f : 0.0f;

        // ---- áudio ----
        std::size_t n = 0;
        std::size_t base = 0;
        if (!ring_.empty()) {
            const std::size_t cap = mask_ + 1;
            const std::size_t w = writeIdx_.load(std::memory_order_acquire);
            if (w - readIdx_ > cap) readIdx_ = w - cap;
            const std::size_t avail = w - readIdx_;
            n = avail < frames ? avail : frames;
            base = readIdx_;
            readIdx_ += n;
        }

        for (std::size_t f = 0; f < frames; ++f) {
            float l = 0.0f, r = 0.0f;
            if (f < n && !ring_.empty()) {
                const std::size_t slot = (base + f) & mask_;
                l = ring_[slot * 2] * gain;
                r = ring_[slot * 2 + 1] * gain;
            }
            gateRamp_ += (gateTarget - gateRamp_) * rampCoef_;
            for (std::size_t c = 0; c < channels; ++c) {
                oL.at(c, f) = l;
                oR.at(c, f) = r;
                oPitch.at(c, f) = pitchV;
                oGate.at(c, f) = gateRamp_;
                oVel.at(c, f) = curVel_;
                oCc.at(c, f) = ccVal_;
            }
            const float clk = clockSample(div, blockStart_ + static_cast<double>(f));
            const float run = running() ? 1.0f : 0.0f;
            const float rst = rstLeft_ > 0 ? 1.0f : 0.0f;
            if (rstLeft_ > 0) --rstLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                if (oClk) oClk->at(c, f) = clk;
                if (oRun) oRun->at(c, f) = run;
                if (oRst) oRst->at(c, f) = rst;
            }
        }
        blockStart_ += static_cast<double>(frames);
    }

private:
    void drainMidi(const int ccNum) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_acquire);
        while (midiRead_ != w) {
            const std::uint32_t ev = midi_[midiRead_ & kMidiMask];
            const float off = midiOff_[midiRead_ & kMidiMask];
            ++midiRead_;
            const std::uint8_t raw = static_cast<std::uint8_t>((ev >> 16) & 0xFF);
            if (raw >= 0xF0) { systemMessage(raw, ev, off); continue; }
            const std::uint8_t st = static_cast<std::uint8_t>((ev >> 16) & 0xF0);
            const std::uint8_t d1 = static_cast<std::uint8_t>((ev >> 8) & 0x7F);
            const std::uint8_t d2 = static_cast<std::uint8_t>(ev & 0x7F);
            switch (st) {
            case 0x90:
                if (d2 > 0) { noteOn(d1, d2); break; }
                [[fallthrough]];   // note-on vel 0 = note-off
            case 0x80:
                noteOff(d1);
                break;
            case 0xB0:
                if (d1 == ccNum) ccVal_ = static_cast<float>(d2) / 127.0f;
                break;
            case 0xE0:
                bendNorm_ = (static_cast<float>((d2 << 7) | d1) - 8192.0f)
                          / 8192.0f;
                break;
            default:
                break;
            }
        }
    }

    void noteOn(const std::uint8_t note, const std::uint8_t vel) noexcept {
        if (noteCount_ < static_cast<int>(stack_.size()))
            stack_[static_cast<std::size_t>(noteCount_++)] = note;
        curNote_ = note;
        curVel_ = static_cast<float>(vel) / 127.0f;
        gateHigh_ = true;
    }

    void noteOff(const std::uint8_t note) noexcept {
        int w = 0;
        for (int i = 0; i < noteCount_; ++i)
            if (stack_[static_cast<std::size_t>(i)] != note)
                stack_[static_cast<std::size_t>(w++)] = stack_[static_cast<std::size_t>(i)];
        noteCount_ = w;
        if (noteCount_ > 0) {
            curNote_ = stack_[static_cast<std::size_t>(noteCount_ - 1)];
            gateHigh_ = true;
        } else {
            gateHigh_ = false;
        }
    }

    // ---- MIDI clock -----------------------------------------------------
    // O tempo de chegada de cada clock é o começo do bloco em que ele foi
    // entregue: um tremor de até um período da placa (5–20 ms). Um DLL de
    // 2ª ordem (Fons Adriaensen, "Using a DLL to filter time", 2005 — o
    // filtro de tempo do JACK; conceito público) estima o instante "limpo"
    // de cada clock e o período; o pulso sai nesse instante + um atraso D
    // que acompanha o maior desvio visto (≈ o período da placa). O atraso
    // é pequeno e CONSTANTE; o tremor some.
    bool running() const noexcept {
        return transportSeen_ ? run_ : (clkCount_ > 0 && !clockStale());
    }
    bool clockStale() const noexcept {   // sem clock há meio segundo
        return blockStart_ - lastClkTime_ > 0.5 * static_cast<double>(sr_);
    }
    void resetClock() noexcept {
        index_ = 0; clkCount_ = 0; qHead_ = qTail_ = 0;
        clkPeriod_ = sr_ * 0.5f / 24.0f;   // 120 BPM até medir
        dllInit_ = false; maxDev_ = 0.0;
        gateLeft_ = 0; rstLeft_ = 0; rstWait_ = false; run_ = false;
        lastClkTime_ = -1e12;
    }
    void schedule(const double when, const long idx) noexcept {
        if (qTail_ - qHead_ >= kQ) ++qHead_;              // cheio: perde o mais velho
        queue_[qTail_ % kQ] = {when, idx};
        ++qTail_;
    }
    void systemMessage(const std::uint8_t st, const std::uint32_t ev,
                       const float offset) noexcept {
        switch (st) {
        case 0xF8: {                                       // clock
            // não antes do clock anterior (carimbos fora de ordem)
            const double t = std::max(blockStart_ + static_cast<double>(offset),
                                      dllInit_ ? lastClkTime_ : -1e18);
            const double P = static_cast<double>(clkPeriod_);
            if (!dllInit_ || t - lastClkTime_ > 0.5 * static_cast<double>(sr_)) {
                dllInit_ = true; t0_ = t; t1_ = t + P; maxDev_ = 0.0; lockN_ = 0; errAvg_ = 0.0;
            } else {
                const double err = t - t1_;
                if (std::abs(err) > 4.0 * P + static_cast<double>(sr_) * 0.05) {
                    // salto de andamento ou de relógio: recomeça a estimativa
                    const double dt = t - lastClkTime_;
                    double nP = P;
                    if (dt > 0.0) nP = dt;
                    t0_ = t; t1_ = t + nP; clkPeriod_ = static_cast<float>(nP); lockN_ = 0;
                } else {
                    // erro bem acima do ruído de sempre = o andamento mudou:
                    // reabre a banda para seguir depressa
                    if (std::abs(err) > std::max(0.2 * P, 4.0 * errAvg_)) lockN_ = 0;
                    errAvg_ += (std::abs(err) - errAvg_) * 0.05;
                    // banda larga para travar depressa (2 Hz), estreitando
                    // até 0,25 Hz: o tremor da placa fica de fora
                    const double bw = 0.25 + 1.75 * std::exp(-static_cast<double>(lockN_) / 48.0);
                    ++lockN_;
                    const double w = 2.0 * 3.14159265358979 * bw * P
                                   / static_cast<double>(sr_);
                    const double b = 1.41421356 * w, c = w * w;
                    t0_ = t1_;
                    t1_ += b * err + P;
                    clkPeriod_ = static_cast<float>(P + c * err);
                }
            }
            clkPeriod_ = std::min(std::max(clkPeriod_, sr_ * 60.0f / (400.0f * 24.0f)),
                                  sr_ * 60.0f / (10.0f * 24.0f));
            // o atraso de ENTREGA: quanto depois do instante limpo o clock
            // chegou ao áudio (≈ o período da placa)
            const double dev = blockStart_ - t0_;
            maxDev_ = std::max(dev, maxDev_ * 0.99995);   // ~0,2 %/s: o atraso fica estável
            ++clkCount_;
            lastClkTime_ = t;
            if (!transportSeen_ || run_) {
                const double d = std::max(0.0, maxDev_) + 16.0;
                schedule(t0_ + d, index_);
                ++index_;
            }
            break;
        }
        case 0xFA:                                         // start
            transportSeen_ = true; run_ = true;
            index_ = 0; qHead_ = qTail_ = 0; rstWait_ = true;
            break;
        case 0xFB: transportSeen_ = true; run_ = true; break;           // continue
        case 0xFC: transportSeen_ = true; run_ = false;                 // stop
                   qHead_ = qTail_ = 0; break;
        case 0xF2:                                         // song position
            index_ = static_cast<long>(((ev & 0x7F) << 7) | ((ev >> 8) & 0x7F)) * 6;
            qHead_ = qTail_ = 0;
            break;
        default: break;
        }
    }
    // Uma amostra do CLK. O primeiro clock depois do START também arma o
    // RST, uma amostra depois do pulso: o CLOCK zera o passo DEPOIS da
    // borda, e o "um" cai no lugar (o reset e a borda na mesma amostra
    // dariam o passo 1).
    float clockSample(const int div, const double now) noexcept {
        while (qHead_ != qTail_ && queue_[qHead_ % kQ].when <= now) {
            const long k = queue_[qHead_ % kQ].index;
            ++qHead_;
            if (k % div == 0) {
                gateLeft_ = std::max(1, static_cast<int>(clkPeriod_ * static_cast<float>(div) * 0.5f));
                gateLeft_ = std::min(gateLeft_, static_cast<int>(sr_ * 0.25f));
            }
            if (k == 0 && rstWait_) { rstWait_ = false; rstDelay_ = 2; }
        }
        if (rstDelay_ > 0 && --rstDelay_ == 0) rstLeft_ = static_cast<int>(sr_ * 0.005f);
        if (gateLeft_ > 0) { --gateLeft_; return 1.0f; }
        return 0.0f;
    }

    static constexpr std::size_t kMidiCap = 1024;
    static constexpr std::size_t kMidiMask = kMidiCap - 1;

    // áudio
    std::vector<float> ring_;
    std::size_t mask_ = 0;
    std::atomic<std::size_t> writeIdx_{0};
    std::size_t readIdx_ = 0;

    // MIDI
    std::array<std::uint32_t, kMidiCap> midi_{};
    std::array<float, kMidiCap> midiOff_{};
    std::atomic<std::size_t> midiWrite_{0};
    std::size_t midiRead_ = 0;

    // voz monofônica
    std::array<std::uint8_t, 16> stack_{};
    int noteCount_ = 0;
    int curNote_ = 60;
    bool gateHigh_ = false;
    float curVel_ = 0.0f;
    float ccVal_ = 0.0f;
    float bendNorm_ = 0.0f;
    float gateRamp_ = 0.0f;
    float rampCoef_ = 1.0f;

    // MIDI clock
    float sr_ = 48000.0f;
    double blockStart_ = 0.0;           // amostras desde o prepare()
    long clkCount_ = 0;                 // clocks vistos (para o período)
    float clkPeriod_ = 1000.0f;         // amostras por clock (1/24 de semínima)
    double lastClkTime_ = -1e12;
    long index_ = 0;                    // clocks desde o START (ou SPP)
    struct Due { double when; long index; };
    static constexpr std::size_t kQ = 64;
    std::array<Due, kQ> queue_{};
    std::size_t qHead_ = 0, qTail_ = 0;
    bool dllInit_ = false;
    double t0_ = 0.0, t1_ = 0.0, maxDev_ = 0.0;
    long lockN_ = 0;
    double errAvg_ = 0.0;   // o ruído típico do erro do DLL (amostras)
    int gateLeft_ = 0, rstLeft_ = 0, rstDelay_ = 0;
    bool rstWait_ = false, run_ = false, transportSeen_ = false;
};

}  // namespace rasgo::modular
