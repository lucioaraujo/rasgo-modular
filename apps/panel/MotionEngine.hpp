#pragma once

#include "core/SignalGraph.hpp"
#include "panel/MotionField.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// ============================================================================
// MotionEngine — camada de composição sobre o grafo
// ============================================================================
//
// Ver `dossies/ESTUDO_seed_composicao_generativa.md §3`.
//
// DOIS modos, coexistem:
//
//  1. `Binding` + `add()` — bindings MANUAIS (Walk/Oscillate/Attract),
//     um por vez, escolhidos à mão. Usado por `examples/peca_generativa_4`
//     e `tests/test_motion_engine`. É o protótipo §3.5/§3.6, mantido.
//
//  2. `inhabit()` + `MotionField` — a "MÃO CAÓTICA" (§3.7, 2026-09-07). A
//     engine varre o grafo e monta uma FIBRA por knob/slider/toggle (só
//     `MIXER`/`MASTER` ficam de fora). Um único campo caótico (Thomas)
//     move todas as fibras EM RELAÇÃO — coerente por construção, nunca
//     repete, nunca congela. A velocidade do campo segue a energia do
//     som (realimentação). "Gosto" = só a amplitude por fibra, de 4
//     pistas de palavra (`MotionField::motionReach`). De vez em quando
//     uma fibra "ousa" (excursão maior, alguns segundos).
//
// `rasgo_modular_core` é zero-dep e não sabe o que é "composição" — isto
// vive em `apps/panel/`, consumidor da API pública do grafo. Escreve por
// `SignalGraph::setParameterBase()` (aditivo — modulação por cabo já
// plugada continua somando por cima). `tick()` roda em taxa BAIXA (1×/
// frame de UI), nunca em `process()`. Determinístico: campo + fibras +
// RNG da ousadia semeados; mesmos `dt` → mesma trajetória.

namespace rasgo::panel {

class MotionEngine {
public:
    enum class Behavior { Walk, Oscillate, Attract };

    struct Binding {
        std::size_t node = 0;
        std::string paramId;
        Behavior behavior = Behavior::Walk;
        float lo = 0.0f, hi = 1.0f;
        float start = std::numeric_limits<float>::quiet_NaN();
        float rateHz = 0.05f;
        std::size_t nodeB = 0;
        std::string paramIdB;
        float gain = 0.3f;
        std::uint64_t seed = 0x9E3779B97F4A7C15ULL;
        float value = 0.0f;
        float target = 0.0f;
        float phase = 0.0f;
        std::uint64_t rng = 0;
    };

    void add(Binding b) noexcept {
        b.rng = b.seed;
        b.value = std::isnan(b.start) ? 0.5f * (b.lo + b.hi)
                                      : clampf(b.start, b.lo, b.hi);
        b.target = b.value;
        b.phase = 0.0f;
        bindings_.push_back(b);
    }

    void clear() noexcept { bindings_.clear(); fibers_.clear(); }
    std::size_t bindingCount() const noexcept { return bindings_.size(); }
    std::size_t fiberCount() const noexcept { return fibers_.size(); }

    // ---- modo 2: a mão caótica ------------------------------------------
    struct Fiber {
        std::size_t node = 0;
        std::string paramId;
        int kind = 0;                 // 0 knob · 1 slider · 2 toggle
        MotionReach reach = MotionReach::Free;
        float wx = 0, wy = 0, wz = 0; // projeção do campo
        float center = 0, pmin = 0, pmax = 1;
        float amp = 0.1f;             // fração do range
        float tau = 1.0f;
        bool cabled = false;
        bool quantize = false;        // knob/slider "de contagem"
        float value = 0.0f;
        float boldMul = 1.0f;
        float toggleDwell = 0.0f;     // s desde o último flip
    };

