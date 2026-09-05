// Teste isolado do `OutputStage` (src/dsp/OutputStage.hpp) — foco no que
// é novo em 2026-09-05: a guarda ULTRASSÔNICA (sempre ligada) e o
// GOVERNADOR DE CORPO (`bodyGuard`, "gosto de barulhos, mas há alguns que
// passam do limite, incomodam o corpo"). O limitador / pico verdadeiro
// têm cobertura própria em `test_true_peak.cpp`.

#include "dsp/OutputStage.hpp"

#include <cmath>
#include <cstdint>
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

// magnitude da componente `freqHz` (Goertzel simples) sobre os últimos
// `tailFrac` do sinal (evita o transitório inicial).
double component(const std::vector<float>& x, const double freqHz,
                 const double tailFrac = 0.5) {
    const std::size_t start =
        static_cast<std::size_t>(x.size() * (1.0 - tailFrac));
    const double w = 2.0 * kPi * freqHz / kSr;
    const double cw = std::cos(w), sw = std::sin(w);
    double re = 0.0, im = 0.0;
    for (std::size_t i = start; i < x.size(); ++i) {
        re += x[i] * std::cos(w * i);
        im -= x[i] * std::sin(w * i);
    }
    (void)cw; (void)sw;
    const double n = static_cast<double>(x.size() - start);
    return 2.0 * std::sqrt(re * re + im * im) / n;
}

// roda uma senoide sustentada por `seconds` e devolve o mono de saída.
std::vector<float> runSine(const float amp, const float freqHz,
                           const float bodyGuard, const float seconds = 1.2f,
                           const bool limit = false) {
    OutputStage os;
    os.prepare(kSr);
    const std::size_t n = static_cast<std::size_t>(kSr * seconds);
    std::vector<float> out;
    out.reserve(n);
    double ph = 0.11;
    for (std::size_t i = 0; i < n; ++i) {
        float s = amp * static_cast<float>(std::sin(ph));
        ph += 2.0 * kPi * freqHz / kSr;
        float l = s, r = s;
        os.process(l, r, false, limit, bodyGuard);
        out.push_back(0.5f * (l + r));
    }
    return out;
}

void testBodyGuardBypassAtZero() {
    // 3,2 kHz alto e sustentado, mas bodyGuard = 0 -> passa intocado
    const auto y = runSine(0.7f, 3200.0f, 0.0f);
    const double c = component(y, 3200.0);
    check(std::fabs(c - 0.7) < 0.03,
          "bodyGuard=0: senoide de 3,2 kHz passa com amplitude ~intacta");
    OutputStage os; os.prepare(kSr);
    for (int i = 0; i < 40000; ++i) {
        float l = 0.7f * std::sin(i * 0.42f), r = l;
        os.process(l, r, false, false, 0.0f);
    }
    check(os.bodyGuardDb() < 0.05f, "bodyGuard=0: telemetria de corte fica ~0");
}

void testBodyGuardCatchesSustainedHarshTone() {
    // MESMO tom (3,2 kHz, ~−2,8 dBFS) com e sem governador
    const auto guarded = runSine(0.72f, 3200.0f, 1.0f, 1.6f);
    const auto raw = runSine(0.72f, 3200.0f, 0.0f, 1.6f);
    const double cg = component(guarded, 3200.0, 0.35);
    const double cr = component(raw, 3200.0, 0.35);
    const double redDb = 20.0 * std::log10((cr + 1e-9) / (cg + 1e-9));
    check(redDb > 2.0,
          "tom perfurante sustentado em 3,2 kHz: governador tira ao menos 2 dB");
    check(redDb < 14.0, "mas a redução é SUAVE (não some o som)");

    OutputStage os; os.prepare(kSr);
    double ph = 0.1;
    for (int i = 0; i < static_cast<int>(kSr * 1.6f); ++i) {
        float l = 0.72f * static_cast<float>(std::sin(ph)); ph += 2.0*kPi*3200.0/kSr;
        float r = l;
        os.process(l, r, false, false, 1.0f);
    }
    check(os.bodyGuardDb() > 1.5f, "e a telemetria reporta a atuação");
}

void testBodyGuardScalesWithAmount() {
    // `bodyGuard` 0..1 escala o corte: metade tira menos que o total
    const auto full = runSine(0.8f, 3200.0f, 1.0f, 1.6f);
    const auto half = runSine(0.8f, 3200.0f, 0.4f, 1.6f);
    const auto off = runSine(0.8f, 3200.0f, 0.0f, 1.6f);
    const double cf = component(full, 3200.0, 0.35);
    const double ch = component(half, 3200.0, 0.35);
    const double co = component(off, 3200.0, 0.35);
    check(cf < ch && ch < co, "mais bodyGuard -> mais atenuação, monotônico");
}

