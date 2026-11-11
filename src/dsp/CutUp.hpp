#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// CUT-UP (#85) — o recorte do Navalha num buffer ao vivo (2026-11-11; dossiê
// `dossies/85_cutup.md`; estudo da família §3 item 11 e §4.6, aprovado:
// "CUT-UP herda daqui o desenho exato"; "avance em todos eles").
//
// Proveniência: o vocabulário e os limites vêm do `FragmentGesture` do
// Navalha 2 (Glerm Soares / Lúcio Araújo, GPL-3.0-or-later — compatível) —
// os números (4 repetições entre 14 e 160 ms; 8 fatias por saco embaralhado
// entre 18 e 72 ms), não o código. Desvio: lá os gestos recortam fatias de um
// sample; aqui recortam o próprio som que passa (os últimos tempos do
// relógio, num buffer de 4 s).
//
// O relógio (CLK) dá o TEMPO (o intervalo entre pulsos; sem cabo, 120 BPM).
// Gestos, por gate (ou sozinhos, com CHANCE a cada pulso):
//   STUT  — o último tempo, 4 vezes, espaçadas de tempo/4 (14–160 ms);
//   BURST — as fatias dos últimos 8 tempos, embaralhadas como um saco (cada
//           uma sai uma vez antes de repetir), espaçadas de tempo/8 (18–72 ms);
//   MICRO — o começo do último tempo, MICRO vezes (2–8) dentro de um tempo;
//   GAP   — um tempo de silêncio, com bordas de 3 ms (nunca um corte seco).
// Cada fragmento tem bordas de 3 ms. MIX = quanto o gesto substitui o
// presente. Saídas: OUT e ATIVO (gate). Buffer alocado em prepare().
namespace rasgo::modular {

class CutUp final : public Signal {
public:
    CutUp() : Signal(
        {{"in", PortKind::Audio, ""},
         {"clock", PortKind::Control, "trig"},
         {"stutter", PortKind::Control, "trig"},
         {"burst", PortKind::Control, "trig"},
         {"micro", PortKind::Control, "trig"},
         {"gap", PortKind::Control, "trig"}},
        {{"out", PortKind::Audio, ""},
         {"active", PortKind::Control, "gate"}},
        {{"chance", 0.0f, 1.0f, 0.0f, ""},
         {"micro_n", 2.0f, 8.0f, 4.0f, ""},
         {"mix", 0.0f, 1.0f, 1.0f, ""},
         {"seed", 0.0f, 999.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CUTUP"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "CUT-UP", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "cutup", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "CHANCE", "chance", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MICRO", "micro_n", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SEED", "seed", 8.0f, 54.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 82.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 19.0f, 82.0f);
        p.add(Widget::Kind::Jack, "STUT", "in:stutter", 7.0f, 98.0f);
        p.add(Widget::Kind::Jack, "BURST", "in:burst", 19.0f, 98.0f);
        p.add(Widget::Kind::Jack, "MICRO", "in:micro", 31.0f, 98.0f);
        p.add(Widget::Kind::Jack, "GAP", "in:gap", 43.0f, 98.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ATIVO", "out:active", 19.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        const std::size_t n = static_cast<std::size_t>(4.0f * sr_);
        for (auto& b : mem_) b.assign(n, 0.0f);
        now_ = 0;
        interval_ = static_cast<std::uint64_t>(0.5f * sr_);   // 120 BPM
        lastClk_ = 0;
        clkSeen_ = false;
        count_ = idx_ = 0;
        prev_.fill(0.0f);
        seedUsed_ = -1.0f;
        gestures_ = 0;
    }

    long gestures() const noexcept { return gestures_; }
    bool active() const noexcept { return idx_ < count_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2 || mem_[0].empty()) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* clk = inputs.size() > 1 ? inputs[1] : nullptr;
        const float chance = std::clamp(safe(parameterValue("chance"), 0.0f), 0.0f, 1.0f);
        const int microN = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("micro_n"), 4.0f), 2.0f, 8.0f)));
        const float mix = std::clamp(safe(parameterValue("mix"), 1.0f), 0.0f, 1.0f);
        const float seed = std::round(std::clamp(safe(parameterValue("seed"), 0.0f), 0.0f, 999.0f));
        if (seed != seedUsed_) {
            seedUsed_ = seed;
            rng_ = 0xC2B2AE3D27D4EB4FULL ^ (static_cast<std::uint64_t>(seed) * 0x9E3779B97F4A7C15ULL);
            if (rng_ == 0) rng_ = 1;
        }
        const std::size_t size = mem_[0].size();
        const std::uint64_t edge = std::max<std::uint64_t>(1, static_cast<std::uint64_t>(0.003f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const std::size_t inCh = in ? std::min<std::size_t>(in->channels(), 2) : 0;
        for (std::size_t f = 0; f < frames; ++f) {
            float live[2] = {0.0f, 0.0f};
            if (in) {
                live[0] = safe(in->at(0, f), 0.0f);
                live[1] = inCh > 1 ? safe(in->at(1, f), 0.0f) : live[0];
            }
            mem_[0][now_ % size] = live[0];
            mem_[1][now_ % size] = live[1];

            bool tick = false;
            if (clk) {
                const float c = safe(clk->at(0, f), 0.0f);
                if (prev_[0] < 0.5f && c >= 0.5f) {
                    if (clkSeen_ && now_ > lastClk_)
                        interval_ = std::clamp<std::uint64_t>(now_ - lastClk_, static_cast<std::uint64_t>(0.05f * sr_),
                                                              static_cast<std::uint64_t>(1.9f * sr_));
                    lastClk_ = now_;
                    clkSeen_ = true;
                    tick = true;
                }
                prev_[0] = c;
            }
            int gesture = -1;
            for (int g = 0; g < 4; ++g) {
                const AudioBlock* gi = inputs.size() > static_cast<std::size_t>(2 + g)
                    ? inputs[static_cast<std::size_t>(2 + g)] : nullptr;
                if (!gi) continue;
                const float v = safe(gi->at(0, f), 0.0f);
                if (prev_[static_cast<std::size_t>(1 + g)] < 0.5f && v >= 0.5f) gesture = g;
                prev_[static_cast<std::size_t>(1 + g)] = v;
            }
            if (gesture < 0 && tick && chance > 0.0f && !active() && uniform() < chance)
                gesture = static_cast<int>(uniform() * 4.0f) % 4;
            if (gesture >= 0 && !active() && now_ > interval_ * 2) plan(gesture, microN);

            float out[2] = {live[0], live[1]};
            if (active()) {
                const Frag& fr = frag_[static_cast<std::size_t>(idx_)];
                const std::uint64_t pos = now_ - fr.at;   // dentro deste fragmento
                if (now_ >= fr.at) {
                    float env = 1.0f;
                    if (pos < edge) env = static_cast<float>(pos) / static_cast<float>(edge);
                    else if (fr.len - pos <= edge) env = static_cast<float>(fr.len - pos) / static_cast<float>(edge);
                    float src[2] = {0.0f, 0.0f};
                    if (!fr.gap) {
                        const std::uint64_t a = fr.src + pos % fr.srcLen;
                        src[0] = mem_[0][a % size];
                        src[1] = mem_[1][a % size];
                    }
                    // o presente some sob o gesto inteiro (bordas no começo e
                    // no fim do gesto, não de cada fragmento)
                    const float w = mix * gestureEnv(edge);
                    for (int c = 0; c < 2; ++c)
                        out[c] = live[c] * (1.0f - w) + src[c] * env * w;
                    if (pos + 1 >= fr.len) ++idx_;
                }
            }
            ++now_;
            const float act = active() ? 1.0f : 0.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = out[c < 2 ? c : 1];
                outputs[1].at(c, f) = act;
            }
        }
    }

private:
    struct Frag { std::uint64_t at = 0, len = 1, src = 0, srcLen = 1; bool gap = false; };

    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    float uniform() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    std::uint64_t ms(const double v) const noexcept {
        return static_cast<std::uint64_t>(v * 0.001 * sr_);
    }
    // a borda do gesto inteiro: entra e sai em 3 ms
    float gestureEnv(const std::uint64_t edge) const noexcept {
        if (count_ <= 0) return 0.0f;
        const std::uint64_t start = frag_[0].at;
        const std::uint64_t end = frag_[static_cast<std::size_t>(count_ - 1)].at
                                + frag_[static_cast<std::size_t>(count_ - 1)].len;
        if (now_ < start + edge) return static_cast<float>(now_ - start) / static_cast<float>(edge);
        if (end - now_ <= edge) return static_cast<float>(end - now_) / static_cast<float>(edge);
        return 1.0f;
    }