    // varre `shown` e monta uma fibra por knob/slider/toggle (menos
    // MIXER/MASTER). `seed` mistura curSeed do painel — 0 = estável.
    void inhabit(rasgo::modular::SignalGraph& graph, const std::uint64_t seed,
                 const std::vector<std::size_t>& shown) {
        fibers_.clear();
        field_.seed(seed ^ 0xA5A5C0FFEEULL);
        boldRng_ = seed ? (seed * 0x2545F4914F6CDD1DULL) ^ 0xD1B54A32D192ED03ULL
                        : 0xD1B54A32D192ED03ULL;
        boldCd_ = 6.0f + 14.0f * bu01();
        for (const std::size_t id : shown) {
            if (id >= graph.nodeCount()) continue;
            auto& node = graph.node(id);
            const std::string t = node.type();
            if (t == "MIXER" || t == "MASTER") continue;   // o músico mistura
            for (const auto& w : node.panel().widgets) {
                int kind;
                switch (w.kind) {
                case rasgo::modular::Widget::Kind::Knob:   kind = 0; break;
                case rasgo::modular::Widget::Kind::Slider: kind = 1; break;
                case rasgo::modular::Widget::Kind::Toggle: kind = 2; break;
                default: continue;
                }
                if (w.bind.empty() || w.bind.rfind("in:", 0) == 0
                    || w.bind.rfind("out:", 0) == 0)
                    continue;
                const rasgo::modular::ParameterDescriptor* d = nullptr;
                for (const auto& pr : node.parameters())
                    if (pr.descriptor.id == w.bind) { d = &pr.descriptor; break; }
                if (!d) continue;
                const float range = d->maximum - d->minimum;
                if (range <= 0.0f) continue;

                Fiber f;
                f.node = id;
                f.paramId = w.bind;
                f.kind = kind;
                f.reach = motionReach(w.bind);
                // slider nunca ganha amplitude Free — é nota ou fader
                if (kind == 1 && f.reach == MotionReach::Free)
                    f.reach = MotionReach::Hot;
                f.amp = motionAmplitude(f.reach);
                f.tau = motionTau(f.reach);
                f.pmin = d->minimum;
                f.pmax = d->maximum;
                f.center = clampf(graph.parameterUserValue(id, w.bind),
                                  d->minimum, d->maximum);
                f.cabled = graph.parameterIsModulated(id, w.bind);
                f.quantize = (kind != 2)
                    && (f.reach == MotionReach::Structural)
                    && (range >= 3.0f);
                f.value = f.center;

                // projeção semeada por (seed, node, paramId)
                std::uint64_t h = 1469598103934665603ULL;
                for (const char c : t) { h ^= (unsigned char)c; h *= 1099511628211ULL; }
                for (const char c : w.bind) { h ^= (unsigned char)c; h *= 1099511628211ULL; }
                h ^= id + 0x9E3779B97F4A7C15ULL; h *= 1099511628211ULL;
                h ^= seed + 0xD1B54A32D192ED03ULL; h *= 1099511628211ULL;
                auto w01 = [&] {
                    h ^= h >> 13; h *= 0xBF58476D1CE4E5B9ULL; h ^= h >> 7;
                    return static_cast<float>((h >> 40)) / 16777216.0f;
                };
                f.wx = 2.0f * w01() - 1.0f;
                f.wy = 2.0f * w01() - 1.0f;
                f.wz = 2.0f * w01() - 1.0f;
                const float n = std::sqrt(f.wx*f.wx + f.wy*f.wy + f.wz*f.wz);
                if (n > 1e-6f) { f.wx /= n; f.wy /= n; f.wz /= n; }

                fibers_.push_back(std::move(f));
            }
        }
    }

