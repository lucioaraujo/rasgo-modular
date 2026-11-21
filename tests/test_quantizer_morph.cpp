// A máquina de escalas do QUANTIZER (21/11/2026): MRPH 0 é o QUANTIZER de
// antes; MRPH 1 é a escala B; no meio aparecem alturas entre as duas.

#include "dsp/Quantizer.hpp"

#include <cmath>
#include <iostream>
#include <set>
#include <vector>

using namespace rasgo::modular;

namespace {
int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}

// varre a CV de −1 a 1 e devolve as alturas (semitons, ×1000 arredondado) que saem
std::set<long> sweep(Quantizer& q) {
    q.prepare(48000.0f, 1);
    AudioBlock cv(48000.0f, 1, 1);
    std::vector<AudioBlock> out(3, AudioBlock(48000.0f, 1, 1));
    std::vector<const AudioBlock*> in(6, nullptr); in[0] = &cv;
    std::set<long> s;
    for (int i = -400; i <= 400; ++i) {
        cv.at(0, 0) = static_cast<float>(i) / 400.0f;
        q.process(in, out);
        s.insert(std::lround(out[0].at(0, 0) * 12.0f * 1000.0f));
    }
    return s;
}
std::set<long> pitchClasses(const std::set<long>& s) {
    std::set<long> pc;
    for (long v : s) pc.insert(((v % 12000) + 12000) % 12000);
    return pc;
}
}  // namespace

int main() {
    Quantizer a; a.setParameter("scale", 1.0f); a.setParameter("hysteresis", 0.0f);   // maior
    Quantizer a0 = a; a0.setParameter("scale_b", 7.0f); a0.setParameter("morph", 0.0f);
    check(sweep(a) == sweep(a0), "MRPH 0: o QUANTIZER de antes");

    Quantizer b1 = a; b1.setParameter("scale_b", 7.0f); b1.setParameter("morph", 1.0f);  // pentatônica menor
    Quantizer pent; pent.setParameter("scale", 7.0f); pent.setParameter("hysteresis", 0.0f);
    check(pitchClasses(sweep(b1)) == pitchClasses(sweep(pent)), "MRPH 1: as classes de altura da escala B");

    Quantizer mid = a; mid.setParameter("scale_b", 7.0f); mid.setParameter("morph", 0.5f);
    bool micro = false;
    for (long v : sweep(mid)) if (v % 1000 != 0) micro = true;
    check(micro, "MRPH 0,5: alturas entre as duas escalas (microtons)");

    if (g_failures == 0) { std::cout << "RASGO Modular QUANTIZER morph tests passed\n"; return 0; }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
