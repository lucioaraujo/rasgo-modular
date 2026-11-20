#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// REFRACT (#93) — um raio de luz atravessando camadas (ideia óptica,
// 9/11/2026, aprovada pelo autor: "aprovo todos"; dossiê
// `dossies/93_refract.md`).
//
// A lei de Snell–Descartes (Ibn Sahl, 984; Snell, 1621; Descartes, 1637):
// ao passar de um meio de índice n₁ para outro de índice n₂, o raio desvia,
// n₁·sen θ₁ = n₂·sen θ₂. Quatro camadas empilhadas, cada uma com o seu
// índice (N1–N4, de 1,0 — o ar — a 2,4 — o diamante). A luz nasce dentro
// da primeira camada e sai, no fim, para o ar. O ângulo de partida varre
// sozinho (RATE, RANGE) ou vem do CV. Em cada camada sai o ângulo do raio,
// como CV. Quando o raio passa de um meio mais denso para um menos
// denso num ângulo raso demais, não atravessa: REFLEXÃO TOTAL (a fibra
// óptica, o brilho do diamante) — e sai um pulso em TIR. EXIT é o desvio
// lateral acumulado do raio ao sair (quanto ele "andou de lado").
//
// Território próprio do Rasgo: o Cardinal não tem módulos ópticos.
//
// Desvio Rasgo:
//  (a) SHIM — miragem: os índices tremem um pouco, como o ar quente sobre o
//      asfalto; perto do ângulo crítico, o tremor liga e desliga a reflexão
//      total, e o TIR vira um ritmo irregular;
//  (b) o CV soma em todos os índices de uma vez (o vidro esquenta).
//
// Controle (taxa de controle: recalcula a cada 32 amostras); process() não
// aloca; determinístico.
namespace rasgo::modular {

class Refract final : public Signal {
public:
    static constexpr int kLayers = 4;

