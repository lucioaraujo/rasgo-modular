#pragma once

#include "core/SignalGraph.hpp"

#include <array>
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

}  // namespace rasgo::modular
