#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// CHAOS — campo caótico de poço duplo (Módulo 36)
// ============================================================================
//
// Fonte genuinamente CAÓTICA — não pseudo-aleatória como `DECISION`/
// `TURING` (que sorteiam), não passeio como `DRIFT` (que soma ruído
// filtrado). Dois integradores (x, y) perseguem uma força restauradora
// não-linear (x − x³) com dois poços estáveis em x≈−1/x≈+1; `drive`/
// `damping` decidem se o sistema assenta num poço, oscila entre os dois
// ou "caça" imprevisível. Sem um "chute" periódico aleatório, o sistema
// é uma EDO determinística sem forçamento: uma vez orbitando um poço,
// nunca alcança o outro (achado do próprio autor do Antitotem, ver
// comentário-fonte) — o chute é o que deixa a trajetória atravessar de
// um poço pro outro de vez em quando.
//
// Estudado de `ANTITOTEM/src/core/ChaosSources.h::ChaosField` (código
// do autor, GPLv3/AGPLv3 — compatível; ver `PESQUISA_MODULOS.md §2.3`),
// reescrito no idioma header-only zero-dep do Rasgo Modular — desvio:
// porta `reseed` (trigger externo) em vez de método chamado pelo host,
// `rate_mod` como entrada de CV (convenção do Rasgo), painel/parâmetros
// próprios.
//
// `rate` cobre de CV lenta (0,02 Hz) a textura de áudio (400 Hz) — o
// mesmo eixo que ENERGIA escala noutros módulos do Antitotem. Saída
// sempre limitada a [−1,1] (os estados internos x/y são clampados
// separadamente, mais largos, pra dar fôlego à dinâmica sem escapar pra
// um valor perigoso). Determinístico (RNG semeado). Sem alocação em
// `process()`.
//
// MODO (11/11/2026, auditoria: "mais atratores como modos (Rössler,
// Aizawa)"; "avance em todos eles"):
//   0 POÇO    — o campo de poço duplo de sempre (nada muda);
//   1 RÖSSLER — O. Rössler (1976): ẋ = −y − z, ẏ = x + a·y, ż = b + z(x − c),
//               a = 0,1 + 0,2·DAMP, b = 0,2, c = 3 + 5·DRIVE — o DRIVE anda
//               pela rota de DUPLICAÇÃO DE PERÍODO: c baixo, um ciclo; mais
//               alto, dois, quatro… até o caos (c ≈ 5,7, o clássico);
//   2 AIZAWA  — o atrator de Aizawa (esfera com um tubo no eixo), com
//               ε = 0,1 + 0,4·DRIVE e o amortecimento em DAMP.
// Os dois são EDOs de 3 dimensões, integradas por RK2 em 4 subpassos; RATE
// escala o tempo do atrator (≈ uma volta por segundo em RATE 1). Saídas
// novas Y e Z (índices 1 e 2) — as outras dimensões do mesmo atrator; no
// POÇO, Y é a velocidade e Z fica em 0. Um atrator que diverge volta ao
// começo (nunca NaN).