    void plan(const int gesture, const int microN) noexcept {
        const std::uint64_t T = interval_;
        const std::uint64_t beatStart = now_ - T;   // o último tempo inteiro
        count_ = 0;
        idx_ = 0;
        if (gesture == 0) {   // STUT: 4×, espaço tempo/4 (14–160 ms)
            const std::uint64_t sp = std::clamp(T / 4, ms(14.0), ms(160.0));
            for (int i = 0; i < 4; ++i)
                frag_[static_cast<std::size_t>(count_++)] = {now_ + sp * static_cast<std::uint64_t>(i), sp, beatStart, sp, false};
        } else if (gesture == 1) {   // BURST: 8 fatias dos últimos 8 tempos, saco
            const std::uint64_t sp = std::clamp(T / 8, ms(18.0), ms(72.0));
            std::array<int, 8> bag{0, 1, 2, 3, 4, 5, 6, 7};
            for (int i = 7; i > 0; --i) {
                const int j = static_cast<int>(uniform() * static_cast<float>(i + 1)) % (i + 1);
                std::swap(bag[static_cast<std::size_t>(i)], bag[static_cast<std::size_t>(j)]);
            }
            // só os tempos que cabem no buffer de 4 s (em andamento lento,
            // menos de 8: o saco repete os que há)
            const int fit = std::clamp(static_cast<int>((mem_[0].size() - T) / T), 1, 8);
            for (int i = 0; i < 8; ++i) {
                const std::uint64_t src = now_ - T * static_cast<std::uint64_t>(bag[static_cast<std::size_t>(i)] % fit + 1);
                frag_[static_cast<std::size_t>(count_++)] = {now_ + sp * static_cast<std::uint64_t>(i), sp, src, sp, false};
            }
        } else if (gesture == 2) {   // MICRO: o começo do tempo, N vezes num tempo
            const std::uint64_t sp = std::max<std::uint64_t>(ms(8.0), T / static_cast<std::uint64_t>(microN));
            for (int i = 0; i < microN; ++i)
                frag_[static_cast<std::size_t>(count_++)] = {now_ + sp * static_cast<std::uint64_t>(i), sp, beatStart, sp, false};
        } else {   // GAP: um tempo de silêncio, com bordas
            frag_[static_cast<std::size_t>(count_++)] = {now_, T, beatStart, T, true};
        }
        ++gestures_;
    }

    float sr_ = 48000.0f;
    std::array<std::vector<float>, 2> mem_;
    std::uint64_t now_ = 0, interval_ = 24000, lastClk_ = 0;
    bool clkSeen_ = false;
    std::array<Frag, 8> frag_{};
    int count_ = 0, idx_ = 0;
    std::array<float, 5> prev_{};
    std::uint64_t rng_ = 1;
    float seedUsed_ = -1.0f;
    long gestures_ = 0;
};

}  // namespace rasgo::modular
