#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// ============================================================================
// MotionEngine — camada de composição sobre o grafo (protótipo)
// ============================================================================
//
// Ver o estudo: `RASGO_MODULAR/dossies/ESTUDO_seed_composicao_generativa.md`
// §3 — este é o "menor passo testável" da §3.5: 3 comportamentos (WALK,
// OSCILLATE, ATTRACT) sobre um punhado de parâmetros escolhidos à mão,
// antes de generalizar pra "qualquer parâmetro, qualquer comportamento".
//
// `rasgo_modular_core` é zero-dep e não sabe o que é "composição" — por
// isso isto vive em `apps/panel/`, como o `PatchSeed.hpp`: um consumidor
// da API pública do grafo, não uma mudança no motor.
//
// A peça que destrava isto é o motor ADITIVO (`connectToParameter`
// aditivo, 2026-09-04): o `MotionEngine` escreve por
// `SignalGraph::setParameterBase()` — exatamente a mesma via que um giro
// de knob usa. Qualquer modulação por cabo já plugada nesse parâmetro
// (`connectToParameter`/`followQuality`) continua somando por cima, sem
// conflito — o `MotionEngine` se comporta como uma mão girando o knob,
// não como um segundo escritor brigando com o primeiro.
//
// `tick()` roda em taxa BAIXA (uma vez por bloco de áudio, ou por frame
// de UI) — nunca dentro de `process()`; não é RT, não tem contrato de
// tempo real. Determinístico: cada `Binding` carrega seu próprio
// xorshift semeado, então duas execuções com os mesmos `add()` e os
// mesmos `dt` produzem a mesma trajetória (renders de exemplo
// reproduzíveis).

namespace rasgo::panel {

class MotionEngine {
public:
    enum class Behavior { Walk, Oscillate, Attract };

    struct Binding {
        std::size_t node = 0;
        std::string paramId;
        Behavior behavior = Behavior::Walk;
        float lo = 0.0f, hi = 1.0f;    // JANELA de operação — uma fração
                                       // pequena do range do parâmetro,
                                       // CENTRADA no valor atual (ver
                                       // `populateMotion` em
                                       // `panel_main.cpp`). Não é o range
                                       // inteiro: a Motion Engine
                                       // "respira" em torno do que o seed
                                       // escolheu, não varre tudo.
        float start = std::numeric_limits<float>::quiet_NaN();  // valor
                                       // inicial (o do knob agora); NaN =
                                       // começa no meio da janela
        float rateHz = 0.05f;         // WALK: ritmo médio de novos alvos.
                                       // OSCILLATE: frequência do ciclo.
                                       // ATTRACT: ignorado (usa `gain`).

        // ATTRACT: persegue o valor de INTENÇÃO (base) de outro
        // parâmetro — pode ser de outro módulo. `gain` = velocidade da
        // perseguição (maior = mais rápido).
        std::size_t nodeB = 0;
        std::string paramIdB;
        float gain = 0.3f;

        // seed própria — cada binding deriva de forma independente e
        // reproduzível
        std::uint64_t seed = 0x9E3779B97F4A7C15ULL;

        // estado interno (preenchido por `add()` / avançado por `tick()`)
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

    void clear() noexcept { bindings_.clear(); }
    std::size_t bindingCount() const noexcept { return bindings_.size(); }

    // avança `dt` segundos e escreve os novos valores no grafo via
    // `setParameterBase` — nunca chamar de dentro de `process()`.
    void tick(rasgo::modular::SignalGraph& graph, const float dt) noexcept {
        for (auto& b : bindings_) {
            float v = b.value;
            switch (b.behavior) {
            case Behavior::Walk: {
                // novo alvo com probabilidade ~rateHz por segundo
                // (Poisson aproximado por passo); desliza até lá a um
                // ritmo atrelado ao mesmo `rateHz` — sem "STATIC" nem
                // "vibra e não vai a lugar nenhum": tem alvo, tem chegada.
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

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    // uniforme em [0,1) — xorshift64*, mesmo padrão usado nos módulos DSP
    static float uniform(std::uint64_t& s) noexcept {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::uint32_t>(x >> 32))
            / 4294967296.0f;
    }

    std::vector<Binding> bindings_;
};

}  // namespace rasgo::panel
