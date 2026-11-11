#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// COHERENCE (#72) — a ordem que se rompe (2026-11-11, decisão do autor:
// "siga com a COERÊNCIA"; dossiê `dossies/72_coherence.md`).
//
// Escuta uma linha de alturas e mede quanto ela está PREVISÍVEL: a entropia
// de Shannon das classes de altura das últimas notas (C. Shannon, 1948),
// virada em ORDEM = 1 − H/log₂(k). Quando a linha fica ordenada demais
// (ORDEM ≥ LIMIAR) por PACIÊNCIA notas seguidas, a pressão acumulada rompe:
// um pulso em COLAPSO. Inverte a suposição de que ruptura vem do caos — a
// ordem extrema também rompe. Conceito do `CoherenceCollapse` do Rasgo
// Synth Studio e da expectativa de D. Huron (*Sweet Anticipation*, 2006);
// a cadeia tensão → limite → ruptura → cicatriz, com histerese e destino
// declarado da energia, vem do Batruc. Código próprio.
//
// Diferente do Studio: aqui não reorganiza nada sozinho — entrega o gate e
// as CVs, e o músico (ou a seed) escolhe o que a ruptura move.
//
// Destino da energia: na ruptura a pressão vira o pulso e volta a 0; a
// metade mais antiga da janela é esquecida (a cicatriz) e, por CICATRIZ
// notas, a pressão não sobe (refratário). Determinístico, sem alocação.
namespace rasgo::modular {

class Coherence final : public Signal {
public:
    static constexpr int kMaxWindow = 32;

