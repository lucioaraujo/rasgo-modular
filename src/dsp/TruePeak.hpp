#pragma once

#include <array>
#include <cmath>
#include <cstddef>

// ============================================================================
// TruePeakEstimator — pico verdadeiro (entre amostras), 4× polifásico
// ============================================================================
//
// Estudado de `NAVALHA2_JUCE/src/core/TruePeakDetector.{h,cpp}` (código do
// próprio autor, GPLv3/AGPLv3 — compatível; ver
// `AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md` §3.4/P0.2 — achado real: um
// limitador que só olha o pico DA AMOSTRA pode deixar passar um pico
// reconstruído entre amostras alto o bastante pra estourar um DAC ou uma
// conversão pra outro formato, mesmo com o pico de amostra dentro do teto).
// Reescrito aqui no idioma header-only zero-dep do Rasgo Modular. Mesma
// contagem de taps da NAVALHA (16 por fase, 64 no total — medido aqui: uma
// versão inicial com 8 taps/fase (32 no total) tentou economizar custo por
// amostra, mas sonda própria mostrou o filtro perdendo precisão perto de
// Nyquist (15 kHz a 48 kHz: erro de até 3,1 % do valor real, PIOR que só
// olhar o pico de amostra) — exatamente a faixa onde estouro entre
// amostras importa mais. Sem espaço pra economizar aqui: o custo de 64
// multiply-adds por amostra, uma vez por bloco no estágio MASTER (não por
// voz), é desprezível. Janela de Blackman-Harris de 4 termos (mesma
// rejeição de banda de parada), ganho DC unitário por fase.
//
// Não substitui o pico de amostra: `OutputStage` usa o MAIOR dos dois na
// decisão do limitador — este estimador só adiciona o que o pico de
// amostra sozinho não vê.
//
// `processSample` não aloca, não trava; o cálculo dos coeficientes (só em
// `prepare()`) usa `<cmath>`, então não é RT-safe — nunca chamar de dentro
// de `process()`.

namespace rasgo::modular {

class TruePeakEstimator {
public:
    static constexpr std::size_t kOversample = 4;
    static constexpr std::size_t kTapsPerPhase = 16;
    static constexpr std::size_t kTotalTaps = kOversample * kTapsPerPhase;

    void prepare() noexcept {
        constexpr double pi = 3.14159265358979323846;
        constexpr double centre = (static_cast<double>(kTotalTaps) - 1.0) * 0.5;
        std::array<double, kTotalTaps> prototype{};
        for (std::size_t tap = 0; tap < kTotalTaps; ++tap) {
            const double position = static_cast<double>(tap) - centre;
            const double phase = 2.0 * pi * static_cast<double>(tap)
                / static_cast<double>(kTotalTaps - 1);
            // Blackman-Harris de 4 termos: rejeição forte de banda de
            // parada evita que um sinal perto de Nyquist vire um "pico
            // falso" dependente da fase entre as amostras.
            const double window = 0.35875
                - 0.48829 * std::cos(phase)
                + 0.14128 * std::cos(2.0 * phase)
                - 0.01168 * std::cos(3.0 * phase);
            const double x = position / static_cast<double>(kOversample);
            const double s = std::fabs(x) < 1.0e-12
                ? 1.0 : std::sin(pi * x) / (pi * x);
            prototype[tap] = s * window;
        }
        // cada fase normalizada pro próprio ganho DC = 1 (garante que um
        // DC puro na entrada não vira "pico" fantasma em nenhuma fase,
        // incluindo a centrada em meia-amostra)
        for (std::size_t ph = 0; ph < kOversample; ++ph) {
            double sum = 0.0;
            for (std::size_t t = 0; t < kTapsPerPhase; ++t)
                sum += prototype[ph + t * kOversample];
            for (std::size_t t = 0; t < kTapsPerPhase; ++t)
                coeffs_[ph][t] = static_cast<float>(
                    prototype[ph + t * kOversample] / sum);
        }
        reset();
    }

    void reset() noexcept {
        hist_.fill(0.0f);
        w_ = 0;
    }

    // devolve a estimativa de pico (|amplitude|) considerando as 4 posições
    // entre-amostra em torno da amostra mais recente. Alimentar amostra a
    // amostra, uma chamada por canal.
    float processSample(const float x) noexcept {
        hist_[w_] = std::isfinite(x) ? x : 0.0f;
        w_ = (w_ + 1) % hist_.size();

        float peak = 0.0f;
        for (const auto& branch : coeffs_) {
            float acc = 0.0f;
            std::size_t r = w_;
            for (std::size_t t = 0; t < kTapsPerPhase; ++t) {
                r = r == 0 ? hist_.size() - 1 : r - 1;
                acc += branch[t] * hist_[r];
            }
            const float m = std::fabs(acc);
            if (m > peak) peak = m;
        }
        return peak;
    }

private:
    std::array<std::array<float, kTapsPerPhase>, kOversample> coeffs_{};
    std::array<float, kTapsPerPhase> hist_{};
    std::size_t w_ = 0;
};

}  // namespace rasgo::modular
