// Exame CASO A CASO dos 19 itens que a triagem deixou em aberto.
//
// A triagem usava excitação genérica, e isso produz falso positivo de
// duas formas: o módulo não recebe o estímulo que ele espera (o gate de
// 27 ms que escondia `ENVELOPE.decay`), ou o parâmetro depende de um
// HABILITADOR que está em zero (a frequência de uma banda de EQ com ganho
// 0 dB). Aqui cada caso recebe o que precisa.
#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>
using namespace rasgo::modular;

constexpr float kSr = 44100.0f;
constexpr std::size_t kB = 256;

using Prep  = std::function<void(Signal&)>;
using Excit = std::function<float(std::size_t porta, PortKind, long t)>;

// devolve a forma de onda de todas as portas
std::vector<std::vector<float>> rodar(const char* tipo, const Prep& prep,
                                      const Excit& ex, const std::string& par,
                                      float valor, int blocos = 900) {
    auto n = rasgo::panel::makeModule(tipo);
    std::vector<std::vector<float>> r;
    if (!n) return r;
    prep(*n);
    try { n->setParameter(par.c_str(), valor); } catch (...) {}
    n->prepare(kSr, kB);
    const std::size_t ni = n->inputCount(), no = n->outputCount();
    std::vector<AudioBlock> in(ni ? ni : 1, AudioBlock(kSr, 2, kB));
    std::vector<AudioBlock> out(no ? no : 1, AudioBlock(kSr, 2, kB));
    std::vector<const AudioBlock*> ins;
    for (std::size_t i = 0; i < ni; ++i) ins.push_back(&in[i]);
    r.resize(no);
    for (int b = 0; b < blocos; ++b) {
        for (std::size_t i = 0; i < ni; ++i)
            for (std::size_t k = 0; k < kB; ++k) {
                const float v = ex(i, n->inputDescriptor(i).kind,
                                   (long)b*(long)kB + (long)k);
                in[i].at(0,k) = v; in[i].at(1,k) = v;
            }
        n->process(ins, out);
        if (b < 200) continue;                     // assenta
        for (std::size_t p = 0; p < no; ++p)
            for (std::size_t k = 0; k < kB; ++k) r[p].push_back(out[p].at(0,k));
    }
    return r;
}

// Diferença relativa entre os extremos REAIS do parâmetro.
//
// A primeira versão deste exame usava 0..1 para todos os casos, e isso
// invalidava parte do resultado: `HARMONY.scale_hi` vai de 1 a 10, então
// comparar 0 com 1 compara o MESMO valor depois do clamp interno, e
// `SWITCH.mode` vai de 0 a 3. Agora a faixa vem do descritor do próprio
// parâmetro — a única que significa algo.
double examinar(const char* tipo, const Prep& prep, const Excit& ex,
                const std::string& par, float loIgn, float hiIgn) {
    float lo = loIgn, hi = hiIgn;
    if (auto proto = rasgo::panel::makeModule(tipo))
        for (const auto& pr : proto->parameters())
            if (pr.descriptor.id == par) {
                lo = pr.descriptor.minimum; hi = pr.descriptor.maximum;
            }
    const auto a = rodar(tipo, prep, ex, par, lo);
    const auto b = rodar(tipo, prep, ex, par, hi);
    double dif = 0, nivel = 0;
    for (std::size_t p = 0; p < std::min(a.size(), b.size()); ++p) {
        const std::size_t n2 = std::min(a[p].size(), b[p].size());
        for (std::size_t k = 0; k < n2; ++k) {
            dif = std::max(dif, (double)std::fabs(a[p][k] - b[p][k]));
            nivel = std::max(nivel, (double)std::fabs(a[p][k]));
            nivel = std::max(nivel, (double)std::fabs(b[p][k]));
        }
    }
    return nivel > 1e-9 ? dif / nivel : -1.0;
}