namespace rasgo::modular {

class Chaos final : public Signal {
public:
    Chaos()
        : Signal(
              {{"reseed", PortKind::Control, "trig"},
               {"rate_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""},
               {"y", PortKind::Audio, ""},
               {"z", PortKind::Audio, ""}},
              {{"rate", 0.02f, 400.0f, 4.0f, "Hz"},
               {"drive", 0.0f, 1.0f, 0.6f, ""},
               {"damping", 0.05f, 1.0f, 0.3f, ""},
               {"freeze", 0.0f, 1.0f, 0.0f, ""},
               {"model", 0.0f, 2.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CHAOS"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        // 8 -> 7 HP no passe de ergonomia 2026-09-06 (só 3 knobs + toggle
        // + 3 jacks); display cheio, knobs 2 col centradas.
        Panel p;
        p.hp = 7;
        p.add(Widget::Kind::Label, "CHAOS", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "field", "", 2.5f, 6.0f, 30.6f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 21.5f, 30.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damping", 5.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "FRZ", "freeze", 23.0f, 54.0f);
        p.add(Widget::Kind::Jack, "RSD", "in:reseed", 7.0f, 98.0f);
        p.add(Widget::Kind::Jack, "RTM", "in:rate_mod", 18.0f, 98.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 29.0f, 98.0f);
        p.add(Widget::Kind::Knob, "MODO", "model", 13.0f, 72.0f);
        p.add(Widget::Kind::Jack, "Y", "out:y", 7.0f, 114.0f);
        p.add(Widget::Kind::Jack, "Z", "out:z", 18.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        resetField();
        rngState_ = 0x9E3779B97F4A7C15ULL;
        prevReseed_ = 0.0f;
        kickCounter_ = 0;
        lastModel_ = -1;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float rateBase = parameterValue("rate");
        const float drive = parameterValue("drive");
        const float damping = parameterValue("damping");
        const bool freeze = parameterValue("freeze") >= 0.5f;

        const AudioBlock* reseedIn = inputs[0];
        const AudioBlock* rateModIn = inputs[1];
        AudioBlock* yOut = outputs.size() > 1 ? &outputs[1] : nullptr;
        AudioBlock* zOut = outputs.size() > 2 ? &outputs[2] : nullptr;
        const int model = static_cast<int>(std::lround(
            clampf(parameterValue("model"), 0.0f, 2.0f)));
        if (model != lastModel_) { resetAttractor(model); lastModel_ = model; }

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (reseedIn != nullptr) {
                const float r = reseedIn->at(0, frame);
                if (prevReseed_ < 0.5f && r >= 0.5f) reseedField();
                prevReseed_ = r;
            }

            if (!freeze && model != 0) {
                const float rate = clampf(
                    rateBase + (rateModIn ? rateModIn->at(0, frame) : 0.0f),
                    0.02f, 400.0f);
                stepAttractor(model, rate, drive, damping);
            }
            if (model != 0) {
                float nx, ny, nz;
                normalized(model, nx, ny, nz);
                for (std::size_t c = 0; c < channels; ++c) {
                    out.at(c, frame) = nx;
                    if (yOut) yOut->at(c, frame) = ny;
                    if (zOut) zOut->at(c, frame) = nz;
                }
                continue;
            }
            if (!freeze) {
                const float rate = clampf(
                    rateBase + (rateModIn ? rateModIn->at(0, frame) : 0.0f),
                    0.02f, 400.0f);
                const float dt = clampf(rate / sampleRate_, 0.0f, 0.02f);
                const float pull = drive * 2.4f;
                const float xNext = x_ + dt * y_ * 6.0f;
                float yNext = y_ + dt
                    * (pull * (x_ - x_ * x_ * x_) - damping * 3.0f * y_);
                // chute periódico -- sem ele o sistema nunca cruza pro
                // outro poço (ver o comentário do topo)
                const std::uint32_t samplesPerCycle = static_cast<std::uint32_t>(
                    std::max(1.0f, sampleRate_ / std::max(rate, 0.02f)));
                if (++kickCounter_ >= samplesPerCycle) {
                    kickCounter_ = 0;
                    yNext += whiteNoise() * drive * 2.5f;
                }
                x_ = clampf(xNext, -2.2f, 2.2f);
                y_ = clampf(yNext, -3.5f, 3.5f);
                if (!std::isfinite(x_) || !std::isfinite(y_)) resetField();
            }

            const float v = clampf(x_, -1.0f, 1.0f);
            const float vy = clampf(y_ / 3.5f, -1.0f, 1.0f);
            for (std::size_t c = 0; c < channels; ++c) {
                out.at(c, frame) = v;
                if (yOut) yOut->at(c, frame) = vy;
                if (zOut) zOut->at(c, frame) = 0.0f;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    // xorshift64* -> [-1,1)
    float whiteNoise() noexcept {
        rngState_ ^= rngState_ >> 12; rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }
    // x=0 é o equilíbrio INSTÁVEL do sistema (o topo entre os dois
    // poços) -- começar exatamente ali deixaria tudo parado até um
    // chute empurrar; 0,15 já entra inclinado pra um lado.
    void resetField() noexcept { x_ = 0.15f; y_ = 0.0f; kickCounter_ = 0; }
    void reseedField() noexcept {
        x_ = clampf(whiteNoise() * 1.6f, -2.2f, 2.2f);
        y_ = clampf(whiteNoise() * 3.5f, -3.5f, 3.5f);
        kickCounter_ = 0;
    }

    // ---- atratores de 3 dimensões (RÖSSLER, AIZAWA) ------------------
    void resetAttractor(const int model) noexcept {
        if (model == 1) { ax_ = 1.0; ay_ = 1.0; az_ = 0.0; }
        else { ax_ = 0.1; ay_ = 0.0; az_ = 0.0; }
    }
    void deriv(const int model, const double x, const double y, const double z,
               const double drive, const double damp,
               double& dx, double& dy, double& dz) const noexcept {
        if (model == 1) {
            const double a = 0.1 + 0.2 * damp, b = 0.2, c = 3.0 + 5.0 * drive;
            dx = -y - z; dy = x + a * y; dz = b + z * (x - c);
        } else {
            const double a = 0.95, b = 0.7, c = 0.6, d = 3.5, e = 0.1 + 0.4 * drive, f = 0.1;
            const double damping = 1.0 + 0.5 * (damp - 0.3);
            dx = (z - b) * x - d * y;
            dy = d * x + (z - b) * y;
            dz = c + a * z - z * z * z / 3.0 - (x * x + y * y) * (1.0 + e * z) + f * z * x * x * x;
            dx *= damping; dy *= damping;
        }
    }
    void stepAttractor(const int model, const float rate, const float drive,
                       const float damp) noexcept {
        // ≈ uma volta por segundo em RATE 1 (Rössler: período ≈ 6; Aizawa ≈ 3)
        const double scale = model == 1 ? 6.0 : 3.0;
        const double h = std::min(0.05, static_cast<double>(rate) * scale / sampleRate_ / 4.0);
        for (int s = 0; s < 4; ++s) {
            double k1x, k1y, k1z, k2x, k2y, k2z;
            deriv(model, ax_, ay_, az_, drive, damp, k1x, k1y, k1z);
            deriv(model, ax_ + 0.5 * h * k1x, ay_ + 0.5 * h * k1y, az_ + 0.5 * h * k1z,
                  drive, damp, k2x, k2y, k2z);
            ax_ += h * k2x; ay_ += h * k2y; az_ += h * k2z;
        }
        if (!std::isfinite(ax_) || !std::isfinite(ay_) || !std::isfinite(az_)
            || std::fabs(ax_) > 100.0 || std::fabs(ay_) > 100.0 || std::fabs(az_) > 100.0)
            resetAttractor(model);
    }
    void normalized(const int model, float& nx, float& ny, float& nz) const noexcept {
        if (model == 1) {
            nx = clampf(static_cast<float>(ax_ / 12.0), -1.0f, 1.0f);
            ny = clampf(static_cast<float>(ay_ / 12.0), -1.0f, 1.0f);
            nz = clampf(static_cast<float>(az_ / 12.0 - 1.0), -1.0f, 1.0f);
        } else {
            nx = clampf(static_cast<float>(ax_ / 1.6), -1.0f, 1.0f);
            ny = clampf(static_cast<float>(ay_ / 1.6), -1.0f, 1.0f);
            nz = clampf(static_cast<float>(az_ - 0.6), -1.0f, 1.0f);
        }
    }
    double ax_ = 1.0, ay_ = 1.0, az_ = 0.0;
    int lastModel_ = -1;

    float x_ = 0.15f, y_ = 0.0f;
    std::uint32_t kickCounter_ = 0;
    std::uint64_t rngState_ = 0x9E3779B97F4A7C15ULL;
    float prevReseed_ = 0.0f;
};

}  // namespace rasgo::modular
