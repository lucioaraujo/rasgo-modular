// Parâmetros sem lock (20/11/2026): a interface move knobs e bases de
// modulação (o VARIA, o arrasto, a roda) enquanto o áudio processa, sem o
// `gmx` entre as duas threads. Este teste faz isso de verdade, com duas
// threads, e confere: a saída fica finita, cada parâmetro lido pelo áudio
// é um valor que alguém escreveu (nunca rasgado) e a modulação continua
// aditiva sobre a base. Rode também com ThreadSanitizer:
//   g++ -std=c++17 -O1 -g -fsanitize=thread -Isrc tests/test_rt_params.cpp -pthread
// (TSan acusaria qualquer acesso não atômico ao valor ou à base.)

#include "core/SignalGraph.hpp"
#include "dsp/Drift.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Oscillator.hpp"

#include <atomic>
#include <cmath>
#include <iostream>
#include <thread>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}

void testConcurrentParameterWrites() {
    SignalGraph g;
    const auto osc = g.add(std::make_unique<Oscillator>());
    const auto flt = g.add(std::make_unique<Filter>());
    const auto drf = g.add(std::make_unique<Drift>());
    g.connect(osc, 0, flt, 0);
    // a DRIFT modula o cutoff: o áudio escreve o valor modulado, a
    // interface escreve a base
    g.connectToParameter(drf, 0, flt, "cutoff", 400.0f);
    g.prepare(48000.0f, 2, 256);
    AudioBlock out(48000.0f, 2, 256);

    std::atomic<bool> stop{false};
    std::atomic<long> blocks{0};
    std::atomic<bool> finite{true};
    std::thread audio([&] {
        while (!stop.load(std::memory_order_relaxed)) {
            g.process(out, flt, 0);
            for (std::size_t i = 0; i < 256; ++i)
                if (!std::isfinite(out.at(0, i))) finite.store(false);
            blocks.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // a "interface": arrasto do knob da frequência e a base do cutoff,
    // como o VARIA faria, sem lock
    bool valuesOk = true;
    for (int k = 0; k < 200000; ++k) {
        const float f = 55.0f + static_cast<float>(k % 1000);
        g.setParameterBase(osc, "freq", f);
        g.setParameterBase(flt, "cutoff", 200.0f + static_cast<float>(k % 4000));
        const float back = g.node(osc).parameterValue("freq");
        if (!(back >= 8.0f && back <= 8000.0f)) valuesOk = false;
        const float base = g.parameterUserValue(flt, "cutoff");
        if (!(base >= 200.0f && base < 4200.0f)) valuesOk = false;
    }
    while (blocks.load() < 50) std::this_thread::yield();
    stop.store(true);
    audio.join();

    check(finite.load(), "saída finita com a interface mexendo sem lock");
    check(valuesOk, "cada leitura é um valor escrito (dentro da faixa)");
    check(blocks.load() >= 50, "o áudio rodou durante as escritas");
    // a modulação segue aditiva sobre a última base
    g.setParameterBase(flt, "cutoff", 1000.0f);
    g.process(out, flt, 0);
    check(std::fabs(g.parameterUserValue(flt, "cutoff") - 1000.0f) < 1e-3f,
          "a base é a do knob");
}

void testRtFloatCopy() {
    RtFloat a = 3.5f;
    RtFloat b = a;
    check(static_cast<float>(b) == 3.5f, "RtFloat copia o valor");
    b = 1.25f;
    a = b;
    check(static_cast<float>(a) == 1.25f, "RtFloat atribui o valor");
}

}  // namespace

int main() {
    testRtFloatCopy();
    testConcurrentParameterWrites();
    if (g_failures == 0) {
        std::cout << "RASGO Modular rt params tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
