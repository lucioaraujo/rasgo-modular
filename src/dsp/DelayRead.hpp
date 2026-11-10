#pragma once

// ============================================================================
// Leitura de linha de atraso com atraso fracionário (10/11/2026)
// ============================================================================
//
// SPRING e PRISM liam a linha com interpolação LINEAR. Ela é um passa-baixa
// que muda com a fração: no meio do caminho entre duas amostras (fração
// 0,5), um seno de 8 kHz a 48 kHz sai com ganho cos(π·8/48) ≈ 0,87
// (−1,25 dB), e quando o atraso anda (a tensão da mola, a dispersão do
// prisma) o brilho oscila junto. Dentro de um laço com realimentação a perda
// se acumula a cada volta.
//
// Aqui: Lagrange de 3ª ordem (4 pontos) — plano até bem mais alto, sem
// alocação, sem estado. Técnica pública: Laakso, Välimäki, Karjalainen &
// Laine, "Splitting the Unit Delay" (IEEE SPM, 1996). Código próprio.
//
// `line` é circular de tamanho `len`; `w` é a posição da PRÓXIMA escrita
// (a amostra mais nova está em w−1); `delay` em amostras. O Lagrange usa um
// ponto à frente, então precisa de `delay` ≥ 3 para não ler a amostra mais
// velha no lugar de uma futura; abaixo disso, cai para a leitura linear.

#include <cmath>
#include <cstddef>

namespace rasgo::modular {

inline float readDelayLinear(const float* line, const std::size_t len,
                             const std::size_t w, const float delay) noexcept {
    float rp = static_cast<float>(w) - delay;
    while (rp < 0.0f) rp += static_cast<float>(len);
    const auto i0 = static_cast<std::size_t>(rp) % len;
    const auto i1 = (i0 + 1) % len;
    const float fr = rp - std::floor(rp);
    return line[i0] + (line[i1] - line[i0]) * fr;
}

inline float readDelayLagrange(const float* line, const std::size_t len,
                               const std::size_t w, const float delay) noexcept {
    if (!(delay >= 3.0f)) return readDelayLinear(line, len, w, delay);
    float rp = static_cast<float>(w) - delay;
    while (rp < 0.0f) rp += static_cast<float>(len);
    const auto i0 = static_cast<std::size_t>(rp) % len;
    const float f = rp - std::floor(rp);
    const float xm = line[(i0 + len - 1) % len];
    const float x0 = line[i0];
    const float x1 = line[(i0 + 1) % len];
    const float x2 = line[(i0 + 2) % len];
    // pontos em −1, 0, 1, 2; avaliado em f ∈ [0, 1)
    const float cm = -f * (f - 1.0f) * (f - 2.0f) / 6.0f;
    const float c0 = (f + 1.0f) * (f - 1.0f) * (f - 2.0f) / 2.0f;
    const float c1 = -(f + 1.0f) * f * (f - 2.0f) / 2.0f;
    const float c2 = (f + 1.0f) * f * (f - 1.0f) / 6.0f;
    return cm * xm + c0 * x0 + c1 * x1 + c2 * x2;
}

}  // namespace rasgo::modular
