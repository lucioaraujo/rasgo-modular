// Exame das PORTAS que a triagem acusou de não produzir nada.
//
// Mesma lição do exame de parâmetros: conectar todas as entradas não é
// excitação neutra, e portas de EVENTO (fim de ciclo, carry, onset) só
// falam quando o evento acontece.
#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>
using namespace rasgo::modular;
constexpr float kSr = 44100.0f;
constexpr std::size_t kB = 256;

using Prep = std::function<void(Signal&)>;
// devolve -1 se a entrada deve ficar DESCONECTADA
using Excit = std::function<float(std::size_t, long)>;

float pulso(long t, double hz, double ms = 30.0) {
    const long per = (long)(kSr/hz);
    return ((t % per) < (long)(kSr*ms/1000.0)) ? 1.0f : 0.0f;
}
float seno(long t, double hz, float a = 0.5f) {
    return a * (float)std::sin(2*M_PI*hz*t/kSr);
}

void exame(const char* tipo, const Prep& prep, const Excit& ex,
           const std::vector<const char*>& portas) {
    auto n = rasgo::panel::makeModule(tipo);
    if (!n) return;
    prep(*n);
    n->prepare(kSr, kB);
    const std::size_t ni = n->inputCount(), no = n->outputCount();
    std::vector<AudioBlock> in(ni ? ni : 1, AudioBlock(kSr, 2, kB));
    std::vector<AudioBlock> out(no ? no : 1, AudioBlock(kSr, 2, kB));
    std::vector<const AudioBlock*> ins(ni, nullptr);
    std::vector<double> pico(no, 0.0);
    for (int b = 0; b < 900; ++b) {
        for (std::size_t i = 0; i < ni; ++i) {
            bool ligada = false;
            for (std::size_t k = 0; k < kB; ++k) {
                const float v = ex(i, (long)b*(long)kB + (long)k);
                if (v >= -0.5f) { ligada = true;
                                  in[i].at(0,k)=v; in[i].at(1,k)=v; }
            }
            ins[i] = ligada ? &in[i] : nullptr;
        }
        n->process(ins, out);
        if (b < 150) continue;
        for (std::size_t p = 0; p < no; ++p)
            for (std::size_t k = 0; k < kB; ++k)
                pico[p] = std::max(pico[p], (double)std::fabs(out[p].at(0,k)));
    }
    for (const char* nome : portas)
        for (std::size_t p = 0; p < no; ++p)
            if (n->outputDescriptor(p).name == nome)
                std::printf("%-10s %-10s pico %8.4f  %s\n", tipo, nome, pico[p],
                            pico[p] > 1e-9 ? "ok" : "SEM SAÍDA");
}

int main() {
    std::printf("%-10s %-10s %13s  %s\n", "MÓDULO", "porta", "pico", "");

    // STAGES.eoc — fim de ciclo: precisa de gate e de `loop`
    exame("STAGES", [](Signal& s){ s.setParameter("loop", 1.0f);
                                   s.setParameter("rate", 0.7f); },
          [](std::size_t i, long t){ return i==0 ? pulso(t, 2.0, 200.0) : -1.0f; },
          {"eoc", "step"});

    // SH.out1/out2 — precisam de gatilho
    exame("SH", [](Signal&){},
          [](std::size_t i, long t){
            if (i==0) return seno(t, 3.0);
            if (i==1) return pulso(t, 7.0);
            if (i==2) return seno(t, 5.0);
            if (i==3) return pulso(t, 6.5);   // NÃO travado com o seno de in2
            return -1.0f; }, {"out1", "out2"});

    // LOGIC.and/flip — AND precisa de A e B altos JUNTOS
    exame("LOGIC", [](Signal&){},
          [](std::size_t i, long t){
            if (i==0) return pulso(t, 8.0);
            if (i==1) return pulso(t, 4.0, 120.0);     // largos: sobrepõem
            if (i==2) return pulso(t, 3.0, 150.0);
            return -1.0f; }, {"and", "or", "xor", "flip"});

    // SEQUENCE.eos — fim de sequência: precisa de clock e voltas
    exame("SEQUENCE", [](Signal& s){ s.setParameter("length", 4.0f); },
          [](std::size_t i, long t){ return i==0 ? pulso(t, 12.0) : -1.0f; },
          {"eos"});

    // TRIGSEQ.t4 — a lane 4 precisa de densidade
    exame("TRIGSEQ", [](Signal& s){ s.setParameter("density4", 1.0f);
                                    s.setParameter("length", 8.0f); },
          [](std::size_t i, long t){ return i==0 ? pulso(t, 10.0) : -1.0f; },
          {"t4", "accent", "any"});

    // ABACUS.carry — estouro do módulo: precisa contar
    exame("ABACUS", [](Signal& s){ s.setParameter("count_step", 1.0f);
                                   s.setParameter("modulus", 4.0f); },
          [](std::size_t i, long t){
            if (i==0) return seno(t, 2.0);
            if (i==2) return pulso(t, 9.0);
            return -1.0f; }, {"carry"});

    // BOXCAR.geiger — disparo estocástico
    exame("BOXCAR", [](Signal& s){ s.setParameter("geiger", 1.0f);
                                   s.setParameter("thresh", 0.2f); },
          [](std::size_t i, long t){ return i==0 ? seno(t, 220.0) : -1.0f; },
          {"geiger"});

    // SWITCH.out_b/c/d — só recebem em DEMUX (dir = 1)
    exame("SWITCH", [](Signal& s){ s.setParameter("dir", 1.0f);
                                   s.setParameter("steps", 4.0f); },
          [](std::size_t i, long t){
            if (i==0) return seno(t, 220.0);
            if (i==4) return pulso(t, 6.0);
            return -1.0f; }, {"out", "out_b", "out_c", "out_d"});

    // SCOPE.onset — detecção de ataque: precisa de transientes
    exame("SCOPE", [](Signal& s){ s.setParameter("sens", 0.9f); },
          [](std::size_t i, long t){
            if (i != 0) return -1.0f;
            // rajadas com ataque abrupto
            return ((t % 8820) < 1500) ? seno(t, 330.0, 0.8f) : 0.0f; },
          {"onset", "trig", "level"});
    return 0;
}
