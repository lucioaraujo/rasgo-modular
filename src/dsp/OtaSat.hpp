#pragma once

#include <cmath>

// ============================================================================
// OtaSaturator — a cor do par diferencial de um OTA, com ADAA de 1ª ordem
// ============================================================================
//
// tanh(d·x)/d antialiasado pela antiderivada (log cosh), uma amostra de
// estado. Saiu do VCA (SAT, 11/11/2026) para o VCA4 usar a MESMA saturação
// (auditoria 2 §3.1, 21/11/2026: "um núcleo só, dois painéis"). A conta é a
// mesma do VCA, linha a linha: o VCA soa idêntico.
// ADAA: Parker, Zavalishin & Le Bivic, DAFx 2016.

namespace rasgo::modular {

struct OtaSaturator {
    float prev = 0.0f;

    void reset() noexcept { prev = 0.0f; }

    // tanh(d·x)/d com ADAA de 1ª ordem (u = d·x)
    float process(const float x, const float d) noexcept {
        const double u1 = static_cast<double>(x) * d;
        const double u0 = static_cast<double>(prev) * d;
        prev = x;
        const double du = u1 - u0;
        const double y = std::fabs(du) > 1.0e-5
            ? (logCosh(u1) - logCosh(u0)) / du
            : std::tanh(0.5 * (u0 + u1));
        return static_cast<float>(y / d);
    }

    // log cosh(u), estável para |u| grande
    static double logCosh(const double u) noexcept {
        const double a = std::fabs(u);
        return a + std::log1p(std::exp(-2.0 * a)) - 0.69314718055994531;
    }
};

}  // namespace rasgo::modular