// ---- excitações ------------------------------------------------------
float pulso(long t, double hz) {   // trem de gates de 30 ms
    const long per = (long)(kSr/hz);
    return ((t % per) < (long)(kSr*0.03)) ? 1.0f : 0.0f;
}
float rampa(long t, double hz) {   // rampa 0..1 lenta
    const long per = (long)(kSr/hz);
    return (float)((t % per) / (double)per);
}
float senoide(long t, double hz, float amp = 0.5f) {
    return amp * (float)std::sin(2*M_PI*hz*t/kSr);
}

int main() {
    struct Caso { const char* mod; const char* par; float lo, hi;
                  Prep prep; Excit ex; const char* nota; };
    const Excit nada = [](std::size_t, PortKind, long){ return 0.0f; };

    std::vector<Caso> casos = {
      // PLL.feedback_type: só tem sentido com feedback_amount > 0
      {"PLL","feedback_type",0.0f,1.0f,
       [](Signal& s){ s.setParameter("feedback_amount", 0.8f);
                      s.setParameter("freq", 220.0f); },
       [](std::size_t i, PortKind, long t){ return i==0 ? senoide(t,110.0) : 0.0f; },
       "com feedback_amount=0,8"},

      // CONTROL.curveN: precisa de scaleN != 0 e de CV varrendo a faixa
      {"CONTROL","curve1",0.0f,1.0f,
       [](Signal& s){ s.setParameter("scale1", 1.0f); s.setParameter("slew1", 0.45f); },
       [](std::size_t i, PortKind, long t){ return i==0 ? rampa(t,0.5) : 0.0f; },
       "com scale1=1, slew1=0,45 e CV em rampa"},
      {"CONTROL","curve2",0.0f,1.0f,
       [](Signal& s){ s.setParameter("scale2", 1.0f); s.setParameter("slew2", 0.45f); },
       [](std::size_t i, PortKind, long t){ return i==1 ? rampa(t,0.5) : 0.0f; },
       "com scale2=1, slew2=0,45 e CV em rampa"},

      // SH: gatilho + sinal para amostrar
      {"SH","slope",0.0f,1.0f,
       [](Signal& s){ s.setParameter("slew1", 0.5f); s.setParameter("slew2", 0.5f); },
       [](std::size_t i, PortKind, long t){
          if (i==0) return senoide(t,3.0);
          if (i==1) return pulso(t,7.0);
          return 0.0f; }, "com slew1/2=0,5 (slope molda o slew)"},
      {"SH","track1",0.0f,1.0f,
       [](Signal&){},
       [](std::size_t i, PortKind, long t){
          if (i==0) return senoide(t,3.0);
          if (i==1) return pulso(t,7.0);
          return 0.0f; }, "idem"},
      {"SH","track2",0.0f,1.0f,
       [](Signal&){},
       [](std::size_t i, PortKind, long t){
          if (i==2) return senoide(t,3.0);
          if (i==3) return pulso(t,7.0);
          return 0.0f; }, "no par 2 (in2/trig2)"},

      // TRIGSEQ: clock externo
      {"TRIGSEQ","density1",0.0f,1.0f,[](Signal&){},
       [](std::size_t i, PortKind, long t){ return i==0 ? pulso(t,8.0) : 0.0f; },
       "com clock a 8 Hz"},
      {"TRIGSEQ","density2",0.0f,1.0f,[](Signal&){},
       [](std::size_t i, PortKind, long t){ return i==0 ? pulso(t,8.0) : 0.0f; },
       "idem"},
      {"TRIGSEQ","swing",0.0f,1.0f,[](Signal&){},
       [](std::size_t i, PortKind, long t){ return i==0 ? pulso(t,8.0) : 0.0f; },
       "idem"},

      // QUANTIZER.hysteresis: CV pairando numa fronteira de escala
      {"QUANTIZER","hysteresis",0.0f,1.0f,
       [](Signal& s){ s.setParameter("glide", 0.0f); },
       [](std::size_t i, PortKind, long t){
          // rampa lenta + tremor cruzando fronteiras, E gatilho: o
          // QUANTIZER amostra o CV quando é disparado
          if (i==0) return rampa(t,0.2) + 0.004f*senoide(t,37.0,1.0f);
          if (i==2) return pulso(t,12.0);
          return 0.0f; },
       "com CV cruzando fronteiras + GATILHO"},

      // HARMONY.scale_hi: precisa avançar
      {"HARMONY","scale_hi",0.0f,1.0f,
       [](Signal& s){ s.setParameter("movement", 0.8f); s.setParameter("hold", 0.0f); },
       [](std::size_t i, PortKind, long t){ return i==0 ? pulso(t,4.0) : 0.0f; },
       "com advance a 4 Hz e movement=0,8"},

      // BOXCAR: sinal + gatilho
      {"BOXCAR","delay",0.0f,1.0f,
       [](Signal& s){ s.setParameter("blend", 1.0f); },
       [](std::size_t i, PortKind, long t){
          if (i==0) return senoide(t,220.0);
          if (i==1) return pulso(t,5.0);
          return 0.0f; }, "com blend=1"},
      {"BOXCAR","thresh",0.0f,1.0f,
       [](Signal& s){ s.setParameter("geiger", 1.0f); },
       [](std::size_t i, PortKind, long t){
          if (i==0) return senoide(t,220.0, 0.3f);
          return 0.0f; }, "com geiger=1"},

      // SWITCH.mode: 4 fontes distintas + clock
      {"SWITCH","mode",0.0f,1.0f,
       [](Signal& s){ s.setParameter("steps", 4.0f); s.setParameter("glide", 0.0f); },
       [](std::size_t i, PortKind, long t){
          if (i<4) return senoide(t, 110.0*(double)(i+1));
          if (i==4) return pulso(t,6.0);
          return 0.0f; }, "4 fontes + clock"},

      // MATRIX: várias entradas e ganhos cruzados
      {"MATRIX","norm",0.0f,1.0f,
       [](Signal& s){ for (const char* g : {"g11","g12","g21","g22"})
                          s.setParameter(g, 0.8f); },
       [](std::size_t i, PortKind, long t){
          return i<4 ? senoide(t, 110.0*(double)(i+1)) : 0.0f; },
       "com 4 ganhos cruzados em 0,8"},
      {"MATRIX","ring",0.0f,1.0f,
       [](Signal& s){ for (const char* g : {"g11","g12","g21","g22"})
                          s.setParameter(g, 0.8f); },
       [](std::size_t i, PortKind, long t){
          return i<4 ? senoide(t, 110.0*(double)(i+1)) : 0.0f; },
       "idem"},

      // SCOPE.trigger: é MEDIDOR — afeta a leitura, não o áudio
      {"SCOPE","trigger",-0.9f,0.9f,
       [](Signal& s){ s.setParameter("sens", 0.8f); },
       [](std::size_t i, PortKind, long t){
          return i==0 ? senoide(t,220.0) : 0.0f; },
       "faixa real -1..1, sinal 0,5"},
    };

    std::printf("%-12s %-14s %10s  %s\n", "MÓDULO", "parâmetro", "diferença", "excitação");
    int mudos = 0;
    for (const auto& c : casos) {
        const double d = examinar(c.mod, c.prep, c.ex, c.par, c.lo, c.hi);
        const char* v = d < 0 ? "SEM SINAL" : (d < 0.001 ? "SEM EFEITO" : "ok");
        if (d < 0.001) ++mudos;
        std::printf("%-12s %-14s %9.4f  %-10s %s\n",
                    c.mod, c.par, d < 0 ? 0.0 : d, v, c.nota);
    }
    std::printf("\n%d de %zu continuam sem efeito com excitação própria\n",
                mudos, casos.size());
    return 0;
}
