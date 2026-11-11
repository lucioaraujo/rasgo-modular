// Trocas de chave sem estalo (11/11/2026).
//
// A auditoria dos módulos (`dossies/AUDITORIA_modulos_e_cardinal.md` §2.2)
// mediu que virar certas chaves — como o VARIA vira, de tempos em tempos,
// e como a mão vira — dava um salto entre amostras muito maior que o do
// módulo parado: LOOPER `hold`/`reverse`, SWIRL `type`, MEMORY `freeze` e o
// MUTE do MIXER. Cada um ganhou a sua passagem (crossfade, mergulho do
// efeito até o seco, costura do buffer, rampa). Este teste vira a chave a
// cada ~100 ms e exige que o maior salto logo após a troca não passe de 2×
// o maior salto do mesmo trecho longe das trocas + 0,03.

#include "panel/ModuleCatalog.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace rasgo::modular;

namespace {

int g_failures = 0;

struct Jumps { float atSwitch = 0.0f, steady = 0.0f; };

// vira a chave a cada ~100 ms entre `a` e `b`; separa o maior salto nos 2
// blocos logo após cada troca do maior salto no resto do mesmo trecho
Jumps measure(const char* type, const std::string& par, const float a, const float b) {
    auto m = rasgo::panel::makeModule(type);
    const float sr = 48000.0f;
    const std::size_t B = 256;
    m->setParameter(par, a);
    m->prepare(sr, B);
    std::vector<AudioBlock> out(m->outputCount(), AudioBlock(sr, 1, B));
    AudioBlock in(sr, 1, B);
    std::vector<const AudioBlock*> ins(m->inputCount(), nullptr);
    for (std::size_t k = 0; k < m->inputCount(); ++k)
        if (m->inputDescriptor(k).kind == PortKind::Audio) { ins[k] = &in; break; }
    double ph = 0.0;
    float last = 0.0f;
    Jumps j;
    bool side = false;
    int sinceSwitch = 99;
    for (int blk = 0; blk < 1200; ++blk) {
        if (blk > 200 && blk % 19 == 0) {   // ~100 ms; antes, enche o buffer
            side = !side;
            m->setParameter(par, side ? b : a);
            sinceSwitch = 0;
        }
        for (std::size_t i = 0; i < B; ++i, ph += 2.0 * M_PI * 220.0 / sr)
            in.at(0, i) = static_cast<float>(0.5 * std::sin(ph));
        m->process(ins, out);
        for (std::size_t i = 0; i < B; ++i) {
            const float v = out[0].at(0, i);
            const float d = std::fabs(v - last);
            last = v;
            if (blk <= 200) continue;
            if (sinceSwitch <= 1) j.atSwitch = std::max(j.atSwitch, d);
            else j.steady = std::max(j.steady, d);
        }
        ++sinceSwitch;
    }
    return j;
}

void expectDeclicked(const char* type, const char* par, const float a, const float b) {
    const Jumps j = measure(type, par, a, b);
    // fora da troca também: a emenda que a troca deixa no buffer (HOLD,
    // FREEZE, linha do SWIRL) aparecia depois, longe da janela da troca
    if (j.steady > 0.06f) {
        std::fprintf(stderr, "FALHOU: %s.%s estala depois da troca (emenda) — salto %.3f\n",
                     type, par, j.steady);
        ++g_failures;
    }
    if (j.atSwitch > 2.0f * j.steady + 0.03f) {
        std::fprintf(stderr, "FALHOU: %s.%s estala na troca — fora da troca %.3f, na troca %.3f\n",
                     type, par, j.steady, j.atSwitch);
        ++g_failures;
    } else if (std::getenv("DECLICK_DB")) {
        std::fprintf(stderr, "  %s.%s: fora da troca %.3f, na troca %.3f\n", type, par, j.steady, j.atSwitch);
    }
}

}  // namespace

// o laço congelado do LOOPER, sem troca nenhuma: a emenda de cada volta
void testLooperHoldLoopSeam() {
    auto m = rasgo::panel::makeModule("LOOPER");
    const float sr = 48000.0f;
    const std::size_t B = 256;
    m->prepare(sr, B);
    std::vector<AudioBlock> out(m->outputCount(), AudioBlock(sr, 1, B));
    AudioBlock in(sr, 1, B);
    std::vector<const AudioBlock*> ins(m->inputCount(), nullptr);
    ins[0] = &in;
    double ph = 0.0;
    float last = 0.0f, worst = 0.0f;
    for (int blk = 0; blk < 1200; ++blk) {
        if (blk == 300) m->setParameter("hold", 1.0f);
        for (std::size_t i = 0; i < B; ++i, ph += 2.0 * M_PI * 220.0 / sr)
            in.at(0, i) = static_cast<float>(0.5 * std::sin(ph));
        m->process(ins, out);
        for (std::size_t i = 0; i < B; ++i) {
            const float v = out[0].at(0, i);
            if (blk > 320) worst = std::max(worst, std::fabs(v - last));
            last = v;
        }
    }
    // o seno de 220 Hz a 0,5 anda no máximo ~0,0144 por amostra; com a mistura
    // do LOOPER e a fita, o laço congelado fica muito abaixo de um estalo
    if (worst > 0.06f) {
        std::fprintf(stderr, "FALHOU: LOOPER em HOLD estala na emenda do laço — salto %.3f\n", worst);
        ++g_failures;
    } else if (std::getenv("DECLICK_DB")) {
        std::fprintf(stderr, "  LOOPER HOLD, emenda do laço: salto %.3f\n", worst);
    }
}

int main() {
    testLooperHoldLoopSeam();
    expectDeclicked("LOOPER", "hold", 0.0f, 1.0f);
    expectDeclicked("LOOPER", "reverse", 0.0f, 1.0f);
    expectDeclicked("SWIRL", "type", 0.0f, 3.0f);
    expectDeclicked("SWIRL", "type", 1.0f, 2.0f);
    expectDeclicked("MEMORY", "freeze", 0.0f, 1.0f);
    expectDeclicked("MIXER", "mute1", 0.0f, 1.0f);
    if (g_failures == 0) std::printf("test_declick: OK\n");
    return g_failures == 0 ? 0 : 1;
}