    // avança `dt` s. `energy` (0..1) do som acelera a mão (realimentação).
    void tick(rasgo::modular::SignalGraph& graph, const float dt,
              const float energy = 0.5f) noexcept {
        tickBindings(graph, dt);
        if (fibers_.empty()) return;

        field_.step(dt, energy);

        // ousadia: de tempos em tempos uma fibra estica a excursão
        boldCd_ -= dt;
        if (boldCd_ <= 0.0f) {
            boldCd_ = 14.0f + 26.0f * bu01();
            if (!fibers_.empty()) {
                const std::size_t i = static_cast<std::size_t>(
                    bu01() * static_cast<float>(fibers_.size())) % fibers_.size();
                if (fibers_[i].reach == MotionReach::Free
                    || fibers_[i].reach == MotionReach::Hot) {
                    boldFiber_ = static_cast<long>(i);
                    boldLeft_ = 3.5f + 6.0f * bu01();
                }
            }
        }
        if (boldLeft_ > 0.0f) boldLeft_ -= dt;

        // realimentação protetora: som perto do teto → puxa tudo pro
        // centro (mais forte nos quentes/nível). Fecha o loop na direção
        // segura — o autor reportou clip com a engine ligada.
        const float duckAll = energy > 0.82f
            ? std::min(1.0f, (energy - 0.82f) * 3.5f) : 0.0f;

        const double fx = field_.nx(), fy = field_.ny(), fz = field_.nz();
        for (std::size_t i = 0; i < fibers_.size(); ++i) {
            Fiber& f = fibers_[i];
            const float goalBold = (boldLeft_ > 0.0f
                                    && boldFiber_ == static_cast<long>(i))
                ? 2.6f : 1.0f;
            f.boldMul += (goalBold - f.boldMul) * (1.0f - std::exp(-dt / 1.5f));

            float raw = f.wx * static_cast<float>(fx)
                      + f.wy * static_cast<float>(fy)
                      + f.wz * static_cast<float>(fz);
            raw = std::tanh(raw);                       // [-1,1]

            const float range = f.pmax - f.pmin;
            const float effAmp = std::min(0.48f, f.amp * f.boldMul);
            const float cableK = f.cabled ? 0.45f : 1.0f;

            if (f.kind == 2) {
                // toggle: histerese + dwell mínimo (estrutural = dwell longo)
                f.toggleDwell += dt;
                const float minDwell = (f.reach == MotionReach::Structural)
                    ? 25.0f : (f.reach == MotionReach::Hot ? 9.0f : 5.0f);
                const float thr = 0.58f;
                if (f.toggleDwell >= minDwell) {
                    if (raw > thr && f.value < 0.5f) {
                        f.value = 1.0f; f.toggleDwell = 0.0f;
                    } else if (raw < -thr && f.value >= 0.5f) {
                        f.value = 0.0f; f.toggleDwell = 0.0f;
                    }
                }
                graph.setParameterBase(f.node, f.paramId, f.value);
                continue;
            }

            float target = f.center + raw * effAmp * range * cableK;
            // não deixa fugir da janela (mesmo com ousadia)
            const float win = std::min(range, effAmp * range * 1.4f + 1e-4f);
            target = clampf(target, f.center - win, f.center + win);
            target = clampf(target, f.pmin, f.pmax);
            // duck protetor: perto do clip, converge pro centro
            const float duck = duckAll
                * ((f.reach == MotionReach::Hot
                    || f.reach == MotionReach::Level) ? 1.0f : 0.55f);
            target += (f.center - target) * duck;
            if (f.quantize) target = std::round(target);

            const float k = 1.0f - std::exp(-dt / std::max(0.05f, f.tau));
            f.value += (target - f.value) * k;
            f.value = clampf(f.value, f.pmin, f.pmax);
            graph.setParameterBase(f.node, f.paramId, f.value);
        }
    }

private:
    void tickBindings(rasgo::modular::SignalGraph& graph, const float dt) noexcept {
        for (auto& b : bindings_) {
            float v = b.value;
            switch (b.behavior) {
            case Behavior::Walk: {
                if (uniform(b.rng) < b.rateHz * dt)
                    b.target = b.lo + uniform(b.rng) * (b.hi - b.lo);
                const float coef = 1.0f - std::exp(-dt * b.rateHz * 3.0f);
                v += (b.target - v) * coef;
                break;
            }
            case Behavior::Oscillate: {
                b.phase += b.rateHz * dt;
                b.phase -= std::floor(b.phase);
                const float s = 0.5f + 0.5f * std::sin(6.2831853f * b.phase);
                v = b.lo + s * (b.hi - b.lo);
                break;
            }
            case Behavior::Attract: {
                const float t = clampf(
                    graph.parameterUserValue(b.nodeB, b.paramIdB), b.lo, b.hi);
                const float coef = 1.0f - std::exp(-dt * b.gain);
                v += (t - v) * coef;
                break;
            }
            }
            v = clampf(v, b.lo, b.hi);
            b.value = v;
            graph.setParameterBase(b.node, b.paramId, v);
        }
    }

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float uniform(std::uint64_t& s) noexcept {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::uint32_t>(x >> 32))
            / 4294967296.0f;
    }
    float bu01() noexcept {
        boldRng_ ^= boldRng_ << 13; boldRng_ ^= boldRng_ >> 7;
        boldRng_ ^= boldRng_ << 17;
        return static_cast<float>(boldRng_ >> 40) / 16777216.0f;
    }

    std::vector<Binding> bindings_;
    std::vector<Fiber> fibers_;
    MotionField field_;
    std::uint64_t boldRng_ = 0xD1B54A32D192ED03ULL;
    float boldCd_ = 10.0f;
    float boldLeft_ = 0.0f;
    long boldFiber_ = -1;
};

}  // namespace rasgo::panel
