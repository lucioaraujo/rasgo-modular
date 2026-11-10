#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>

// ============================================================================
// MIXER — soma estéreo de fontes (Módulo 16)
// ============================================================================
//
// Onde as vozes viram uma imagem. 4 canais mono, cada um com ganho (dB),
// posição no campo estéreo (`pan`, lei de potência constante) e mute.
// Saída estéreo (esquerda no canal 0, direita no canal 1). Se o grafo
// estiver em mono, a saída é a soma L+R.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/16_mixer.md`.
//
// Fontes ESTUDADAS (prática de estúdio, não código):
//   - lei de pan de potência constante (sin/cos) - energia constante ao
//     varrer o pan, sem o buraco no centro da lei linear;
//   - mixer de barramento clássico - ganho por canal + soma + ganho de
//     saída; fan-in explícito (aqui um nó, não N cabos numa porta).
//
// GRAVE NO CENTRO (2026-11-10, aprovado pelo autor): cada canal se divide
// num grave (um polo em ~150 Hz) e no resto (= sinal − grave, soma exata);
// só o resto segue o PAN, o grave fica sempre no centro — o grave não
// pende para um lado nem enfraquece em mono. Com PAN no centro o resultado
// é o de antes. O PAN anda por uma rampa (~30 ms), sem degrau nem clique.
// Princípio lido no Asterismus (`PerformanceSkyPan.hpp`, do autor);
// crossover complementar de um polo é técnica pública.
//
// SEGURANÇA DE BARRAMENTO (2026-09-05): a soma passa por um joelho `tanh`
// bem suave — **transparente até 1,0 (0 dBFS)** (mixagem sensata não é
// tocada; os testes de soma linear continuam exatos), comprime de leve
// acima, teto ~1,35 (+2,6 dBFS). Sem isto um patch generativo quente (ou
// uma mutação) mandava +5..+10 dB pro MASTER, que aí limitava DEMAIS /
// bombeava / distorcia — o "clipe" reportado pelo autor. Não é limitador
// (isso é do MASTER), é só não deixar a soma passar do razoável.
//
// PALCO (2026-11-10, aprovado pelo autor: "os quatro servem; contrapeso
// ativo...; no mixer"). Um seletor de modo:
//   0 MANUAL  — como sempre: o PAN é o lugar; nada se move sozinho;
//   1 RODA    — os canais giram em torno das suas casas, fases defasadas
//               por igual (a soma dos deslocamentos é zero);
//   2 CÂNONE  — um trajeto lento; cada canal o repete atrasado e
//               espelhado (ideia do "pan follow" do MetroCityBus);
//   3 GESTO   — a cada FRASE (gatilho) ou a cada tanto, um canal vai a um
//               lugar novo escolhido para REEQUILIBRAR, deslizando 1–4 s;
//   4 PÊNDULO — só as entradas PAN 1–4 movem (a seed liga o PENDULUM).
// Fora do MANUAL, o knob PAN de cada canal é a CASA (o centro do
// movimento); VELOC dá o período (60 s … 6 s), ABERT a amplitude. As
// entradas PAN 1–4 (−1..+1) somam ao lugar em qualquer modo.
//
// O CONTRAPESO — um MÓBILE (ideia do autor e nossa; Calder): fora do
// MANUAL, cada canal PESA o som que está tocando (energia lenta, ~1,5 s,
// só da parte aguda: o grave mora no centro e não pesa). Lei da alavanca:
// quanto mais pesado, mais curto o braço (alcance 0,55·√(peso médio /
// peso), entre 40 % e 100 %). O centro de gravidade do palco fica em
// ±0,10; passou, corrige — e quem anda é o LEVE (correção ∝ 1/peso). A
// correção é uma mola amortecida (~2,5 s, ζ 0,6): o palco balança e
// assenta. Nenhum mixer do Cardinal pesa o estéreo pelo som (lido em
// 10/11: todo pan lá é posição dada). Teto de abertura 0,55 (aprovado).
//
// Determinístico (sem RNG). Sem alocação.

