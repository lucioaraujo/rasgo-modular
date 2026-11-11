#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// TAP (#78) — o andamento batido por quem toca (2026-11-11; dossiê
// `dossies/78_tap.md`; estudo da família §4.9, aprovado; conceito do
// `tap_tempo.py` do Maravi, mesmo autor: "liberar o áudio para detectar uma
// sequência de batidas feitas pelo músico e o maravi detecta o andamento").
//
// Cada gatilho em TAP é uma batida. Batidas próximas (o intervalo não passa
// de 2,5 × a mediana nem de 3 s) formam uma CONTAGEM; o andamento é a
// MEDIANA dos intervalos da contagem (até 8), a partir da TERCEIRA batida
// (dois intervalos: o mínimo para medir a regularidade). Se a dispersão (o maior desvio
// relativo à mediana) passa de DISP, a contagem não vale — "melhor falhar do
// que devolver um andamento inventado" (Maravi). Fora de 20–400 BPM também
// não vale.
//
// O "um": a primeira batida da contagem é o tempo forte (quem conta "1, 2,
// 3, 4" começa no um). Depois da contagem o relógio segue sozinho, na fase
// das batidas: CLK a cada tempo, UM a cada COMP tempos. Uma contagem nova
// troca o andamento e a fase. BPM sai como CV (bpm/300, o formato do knob do
// CLOCK em 0–1); TRAV fica alto quando há andamento. Sem batida nenhuma, o
// TAP fica quieto (é um módulo de escuta). Determinístico.
namespace rasgo::modular {

class Tap final : public Signal {
public:
    static constexpr int kMaxTaps = 8;

    Tap() : Signal(
        {{"tap", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"}},
        {{"clock", PortKind::Control, "trig"},
         {"one", PortKind::Control, "trig"},
         {"bpm", PortKind::Control, ""},
         {"locked", PortKind::Control, "gate"}},
        {{"beats", 1.0f, 16.0f, 4.0f, ""},
         {"spread", 0.05f, 0.6f, 0.35f, ""}}) {}

    std::string type() const override { return "TAP"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "TAP", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "tap", "", 2.5f, 6.0f, 35.6f);
        p.add(Widget::Kind::Knob, "COMP", "beats", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DISP", "spread", 26.0f, 30.0f);
        p.add(Widget::Kind::Jack, "TAP", "in:tap", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 26.0f, 92.0f);
        p.add(Widget::Kind::Jack, "CLK", "out:clock", 6.0f, 106.0f);
        p.add(Widget::Kind::Jack, "UM", "out:one", 18.0f, 106.0f);
        p.add(Widget::Kind::Jack, "BPM", "out:bpm", 30.0f, 106.0f);
        p.add(Widget::Kind::Jack, "TRAV", "out:locked", 18.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearAll();
    }

    double bpm() const noexcept { return period_ > 0.0 ? 60.0 * sr_ / period_ : 0.0; }
    bool locked() const noexcept { return period_ > 0.0; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* tap = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        const int beats = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("beats"), 4.0f), 1.0f, 16.0f)));
        const double maxSpread = std::clamp(safe(parameterValue("spread"), 0.35f), 0.05f, 0.6f);
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            now_ += 1.0;
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearAll();
                prevReset_ = r;
            }
            if (tap) {
                const float x = safe(tap->at(0, f), 0.0f);
                if (prevTap_ < 0.5f && x >= 0.5f) onTap(beats, maxSpread, pulse);
                prevTap_ = x;
            }
            // o relógio, na fase das batidas
            if (period_ > 0.0 && now_ >= nextBeat_) {
                nextBeat_ += period_;
                beatIndex_ = (beatIndex_ + 1) % beats;
                clkLeft_ = pulse;
                if (beatIndex_ == 0) oneLeft_ = pulse;
            }
            const float clk = clkLeft_ > 0 ? 1.0f : 0.0f;
            const float one = oneLeft_ > 0 ? 1.0f : 0.0f;
            if (clkLeft_ > 0) --clkLeft_;
            if (oneLeft_ > 0) --oneLeft_;
            const float bpmV = static_cast<float>(bpm() / 300.0);
            const float lk = locked() ? 1.0f : 0.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = clk;
                outputs[1].at(c, f) = one;
                outputs[2].at(c, f) = bpmV;
                outputs[3].at(c, f) = lk;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    void clearAll() noexcept {
        count_ = 0;
        period_ = 0.0;
        now_ = 0.0;
        nextBeat_ = 0.0;
        beatIndex_ = 0;
        clkLeft_ = oneLeft_ = 0;
        prevTap_ = prevReset_ = 0.0f;
    }

