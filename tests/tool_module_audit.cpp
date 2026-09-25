// AUDITORIA DE TODOS OS MÓDULOS: item sem função ou bugado.
//
// Pedido do autor em 25 set. 2026. Faz três perguntas por medição, e não
// por leitura:
//
//   1. cada PARÂMETRO muda a saída quando é mexido? (senão: sem função)
//   2. cada PORTA DE SAÍDA produz algo? (senão: porta morta)
//   3. a saída é sempre finita, sem NaN/Inf? (senão: bugado)
//
// Excita cada módulo de forma genérica: ruído nas entradas de áudio e
// um pulso/rampa nas de controle. Um módulo que precise de excitação
// específica pode aparecer como falso positivo — por isso o relatório
// separa "sem efeito" de "sem saída", e não conclui nada sozinho.
#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include <cmath>
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
using namespace rasgo::modular;

constexpr float kSr = 44100.0f;
constexpr std::size_t kB = 256;

// Guarda a FORMA DE ONDA, não só o RMS.
//
// A primeira versão comparava RMS e acusou 108 parâmetros "sem efeito" —
// mas RMS não vê TIMBRE. `OSC.pw`, `MATTER.structure`, `STRING.position`
// e todos os `drift` mudam o som sem mudar a energia, e apareciam como
// sem função estando perfeitamente funcionais. Como a excitação é
// determinística, comparar amostra a amostra responde a pergunta certa:
// mexer neste parâmetro muda ALGUMA COISA na saída?
struct Saida {
    std::vector<float> onda[8];
    double rms[8];
    bool finito;
};

// roda o módulo com um valor de parâmetro e devolve o RMS de cada porta
// `comEntradas = false` deixa TODAS as entradas nulas (desconectadas).
//
// Existe porque a primeira passada acusou todos os `*.rate` e o
// `CLOCK.bpm` como "sem efeito" — e eles estavam certos: eu injetava um
// pulso na entrada de clock, e clock EXTERNO deve sobrepor o interno.
// Comportamento correto sendo acusado de defeito. Um parâmetro é
// funcional se age COM ou SEM entradas.
Saida rodar(const char* tipo, const std::string& par, float valor,
            bool usarPar, bool comEntradas = true) {
    auto n = rasgo::panel::makeModule(tipo);
    Saida r{}; r.finito = true;
    if (!n) return r;
    if (usarPar) { try { n->setParameter(par.c_str(), valor); } catch (...) {} }
    n->prepare(kSr, kB);

    const std::size_t ni = n->inputCount(), no = n->outputCount();
    std::vector<AudioBlock> in(ni ? ni : 1, AudioBlock(kSr, 2, kB));
    std::vector<AudioBlock> out(no ? no : 1, AudioBlock(kSr, 2, kB));
    std::vector<const AudioBlock*> ins;
    for (std::size_t i = 0; i < ni; ++i)
        ins.push_back(comEntradas ? &in[i] : nullptr);

    std::uint64_t rs = 99991; double ph = 0;
    double acc[8] = {0}; long cnt = 0;
    for (int b = 0; b < 400; ++b) {
        for (std::size_t i = 0; i < ni; ++i) {
            const bool ctrl = n->inputDescriptor(i).kind == PortKind::Control;
            for (std::size_t k = 0; k < kB; ++k) {
                float v;
                if (ctrl) {
                    // pulso periódico + rampa lenta: serve de gate, clock e CV
                    const long t = (long)b*(long)kB + (long)k;
                    v = ((t % 11025) < 1200) ? 1.0f : 0.0f;
                    if (i % 2) v = 0.5f * std::sin(2*M_PI*0.7*t/kSr);
                } else {
                    rs ^= rs<<13; rs ^= rs>>7; rs ^= rs<<17;
                    v = 0.35f * (2.0f*((float)(rs>>40)/16777216.0f) - 1.0f)
                      + 0.35f * std::sin(ph);
                    ph += 2*M_PI*220.0/kSr;
                }
                in[i].at(0,k) = v; in[i].at(1,k) = v;
            }
        }
        n->process(ins, out);
        if (b < 150) continue;                      // deixa assentar
        for (std::size_t p = 0; p < no && p < 8; ++p)
            for (std::size_t k = 0; k < kB; ++k) {
                const double s = out[p].at(0,k);
                if (!std::isfinite(s)) r.finito = false;
                acc[p] += s*s;
                r.onda[p].push_back((float)s);
            }
        cnt += (long)kB;
    }
    for (std::size_t p = 0; p < no && p < 8; ++p)
        r.rms[p] = std::sqrt(acc[p]/std::max(1L,cnt));
    return r;
}

