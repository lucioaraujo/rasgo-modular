#pragma once

// Telemetria de visualização por módulo — o que os retângulos `Display`
// do painel desenham, compartilhado entre os front-ends.
//
// Um `ScopeTrace` por módulo exibido: um anel de amostras da saída 0 (a
// onda do osciloscópio, e a matéria-prima do espectro) e, para o TRIGSEQ,
// quatro anéis de gate (as lanes t1–t4 do piano-roll). Quem enche é o
// THREAD DE ÁUDIO — por isso tudo é pré-alocado na construção e nenhum
// método aloca, redimensiona ou lança.
//
// Existe aqui, e não dentro de um front-end, porque o painel X11
// (`apps/panel/`) e o app JUCE (`apps/juce/`) precisam desenhar o MESMO
// gráfico. Duas cópias da mesma matemática divergem — foi o que já
// aconteceu com a geometria de pegada (ver `PanelGeometry.hpp`).
//
// Framework-free de propósito: só `<array>`/`<cmath>`/`<vector>`. Não
// sabe o que é X11, JUCE ou pixel.

#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace rasgo::ui {

// 220 amostras ≈ a largura útil de um Display de 16 mm nos zooms usuais:
// uma amostra por coluna, sem reamostrar na hora de desenhar.
inline constexpr std::size_t kScopeLen = 220;
// 128 passos de histórico de gate por lane; 4 lanes ≈ 2 KB por módulo.
inline constexpr std::size_t kLaneLen = 128;

struct ScopeTrace {
    std::vector<float> buf = std::vector<float>(kScopeLen, 0.0f);
    std::size_t w = 0;

    std::array<std::vector<float>, 4> lanes{
        std::vector<float>(kLaneLen, 0.0f), std::vector<float>(kLaneLen, 0.0f),
        std::vector<float>(kLaneLen, 0.0f), std::vector<float>(kLaneLen, 0.0f)};
    std::size_t lw = 0;

    void push(const float v) noexcept {
        buf[w] = v;
        w = (w + 1) % kScopeLen;
    }
    void pushLanes(const float a, const float b, const float c,
                   const float d) noexcept {
        lanes[0][lw] = a; lanes[1][lw] = b; lanes[2][lw] = c; lanes[3][lw] = d;
        lw = (lw + 1) % kLaneLen;
    }

    // amostra `k` em ordem CRONOLÓGICA (0 = mais antiga do anel)
    float at(const std::size_t k) const noexcept {
        return buf[(w + k) % buf.size()];
    }
    float lane(const std::size_t L, const std::size_t k) const noexcept {
        return lanes[L][(lw + k) % kLaneLen];
    }

    float peak() const noexcept {
        float p = 0.0f;
        for (const float v : buf) p = std::max(p, std::fabs(v));
        return p;
    }
};

// ---- espectro: banco de Goertzel logarítmico ---------------------------
//
// `nb` bandas espaçadas em log entre 60 Hz e 60·200 = 12 kHz, calculadas
// direto sobre o anel. Goertzel em vez de FFT porque são poucas bandas
// sobre poucas amostras: um laço por banda custa menos que montar uma FFT,
// e não precisa de dependência nova.
//
// `binRateHz` é a taxa de amostragem DO ANEL — que não é a do áudio: o
// thread de áudio empurra uma amostra a cada `stride`, então o anel roda a
// `sampleRate/stride`.
//
// ATENÇÃO — o painel X11 sempre passou 24000 aqui, um valor que não
// corresponde nem à taxa do áudio (48 kHz) nem à do anel decimado
// (~9,6 kHz com bloco de 256). O rótulo de frequência das bandas, portanto,
// nunca bateu com o conteúdo real. Está preservado como estava porque
// mudar altera a APARÊNCIA do espectro que já existe, e isso é decisão do
// autor, não minha — mas o parâmetro está aqui, explícito, para que a
// correção seja uma linha quando ele decidir. Ver `apps/juce/PARIDADE.md`.
inline void scopeSpectrum(const ScopeTrace& sc, float* mag, const int nb,
                          const float binRateHz) noexcept {
    const int n = static_cast<int>(sc.buf.size());
    for (int b = 0; b < nb; ++b) {
        const float f = 60.0f * std::pow(200.0f,
            static_cast<float>(b) / static_cast<float>(nb - 1));
        const float wn = 6.2831853f * f / binRateHz;
        const float cr = std::cos(wn);
        float s1 = 0.0f, s2 = 0.0f;
        for (int k = 0; k < n; ++k) {
            const float s0 = sc.at(static_cast<std::size_t>(k))
                             + 2.0f * cr * s1 - s2;
            s2 = s1; s1 = s0;
        }
        mag[b] = std::sqrt(std::fabs(s1 * s1 + s2 * s2 - 2.0f * cr * s1 * s2))
                 / static_cast<float>(n);
    }
}

// a taxa que o painel X11 sempre usou — ver a ressalva acima
inline constexpr float kLegacySpectrumRateHz = 24000.0f;

}  // namespace rasgo::ui