    Refract() : Signal(
        {{"angle_cv", PortKind::Control, ""},
         {"index_cv", PortKind::Control, ""}},
        {{"a1", PortKind::Control, ""},
         {"a2", PortKind::Control, ""},
         {"a3", PortKind::Control, ""},
         {"a4", PortKind::Control, ""},
         {"tir", PortKind::Control, "gate"},
         {"exit", PortKind::Control, ""}},
        {{"n1", 1.0f, 2.4f, 1.5f, ""},     // vidro
         {"n2", 1.0f, 2.4f, 1.33f, ""},    // água
         {"n3", 1.0f, 2.4f, 2.0f, ""},
         {"n4", 1.0f, 2.4f, 1.2f, ""},
         {"rate", 0.01f, 5.0f, 0.15f, "Hz"},
         {"range", 0.0f, 89.0f, 75.0f, "°"},
         {"shimmer", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "REFRACT"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "REFRACT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "refract", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "N1", "n1", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "N2", "n2", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "N3", "n3", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "N4", "n4", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 36.0f, 50.0f);
        p.add(Widget::Kind::Knob, "SHIM", "shimmer", 5.0f, 70.0f);
        p.add(Widget::Kind::Jack, "ANG", "in:angle_cv", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "IDX", "in:index_cv", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "TIR", "out:tir", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "EXIT", "out:exit", 44.0f, 96.0f);
        p.add(Widget::Kind::Jack, "A1", "out:a1", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "A2", "out:a2", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "A3", "out:a3", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "A4", "out:a4", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        phase_ = 0.0;
        counter_ = 0;
        rng_ = 0x5E11ACDCULL;
        for (auto& s : shim_) s = 0.0f;
        angles_.fill(0.0f);
        reached_ = kLayers;
        exit_ = 0.0f;
        tirNow_ = tirPrev_ = false;
        gateLeft_ = 0;
    }

    // Para os testes e o mostrador.
    float angle(int layer) const noexcept { return angles_[static_cast<std::size_t>(std::clamp(layer, 0, kLayers - 1))]; }
    int reached() const noexcept { return reached_; }   // quantas das 4 passagens o raio fez (4 = saiu ao ar)
    float incidence() const noexcept { return incidence_; }

    // O caminho do raio. A luz nasce DENTRO da primeira camada (como a de uma
    // lanterna no fundo da piscina), com o ângulo θ₁ (graus, 0 =
    // perpendicular), atravessa as camadas 2–4 e sai para o ar (n = 1). Vindo
    // do ar não haveria reflexão total: ela só acontece de um meio mais denso
    // para um menos denso. Devolve os ângulos em cada camada e quantas das 4
    // passagens (2, 3, 4 e a saída ao ar) o raio fez.
    static int trace(float theta1Deg, const std::array<float, kLayers>& n,
                     std::array<float, kLayers>& out, float& lateral) noexcept {
        const float th1 = std::clamp(theta1Deg, -89.0f, 89.0f) * 0.017453293f;
        const float s = n[0] * std::sin(th1);   // o invariante de Snell: n·sen θ
        out.fill(0.0f);
        out[0] = theta1Deg;
        lateral = std::tan(th1);
        for (int i = 1; i <= kLayers; ++i) {
            const float ni = i < kLayers ? n[static_cast<std::size_t>(i)] : 1.0f;   // a 5ª é o ar
            const float si = s / ni;
            if (std::fabs(si) >= 1.0f) {
                // reflexão total: daqui em diante o raio não passa (rasante)
                for (int k = i; k < kLayers; ++k) out[static_cast<std::size_t>(k)] = si > 0 ? 90.0f : -90.0f;
                return i - 1;
            }
            if (i < kLayers) {
                const float th = std::asin(si);
                out[static_cast<std::size_t>(i)] = th * 57.29578f;
                lateral += std::tan(th);   // camadas de espessura 1
            }
        }
        return kLayers;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* acv = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* icv = inputs.size() > 1 ? inputs[1] : nullptr;

        std::array<float, kLayers> nk{};
        nk[0] = std::clamp(safe(parameterValue("n1"), 1.5f), 1.0f, 2.4f);
        nk[1] = std::clamp(safe(parameterValue("n2"), 1.33f), 1.0f, 2.4f);
        nk[2] = std::clamp(safe(parameterValue("n3"), 2.0f), 1.0f, 2.4f);
        nk[3] = std::clamp(safe(parameterValue("n4"), 1.2f), 1.0f, 2.4f);
        const float rate = std::clamp(safe(parameterValue("rate"), 0.15f), 0.01f, 5.0f);
        const float range = std::clamp(safe(parameterValue("range"), 75.0f), 0.0f, 89.0f);
        const float shimmer = std::clamp(safe(parameterValue("shimmer"), 0.0f), 0.0f, 1.0f);
        const int gateLen = std::max(1, static_cast<int>(0.02f * sr_));
        constexpr int kStep = 32;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            phase_ += static_cast<double>(rate) / sr_;
            if (phase_ >= 1.0) phase_ -= 1.0;
            if (++counter_ >= kStep) {
                counter_ = 0;
                // o ângulo de entrada: a varredura senoidal, ou o CV (−1..1 → ±RANGE)
                float a = static_cast<float>(std::sin(6.283185307179586 * phase_));
                if (acv) a = std::clamp(safe(acv->at(0, f), 0.0f), -1.0f, 1.0f);
                incidence_ = a * range;
                const float shift = icv ? 0.7f * std::clamp(safe(icv->at(0, f), 0.0f), -1.0f, 1.0f) : 0.0f;
                std::array<float, kLayers> n{};
                for (int i = 0; i < kLayers; ++i) {
                    // a miragem: um passeio lento e limitado em cada índice
                    auto& sh = shim_[static_cast<std::size_t>(i)];
                    sh = 0.995f * sh + 0.06f * (unit() - 0.5f);
                    n[static_cast<std::size_t>(i)] = std::clamp(nk[static_cast<std::size_t>(i)] + shift
                                                         + shimmer * 0.25f * sh, 1.0f, 2.6f);
                }
                float lateral = 0.0f;
                reached_ = trace(incidence_, n, angles_, lateral);
                exit_ = std::clamp(lateral / 8.0f, -1.0f, 1.0f);
                tirNow_ = reached_ < kLayers;
                if (tirNow_ && !tirPrev_) gateLeft_ = gateLen;
                tirPrev_ = tirNow_;
            }
            const float gate = gateLeft_ > 0 ? 1.0f : 0.0f;
            if (gateLeft_ > 0) --gateLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                for (int i = 0; i < kLayers; ++i)
                    outputs[static_cast<std::size_t>(i)].at(c, f) = angles_[static_cast<std::size_t>(i)] / 90.0f;
                outputs[4].at(c, f) = gate;
                outputs[5].at(c, f) = exit_;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    double phase_ = 0.0;
    int counter_ = 0, reached_ = kLayers, gateLeft_ = 0;
    std::uint64_t rng_ = 0x5E11ACDCULL;
    std::array<float, kLayers> shim_{}, angles_{};
    float exit_ = 0.0f, incidence_ = 0.0f;
    bool tirNow_ = false, tirPrev_ = false;
};

} // namespace rasgo::modular