    double median(std::array<double, kMaxTaps> v, const int n) const noexcept {
        std::sort(v.begin(), v.begin() + n);
        return n % 2 ? v[static_cast<std::size_t>(n / 2)]
                     : 0.5 * (v[static_cast<std::size_t>(n / 2 - 1)] + v[static_cast<std::size_t>(n / 2)]);
    }

    void onTap(const int beats, const double maxSpread, const int pulse) noexcept {
        // a batida fecha a contagem anterior se veio longe demais
        if (count_ > 0) {
            const double gap = now_ - taps_[static_cast<std::size_t>(count_ - 1)];
            double ref = 3.0 * sr_;
            if (count_ >= 2) {
                std::array<double, kMaxTaps> iv{};
                for (int i = 1; i < count_; ++i)
                    iv[static_cast<std::size_t>(i - 1)] = taps_[static_cast<std::size_t>(i)] - taps_[static_cast<std::size_t>(i - 1)];
                ref = std::min(ref, 2.5 * median(iv, count_ - 1));
            }
            if (gap > ref) count_ = 0;
        }
        if (count_ == kMaxTaps) {   // guarda as últimas 8
            for (int i = 1; i < kMaxTaps; ++i) taps_[static_cast<std::size_t>(i - 1)] = taps_[static_cast<std::size_t>(i)];
            --count_;
            ++dropped_;
        }
        if (count_ == 0) dropped_ = 0;
        taps_[static_cast<std::size_t>(count_++)] = now_;
        // três batidas (dois intervalos): com uma só não há como medir a
        // regularidade, e um par solto travaria num andamento qualquer
        if (count_ < 3) return;

        std::array<double, kMaxTaps> iv{};
        for (int i = 1; i < count_; ++i)
            iv[static_cast<std::size_t>(i - 1)] = taps_[static_cast<std::size_t>(i)] - taps_[static_cast<std::size_t>(i - 1)];
        const double med = median(iv, count_ - 1);
        if (med <= 0.0) return;
        double spread = 0.0;
        for (int i = 0; i < count_ - 1; ++i)
            spread = std::max(spread, std::fabs(iv[static_cast<std::size_t>(i)] - med) / med);
        const double b = 60.0 * sr_ / med;
        if (spread > maxSpread || b < 20.0 || b > 400.0) return;   // não vale: não inventa

        // vale: andamento novo, e a fase na batida que acabou de cair.
        // A 1ª batida da contagem é o "um".
        period_ = med;
        const int tapsSoFar = count_ + dropped_;
        beatIndex_ = (tapsSoFar - 1) % beats;
        nextBeat_ = now_ + period_;
        clkLeft_ = pulse;
        if (beatIndex_ == 0) oneLeft_ = pulse;
    }

    float sr_ = 48000.0f;
    std::array<double, kMaxTaps> taps_{};
    int count_ = 0, dropped_ = 0;
    double period_ = 0.0, now_ = 0.0, nextBeat_ = 0.0;
    int beatIndex_ = 0, clkLeft_ = 0, oneLeft_ = 0;
    float prevTap_ = 0.0f, prevReset_ = 0.0f;
};

}  // namespace rasgo::modular
