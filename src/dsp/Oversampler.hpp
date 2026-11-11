#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

// ============================================================================
// Oversampler2x — 2× para uma etapa não-linear por amostra
// ============================================================================
//
// Qualquer não-linearidade (waveshaper, ceifador, filtro com saturação no
// laço) gera energia harmônica que a entrada não tinha; parte dela cai
// acima de Nyquist se computada no rate base e volta dobrada como alias —
// o mesmo problema de um oscilador serra/pulso sem band-limiting.
//
// Estratégia ASSIMÉTRICA (a barata que funciona pra waveshaper):
//   - upsample por INTERPOLAÇÃO LINEAR — a entrada de áudio já é ~limitada
//     em banda, suas imagens perto de fs são fracas, então um kernel
//     triangular basta;
//   - downsample por FIR MEIA-BANDA de 13 taps (janela de Blackman sobre
//     sinc) — este é o filtro que remove o que a não-linearidade criou
//     entre fs/2 e fs, antes de decimar.
//
// Fase linear; atraso de grupo ~2,5 amostras no rate base. Autocontido por
// canal (guarda o próprio histórico) — o laço de quem chama não muda de
// forma: basta trocar a chamada direta pela `process()` aqui. Sem
// alocação, sem RNG: determinístico.
//
// Prior art consultado (não copiado): `RASGO_SYNTH/.../DiodeShaper.hpp`
// faz um 2× "leve" com upsample linear + decimação por MÉDIA; aqui a
// decimação por meia-banda real dá ~40 dB a mais de rejeição pelo mesmo
// custo de upsample.

namespace rasgo::modular {

class Oversampler2x {
public:
    void reset() noexcept {
        for (auto& c : chans_) c = Chan{};
    }

    // Roda `fn` (assinatura `float(float)`, PODE ter estado próprio — ex.:
    // um filtro) duas vezes por amostra de entrada, no rate dobrado, e
    // devolve uma amostra decimada. `fn` é chamada em ordem temporal: o
    // ponto interpolado (t = n − ½) primeiro, a amostra atual (t = n)
    // depois.
    template <typename Fn>
    float process(const std::size_t channel, const float x, Fn&& fn) noexcept {
        Chan& c = chans_[channel];
        const float mid = 0.5f * (c.prevIn + x);
        c.prevIn = x;
        const float s0 = fn(mid);
        const float s1 = fn(x);
        c.ring[c.w & kMask] = s0;
        ++c.w;
        c.ring[c.w & kMask] = s1;
        ++c.w;
        const auto tap = [&](const std::size_t back) -> float {
            return c.ring[(c.w - 1u - back) & kMask];
        };
        // FIR meia-banda centrado em tap(5): taps pares (±2, ±4) são zero.
        return kH0 * tap(5)
             + kH1 * (tap(4) + tap(6))
             + kH3 * (tap(2) + tap(8))
             + kH5 * (tap(0) + tap(10));
    }

    // Atraso de grupo introduzido, em amostras do rate base.
    static constexpr float latencySamples() noexcept { return 2.5f; }

private:
    // meia-banda janelada (Blackman, 15 pontos; n = ±7 zera na janela):
    // n = 0 : 0,5 ; n = ±1 : 0,292954 ; n = ±3 : −0,048720 ; n = ±5 :
    // 0,005759. Ganho DC = 0,999986 (perda inaudível).
    static constexpr float kH0 = 0.5f;
    static constexpr float kH1 = 0.292954f;
    static constexpr float kH3 = -0.048720f;
    static constexpr float kH5 = 0.005759f;
    static constexpr std::size_t kMask = 15u;

    struct Chan {
        float prevIn = 0.0f;
        std::array<float, 16> ring{};
        std::size_t w = 0u;
    };
    std::array<Chan, AudioBlock::maxChannels> chans_{};
};

// ============================================================================
// HalfbandDecimator — 2:1 para GERADORES sobreamostrados (11/11/2026)
// ============================================================================
//
// O `Oversampler2x` acima serve a processadores (uma função aplicada a uma
// entrada). Um gerador sobreamostrado — o OPERATOR rodando a 4× — produz
// ele mesmo as amostras no rate alto; só precisa DECIMAR. Mesmo FIR
// meia-banda (mesmos coeficientes); dois em cascata dão 4:1. Sem alocação.
class HalfbandDecimator {
public:
    // 63 coeficientes (sinc meia-banda × janela de Kaiser β = 8): rejeição
    // da ordem de 80 dB acima de ~0,28·fs — um GERADOR põe energia até o
    // topo do rate alto, e o 13 taps de cima (feito para waveshaper, cuja
    // entrada já é limitada em banda) deixava ela dobrar para dentro do
    // áudio. Coeficientes calculados uma vez, fora do áudio.
    static constexpr std::size_t kTaps = 63;
    HalfbandDecimator() noexcept { reset(); }
    void reset() noexcept { ring_.fill(0.0f); w_ = 0u; }
    float push(const float s0, const float s1) noexcept {
        ring_[w_ & kMask] = s0;
        ++w_;
        ring_[w_ & kMask] = s1;
        ++w_;
        const auto& h = coefs();
        float acc = 0.0f;
        // centro em (kTaps−1)/2 = 31; meia-banda: só ímpares do centro + centro
        const std::size_t newest = w_ - 1u;
        acc += h[31] * ring_[(newest - 31u) & kMask];
        for (std::size_t k = 1; k <= 31; k += 2)
            acc += h[31 - k] * (ring_[(newest - 31u + k) & kMask]
                                + ring_[(newest - 31u - k) & kMask]);
        return acc;
    }
    // atraso de grupo, em amostras do rate ALTO
    static constexpr float latencyHighRate() noexcept { return 31.0f; }

private:
    static constexpr std::size_t kMask = 63u;
    static double besselI0(const double x) noexcept {
        double sum = 1.0, term = 1.0;
        for (int k = 1; k < 40; ++k) {
            term *= (x / (2.0 * k)) * (x / (2.0 * k));
            sum += term;
        }
        return sum;
    }
    static const std::array<float, kTaps>& coefs() noexcept {
        static const std::array<float, kTaps> h = [] {
            std::array<float, kTaps> c{};
            const double beta = 8.0, m = 31.0;
            double dc = 0.0;
            for (std::size_t i = 0; i < kTaps; ++i) {
                const double n = static_cast<double>(i) - m;
                const double sinc = n == 0.0 ? 0.5
                    : std::sin(3.14159265358979 * 0.5 * n) / (3.14159265358979 * n);
                const double r = n / m;
                const double win = besselI0(beta * std::sqrt(std::max(0.0, 1.0 - r * r)))
                                 / besselI0(beta);
                c[i] = static_cast<float>(sinc * win);
                dc += sinc * win;
            }
            for (auto& v : c) v = static_cast<float>(v / dc);   // ganho DC 1
            return c;
        }();
        return h;
    }
    std::array<float, 64> ring_{};
    std::size_t w_ = 0u;
};

}  // namespace rasgo::modular
