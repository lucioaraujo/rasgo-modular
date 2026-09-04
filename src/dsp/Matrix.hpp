#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>
#include <string>

// ============================================================================
// MATRIX — Matriz de roteamento (Módulo 33)
// ============================================================================
//
// O `MIXER` soma 4 → 1. Esta é a MATRIZ 4×4 — cada cruzamento fonte×destino
// é um ganho (atenuversor), como as matrizes de pinos do EMS Synthi / ARP
// 2500 ou o Doepfer A-138m. Patch denso sem espaguete: 8 cabos, 16 knobs.
// `out_k = level · sat(Σ_j in_j · g_jk)`. Padrão = identidade (passa-direto).
// `norm` = nível constante por coluna; `sat` = matriz segura em laço;
// `drift` = os ganhos respiram (assinatura RASGO).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/33_matrix.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - EMS Synthi / ARP 2500 (matriz de pinos — prática pública);
//   - Doepfer A-138m, Befaco / Erica matrix mixer (N×M com ganho por célula);
//   - Serge / Buchla matrix mixer (normalização por coluna);
//   - camada MATRIZ do `SignalGraph` do Rasgo (`matrixCell`, marco 2).
//
// Determinístico: o `drift` é soma de senos, sem RNG.

namespace rasgo::modular {

class Matrix final : public Signal {
public:
    Matrix()
        : Signal(makeInputs(), makeOutputs(), makeParams()) {}

    std::string type() const override { return "MATRIX"; }

    Panel panel() const override {
        Panel p;
        p.hp = 20;
        p.add(Widget::Kind::Label, "MATRIX", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "grid", "", 2.5f, 7.0f, 74.0f);
        // grade 4×4 de knobs
        for (int j = 0; j < 4; ++j) {
            const float y = 30.0f + static_cast<float>(j) * 18.0f;
            for (int k = 0; k < 4; ++k) {
                const float x = 22.0f + static_cast<float>(k) * 17.0f;
                const std::string id = cellId(j, k);
                p.add(Widget::Kind::Knob, id.substr(1), id, x, y);
            }
            p.add(Widget::Kind::Jack, "IN" + std::to_string(j + 1),
                  "in:in" + std::to_string(j + 1), 8.0f, y + 3.0f);
        }
        for (int k = 0; k < 4; ++k) {
            const float x = 22.0f + static_cast<float>(k) * 17.0f;
            p.add(Widget::Kind::Jack, "OUT" + std::to_string(k + 1),
                  "out:out" + std::to_string(k + 1), x, 104.0f);
        }
        p.add(Widget::Kind::Knob, "LEVEL", "level", 90.0f, 26.0f);
        p.add(Widget::Kind::Knob, "NORM", "norm", 90.0f, 42.0f);
        p.add(Widget::Kind::Knob, "RING", "ring", 90.0f, 58.0f);
        p.add(Widget::Kind::Knob, "SAT", "sat", 90.0f, 74.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 90.0f, 90.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        driftInc_ = 2.0f * 3.14159265f * 0.03f
                    * static_cast<float>(blockSize) / sr_;
        driftPhase_ = 0.0f;
        for (auto& row : dw_)
            for (auto& c : row) c = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const float level = clampf(parameterValue("level"), 0.0f, 2.0f);
        const float norm = clamp01(parameterValue("norm"));
        const float ring = clamp01(parameterValue("ring"));
        const float sat = clamp01(parameterValue("sat"));
        const float drift = clamp01(parameterValue("drift"));

        float g[4][4];
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                g[j][k] = clampf(parameterValue(cellId(j, k)), -1.0f, 1.0f);

        // deriva: 1 seno por célula por bloco
        driftPhase_ += driftInc_;
        if (driftPhase_ > 6.2831853f) driftPhase_ -= 6.2831853f;
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                dw_[j][k] = drift * 0.12f
                    * std::sin(driftPhase_ * (1.0f + 0.3f * j) + 1.7f * k);

        float geff[4][4], invn[4];
        for (int k = 0; k < 4; ++k) {
            float wsum = 0.0f;
            for (int j = 0; j < 4; ++j) {
                geff[j][k] = clampf(g[j][k] + dw_[j][k], -1.5f, 1.5f);
                const float a = geff[j][k] < 0.0f ? -geff[j][k] : geff[j][k];
                wsum += a;
            }
            const float divisor = 1.0f + norm * (std::max(1.0f, wsum) - 1.0f);
            invn[k] = 1.0f / divisor;
        }

        const AudioBlock* in[4] = {inputs[0], inputs[1], inputs[2], inputs[3]};

        for (std::size_t f = 0; f < frames; ++f) {
            float xin[4];
            for (int j = 0; j < 4; ++j)
                xin[j] = in[j] != nullptr ? in[j]->at(0, f) : 0.0f;

            for (int k = 0; k < 4; ++k) {
                float acc = 0.0f;
                // `ring` cruza a coluna entre a soma linear e o PRODUTO
                // das entradas ponderado pelo ganho: célula g~0 = fator
                // unitário (bypass), g~±1 = ±entrada. Dois ganhos em 1 →
                // ring-mod de 4 quadrantes clássico.
                float prod = 1.0f;
                for (int j = 0; j < 4; ++j) {
                    acc += xin[j] * geff[j][k];
                    if (ring > 0.0f) {
                        const float ag = geff[j][k] < 0.0f ? -geff[j][k]
                                                           : geff[j][k];
                        prod *= xin[j] * geff[j][k]
                            + (1.0f - (ag > 1.0f ? 1.0f : ag));
                    }
                }
                float combined = acc;
                if (ring > 0.0f)
                    combined = acc
                        + (clampf(prod, -4.0f, 4.0f) - acc) * ring;
                float y = level * combined * invn[k];
                const float sc = y / (1.0f + (y < 0.0f ? -y : y) * 0.7f);
                y = y + (sc - y) * sat;
                for (std::size_t c = 0; c < channels; ++c)
                    outputs[static_cast<std::size_t>(k)].at(c, f) = y;
            }
        }
    }

private:
    static std::string cellId(const int j, const int k) {
        return std::string("g") + static_cast<char>('1' + j)
            + static_cast<char>('1' + k);
    }

    static std::vector<PortDescriptor> makeInputs() {
        std::vector<PortDescriptor> v;
        for (int j = 1; j <= 4; ++j)
            v.push_back({"in" + std::to_string(j), PortKind::Audio, ""});
        return v;
    }
    static std::vector<PortDescriptor> makeOutputs() {
        std::vector<PortDescriptor> v;
        for (int k = 1; k <= 4; ++k)
            v.push_back({"out" + std::to_string(k), PortKind::Audio, ""});
        return v;
    }
    static std::vector<ParameterDescriptor> makeParams() {
        std::vector<ParameterDescriptor> v;
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                v.push_back({cellId(j, k), -1.0f, 1.0f,
                             j == k ? 1.0f : 0.0f, ""});
        v.push_back({"level", 0.0f, 2.0f, 1.0f, ""});
        v.push_back({"norm", 0.0f, 1.0f, 0.0f, ""});
        v.push_back({"ring", 0.0f, 1.0f, 0.0f, ""});
        v.push_back({"sat", 0.0f, 1.0f, 0.0f, ""});
        v.push_back({"drift", 0.0f, 1.0f, 0.0f, ""});
        return v;
    }

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float sr_ = 48000.0f;
    float driftInc_ = 0.0f;
    float driftPhase_ = 0.0f;
    float dw_[4][4] = {};
};

}  // namespace rasgo::modular
