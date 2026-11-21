// FORM, TEMPO e VOLTA (21/11/2026): os critérios de pronto da proposta
// (`dossies/PROPOSTA_forma_e_andamento.md` §7) e a reversibilidade da VOLTA.

#include "dsp/Form.hpp"
#include "dsp/Tempo.hpp"
#include "dsp/Volta.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {
int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
constexpr float kSr = 8000.0f;   // taxa baixa: peças de minutos em pouco tempo
constexpr std::size_t kB = 256;

// ---- FORM ----
struct FormRun { double minDb = 0, maxDb = -200, maxGain = 0; bool ended = false, restarted = false;
                 int sections = 0, climaxes = 0; double maxStep = 0; };

FormRun runForm(Form& fm, double seconds, int layer = 2) {
    fm.prepare(kSr, kB);
    std::vector<AudioBlock> out(11, AudioBlock(kSr, 1, kB));
    AudioBlock one(kSr, 1, kB);
    for (std::size_t i = 0; i < kB; ++i) one.at(0, i) = 1.0f;   // DC 1: o ganho aparece direto
    std::vector<const AudioBlock*> in(8, nullptr);
    for (int k = 0; k < 4; ++k) in[static_cast<std::size_t>(k)] = &one;
    FormRun r; float prev = -1.0f, prevSect = 0.0f, prevClm = 0.0f; bool wasEnded = false;
    const long blocks = static_cast<long>(seconds * kSr / kB);
    for (long b = 0; b < blocks; ++b) {
        fm.process(in, out);
        for (std::size_t i = 0; i < kB; ++i) {
            const float g = out[static_cast<std::size_t>(layer)].at(0, i);
            r.maxGain = std::max<double>(r.maxGain, g);
            if (prev >= 0.0f) r.maxStep = std::max<double>(r.maxStep, std::fabs(g - prev));
            prev = g;
            if (!wasEnded && out[10].at(0, i) < 0.5f && out[9].at(0, i) < 0.5f && g > 1e-4f) {
                const double db = 20.0 * std::log10(g);
                r.minDb = std::min(r.minDb, db); r.maxDb = std::max(r.maxDb, db);
            }
            const float s = out[6].at(0, i), c = out[8].at(0, i);
            if (prevSect < 0.5f && s >= 0.5f) ++r.sections;
            if (prevClm < 0.5f && c >= 0.5f) ++r.climaxes;
            prevSect = s; prevClm = c;
        }
        if (fm.ended()) { r.ended = true; wasEnded = true; }
        else if (wasEnded) r.restarted = true;
    }
    return r;
}

void testFormArc() {
    Form fm; fm.setParameter("length", 1.0f); fm.setParameter("depth", 11.0f);
    fm.setParameter("gesture", 0.0f);
    const FormRun r = runForm(fm, 150.0);
    std::cerr << "  FORM ARCO: " << r.minDb << " .. " << r.maxDb << " dB, seções " << r.sections << '\n';
    check(r.maxGain <= 1.0 + 1e-6, "FORM: ganho nunca acima de 0 dB");
    check(std::fabs((r.maxDb - r.minDb) - 11.0) < 1.0, "FORM: vale–clímax = DEPTH (11 ± 1 dB)");
    check(r.ended, "FORM: a coda chega e END sobe");
    check(r.restarted, "FORM: LOOP recomeça depois da coda");
    check(r.climaxes >= 1, "FORM: CLMX ao chegar ao clímax");
    check(r.maxStep < 0.01, "FORM: sem degrau no ganho (rampas)");
}

void testFormGrammarsAndDeterminism() {
    for (int g = 0; g <= 4; ++g) {
        Form a; a.setParameter("grammar", float(g)); a.setParameter("length", 1.0f);
        const FormRun r = runForm(a, 150.0);
        check(r.ended && r.maxGain <= 1.0 + 1e-6, "FORM: cada gramática termina e não passa de 0 dB");
    }
    Form a, b; a.setParameter("length", 1.0f); b.setParameter("length", 1.0f);
    const FormRun ra = runForm(a, 60.0), rb = runForm(b, 60.0);
    check(ra.sections == rb.sections && ra.minDb == rb.minDb, "FORM: determinístico pela seed");
}

void testFormEssentialLayer() {
    Form f1; f1.setParameter("length", 1.0f); f1.setParameter("gesture", 0.0f);
    Form f4; f4.setParameter("length", 1.0f); f4.setParameter("gesture", 0.0f);
    const FormRun e = runForm(f1, 150.0, 0), o = runForm(f4, 150.0, 3);
    check(e.minDb > o.minDb, "FORM: a camada 1 (essencial) desce menos que a 4 (enfeite)");
}

