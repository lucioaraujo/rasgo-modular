#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// ELASTIC (#92) — uma corrente de massas e molas que o som faz vibrar (ideia
// do autor, 9/11/2026: "elásticos"; lote 2 das ideias físicas, aprovado;
// dossiê `dossies/92_elastic.md`).
//
// N massas (4–16) ligadas por molas, presas nas duas pontas. O sinal da
// entrada é uma FORÇA na massa de excitação (EXC); a saída é o deslocamento
// de duas massas de escuta (PICK e o seu espelho), em L e R. Os modos da
// corrente são os de uma corda discreta: ω_n = 2·ω₀·sen(nπ / 2(N+1)) — não
// harmônicos, cada vez mais apertados no agudo, o timbre de uma malha
// elástica. TUNE é ω₀/2π; DAMP é a perda; STRETCH endurece a mola (força
// cúbica, como o elástico que resiste mais quanto mais estica): com o sinal
// forte, os modos sobem e o som entorta.
//
// Conceito lido no Sapphire Elastika (GPL; uma malha de bolas e molas que o
// áudio atravessa); nenhum código copiado. Integração de Euler simplético
// por amostra (estável enquanto 2·ω₀·dt < 2; TUNE é limitado para isso).
//
// Desvio Rasgo:
//  (a) PLUCK — um gatilho que puxa a massa de excitação e solta: o módulo
//      soa sozinho, como uma corda de elástico dedilhada;
//  (b) STIFF por CV — a tensão do elástico se move (um vibrato de matéria).
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Elastic final : public Signal {
public:
    static constexpr int kMax = 16;

    Elastic() : Signal(
        {{"in", PortKind::Audio, ""},
         {"pluck", PortKind::Control, "trig"},
         {"tune_cv", PortKind::Control, "v/oct"}},
        {{"l", PortKind::Audio, ""},
         {"r", PortKind::Audio, ""}},
        {{"masses", 4.0f, 16.0f, 8.0f, ""},
         {"tune", 30.0f, 1500.0f, 180.0f, "Hz"},
         {"damping", 0.0f, 1.0f, 0.3f, ""},
         {"stretch", 0.0f, 1.0f, 0.2f, ""},
         {"pickup", 0.0f, 1.0f, 0.3f, ""},
         {"mix", 0.0f, 1.0f, 0.8f, ""}}) {}

    std::string type() const override { return "ELASTIC"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "ELASTIC", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "elastic", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "N", "masses", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TUNE", "tune", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damping", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "STRCH", "stretch", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "PICK", "pickup", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 36.0f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PLUK", "in:pluck", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "TUNE", "in:tune_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "L", "out:l", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        x_.fill(0.0f);
        v_.fill(0.0f);
        prevPluck_ = 0.0f;
        // começa dedilhado: o módulo soa ao carregar, mesmo sem entrada
        pluckNow_ = true;
    }

    // Para os testes e o mostrador.
    float position(int i) const noexcept { return x_[static_cast<std::size_t>(std::clamp(i, 0, kMax - 1))]; }
    int count() const noexcept { return n_; }
    // a frequência teórica do modo n (1 = o mais grave), em Hz
    static float modeHz(float tuneHz, int masses, int n) noexcept {
        return 2.0f * tuneHz * std::sin(3.14159265f * static_cast<float>(n) / (2.0f * static_cast<float>(masses + 1)));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* pluck = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* tcv = inputs.size() > 2 ? inputs[2] : nullptr;

        const int n = static_cast<int>(std::lround(std::clamp(safe(parameterValue("masses"), 8.0f), 4.0f, 16.0f)));
        if (n != n_) { n_ = n; x_.fill(0.0f); v_.fill(0.0f); }
        const float tuneK = std::clamp(safe(parameterValue("tune"), 180.0f), 30.0f, 1500.0f);
        const float damp = std::clamp(safe(parameterValue("damping"), 0.3f), 0.0f, 1.0f);
        const float stretch = std::clamp(safe(parameterValue("stretch"), 0.2f), 0.0f, 1.0f);
        const float pick = std::clamp(safe(parameterValue("pickup"), 0.3f), 0.0f, 1.0f);
        const float mix = std::clamp(safe(parameterValue("mix"), 0.8f), 0.0f, 1.0f);
        const float dt = 1.0f / sr_;
        // a massa de excitação fica perto de uma ponta (como o dedo na corda)
        const int exc = 1;
        const int pa = std::clamp(static_cast<int>(std::lround(pick * static_cast<float>(n - 1))), 0, n - 1);
        const int pb = n - 1 - pa;
        // a perda: Q de ~400 (DAMP 0) a ~3 (DAMP 1), em escala exponencial
        const float q = 400.0f * std::pow(3.0f / 400.0f, damp);
        // ganho de saída: compensa a ressonância (Q alto soa mais alto)
        const float outGain = 0.6f / std::sqrt(q);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (pluck) {
                const float pv = safe(pluck->at(0, f), 0.0f);
                if (prevPluck_ < 0.5f && pv >= 0.5f) pluckNow_ = true;
                prevPluck_ = pv;
            }
            float tune = tuneK;
            if (tcv) tune *= std::exp2(std::clamp(safe(tcv->at(0, f), 0.0f), -4.0f, 4.0f));
            // estável: 2·ω₀·dt < 2 ⇒ ω₀ < 1/dt; fica com folga
            tune = std::clamp(tune, 20.0f, 0.12f * sr_);
            const float w0 = 6.2831853f * tune;
            const float k = w0 * w0;
            const float c = w0 / q;
            if (pluckNow_) {
                pluckNow_ = false;
                // puxa a massa de excitação e as vizinhas num triângulo e solta
                for (int i = 0; i < n_; ++i) {
                    const float d = 1.0f - std::fabs(static_cast<float>(i - exc)) / static_cast<float>(n_);
                    x_[static_cast<std::size_t>(i)] += 0.6f * std::max(0.0f, d);
                }
            }
            const float dry = in ? safe(in->at(0, f), 0.0f) : 0.0f;
            const float force = 8.0f * dry * k;   // medido: ruído 0,5 entra e sai na mesma faixa
            // v += a·dt (com as posições antigas), depois x += v·dt
            for (int i = 0; i < n_; ++i) {
                const float xl = i > 0 ? x_[static_cast<std::size_t>(i - 1)] : 0.0f;
                const float xr = i < n_ - 1 ? x_[static_cast<std::size_t>(i + 1)] : 0.0f;
                const float xi = x_[static_cast<std::size_t>(i)];
                const float dl = xi - xl, dr = xr - xi;
                // mola endurecida: F = k·(d + s·d³)
                const float s = 4.0f * stretch;
                const float fl = k * (dl + s * dl * dl * dl);
                const float fr = k * (dr + s * dr * dr * dr);
                float a = fr - fl - c * v_[static_cast<std::size_t>(i)];
                if (i == exc) a += force;
                v_[static_cast<std::size_t>(i)] += a * dt;
            }
            for (int i = 0; i < n_; ++i) {
                auto& xi = x_[static_cast<std::size_t>(i)];
                xi += v_[static_cast<std::size_t>(i)] * dt;
                if (!std::isfinite(xi) || std::fabs(xi) > 50.0f) {   // nunca explode
                    x_.fill(0.0f); v_.fill(0.0f);
                    break;
                }
            }
            const float wl = std::tanh(outGain * x_[static_cast<std::size_t>(pa)] * 4.0f);
            const float wr = std::tanh(outGain * x_[static_cast<std::size_t>(pb)] * 4.0f);
            const float l = (1.0f - mix) * dry + mix * wl;
            const float r = (1.0f - mix) * dry + mix * wr;
            for (std::size_t ch = 0; ch < channels; ++ch) {
                outputs[0].at(ch, f) = l;
                outputs[1].at(ch, f) = r;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    std::array<float, kMax> x_{}, v_{};
    int n_ = 8;
    float prevPluck_ = 0.0f;
    bool pluckNow_ = true;
};

} // namespace rasgo::modular
