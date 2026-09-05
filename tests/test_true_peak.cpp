// Teste isolado do `TruePeakEstimator` (src/dsp/TruePeak.hpp) e da sua
// integração no `OutputStage` — pico verdadeiro (entre amostras). Ver
// `dossies/17_master.md` e o achado da NAVALHA 2
// (`AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md` §3.4/P0.2/P0.3).

#include "dsp/OutputStage.hpp"
#include "dsp/TruePeak.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr double kPi = 3.14159265358979323846;

// uma senoide de amplitude A tem pico CONTÍNUO == A sempre (definição) —
// mede o quanto o estimador chega perto disso, amostrando numa fase que
// não bate exatamente na crista.
float truePeakErrorFor(const float amplitude, const float freqHz,
                       const double phaseOffset) {
    TruePeakEstimator est;
    est.prepare();
    double phase = phaseOffset;
    float measured = 0.0f;
    for (int i = 0; i < 4000; ++i) {
        const float s = amplitude * static_cast<float>(std::sin(phase));
        phase += 2.0 * kPi * freqHz / kSr;
        const float tp = est.processSample(s);
        if (tp > measured) measured = tp;
    }
    return std::fabs(amplitude - measured);
}

void testTruePeakTracksKnownAmplitude() {
    // tolerância medida (não normativa EBU — não temos as fixtures
    // oficiais Tech 3341 pra validar contra elas, ver TAREFAS.md): sonda
    // manual mediu erro máximo ~3,3% perto de Nyquist (15 kHz a 48 kHz);
    // 4% dá uma margem sem mascarar uma regressão real.
    const float A = 0.85f;
    for (const float freq : {200.0f, 1000.0f, 3000.0f, 8000.0f, 15000.0f}) {
        for (const double ph : {0.1, 0.33, 0.57, 0.81}) {
            const float err = truePeakErrorFor(A, freq, ph);
            check(err < 0.04f * A, "pico verdadeiro fica a < 4% da amplitude real");
        }
    }
}

void testTruePeakNeverGrosslyUnderestimates() {
    // a propriedade que importa pra segurança: nunca subestimar MUITO (o
    // padrão real permite superestimar um pouco -- é o lado seguro).
    const float A = 0.9f;
    for (const float freq : {2000.0f, 6000.0f, 12000.0f, 18000.0f}) {
        TruePeakEstimator est;
        est.prepare();
        double phase = 0.2;
        float measured = 0.0f;
        for (int i = 0; i < 4000; ++i) {
            const float s = A * static_cast<float>(std::sin(phase));
            phase += 2.0 * kPi * freq / kSr;
            measured = std::max(measured, est.processSample(s));
        }
        check(measured > A * 0.9f, "nunca subestima o pico real por mais de ~10%");
    }
}

void testOutputStageCatchesInterSampleOver() {
    // caso construído: amplitude real 0,95 (acima do teto -1 dBFS =
    // 0,891), mas o pico DE AMOSTRA (8 kHz, fase 0,15) fica em ~0,88,
    // abaixo do teto -- um limitador cego a isso ficaria transparente.
    OutputStage out;
    out.prepare(kSr, -1.0f, 3.0f, 120.0f);
    const float amplitude = 0.95f;
    double phase = 0.15;
    float samplePeak = 0.0f;
    for (int i = 0; i < 64 * 80; ++i) {
        const float s = amplitude * static_cast<float>(std::sin(phase));
        phase += 2.0 * kPi * 8000.0 / kSr;
        samplePeak = std::max(samplePeak, std::fabs(s));
        float l = s, r = s;
        out.process(l, r, false, true);
    }
    check(samplePeak < out.ceilingLinear(),
          "pré-condição do caso: o pico DE AMOSTRA sozinho pareceria seguro");
    check(out.gainReductionDb() > 0.1f,
          "o pico verdadeiro aciona reducao de ganho que o pico de amostra sozinho não acionaria");
    check(out.outputPeak() <= out.ceilingLinear() + 0.01f,
          "a saida final fica dentro do teto (com folga pro joelho suave)");
}

void testOutputStageTransparentOnCalmSignal() {
    OutputStage out;
    out.prepare(kSr, -1.0f, 3.0f, 120.0f);
    double phase = 0.0;
    for (int i = 0; i < 64 * 80; ++i) {
        float s = 0.5f * static_cast<float>(std::sin(phase));
        phase += 2.0 * kPi * 220.0 / kSr;
        float l = s, r = s;
        out.process(l, r, false, true);
    }
    check(out.gainReductionDb() < 0.05f,
          "sinal calmo (longe do teto) fica praticamente transparente");
}

void testDeterminism() {
    TruePeakEstimator a, b;
    a.prepare(); b.prepare();
    double phase = 0.4;
    bool same = true;
    for (int i = 0; i < 500; ++i) {
        const float s = 0.7f * static_cast<float>(std::sin(phase));
        phase += 2.0 * kPi * 5000.0 / kSr;
        if (a.processSample(s) != b.processSample(s)) same = false;
    }
    EXPECT(same);
}

}  // namespace

int main() {
    testTruePeakTracksKnownAmplitude();
    testTruePeakNeverGrosslyUnderestimates();
    testOutputStageCatchesInterSampleOver();
    testOutputStageTransparentOnCalmSignal();
    testDeterminism();
    if (g_failures == 0) {
        std::cout << "RASGO Modular true-peak tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