int main() {
    int semEfeito = 0, semSaida = 0, naoFinito = 0, totalPar = 0, totalPortas = 0;
    std::vector<std::string> listaSemEfeito, listaSemSaida, listaNaoFinito;

    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types) {
            auto proto = rasgo::panel::makeModule(t);
            if (!proto) continue;

            // 3. finitude e 2. portas mortas, no estado padrão
            const Saida base = rodar(t, "", 0.0f, false);
            if (!base.finito) { ++naoFinito; listaNaoFinito.push_back(t); }
            for (std::size_t p = 0; p < proto->outputCount() && p < 8; ++p) {
                ++totalPortas;
                if (base.rms[p] < 1e-9) {
                    ++semSaida;
                    listaSemSaida.push_back(std::string(t) + "." +
                        proto->outputDescriptor(p).name);
                }
            }

            // 1. cada parâmetro muda algo?
            for (const auto& pr : proto->parameters()) {
                ++totalPar;
                const float lo = pr.descriptor.minimum, hi = pr.descriptor.maximum;
                // duas condições: com e sem entradas
                double melhorDif = 0.0, melhorMaior = 0.0;
                for (const bool comEnt : {true, false}) {
                    const Saida a = rodar(t, pr.descriptor.id, lo, true, comEnt);
                    const Saida b = rodar(t, pr.descriptor.id, hi, true, comEnt);
                    if (!a.finito || !b.finito) {
                        const std::string nm = std::string(t) + "." + pr.descriptor.id;
                        bool tem = false;
                        for (const auto& x : listaNaoFinito) if (x == nm) tem = true;
                        if (!tem) { ++naoFinito; listaNaoFinito.push_back(nm); }
                    }
                    double d = 0, mx = 0;
                    for (std::size_t p = 0; p < proto->outputCount() && p < 8; ++p) {
                        mx = std::max(mx, std::max(a.rms[p], b.rms[p]));
                        const std::size_t n2 =
                            std::min(a.onda[p].size(), b.onda[p].size());
                        for (std::size_t k = 0; k < n2; ++k)
                            d = std::max(d, (double)std::fabs(a.onda[p][k]-b.onda[p][k]));
                    }
                    // guarda a condição em que o parâmetro se mostrou MAIS
                    if (mx > 1e-9 && (melhorMaior <= 1e-9 || d/mx > melhorDif/std::max(melhorMaior,1e-30))) {
                        melhorDif = d; melhorMaior = mx;
                    }
                }
                const Saida a{}, b{};
                (void)a; (void)b;
                const double dif = melhorDif, maior = melhorMaior;
                if (maior > 1e-9 && dif / maior < 0.001) {
                    ++semEfeito;
                    listaSemEfeito.push_back(std::string(t) + "." + pr.descriptor.id);
                } else if (maior <= 1e-9) {
                    // módulo mudo nos dois extremos: já contado como porta morta
                }

            }
        }

    std::printf("AUDITORIA DOS MÓDULOS\n");
    std::printf("  %d parâmetros · %d portas de saída\n\n", totalPar, totalPortas);
    std::printf("  NÃO-FINITO (NaN/Inf)      : %d\n", naoFinito);
    std::printf("  portas SEM SAÍDA          : %d\n", semSaida);
    std::printf("  parâmetros SEM EFEITO     : %d\n\n", semEfeito);

    auto lista = [](const char* t, const std::vector<std::string>& v, int max) {
        if (v.empty()) return;
        std::printf("=== %s ===\n", t);
        int i = 0;
        for (const auto& s : v) {
            if (i++ >= max) { std::printf("  … e mais %d\n", (int)v.size()-max); break; }
            std::printf("  %s\n", s.c_str());
        }
        std::printf("\n");
    };
    lista("NÃO-FINITO — o mais grave", listaNaoFinito, 40);
    lista("PORTAS SEM SAÍDA", listaSemSaida, 40);
    lista("PARÂMETROS SEM EFEITO", listaSemEfeito, 60);
    return 0;
}