    Coherence() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"gate", PortKind::Control, "gate"},
         {"reset", PortKind::Control, "trig"}},
        {{"order", PortKind::Control, ""},
         {"pressure", PortKind::Control, ""},
         {"collapse", PortKind::Control, "trig"},
         {"surprise", PortKind::Control, ""}},
        {{"window", 4.0f, 32.0f, 12.0f, ""},
         {"threshold", 0.0f, 1.0f, 0.7f, ""},
         {"patience", 1.0f, 16.0f, 6.0f, ""},
         {"scar", 0.0f, 16.0f, 4.0f, ""}}) {}

    std::string type() const override { return "COHERENCE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "COHERENCE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "coherence", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "JAN", "window", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LIM", "threshold", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "PAC", "patience", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CIC", "scar", 20.5f, 52.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "ORD", "out:order", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PRES", "out:pressure", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "COL", "out:collapse", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SURP", "out:surprise", 43.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearAll();
    }

    // Para os testes e o mostrador.
    float order() const noexcept { return order_; }
    float pressure() const noexcept { return pressure_; }
    int notes() const noexcept { return count_; }
    long collapses() const noexcept { return collapses_; }
    // quantas notas da janela caem em cada classe de altura
    std::array<int, 12> histogram() const noexcept { return hist_; }

    static int pitchClass(float v) noexcept {
        const int semis = static_cast<int>(std::lround(v * 12.0f));
        return ((semis % 12) + 12) % 12;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* pitch = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* gate = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* reset = inputs.size() > 2 ? inputs[2] : nullptr;

        const int window = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("window"), 12.0f), 4.0f, 32.0f)));
        const float threshold = std::clamp(safe(parameterValue("threshold"), 0.7f), 0.0f, 1.0f);
        const float patience = std::clamp(std::round(safe(parameterValue("patience"), 6.0f)), 1.0f, 16.0f);
        const int scar = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("scar"), 4.0f), 0.0f, 16.0f)));
        const int pulseLen = std::max(1, static_cast<int>(0.010f * sr_));   // 10 ms

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearAll();
                prevReset_ = r;
            }
            // uma NOTA: borda de subida do gate; sem gate, mudança de classe
            bool noteNow = false;
            int pc = lastPc_;
            if (pitch) pc = pitchClass(safe(pitch->at(0, f), 0.0f));
            if (gate) {
                const float g = safe(gate->at(0, f), 0.0f);
                if (prevGate_ < 0.5f && g >= 0.5f) noteNow = pitch != nullptr;
                prevGate_ = g;
            } else if (pitch && pc != lastPc_) {
                noteNow = true;
            }
            lastPc_ = pc;
            if (noteNow) onNote(pc, window, threshold, patience, scar, pulseLen);

            const float col = pulseLeft_ > 0 ? 1.0f : 0.0f;
            if (pulseLeft_ > 0) --pulseLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = order_;
                outputs[1].at(c, f) = pressure_;
                outputs[2].at(c, f) = col;
                outputs[3].at(c, f) = surprise_;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    void clearAll() noexcept {
        ring_.fill(0);
        hist_.fill(0);
        head_ = 0;
        count_ = 0;
        order_ = pressure_ = surprise_ = 0.0f;
        refractory_ = 0;
        pulseLeft_ = 0;
        prevGate_ = prevReset_ = 0.0f;
        lastPc_ = -1;
        collapses_ = 0;
    }

    // tira a nota mais antiga da janela
    void dropOldest() noexcept {
        if (count_ <= 0) return;
        const int oldest = (head_ - count_ + kMaxWindow) % kMaxWindow;
        --hist_[static_cast<std::size_t>(ring_[static_cast<std::size_t>(oldest)])];
        --count_;
    }

    void onNote(const int pc, const int window, const float threshold,
                const float patience, const int scar, const int pulseLen) noexcept {
        // SURPRESA: quão rara era esta classe na janela ANTES dela entrar
        // (−log₂ p, com p suavizado para nunca ser 0; normalizada por log₂ 24)
        {
            const float p = (static_cast<float>(hist_[static_cast<std::size_t>(pc)]) + 0.5f)
                          / (static_cast<float>(count_) + 6.0f);
            surprise_ = std::clamp(-std::log2(p) / std::log2(24.0f), 0.0f, 1.0f);
        }
        // entra na janela; a janela encolhe se o knob encolheu
        while (count_ >= window) dropOldest();
        ring_[static_cast<std::size_t>(head_)] = pc;
        head_ = (head_ + 1) % kMaxWindow;
        ++hist_[static_cast<std::size_t>(pc)];
        ++count_;

        // ORDEM = 1 − entropia normalizada (k = classes possíveis na janela)
        if (count_ >= 2) {
            double h = 0.0;
            for (const int n : hist_) {
                if (n <= 0) continue;
                const double q = static_cast<double>(n) / static_cast<double>(count_);
                h -= q * std::log2(q);
            }
            const double hmax = std::log2(static_cast<double>(std::min(count_, 12)));
            order_ = static_cast<float>(std::clamp(1.0 - h / hmax, 0.0, 1.0));
        } else {
            order_ = 0.0f;
        }

        // PRESSÃO com histerese: sobe só com ordem seguida, cai pela metade
        if (refractory_ > 0) {
            --refractory_;
        } else if (order_ >= threshold) {
            pressure_ += 1.0f / patience;
        } else {
            pressure_ *= 0.5f;
        }
        // RUPTURA: a pressão vira o pulso (vai a 0), a janela esquece a
        // metade mais antiga, e vem a cicatriz (refratário)
        if (pressure_ >= 1.0f - 1.0e-4f) {
            pulseLeft_ = pulseLen;
            pressure_ = 0.0f;
            const int forget = count_ / 2;
            for (int i = 0; i < forget; ++i) dropOldest();
            refractory_ = scar;
            ++collapses_;
        }
        pressure_ = std::clamp(pressure_, 0.0f, 1.0f);
    }

    float sr_ = 48000.0f;
    std::array<int, kMaxWindow> ring_{};
    std::array<int, 12> hist_{};
    int head_ = 0, count_ = 0, refractory_ = 0, pulseLeft_ = 0, lastPc_ = -1;
    float order_ = 0.0f, pressure_ = 0.0f, surprise_ = 0.0f;
    float prevGate_ = 0.0f, prevReset_ = 0.0f;
    long collapses_ = 0;
};

}  // namespace rasgo::modular