void testBodyGuardIgnoresTransients() {
    // rajadas de 3 kHz de 18 ms a cada 380 ms — ritmo, não grito sustentado
    OutputStage os; os.prepare(kSr);
    const std::size_t n = static_cast<std::size_t>(kSr * 3.0f);
    const std::size_t burst = static_cast<std::size_t>(kSr * 0.018f);
    const std::size_t period = static_cast<std::size_t>(kSr * 0.380f);
    double ph = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const bool on = (i % period) < burst;
        float s = on ? 0.8f * static_cast<float>(std::sin(ph)) : 0.0f;
        ph += 2.0 * kPi * 3000.0 / kSr;
        float l = s, r = s;
        os.process(l, r, false, false, 1.0f);
    }
    check(os.bodyGuardDb() < 0.8f,
          "transientes rítmicos de 3 kHz NÃO acionam o governador (ataque lento)");
}

void testBodyGuardLetsBroadbandNoiseThrough() {
    // ruído branco ALTO (~−9 dBFS RMS) — o autor gosta de barulho; a
    // energia é espalhada, o gate de concentração deixa passar
    OutputStage os; os.prepare(kSr);
    std::uint32_t st = 0x1234567u;
    const std::size_t n = static_cast<std::size_t>(kSr * 1.5f);
    for (std::size_t i = 0; i < n; ++i) {
        st ^= st << 13; st ^= st >> 17; st ^= st << 5;
        float s = (static_cast<float>(static_cast<int32_t>(st)) / 2.147483648e9f)
                  * 0.5f;
        float l = s, r = s;
        os.process(l, r, false, false, 1.0f);
    }
    check(os.bodyGuardDb() < 1.5f,
          "ruído de banda larga alto passa quase intocado (gate de concentração)");
}

void testUltrasonicGuardTamesNearNyquist() {
    const auto lo = runSine(0.6f, 1000.0f, 1.0f);
    const auto hi = runSine(0.6f, 22000.0f, 1.0f);
    const double clo = component(lo, 1000.0);
    const double chi = component(hi, 22000.0);
    check(std::fabs(clo - 0.6) < 0.03, "1 kHz passa (guarda transparente)");
    check(chi < 0.45, "22 kHz é atenuado pela guarda ultrassônica (< ~−2,5 dB)");
}

void testUltrasonicGuardTransparentMidband() {
    // fora da faixa do governador de corpo (2,5–8 kHz), a guarda
    // ultrassônica é transparente até bem alto
    for (const float f : {1000.0f, 11000.0f, 14000.0f}) {
        const auto y = runSine(0.5f, f, 1.0f);
        const double c = component(y, f);
        check(std::fabs(c - 0.5) < 0.025,
              "banda alta (fora de 2,5–8 kHz) passa a < ~0,4 dB");
    }
}

void testConstantPassesExactly() {
    OutputStage os; os.prepare(kSr);
    float l = 0.0f, r = 0.0f;
    for (int i = 0; i < 20000; ++i) { l = 0.5f; r = 0.5f;
        os.process(l, r, false, false, 1.0f); }
    check(std::fabs(l - 0.5f) < 1e-4f && std::fabs(r - 0.5f) < 1e-4f,
          "sinal constante passa exato (guardas não mexem em DC)");
}

void testDeterminism() {
    OutputStage a, b;
    a.prepare(kSr); b.prepare(kSr);
    std::uint32_t st = 99u;
    bool same = true;
    for (int i = 0; i < 5000; ++i) {
        st ^= st << 13; st ^= st >> 17; st ^= st << 5;
        const float s = static_cast<float>(static_cast<int32_t>(st)) / 3.0e9f;
        float la = s, ra = s, lb = s, rb = s;
        a.process(la, ra, true, true, 1.0f);
        b.process(lb, rb, true, true, 1.0f);
        if (la != lb || ra != rb) same = false;
    }
    EXPECT(same);
}

}  // namespace

int main() {
    testBodyGuardBypassAtZero();
    testBodyGuardCatchesSustainedHarshTone();
    testBodyGuardScalesWithAmount();
    testBodyGuardIgnoresTransients();
    testBodyGuardLetsBroadbandNoiseThrough();
    testUltrasonicGuardTamesNearNyquist();
    testUltrasonicGuardTransparentMidband();
    testConstantPassesExactly();
    testDeterminism();
    if (g_failures == 0) {
        std::cout << "RASGO Modular output-stage tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
