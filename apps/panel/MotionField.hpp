#pragma once

#include <cmath>
#include <cstdint>
#include <string>

// ============================================================================
// MotionField — a "mão" caótica que toca o patch (Motion Engine v3)
// ============================================================================
//
// Ver `dossies/ESTUDO_seed_composicao_generativa.md §3.7`.
//
// NÃO é uma máquina de intenções nem uma matriz de qualidades (o autor
// rejeitou as duas — "instrumento de composição, não de regras prontas,
// de determinismos congelados, de padrões limitados de IA"). É:
//
//   - um sistema caótico pequeno (Thomas cyclically-symmetric, 3 vars) que
//     NUNCA repete, NUNCA para, é levemente imprevisível — mas semeado, e
//     de período longo demais pra o ouvido pegar um ciclo;
//   - cada knob/slider animável projeta esse campo do SEU jeito (pesos
//     semeados) → os controles se movem em RELAÇÃO (coerente por
//     construção), cada um por um caminho seu;
//   - a velocidade global do campo segue a ENERGIA do som (realimentação
//     — o loop fechado de onde vem o inaudito);
//   - "gosto" = só a AMPLITUDE por controle, de 4 pistas de palavra no id.
//
// Header puro, testável isolado. `apps/panel/` — o core não sabe disto.

namespace rasgo::panel {

// ---- alcance: a ÚNICA classificação, e só decide a amplitude ----------
enum class MotionReach { Structural, Level, Hot, Free };

inline MotionReach motionReach(const std::string& id) {
    const std::string s = id;
    auto has = [&](const char* w) { return s.find(w) != std::string::npos; };
    // estrutura da composição — mal se move, e devagar
    if (has("step") || has("length") || has("slice") || has("scale")
        || has("root") || has("bpm") || has("mult") || has("ratio")
        || has("voices") || has("pattern") || has("heads") || has("modulus")
        || has("octave") || has("_div") || has("count") || has("bits")
        || has("mode") || has("algo") || has("wave") || has("map")
        || has("quant") || has("degree") || has("interval")
        || s == "freq" || has("pitch") || has("tune") || has("transpose")
        || has("note") || has("key"))
        return MotionReach::Structural;
    // ganho de barramento — clamp duro, quase não mexe
    if (has("gain") || has("level") || has("output") || has("master")
        || has("volume") || has("trim") || has("out_gain"))
        return MotionReach::Level;
    // poderosos perto do extremo (aspereza / auto-oscilação) — respira, não varre
    if (has("reson") || has("feedback") || has("regen") || has("drive")
        || has("grit") || has("fold") || has("crush") || has("fm_amount")
        || has("index") || has("bite") || has("chaos") || id == "pw"
        || has("fine") || has("detune") || has("damp"))
        return MotionReach::Hot;
    return MotionReach::Free;
}

// amplitude como fração do range do parâmetro. Afinação 2026-09-07 (2ª):
// o autor viu `pw`/`reso`/`fold` "frenéticos" — os QUENTES têm efeito
// perceptual enorme por unidade de knob, então ±3–4% já é bastante.
inline float motionAmplitude(const MotionReach r) {
    switch (r) {
        case MotionReach::Structural: return 0.012f;
        case MotionReach::Level:      return 0.030f;
        case MotionReach::Hot:        return 0.040f;
        case MotionReach::Free:       return 0.150f;
    }
    return 0.08f;
}

// tempo de easing (s) — quanto o alvo demora a ser alcançado (maior =
// mais lento/suave)
inline float motionTau(const MotionReach r) {
    switch (r) {
        case MotionReach::Structural: return 8.0f;
        case MotionReach::Level:      return 4.0f;
        case MotionReach::Hot:        return 3.5f;
        case MotionReach::Free:       return 1.8f;
    }
    return 2.5f;
}

// ---- o campo caótico -------------------------------------------------------
struct MotionField {
    double x = 0.10, y = 0.0, z = 0.0;
    double speed = 1.0;              // multiplicador de passo (vem da energia)
    std::uint64_t rng = 0x9E3779B97F4A7C15ULL;

    std::uint64_t nextu() noexcept {
        rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
        return rng;
    }
    float u01() noexcept {
        return static_cast<float>(nextu() >> 40) / 16777216.0f;
    }

    void seed(const std::uint64_t s) noexcept {
        rng = s ? s : 0x1234567ULL;
        x = 0.10 + 0.02 * static_cast<double>(nextu() % 128);
        y = -0.05 + 0.02 * static_cast<double>(nextu() % 128);
        z = 0.03 * static_cast<double>(nextu() % 128) - 1.0;
        speed = 1.0;
    }

    // avança `dt` s; `energy` (0..1) do som acelera/desacelera a mão
    void step(const double dt, const double energy) noexcept {
        const double tgt = 0.35 + 0.9 * clamp01(energy);
        speed += (tgt - speed) * (1.0 - std::exp(-dt * 0.20));
        const double h = dt * 0.45 * speed;      // passo do integrador (lento)
        const double b = 0.1899;                 // Thomas — caótico (< ~0.208)
        const double nx = x + h * (std::sin(y) - b * x);
        const double ny = y + h * (std::sin(z) - b * y);
        const double nz = z + h * (std::sin(x) - b * z);
        x = nx; y = ny; z = nz;
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
            x = 0.1; y = 0.0; z = 0.0;
        }
    }

    // estado normalizado (~[-1,1] com distribuição orgânica)
    double nx() const noexcept { return std::tanh(x * 0.42); }
    double ny() const noexcept { return std::tanh(y * 0.42); }
    double nz() const noexcept { return std::tanh(z * 0.42); }

    static double clamp01(const double v) noexcept {
        return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
    }
};

}  // namespace rasgo::panel
