#pragma once

// As LUZES das saídas (26/11/2026, frente de human polish; autor: "siga
// para as luzes"). Convenção do VCV: cada saída tem um LED que acende com
// o sinal — verde para o lado positivo, vermelho para o negativo (LED
// bipolar), o brilho pela amplitude. Um painel com luz parece vivo.
//
// Uma regra só para os front-ends: o motor junta, a cada bloco, o PICO COM
// SINAL de cada saída (o valor de maior módulo desde a última leitura, para
// um gate curto não sumir entre dois quadros); quem desenha lê, zera, e
// deixa a luz cair devagar (`kLightDecay` por quadro de ~30 Hz).
//
// Framework-free, como o resto de `src/ui/`.

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rasgo::ui {

// posição do LED em relação ao centro do jack (mm): em cima, à direita,
// fora do anel (raio 2,4 mm) e abaixo do rótulo
inline constexpr float kLightDX = 3.4f, kLightDY = -2.6f, kLightRadius = 0.8f;
inline constexpr std::uint32_t kLightPositive = 0x8fe39a;   // verde
inline constexpr std::uint32_t kLightNegative = 0xff6b5b;   // vermelho (o `warning`)
inline constexpr std::uint32_t kLightOff = 0x0e1015;        // o LED apagado (`recessed`)
inline constexpr float kLightDecay = 0.82f;

// o pico com sinal de um bloco, somado ao que já havia
inline float signedPeak(const float* x, const std::size_t n, float atual) {
    for (std::size_t i = 0; i < n; ++i)
        if (std::fabs(x[i]) > std::fabs(atual)) atual = x[i];
    return atual;
}

// o mesmo, de um bloco do grafo (canal 0; `frames()`/`at(c, i)`)
template <class Block>
inline float blockSignedPeak(const Block& b, float atual) {
    for (std::size_t i = 0; i < b.frames(); ++i) {
        const float v = b.at(0, i);
        if (std::fabs(v) > std::fabs(atual)) atual = v;
    }
    return atual;
}

// o que a luz mostra agora: a leitura nova ou a de antes caindo
inline float lightFollow(const float shown, const float novo) {
    const float caindo = shown * kLightDecay;
    return std::fabs(novo) >= std::fabs(caindo) ? novo : caindo;
}

// brilho 0..1 (raiz: um sinal de -20 dB ainda se vê)
inline float lightBrightness(const float v) {
    const float a = std::fabs(v);
    return a < 0.01f ? 0.0f : std::sqrt(a > 1.0f ? 1.0f : a);
}

}  // namespace rasgo::ui