// ---- TEMPO ----
void testTempoRitardando() {
    Tempo t; t.setParameter("mode", 0.0f); t.setParameter("final", 0.6f); t.setParameter("ritard", 4.0f);
    t.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock coda(kSr, 1, kB);
    std::vector<const AudioBlock*> in(5, nullptr); in[2] = &coda;
    double prev = 10.0; bool mono = true, ended = false;
    for (long b = 0; b < long(12 * kSr / kB); ++b) {
        const float v = b > long(2 * kSr / kB) ? 1.0f : 0.0f;
        for (std::size_t i = 0; i < kB; ++i) coda.at(0, i) = v;
        t.process(in, out);
        if (v > 0.5f && b > long(3 * kSr / kB)) {
            if (t.ratio() > prev + 1e-6) mono = false;
            prev = t.ratio();
        }
        if (out[3].at(0, kB - 1) > 0.5f) ended = true;
    }
    std::cerr << "  TEMPO ritardando chega a " << t.ratio() << '\n';
    check(mono, "TEMPO: ritardando monotônico");
    check(std::fabs(t.ratio() - 0.6) < 0.006, "TEMPO: chega a FINAL ± 1 %");
    check(ended, "TEMPO: END no último tempo");
}

void testTempoRubatoZeroMean() {
    // com CLK a 2 Hz e frase de 8 tempos: a média da razão numa frase ≈ 1
    Tempo t; t.setParameter("mode", 2.0f); t.setParameter("rubato", 1.0f);
    t.setParameter("arch_depth", 0.1f); t.setParameter("spring", 2.0f);
    t.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock clk(kSr, 1, kB);
    std::vector<const AudioBlock*> in(5, nullptr); in[4] = &clk;
    double sum = 0; long n = 0, k = 0;
    for (long b = 0; b < long(40 * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++k) clk.at(0, i) = (k % 4000) < 200 ? 1.0f : 0.0f;
        t.process(in, out);
        if (b > long(8 * kSr / kB)) { sum += t.ratio(); ++n; }
    }
    const double mean = sum / double(n);
    std::cerr << "  TEMPO média da razão " << mean << '\n';
    check(std::fabs(mean - 1.0) < 0.01, "TEMPO: arco e rubato devolvem o tempo (média ≈ 1)");
}

void testTempoFermata() {
    Tempo t; t.setParameter("mode", 0.0f); t.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock ferm(kSr, 1, kB);
    std::vector<const AudioBlock*> in(5, nullptr); in[3] = &ferm;
    bool held = false;
    for (long b = 0; b < long(6 * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i) ferm.at(0, i) = (b == long(2 * kSr / kB) && i < 40) ? 1.0f : 0.0f;
        t.process(in, out);
        if (t.ratio() < 0.1) held = true;
    }
    check(held, "TEMPO: FERM segura o relógio");
    check(std::fabs(t.ratio() - 1.0) < 0.02, "TEMPO: e volta ao andamento");
}

// ---- VOLTA ----
std::vector<float> runVolta(Volta& v, float mut, float prot = 0.0f, float ero = 0.0f, int steps = 8) {
    v.setParameter("mutate", mut); v.setParameter("protect", prot); v.setParameter("erode", ero);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, 1));
    AudioBlock clk(kSr, 1, 1);
    std::vector<const AudioBlock*> in(5, nullptr); in[0] = &clk;
    std::vector<float> seq;
    for (int s = 0; s < steps; ++s) {
        clk.at(0, 0) = 1.0f; v.process(in, out);
        seq.push_back(out[0].at(0, 0) + 10.0f * out[2].at(0, 0));
        clk.at(0, 0) = 0.0f; v.process(in, out);
    }
    return seq;
}

void testVoltaReversible() {
    Volta v; v.prepare(kSr, 1);
    const auto base = runVolta(v, 0.0f);
    const auto mutated = runVolta(v, 0.7f);
    const auto back = runVolta(v, 0.0f);
    check(base != mutated, "VOLTA: MUT transforma o laço");
    check(base == back, "VOLTA: voltar o MUT devolve o laço exato (reversível)");
    const auto fullyProtected = runVolta(v, 1.0f, 1.0f);
    check(fullyProtected == base, "VOLTA: MEM protege — passos protegidos não mudam");
    const auto eroded = runVolta(v, 0.0f, 0.0f, 1.0f);
    bool silent = true; for (float x : eroded) if (x >= 5.0f) silent = false;
    check(silent, "VOLTA: ERODE 1 cala todos os pulsos");
}
}  // namespace

int main() {
    testFormArc();
    testFormGrammarsAndDeterminism();
    testFormEssentialLayer();
    testTempoRitardando();
    testTempoRubatoZeroMean();
    testTempoFermata();
    testVoltaReversible();
    if (g_failures == 0) { std::cout << "RASGO Modular FORM/TEMPO/VOLTA tests passed\n"; return 0; }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
