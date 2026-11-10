// Régua de QUALIDADE TÉCNICA do som das seeds (10/11/2026).
//
// O autor liberou mudar a composição de todas as seeds ("não há problema
// de mudar a composição de todas as seeds. só não desejo que haja
// problemas técnicos de qualidade de som"). Este teste é a condição: roda
// 24 seeds por 6 s cada, depois do equilíbrio de volume, e reprova se
// alguma mudança de composição trouxer:
//
//   - amostra não finita (NaN/Inf);
//   - pico acima do teto do sink;
//   - LIMITADOR do MASTER segurando mais de 5 % do tempo (patch quente
//     demais: o som é esmagado, não equilibrado);
//   - DC acima de 0,005 (deslocamento que come headroom e estala);
//   - AGUDO excessivo: energia acima de 8 kHz maior que −15 dB do total
//     (o "hiper agudo" já reclamado pelo autor);
//   - VOLUME fora da faixa: nenhuma seed acima de −16 LUFS nem abaixo de
//     −45 (inaudível); no máximo 2 em 24 fora de −34…−19.
//
// Limites com folga sobre a linha de base de 10/11/2026 (40 seeds):
// limitador 0 %, pico ≤ 0,55, DC ≤ 0,0011, agudo ≤ −21,6 dB, LUFS −37,4 a
// −21,9 (mediana −25,9). Não substitui escuta: garante que nada do que
// não deveria chegar ao alto-falante chegue.

#include "core/SignalGraph.hpp"
#include "dsp/Loudness.hpp"
#include "dsp/Master.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

using namespace rasgo::modular;

namespace {

int falhas = 0;
void check(const bool ok, const char* o_que, const int seed) {
    if (!ok) { std::printf("  FALHOU (seed n.º %d): %s\n", seed, o_que); ++falhas; }
}

constexpr float kSr = 44100.0f;
constexpr std::size_t kB = 256;
constexpr int kSeeds = 24;
constexpr double kSegundos = 6.0;

struct Medida {
    bool finito = true;
    double pico = 0.0, lufs = -70.0, agudoDb = -99.0, limitPct = 0.0, dc = 0.0;
};

Medida medir(const std::uint64_t seed) {
    SignalGraph g;
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g.add(std::move(n));
    const auto sink = g.add(std::make_unique<rasgo::panel::SinkOut>());
    rasgo::panel::seedPatch(g, seed);
    g.prepare(kSr, 2, kB);
    g.setActiveOutput(sink);
    rasgo::panel::balanceSeedLevel(g, seed, kSr, kB);

    Master* ms = nullptr;
    for (std::size_t i = 0; i < g.nodeCount(); ++i)
        if (auto* m = dynamic_cast<Master*>(&g.node(i))) ms = m;

    LoudnessMeter lm; lm.prepare(kSr);
    AudioBlock o(kSr, 2, kB);
    Medida md;
    double soma = 0.0, e = 0.0, eh = 0.0;
    long n = 0, blocosLim = 0;
    // passa-altas de 2 polos em 8 kHz (dois estágios de 1 polo)
    const float a = std::exp(-2.0f * 3.14159265f * 8000.0f / kSr);
    float x1 = 0, y1 = 0, h1 = 0, h2 = 0;
    const long nb = static_cast<long>(kSegundos * kSr / kB);
    for (long b = 0; b < nb; ++b) {
        g.process(o, sink, 0);
        if (ms && ms->gainReductionDb() > 1.0f) ++blocosLim;
        for (std::size_t i = 0; i < kB; ++i) {
            const float l = o.at(0, i), r = o.at(1, i);
            if (!std::isfinite(l) || !std::isfinite(r)) { md.finito = false; continue; }
            lm.push(l, r);
            md.pico = std::max(md.pico, static_cast<double>(std::max(std::fabs(l), std::fabs(r))));
            soma += l; ++n; e += static_cast<double>(l) * l;
            const float y = a * (y1 + l - x1); x1 = l; y1 = y;
            const float z = a * (h2 + y - h1); h1 = y; h2 = z;
            eh += static_cast<double>(z) * z;
        }
    }
    md.lufs = lm.integrated();
    md.agudoDb = e > 0.0 ? 10.0 * std::log10(eh / e + 1e-12) : -99.0;
    md.limitPct = 100.0 * static_cast<double>(blocosLim) / static_cast<double>(nb);
    md.dc = std::fabs(soma / static_cast<double>(std::max(1L, n)));
    return md;
}

}  // namespace

int main() {
    int foraDaFaixa = 0;
    for (int k = 1; k <= kSeeds; ++k) {
        const std::uint64_t s = static_cast<std::uint64_t>(k) * 0x9E3779B97F4A7C15ULL;
        const Medida m = medir(s);
        check(m.finito, "saída finita", k);
        check(m.pico <= rasgo::panel::SinkOut::kCeiling + 1e-4, "pico dentro do teto", k);
        check(m.limitPct <= 5.0, "limitador do MASTER segurando no máximo 5 % do tempo", k);
        check(m.dc <= 0.005, "DC abaixo de 0,005", k);
        check(m.agudoDb <= -15.0, "agudo acima de 8 kHz abaixo de -15 dB do total", k);
        check(m.lufs <= -16.0, "nenhuma seed acima de -16 LUFS", k);
        check(m.lufs >= -45.0, "nenhuma seed inaudível (abaixo de -45 LUFS)", k);
        if (m.lufs > -19.0 || m.lufs < -34.0) {
            ++foraDaFaixa;
            std::printf("  (seed n.º %d fora de -34…-19: %.1f LUFS)\n", k, m.lufs);
        }
    }
    check(foraDaFaixa <= 2, "no máximo 2 de 24 seeds fora de -34…-19 LUFS", 0);
    if (falhas == 0) std::printf("seed quality: todos os testes passaram\n");
    return falhas == 0 ? 0 : 1;
}
