#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/Oversampler.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>

// LADDER (#74) — o filtro do grave (2026-11-11; dossiê `dossies/74_ladder.md`).
//
// Escada de quatro polos (R. Moog, 1969) com a ressonância como
// realimentação da saída do 4º degrau. DIODO acopla os degraus entre si
// (0 = transistor, degraus isolados; 1 = diodo, cada degrau carrega o
// vizinho, como na escada da EMS/TB-303) — contínuo, não chave.
//
// Discretização sem atraso no laço (ZDF/TPT, V. Zavalishin, *The Art of VA
// Filter Design*): os quatro integradores trapezoidais formam um sistema
// tridiagonal 4×4 resolvido exatamente a cada amostra (Thomas). Saturação
// tanh no nó de soma da entrada, aplicada à solução linear instantânea do
// laço; o laço roda a 2× (`Oversampler2x`).
//
// Desvio RASGO:
//  (a) o pico fica AFINADO no CUT em qualquer DIODO (o acoplamento desloca a
//      frequência de fase −180°; o corte é corrigido pela razão calculada);
//  (b) RESO = 1 é o limiar de auto-oscilação de CADA acoplamento (k_crit
//      calculado da resposta analógica), não um número fixo;
//  (c) COMP, compensação do grave exata: a entrada × (1 + COMP·k) — em
//      COMP = 1 o ganho em DC é 1 com qualquer RESO.
//
// Prior art lido (não portado): `MoogFilter.hpp` do Rasgo Synth Studio
// (Stilson–Smith, ganho de compensação de ouvido). Compensação como knob:
// conceito visto no Bidoo lIMbO e kocmoc DIOD; nenhum código lido. Código
// próprio, determinístico, sem alocação no áudio.
namespace rasgo::modular {

class Ladder final : public Signal {
public:
    Ladder() : Signal(
        {{"in", PortKind::Audio, ""},
         {"cutoff_mod", PortKind::Control, "v/oct"},
         {"res_mod", PortKind::Control, ""}},
        {{"lp4", PortKind::Audio, ""},
         {"lp2", PortKind::Audio, ""}},
        {{"cutoff", 20.0f, 20000.0f, 400.0f, "Hz"},
         {"resonance", 0.0f, 1.0f, 0.3f, ""},
         {"drive", 0.0f, 1.0f, 0.2f, ""},
         {"comp", 0.0f, 1.0f, 0.5f, ""},
         {"diode", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "LADDER"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "LADDER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "ladder", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "CUT", "cutoff", 8.0f, 28.0f, 12.0f);
        p.add(Widget::Kind::Knob, "RESO", "resonance", 31.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 5.0f, 52.0f);
        p.add(Widget::Kind::Knob, "COMP", "comp", 20.5f, 52.0f);
        p.add(Widget::Kind::Knob, "DIODO", "diode", 36.0f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "FC", "in:cutoff_mod", 20.0f, 92.0f);
        p.add(Widget::Kind::Jack, "Q", "in:res_mod", 31.0f, 92.0f);
        p.add(Widget::Kind::Jack, "LP4", "out:lp4", 14.0f, 114.0f);
        p.add(Widget::Kind::Jack, "LP2", "out:lp2", 36.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        for (auto& s : st_) s = State{};
        os_.reset();
        os2_.reset();
        smoothCutoff_ = parameterValue("cutoff");
        smoothRes_ = parameterValue("resonance");
        smoothDrive_ = parameterValue("drive");
        smoothDiode_ = parameterValue("diode");
        smoothComp_ = parameterValue("comp");
        paramCoeff_ = std::exp(-1.0f / (0.005f * sr_));
    }

    // Da resposta ANALÓGICA, para o acoplamento c ∈ [0, 1]: o ganho de laço
    // crítico (k de auto-oscilação) e a razão Ω*/ω entre a frequência de
    // fase −180° e o corte. c = 0 → (4, 1), a escada clássica.
    struct Critical { float k; float ratio; };
    static Critical critical(const float c) noexcept {
        const auto& t = table();
        const float pos = std::clamp(c, 0.0f, 1.0f) * static_cast<float>(kTable - 1);
        const int i = std::min(static_cast<int>(pos), kTable - 2);
        const float a = pos - static_cast<float>(i);
        return {t[static_cast<std::size_t>(i)].k * (1.0f - a) + t[static_cast<std::size_t>(i + 1)].k * a,
                t[static_cast<std::size_t>(i)].ratio * (1.0f - a) + t[static_cast<std::size_t>(i + 1)].ratio * a};
    }

    // Latência do 2× (para quem precisa alinhar).
    static constexpr float latencySamples() noexcept { return Oversampler2x::latencySamples(); }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* fcMod = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* qMod = inputs.size() > 2 ? inputs[2] : nullptr;
        AudioBlock& lp4 = outputs[0];
        AudioBlock& lp2 = outputs[1];
        const std::size_t frames = lp4.frames();
        const std::size_t channels = std::min(lp4.channels(), AudioBlock::maxChannels);

        const float cutP = safe(parameterValue("cutoff"), 400.0f);
        const float resP = safe(parameterValue("resonance"), 0.3f);
        const float driveP = std::clamp(safe(parameterValue("drive"), 0.2f), 0.0f, 1.0f);
        const float compP = std::clamp(safe(parameterValue("comp"), 0.5f), 0.0f, 1.0f);
        const float diodeP = std::clamp(safe(parameterValue("diode"), 0.0f), 0.0f, 1.0f);
        const float osRate = 2.0f * sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            const float pc = paramCoeff_;
            smoothCutoff_ = cutP + pc * (smoothCutoff_ - cutP);
            smoothRes_ = resP + pc * (smoothRes_ - resP);
            smoothDrive_ = driveP + pc * (smoothDrive_ - driveP);
            smoothComp_ = compP + pc * (smoothComp_ - compP);
            smoothDiode_ = diodeP + pc * (smoothDiode_ - diodeP);

            float cutoff = smoothCutoff_;
            if (fcMod) cutoff *= std::exp2(std::clamp(safe(fcMod->at(0, f), 0.0f), -10.0f, 10.0f));
            float res = smoothRes_ + (qMod ? safe(qMod->at(0, f), 0.0f) : 0.0f);
            res = std::clamp(res, 0.0f, 1.0f);

            const float c = smoothDiode_;
            const Critical cr = critical(c);
            // o corte do modelo, para que o pico caia no CUT; teto bem
            // abaixo do Nyquist do 2×
            const float fModel = std::clamp(cutoff / cr.ratio, 10.0f, 0.23f * osRate);
            const float g = std::tan(3.14159265f * fModel / osRate);
            // RESO: até ~0,93 abaixo do limiar; acima, o laço passa do
            // crítico (1,25 × em RESO 1) e o tanh prende a oscilação
            const float r6 = res * res * res * res * res * res;
            const float k = (res + 0.25f * r6 * r6) * cr.k;
            // reserva do tanh: cresce com a RESO até ser proporcional ao
            // laço — a auto-oscilação (≈ L/k) sai no mesmo nível em qualquer
            // DIODO; em RESO baixa é 1 para todos (o DRIVE soa igual)
            const float headroom = 1.0f + (cr.k * 0.25f - 1.0f) * res * res;
            const float drive = 1.0f + 7.0f * smoothDrive_;
            const float inGain = drive * (1.0f + smoothComp_ * k);
            // nível: o ganho efetivo do saturador num sinal nominal (pico
            // 0,5) com reserva L, relativo ao de reserva 1 — o DRIVE soa
            // parecido com qualquer reserva
            // (normalizado para DRIVE 0 dar ganho 1 exato em sinal pequeno)
            const auto satGain = [](const float d, const float L) noexcept {
                return L * std::tanh(d * 0.5f / L) / 0.5f;
            };
            const float outGain = (satGain(drive, 1.0f) / satGain(drive, headroom))
                                / (satGain(1.0f, 1.0f) / satGain(1.0f, headroom))
                                / std::sqrt(drive);

            // eliminação de Thomas da matriz M = I − g·A (fixa nesta amostra;
            // as duas sub-amostras do 2× a reusam)
            Coeffs co;
            co.prepare(g, c);

            for (std::size_t ch = 0; ch < channels; ++ch) {
                float x = in ? safe(in->at(ch, f), 0.0f) : 0.0f;
                x = std::clamp(x, -8.0f, 8.0f) * inGain;
                State& s = st_[ch];
                std::array<float, 2> tap2{};
                int ti = 0;
                const float y4 = os_.process(ch, x, [&](const float xs) noexcept {
                    float y2 = 0.0f;
                    const float out = s.step(co, xs, k, headroom, y2);
                    tap2[static_cast<std::size_t>(ti++ & 1)] = y2;
                    return out;
                });
                // a tomada LP2 passa pelo MESMO decimador (outra instância):
                // devolve as duas sub-amostras gravadas, na ordem
                int ri = 0;
                const float y2 = os2_.process(ch, 0.0f, [&](float) noexcept {
                    return tap2[static_cast<std::size_t>(ri++ & 1)];
                });
                lp4.at(ch, f) = softLimit(y4 * outGain);
                lp2.at(ch, f) = softLimit(y2 * outGain);
            }
        }
    }

private:
    static constexpr int kTable = 33;

    static float safe(const float v, const float fallback) noexcept {
        return std::isfinite(v) ? v : fallback;
    }
    static float softLimit(const float v) noexcept {
        if (v > 0.9f) return 0.9f + 0.1f * std::tanh((v - 0.9f) * 10.0f);
        if (v < -0.9f) return -0.9f + 0.1f * std::tanh((v + 0.9f) * 10.0f);
        return v;
    }

    // M = I − g·A, A tridiagonal (diagonal −(1+c) nos três primeiros
    // degraus e −1 no último; sub-diagonal 1; super-diagonal c).
    struct Coeffs {
        // fatores da eliminação: cp (super-diagonal normalizada), inv (1/pivô)
        std::array<double, 4> cp{}, inv{}, sub{};
        std::array<double, 4> q{};   // resposta do sistema a u = 1 (coluna g·b)
        double g = 0.0;

        void prepare(const double gg, const double c) noexcept {
            g = gg;
            const double d0 = 1.0 + g * (1.0 + c);
            const double d3 = 1.0 + g;
            const double up = -g * c;
            const double lo = -g;
            std::array<double, 4> diag{d0, d0, d0, d3};
            // forward
            inv[0] = 1.0 / diag[0];
            cp[0] = up * inv[0];
            for (int i = 1; i < 4; ++i) {
                const double piv = diag[static_cast<std::size_t>(i)] - lo * cp[static_cast<std::size_t>(i - 1)];
                inv[static_cast<std::size_t>(i)] = 1.0 / piv;
                cp[static_cast<std::size_t>(i)] = i < 3 ? up * inv[static_cast<std::size_t>(i)] : 0.0;
            }
            for (auto& v : sub) v = lo;
            q = solve({g, 0.0, 0.0, 0.0});
        }

        std::array<double, 4> solve(const std::array<double, 4>& r) const noexcept {
            std::array<double, 4> dp{};
            dp[0] = r[0] * inv[0];
            for (int i = 1; i < 4; ++i)
                dp[static_cast<std::size_t>(i)] =
                    (r[static_cast<std::size_t>(i)] - sub[static_cast<std::size_t>(i)] * dp[static_cast<std::size_t>(i - 1)])
                    * inv[static_cast<std::size_t>(i)];
            std::array<double, 4> y{};
            y[3] = dp[3];
            for (int i = 2; i >= 0; --i)
                y[static_cast<std::size_t>(i)] =
                    dp[static_cast<std::size_t>(i)] - cp[static_cast<std::size_t>(i)] * y[static_cast<std::size_t>(i + 1)];
            return y;
        }
    };

    struct State {
        std::array<double, 4> s{};   // estados dos integradores TPT

        float step(const Coeffs& co, const float x, const float k, const float headroom,
                   float& y2out) noexcept {
            // y = P + Q·u, P = M⁻¹ s
            const std::array<double, 4> p = co.solve(s);
            const double kd = static_cast<double>(k);
            const double uLin = (static_cast<double>(x) - kd * p[3]) / (1.0 + kd * co.q[3]);
            const double hl = static_cast<double>(headroom);
            const double u = hl * std::tanh(uLin / hl);
            std::array<double, 4> y{};
            for (std::size_t i = 0; i < 4; ++i) {
                y[i] = p[i] + co.q[i] * u;
                s[i] = 2.0 * y[i] - s[i];
                if (!std::isfinite(s[i])) s = {};
            }
            y2out = static_cast<float>(y[1]);
            return static_cast<float>(y[3]);
        }
    };

    static const std::array<Critical, kTable>& table() noexcept {
        static const std::array<Critical, kTable> t = [] {
            std::array<Critical, kTable> r{};
            for (int i = 0; i < kTable; ++i)
                r[static_cast<std::size_t>(i)] = computeCritical(static_cast<double>(i) / (kTable - 1));
            return r;
        }();
        return t;
    }

    // H₄(jΩ) do modelo analógico com ω = 1, entrada u
    static std::complex<double> h4(const double w, const double c) noexcept {
        using cd = std::complex<double>;
        const cd s(0.0, w);
        // (sI − A) y = b; diagonal s+1+c (s+1 no último), sub −1, super −c
        const cd d0 = s + 1.0 + c, d3 = s + 1.0;
        const cd up(-c, 0.0), lo(-1.0, 0.0);
        std::array<cd, 4> diag{d0, d0, d0, d3}, cp{}, dp{};
        std::array<cd, 4> rhs{cd(1.0, 0.0), cd(0.0, 0.0), cd(0.0, 0.0), cd(0.0, 0.0)};
        cp[0] = up / diag[0];
        dp[0] = rhs[0] / diag[0];
        for (std::size_t i = 1; i < 4; ++i) {
            const cd piv = diag[i] - lo * cp[i - 1];
            cp[i] = i < 3 ? up / piv : cd(0.0, 0.0);
            dp[i] = (rhs[i] - lo * dp[i - 1]) / piv;
        }
        std::array<cd, 4> y{};
        y[3] = dp[3];
        for (int i = 2; i >= 0; --i)
            y[static_cast<std::size_t>(i)] = dp[static_cast<std::size_t>(i)] - cp[static_cast<std::size_t>(i)] * y[static_cast<std::size_t>(i + 1)];
        return y[3];
    }

    static Critical computeCritical(const double c) noexcept {
        // procura a primeira passagem de Im(H₄) por 0 com Re < 0 (fase −180°)
        double prevW = 0.05;
        double prevIm = h4(prevW, c).imag();
        for (int n = 1; n <= 400; ++n) {
            const double w = 0.05 * std::pow(10.0, n * 3.0 / 400.0);   // 0,05 … 50
            const auto hv = h4(w, c);
            if (hv.real() < 0.0 && (hv.imag() >= 0.0) != (prevIm >= 0.0)) {
                double a = prevW, b = w;
                for (int it = 0; it < 60; ++it) {
                    const double m = 0.5 * (a + b);
                    if ((h4(m, c).imag() >= 0.0) == (prevIm >= 0.0)) a = m; else b = m;
                }
                const double ws = 0.5 * (a + b);
                const double re = h4(ws, c).real();
                return {static_cast<float>(-1.0 / re), static_cast<float>(ws)};
            }
            prevW = w;
            prevIm = hv.imag();
        }
        return {4.0f, 1.0f};
    }

    float sr_ = 48000.0f;
    std::array<State, AudioBlock::maxChannels> st_{};
    Oversampler2x os_, os2_;
    float smoothCutoff_ = 400.0f, smoothRes_ = 0.3f, smoothDrive_ = 0.2f;
    float smoothComp_ = 0.5f, smoothDiode_ = 0.0f;
    float paramCoeff_ = 0.0f;
};

}  // namespace rasgo::modular
