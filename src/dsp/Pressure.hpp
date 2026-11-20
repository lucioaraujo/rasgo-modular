#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// PRESSURE (#91) — gás numa caixa (ideia do autor, 9/11/2026: "pressão /
// gás"; lote 2 das ideias físicas, aprovado: "aprovo todos"; dossiê
// `dossies/91_pressure.md`).
//
// Teoria cinética dos gases (Maxwell, 1860; Boltzmann): N partículas numa
// caixa 2D, cada uma em linha reta até bater numa parede e refletir. A
// PRESSÃO é o impulso entregue às paredes por unidade de tempo — cresce com
// a temperatura e com a compressão (P·V ∝ N·T). Cada choque na parede direita
// é um GATE: com poucas partículas, um ritmo esparso e irregular; com muitas,
// um chiado de eventos. O PISTÃO (a parede direita) se move por VOL, e
// comprimir AQUECE o gás (a parede que avança devolve a partícula mais
// rápida), como na compressão adiabática.
//
// Conceito lido no Sapphire Nucleus (GPL; partículas que se atraem e se
// repelem); nenhum código copiado.
//
// Desvio Rasgo:
//  (a) ATTRACT — as partículas se atraem (gás real, de van der Waals) ou se
//      repelem: com atração forte, condensam numa gota e a pressão cai;
//  (b) HEAT — um gatilho que dá energia a todas de uma vez (um estalo).
//
// Simulação a cada 16 amostras (passo fixo); process() não aloca;
// determinístico (semente própria).
namespace rasgo::modular {

class Pressure final : public Signal {
public:
    static constexpr int kMax = 24;

    Pressure() : Signal(
        {{"heat", PortKind::Control, "trig"},
         {"volume_cv", PortKind::Control, ""},
         {"temp_cv", PortKind::Control, ""}},
        {{"hit", PortKind::Control, "gate"},
         {"pressure", PortKind::Control, ""},
         {"temperature", PortKind::Control, ""},
         {"x", PortKind::Control, ""},
         {"y", PortKind::Control, ""}},
        {{"particles", 1.0f, 24.0f, 6.0f, ""},
         {"temp", 0.05f, 1.0f, 0.3f, ""},
         {"volume", 0.2f, 1.0f, 1.0f, ""},
         {"attract", -1.0f, 1.0f, 0.0f, ""},
         {"gate_len", 0.002f, 0.2f, 0.03f, "s"}}) {}

    std::string type() const override { return "PRESSURE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "PRESSURE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "pressure", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "N", "particles", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TEMP", "temp", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "VOL", "volume", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ATTR", "attract", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "HEAT", "in:heat", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "VOL", "in:volume_cv", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "TEMP", "in:temp_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "HIT", "out:hit", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PRES", "out:pressure", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "TEMP", "out:temperature", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "X", "out:x", 44.0f, 106.0f);
        p.add(Widget::Kind::Jack, "Y", "out:y", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x9E55C0DE11ULL;
        counter_ = 0;
        gateLeft_ = 0;
        prevHeat_ = 0.0f;
        impulse_ = pressure_ = 0.0f;
        wall_ = 1.0f;
        for (int i = 0; i < kMax; ++i) {
            x_[i] = 0.1f + 0.8f * unit();
            y_[i] = 0.1f + 0.8f * unit();
            const float a = 6.2831853f * unit();
            vx_[i] = std::cos(a);
            vy_[i] = std::sin(a);
        }
        normalizeTo(0.3f, 24);
    }

    // Para os testes e o mostrador.
    float wall() const noexcept { return wall_; }
    float px(int i) const noexcept { return x_[static_cast<std::size_t>(std::clamp(i, 0, kMax - 1))]; }
    float py(int i) const noexcept { return y_[static_cast<std::size_t>(std::clamp(i, 0, kMax - 1))]; }
    int count() const noexcept { return n_; }
    float kinetic() const noexcept { return meanKinetic(n_); }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const AudioBlock* heat = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* vcv = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* tcv = inputs.size() > 2 ? inputs[2] : nullptr;

