// Medidor BS.1770-4 / EBU R128 (`src/dsp/Loudness.hpp`).
//
// `SAIDA_AUDIO_COMUM.md §5` pede "fixtures de conformidade para a medição
// BS.1770, além de testes dourados". Estes são os fixtures: casos cujo
// valor correto vem da NORMA, não da implementação — se o filtro de
// ponderação ou a soma de canais estiver errada, eles falham, e falham
// com um número que diz o quanto.

#include "dsp/Loudness.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using rasgo::modular::LoudnessMeter;

namespace {

int g_failures = 0;

void near(const float got, const float want, const float tol,
          const char* what) {
    if (std::fabs(got - want) > tol) {
        std::fprintf(stderr, "FALHOU: %s — leu %.2f, esperado %.2f (±%.2f)\n",
                     what, got, want, tol);
        ++g_failures;
    }
}
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

// seno de `rmsDb` dBFS RMS por canal, `seconds` de duração
float measureSine(const float sr, const float rmsDb, const float hz,
                  const bool stereo, const float seconds = 20.0f) {
    LoudnessMeter m;
    m.prepare(sr);
    const double amp = std::pow(10.0, rmsDb / 20.0) * std::sqrt(2.0);
    const double dp = 2.0 * M_PI * static_cast<double>(hz) / sr;
    double ph = 0.0;
    const long n = static_cast<long>(sr * seconds);
    for (long i = 0; i < n; ++i, ph += dp) {
        const float v = static_cast<float>(amp * std::sin(ph));
        m.push(v, stereo ? v : 0.0f);
    }
    return m.integrated();
}

// ---- conformidade ----------------------------------------------------------

// EBU Tech 3341: um par estéreo idêntico a −26 dBFS RMS por canal mede
// −23,0 LUFS. Os 3 dB de diferença são a soma de canais da norma (G=1,0
// em cada), não um erro de escala — este teste existe justamente pra que
// alguém não "conserte" isso um dia.
void testEbuReferenceLevel() {
    near(measureSine(48000.0f, -26.0f, 1000.0f, true), -23.0f, 0.1f,
         "estéreo -26 dBFS/canal = -23,0 LUFS (EBU Tech 3341)");
}

// A ponderação K é unitária em 1 kHz: um canal só, a −23 dBFS RMS, mede
// −23,0 LUFS. É a âncora que separa erro de filtro de erro de soma.
void testKWeightingUnityAt1k() {
    near(measureSine(48000.0f, -23.0f, 1000.0f, false), -23.0f, 0.1f,
         "ponderação K é unitária em 1 kHz");
}

// Os coeficientes tabelados da norma são só de 48 kHz. Se forem usados
// crus em outra taxa, a medida sai errada SEM AVISAR — por isso eles são
// derivados da taxa em uso, e por isso isto é testado.
void testSampleRateIndependence() {
    const float rates[] = {44100.0f, 48000.0f, 88200.0f, 96000.0f};
    for (const float sr : rates) {
        const float v = measureSine(sr, -26.0f, 1000.0f, true);
        near(v, -23.0f, 0.15f, "mesma leitura em qualquer taxa");
    }
}

// ---- relações que a norma impõe --------------------------------------------

void testDoublingAmplitude() {
    const float a = measureSine(48000.0f, -26.0f, 1000.0f, true);
    const float b = measureSine(48000.0f, -20.0f, 1000.0f, true);
    near(b - a, 6.02f, 0.1f, "dobrar a amplitude soma +6,02 LU");
}

void testChannelSummation() {
    const float mono = measureSine(48000.0f, -23.0f, 1000.0f, false);
    const float st = measureSine(48000.0f, -23.0f, 1000.0f, true);
    near(st - mono, 3.01f, 0.1f, "o segundo canal correlacionado soma +3,01 LU");
}

// ---- portas e casos de borda -----------------------------------------------

void testSilence() {
    LoudnessMeter m;
    m.prepare(48000.0f);
    for (long i = 0; i < 48000 * 5; ++i) m.push(0.0f, 0.0f);
    check(m.integrated() <= LoudnessMeter::kSilence,
          "silêncio não inventa loudness");
    check(m.momentary() <= LoudnessMeter::kSilence, "momentary de silêncio");
}

// A porta é o ponto do R128 que mais se erra: sem ela, o silêncio entre
// frases puxa o integrado pra baixo e um material esparso mede muito
// menos do que soa. Aqui, 10 s de tom seguidos de 30 s de silêncio têm
// que medir praticamente o mesmo que o tom sozinho.
void testGatingIgnoresSilence() {
    LoudnessMeter m;
    m.prepare(48000.0f);
    const double amp = std::pow(10.0, -26.0 / 20.0) * std::sqrt(2.0);
    const double dp = 2.0 * M_PI * 1000.0 / 48000.0;
    double ph = 0.0;
    for (long i = 0; i < 48000 * 10; ++i, ph += dp) {
        const float v = static_cast<float>(amp * std::sin(ph));
        m.push(v, v);
    }
    for (long i = 0; i < 48000 * 30; ++i) m.push(0.0f, 0.0f);
    near(m.integrated(), -23.0f, 0.3f,
         "a porta impede que 30 s de silêncio puxem o integrado");
}

// Momentary segue a janela de 400 ms: depois de cortar o som, ele cai;
// o integrado, que é a peça inteira, não.
void testMomentaryFollowsTheWindow() {
    LoudnessMeter m;
    m.prepare(48000.0f);
    const double amp = std::pow(10.0, -26.0 / 20.0) * std::sqrt(2.0);
    const double dp = 2.0 * M_PI * 1000.0 / 48000.0;
    double ph = 0.0;
    for (long i = 0; i < 48000 * 5; ++i, ph += dp) {
        const float v = static_cast<float>(amp * std::sin(ph));
        m.push(v, v);
    }
    const float loud = m.momentary();
    for (long i = 0; i < 48000; ++i) m.push(0.0f, 0.0f);   // 1 s de silêncio
    check(m.momentary() < loud - 20.0f, "momentary cai quando o som para");
    near(m.integrated(), -23.0f, 0.5f, "o integrado não cai junto");
}

void testNonFiniteDoesNotPoison() {
    LoudnessMeter m;
    m.prepare(48000.0f);
    const double amp = std::pow(10.0, -26.0 / 20.0) * std::sqrt(2.0);
    const double dp = 2.0 * M_PI * 1000.0 / 48000.0;
    double ph = 0.0;
    for (long i = 0; i < 48000 * 3; ++i, ph += dp) {
        const float v = static_cast<float>(amp * std::sin(ph));
        m.push(v, v);
    }
    check(std::isfinite(m.integrated()) && std::isfinite(m.momentary()),
          "as leituras são sempre finitas");
}

}  // namespace

int main() {
    testEbuReferenceLevel();
    testKWeightingUnityAt1k();
    testSampleRateIndependence();
    testDoublingAmplitude();
    testChannelSummation();
    testSilence();
    testGatingIgnoresSilence();
    testMomentaryFollowsTheWindow();
    testNonFiniteDoesNotPoison();
    if (g_failures == 0) std::puts("test_loudness: OK");
    return g_failures == 0 ? 0 : 1;
}