namespace rasgo::modular {

class Mixer final : public Signal {
public:
    Mixer()
        : Signal(
              {{"ch1", PortKind::Audio, ""},
               {"ch2", PortKind::Audio, ""},
               {"ch3", PortKind::Audio, ""},
               {"ch4", PortKind::Audio, ""},
               {"pan1", PortKind::Control, ""},
               {"pan2", PortKind::Control, ""},
               {"pan3", PortKind::Control, ""},
               {"pan4", PortKind::Control, ""},
               {"phrase", PortKind::Control, "trig"}},
              {{"out", PortKind::Audio, ""}},
              {{"gain1", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan1", -1.0f, 1.0f, 0.0f, ""},
               {"mute1", 0.0f, 1.0f, 0.0f, ""},
               {"gain2", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan2", -1.0f, 1.0f, 0.0f, ""},
               {"mute2", 0.0f, 1.0f, 0.0f, ""},
               {"gain3", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan3", -1.0f, 1.0f, 0.0f, ""},
               {"mute3", 0.0f, 1.0f, 0.0f, ""},
               {"gain4", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan4", -1.0f, 1.0f, 0.0f, ""},
               {"mute4", 0.0f, 1.0f, 0.0f, ""},
               {"out_gain", -24.0f, 12.0f, 0.0f, "dB"},
               {"stage", 0.0f, 4.0f, 0.0f, ""},
               {"stage_rate", 0.0f, 1.0f, 0.4f, ""},
               {"stage_width", 0.0f, 1.0f, 0.6f, ""}}) {}

    static constexpr float kStageMax = 0.55f;   // abertura máxima aprovada
    enum StageMode { kManual = 0, kRoda, kCanone, kGesto, kPendulo };

    // Para o visor (lidos pela UI sem lock).
    float stagePosition(const int c) const noexcept { return shownPos_[c & 3].load(std::memory_order_relaxed); }
    float stageWeight(const int c) const noexcept { return shownW_[c & 3].load(std::memory_order_relaxed); }
    float stageCentre() const noexcept { return shownCog_.load(std::memory_order_relaxed); }

    std::string type() const override { return "MIXER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 4 tiras de canal verticais + saída embaixo
        Panel p;
        // 18 HP desde 10/11 (eram 14): a coluna PALCO à direita
        p.hp = 18;
        p.add(Widget::Kind::Label, "MIXER", "", 2.5f, 2.0f);
        // Layout de 10/11/2026 (18 HP, pedido do autor: "melhore o layout
        // do mixer"). Quatro tiras à esquerda, com o jack alinhado ao
        // centro do knob; o visor do PALCO embaixo, na LARGURA das tiras —
        // o eixo do visor é o próprio estéreo (esquerda → direita); a
        // coluna da direita junta os controles do PALCO e a saída.
        for (int c = 0; c < 4; ++c) {
            const std::string n = std::to_string(c + 1);
            const float x = 4.0f + static_cast<float>(c) * 15.0f;   // borda do knob/slider
            const float cx = x + 4.5f;                                 // centro da tira
            p.add(Widget::Kind::Slider, ("CH" + n), "gain" + n, x + 0.5f, 9.0f);
            p.add(Widget::Kind::Knob, "PAN", "pan" + n, x, 52.0f);
            p.add(Widget::Kind::Toggle, "MUTE", "mute" + n, x, 64.0f);
            p.add(Widget::Kind::Jack, "P" + n, "in:pan" + n, cx, 82.0f);
            p.add(Widget::Kind::Jack, n, "in:ch" + n, cx, 95.0f);
        }
        p.add(Widget::Kind::Display, "palco", "", 3.0f, 102.5f, 57.5f);
        p.tall(19.0f);
        // coluna PALCO + saída
        p.add(Widget::Kind::Label, "PALCO", "", 67.0f, 9.0f);
        p.add(Widget::Kind::Knob, "MODO", "stage", 70.0f, 19.0f);
        p.add(Widget::Kind::Knob, "VELOC", "stage_rate", 70.0f, 37.0f);
        p.add(Widget::Kind::Knob, "ABERT", "stage_width", 70.0f, 55.0f);
        p.add(Widget::Kind::Jack, "FRASE", "in:phrase", 74.5f, 82.0f);
        p.add(Widget::Kind::Knob, "OUT", "out_gain", 70.0f, 96.0f);
        p.add(Widget::Kind::Jack, "L+R", "out:out", 74.5f, 120.0f);
        return p;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        float gainLin[4], panTarget[4];
        bool on[4];
        for (int c = 0; c < 4; ++c) {
            const std::string n = std::to_string(c + 1);
            on[c] = parameterValue("mute" + n) < 0.5f && inputs[c] != nullptr;
            gainLin[c] = dbToGain(parameterValue("gain" + n));
            panTarget[c] = clampf(parameterValue("pan" + n), -1.0f, 1.0f);
        }
        stage(inputs, frames, on, panTarget);
        for (int c = 0; c < 4; ++c)
            if (!primed_) pan_[c] = panTarget[c];
        primed_ = true;
        float hiPow[4] = {0, 0, 0, 0};
        const float outGain = dbToGain(parameterValue("out_gain"));
        constexpr float kCentre = 0.70710678f;   // cos(π/4): o centro da lei

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float l = 0.0f, r = 0.0f;
            for (int c = 0; c < 4; ++c) {
                pan_[c] += (panTarget[c] - pan_[c]) * panK_;
                if (!on[c])
                    continue;
                const float s = inputs[c]->at(0, frame) * gainLin[c];
                low_[c] += (s - low_[c]) * lowK_;
                const float lo = low_[c];
                const float hi = s - lo;
                hiPow[c] += hi * hi;
                const float t = (pan_[c] + 1.0f) * 0.7853982f;  // 0..π/2
                l += lo * kCentre + hi * std::cos(t);
                r += lo * kCentre + hi * std::sin(t);
            }
            l = busKnee(l * outGain);
            r = busKnee(r * outGain);
            if (channels >= 2) {
                out.at(0, frame) = l;
                out.at(1, frame) = r;
                for (std::size_t ch = 2; ch < channels; ++ch)
                    out.at(ch, frame) = 0.5f * (l + r);
            } else {
                out.at(0, frame) = 0.5f * (l + r);
            }
        }
        // o peso de cada canal: energia lenta da parte aguda (~1,5 s)
        if (frames > 0) {
            const float k = 1.0f - std::exp(-static_cast<float>(frames) / (1.5f * sr_));
            for (int c = 0; c < 4; ++c) {
                const float m = on[c] ? hiPow[c] / static_cast<float>(frames) : 0.0f;
                pow_[c] += (m - pow_[c]) * k;
                if (!(pow_[c] >= 0.0f) || !std::isfinite(pow_[c])) pow_[c] = 0.0f;
            }
        }
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        const float sr = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? sampleRate : 48000.0f;
        lowK_ = 1.0f - std::exp(-2.0f * 3.14159265f * 150.0f / sr);
        panK_ = 1.0f - std::exp(-1.0f / (0.030f * sr));
        for (int c = 0; c < 4; ++c) {
            low_[c] = 0.0f; pow_[c] = 0.0f; corr_[c] = 0.0f; corrV_[c] = 0.0f;
            gP_[c] = 0.0f; gT_[c] = 0.0f; gTau_[c] = 2.0f;
        }
        sr_ = sr;
        phase_ = 0.0; gTimer_ = 0.0f; gCount_ = 0; prevPhrase_ = 0.0f;
        primed_ = false;
    }

private:
    // ---- PALCO: o lugar de cada canal neste bloco -------------------
    void stage(const std::vector<const AudioBlock*>& inputs, const std::size_t frames,
               const bool on[4], float panTarget[4]) noexcept {
        const int mode = static_cast<int>(std::lround(clampf(parameterValue("stage"), 0.0f, 4.0f)));
        const float dt = static_cast<float>(frames) / sr_;
        float cv[4];
        for (int c = 0; c < 4; ++c) {
            const AudioBlock* b = inputs.size() > static_cast<std::size_t>(4 + c) ? inputs[4 + c] : nullptr;
            const float v = (b && b->frames() > 0) ? b->at(0, 0) : 0.0f;
            cv[c] = std::isfinite(v) ? clampf(v, -1.0f, 1.0f) : 0.0f;
        }
        if (mode == kManual) {
            for (int c = 0; c < 4; ++c) {
                panTarget[c] = clampf(panTarget[c] + cv[c], -1.0f, 1.0f);
                corr_[c] = corrV_[c] = 0.0f;
                publish(c, panTarget[c]);
            }
            shownCog_.store(0.0f, std::memory_order_relaxed);
            return;
        }
        const float rate = clampf(parameterValue("stage_rate"), 0.0f, 1.0f);
        const float width = clampf(parameterValue("stage_width"), 0.0f, 1.0f);
        const float period = 60.0f * std::pow(0.1f, rate);     // 60 s … 6 s
        const float amp = 0.32f * width;
        phase_ += 2.0 * 3.14159265358979 * static_cast<double>(dt / period);
        if (phase_ > 1.0e6) phase_ = std::fmod(phase_, 2.0 * 3.14159265358979);

        // quem soa (pesa) e o peso de cada um (amplitude lenta)
        int act[4], n = 0;
        float w[4], wsum = 0.0f;
        for (int c = 0; c < 4; ++c) {
            w[c] = std::sqrt(std::max(0.0f, pow_[c]));
            if (on[c] && w[c] > 1.0e-4f) { act[n++] = c; wsum += w[c]; }
        }
        const float wbar = n > 0 ? wsum / static_cast<float>(n) : 0.0f;

        // o vento: o deslocamento que o trajeto propõe
        float x[4], reach[4];
        for (int c = 0; c < 4; ++c) {
            const float home = clampf(panTarget[c], -kStageMax, kStageMax);
            reach[c] = kStageMax;
            if (n > 0 && w[c] > 1.0e-4f)
                reach[c] = kStageMax * clampf(std::sqrt(wbar / w[c]), 0.4f, 1.0f);
            x[c] = home + cv[c] * kStageMax;
        }
        const double ph = phase_;
        if (mode == kRoda || mode == kCanone) {
            for (int k = 0; k < n; ++k) {
                const int c = act[k];
                if (mode == kRoda) {
                    x[c] += amp * static_cast<float>(std::sin(ph + 2.0 * 3.14159265358979 * k / n));
                } else {
                    const double q = ph - k * 1.5707963;   // um quarto de volta de atraso
                    const float path = static_cast<float>(0.7 * std::sin(q) + 0.3 * std::sin(2.618 * q + 1.0));
                    x[c] += amp * (k % 2 == 0 ? path : -path);
                }
            }
        } else if (mode == kGesto) {
            const AudioBlock* fr = inputs.size() > 8 ? inputs[8] : nullptr;
            bool fire = false;
            if (fr && fr->frames() > 0) {
                for (std::size_t f = 0; f < fr->frames(); ++f) {
                    const float v = fr->at(0, f);
                    if (prevPhrase_ < 0.5f && v >= 0.5f) fire = true;
                    prevPhrase_ = v;
                }
            } else {
                gTimer_ += dt;
                const float every = period / static_cast<float>(2 * std::max(1, n));
                if (gTimer_ >= every) { gTimer_ = 0.0f; fire = true; }
            }
            if (fire && n > 0) {
                const int c = act[gCount_ % n];
                // o lado com menos peso (o torque dos outros manda o gesto
                // para o lado oposto); sequência de razão áurea, não sorteio
                float torque = 0.0f;
                for (int k = 0; k < n; ++k)
                    if (act[k] != c) torque += w[act[k]] * (x[act[k]] + gP_[act[k]]);
                const float side = std::fabs(torque) > 1.0e-6f ? (torque > 0.0f ? -1.0f : 1.0f)
                                                               : (gCount_ % 2 ? 1.0f : -1.0f);
                const float u = static_cast<float>(std::fmod(gCount_ * 0.6180339887, 1.0));
                const float v = static_cast<float>(std::fmod(gCount_ * 0.4142135624, 1.0));
                gT_[c] = side * amp * (0.4f + 0.6f * u) * 1.6f;
                gTau_[c] = 1.0f + 3.0f * v;
                ++gCount_;
            }
            for (int c = 0; c < 4; ++c) {
                gP_[c] += (gT_[c] - gP_[c]) * (1.0f - std::exp(-dt / gTau_[c]));
                x[c] += gP_[c];
            }
        }
        for (int c = 0; c < 4; ++c) x[c] = clampf(x[c], -reach[c], reach[c]);

        // o móbile: equilíbrio pela lei da alavanca, o leve se move
        float want[4] = {0, 0, 0, 0};
        if (n >= 2) {
            float rem = 0.0f;
            bool free_[4] = {true, true, true, true};
            for (int it = 0; it < 3; ++it) {
                float tq = 0.0f;
                for (int k = 0; k < n; ++k) tq += w[act[k]] * (x[act[k]] + want[act[k]]);
                const float cog = tq / wsum;
                rem = cog - clampf(cog, -0.10f, 0.10f);
                if (std::fabs(rem) < 1.0e-5f) break;
                int nf = 0;
                for (int k = 0; k < n; ++k) if (free_[act[k]]) ++nf;
                if (nf == 0) break;
                // Σ w·Δ = −rem·Σw com Δ_c = −g/w_c  →  g = rem·Σw/nf
                const float gq = rem * wsum / static_cast<float>(nf);
                for (int k = 0; k < n; ++k) {
                    const int c = act[k];
                    if (!free_[c]) continue;
                    float d = want[c] - gq / w[c];
                    const float lim = reach[c];
                    if (x[c] + d > lim) { d = lim - x[c]; free_[c] = false; }
                    if (x[c] + d < -lim) { d = -lim - x[c]; free_[c] = false; }
                    want[c] = d;
                }
            }
        }
        // assentamento: mola amortecida (~2,5 s, ζ 0,6) — balança e assenta
        const float om = 2.0f * 3.14159265f / 2.5f, zeta = 0.6f;
        const int steps = std::max(1, static_cast<int>(std::ceil(dt / 0.01f)));
        const float h = dt / static_cast<float>(steps);
        for (int c = 0; c < 4; ++c) {
            for (int s = 0; s < steps; ++s) {
                const float a = om * om * (want[c] - corr_[c]) - 2.0f * zeta * om * corrV_[c];
                corrV_[c] += a * h;
                corr_[c] += corrV_[c] * h;
            }
            if (!std::isfinite(corr_[c])) corr_[c] = corrV_[c] = 0.0f;
            panTarget[c] = clampf(x[c] + corr_[c], -kStageMax, kStageMax);
            publish(c, panTarget[c]);
        }
        float tq = 0.0f;
        for (int k = 0; k < n; ++k) tq += w[act[k]] * panTarget[act[k]];
        shownCog_.store(wsum > 0.0f ? tq / wsum : 0.0f, std::memory_order_relaxed);
        for (int c = 0; c < 4; ++c)
            shownW_[c].store(wbar > 0.0f ? w[c] / (wbar * static_cast<float>(std::max(1, n))) : 0.0f,
                             std::memory_order_relaxed);
    }
    void publish(const int c, const float p) noexcept {
        shownPos_[c].store(p, std::memory_order_relaxed);
    }

    float pow_[4] = {0, 0, 0, 0};
    float corr_[4] = {0, 0, 0, 0}, corrV_[4] = {0, 0, 0, 0};
    float gP_[4] = {0, 0, 0, 0}, gT_[4] = {0, 0, 0, 0}, gTau_[4] = {2, 2, 2, 2};
    double phase_ = 0.0;
    float gTimer_ = 0.0f, prevPhrase_ = 0.0f, sr_ = 48000.0f;
    long gCount_ = 0;
    std::atomic<float> shownPos_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    std::atomic<float> shownW_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    std::atomic<float> shownCog_{0.0f};

    float low_[4] = {0, 0, 0, 0};
    float pan_[4] = {0, 0, 0, 0};
    float lowK_ = 0.02f, panK_ = 0.0007f;
    bool primed_ = false;

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float dbToGain(const float db) noexcept {
        return db <= -60.0f ? 0.0f : std::pow(10.0f, db / 20.0f);
    }
    // joelho de barramento: |x| <= 1,0 passa EXATO; acima, tanh suave,
    // teto ~1,35. Um patch quente não estoura seco no MASTER.
    static float busKnee(const float x) noexcept {
        const float a = x < 0.0f ? -x : x;
        if (a <= 1.0f) return x;
        const float shaped = 1.0f + 0.35f * std::tanh((a - 1.0f) / 0.35f);
        return x < 0.0f ? -shaped : shaped;
    }
};

}  // namespace rasgo::modular