        n_ = static_cast<int>(std::lround(std::clamp(safe(parameterValue("particles"), 6.0f), 1.0f, 24.0f)));
        const float tempK = std::clamp(safe(parameterValue("temp"), 0.3f), 0.05f, 1.0f);
        const float volK = std::clamp(safe(parameterValue("volume"), 1.0f), 0.2f, 1.0f);
        const float attract = std::clamp(safe(parameterValue("attract"), 0.0f), -1.0f, 1.0f);
        const int gateLen = std::max(1, static_cast<int>(
            std::clamp(safe(parameterValue("gate_len"), 0.03f), 0.002f, 0.2f) * sr_));
        constexpr int kStep = 16;
        const float dt = static_cast<float>(kStep) / sr_;
        // o gás corre 4× mais rápido que o relógio: na TEMP padrão, um
        // choque por segundo por partícula, mais ou menos
        const float dtSim = 4.0f * dt;
        // a pressão é uma média do impulso nas paredes (≈ 0,25 s)
        const float smooth = 1.0f - std::exp(-dt / 0.25f);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (heat) {
                const float h = safe(heat->at(0, f), 0.0f);
                if (prevHeat_ < 0.5f && h >= 0.5f)
                    for (int i = 0; i < n_; ++i) { vx_[i] *= 1.8f; vy_[i] *= 1.8f; }
                prevHeat_ = h;
            }
            if (++counter_ >= kStep) {
                counter_ = 0;
                const float temp = std::clamp(tempK + (tcv ? 0.5f * safe(tcv->at(0, f), 0.0f) : 0.0f), 0.02f, 1.5f);
                const float target = std::clamp(volK + (vcv ? 0.4f * safe(vcv->at(0, f), 0.0f) : 0.0f), 0.15f, 1.0f);
                // o pistão anda devagar até o alvo; a velocidade dele entra no choque
                const float prevWall = wall_;
                wall_ += (target - wall_) * std::min(1.0f, dt * 4.0f);
                const float wallSpeed = (wall_ - prevWall) / dtSim;
                bool hitNow = false;
                step(dtSim, attract, wallSpeed, hitNow);
                // o termostato: puxa a energia média para TEMP (sem apagar o caos)
                const float ke = meanKinetic(n_);
                const float want = temp * temp;
                if (ke > 1.0e-9f) {
                    const float k = std::sqrt(std::max(0.0f, 1.0f + 0.02f * (want / ke - 1.0f)));
                    for (int i = 0; i < n_; ++i) { vx_[i] *= k; vy_[i] *= k; }
                }
                // impulso por unidade de tempo e de comprimento de parede, normalizado
                const float perimeter = 2.0f * (wall_ + 1.0f);
                const float p = impulse_ / (dt * perimeter) / 8.0f;
                impulse_ = 0.0f;
                pressure_ += (p - pressure_) * smooth;
                if (hitNow) gateLeft_ = gateLen;
            }
            const float gate = gateLeft_ > 0 ? 1.0f : 0.0f;
            if (gateLeft_ > 0) --gateLeft_;
            // saturação suave: 0 = vácuo … perto de 1 = gás muito comprimido e quente
            const float pres = 1.0f - std::exp(-std::max(0.0f, pressure_));
            const float tOut = std::clamp(std::sqrt(meanKinetic(n_)), 0.0f, 1.0f);
            const float xo = x_[0] * 2.0f - 1.0f, yo = y_[0] * 2.0f - 1.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = gate;
                outputs[1].at(c, f) = pres;
                outputs[2].at(c, f) = tOut;
                outputs[3].at(c, f) = xo;
                outputs[4].at(c, f) = yo;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    float meanKinetic(int n) const noexcept {
        if (n <= 0) return 0.0f;
        float s = 0.0f;
        for (int i = 0; i < n; ++i) s += 0.5f * (vx_[i] * vx_[i] + vy_[i] * vy_[i]);
        return s / static_cast<float>(n);
    }
    void normalizeTo(float temp, int n) noexcept {
        const float ke = meanKinetic(n);
        if (ke <= 1.0e-9f) return;
        const float k = temp / std::sqrt(ke);
        for (int i = 0; i < n; ++i) { vx_[i] *= k; vy_[i] *= k; }
    }

    void step(float dt, float attract, float wallSpeed, bool& hitNow) noexcept {
        // forças entre pares (suaves, com raio de corte): atração ou repulsão
        if (std::fabs(attract) > 1.0e-4f && n_ > 1) {
            for (int i = 0; i < n_; ++i)
                for (int j = i + 1; j < n_; ++j) {
                    const float dx = x_[j] - x_[i], dy = y_[j] - y_[i];
                    const float d2 = dx * dx + dy * dy + 0.002f;
                    if (d2 > 0.16f) continue;
                    // núcleo repulsivo curto (as partículas não se fundem)
                    const float core = 0.0004f / (d2 * d2);
                    const float pull = attract * 0.6f / d2;
                    const float fmag = (pull - core) * dt;
                    const float inv = 1.0f / std::sqrt(d2);
                    const float fx = std::clamp(fmag * dx * inv, -0.5f, 0.5f);
                    const float fy = std::clamp(fmag * dy * inv, -0.5f, 0.5f);
                    vx_[i] += fx; vy_[i] += fy;
                    vx_[j] -= fx; vy_[j] -= fy;
                }
        }
        for (int i = 0; i < n_; ++i) {
            x_[i] += vx_[i] * dt;
            y_[i] += vy_[i] * dt;
            if (x_[i] < 0.0f) { x_[i] = -x_[i]; impulse_ += 2.0f * std::fabs(vx_[i]); vx_[i] = std::fabs(vx_[i]); }
            if (x_[i] > wall_) {
                // a parede que avança devolve a partícula mais rápida (aquece)
                const float rel = vx_[i] - wallSpeed;
                x_[i] = wall_ - (x_[i] - wall_);
                if (x_[i] < 0.0f) x_[i] = 0.5f * wall_;
                impulse_ += 2.0f * std::fabs(rel);
                vx_[i] = -std::fabs(rel) + wallSpeed;
                if (vx_[i] > 0.0f) vx_[i] = -vx_[i];
                hitNow = true;
            }
            if (y_[i] < 0.0f) { y_[i] = -y_[i]; impulse_ += 2.0f * std::fabs(vy_[i]); vy_[i] = std::fabs(vy_[i]); }
            if (y_[i] > 1.0f) { y_[i] = 2.0f - y_[i]; impulse_ += 2.0f * std::fabs(vy_[i]); vy_[i] = -std::fabs(vy_[i]); }
            x_[i] = std::clamp(x_[i], 0.0f, wall_);
            y_[i] = std::clamp(y_[i], 0.0f, 1.0f);
            // teto de velocidade (estabilidade)
            vx_[i] = std::clamp(vx_[i], -8.0f, 8.0f);
            vy_[i] = std::clamp(vy_[i], -8.0f, 8.0f);
        }
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x9E55C0DE11ULL;
    std::array<float, kMax> x_{}, y_{}, vx_{}, vy_{};
    int n_ = 6, counter_ = 0, gateLeft_ = 0;
    float prevHeat_ = 0.0f, impulse_ = 0.0f, pressure_ = 0.0f, wall_ = 1.0f;
};

} // namespace rasgo::modular
