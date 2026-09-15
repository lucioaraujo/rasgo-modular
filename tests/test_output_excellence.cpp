// Bateria de casos-limite da saída — `RASGO_DOCUMENTATION/architecture/
// SAIDA_AUDIO_COMUM.md §5`, a mesma lista que Navalha 2, Antitotem e
// Rasgo Synth seguem.
//
// O que está aqui é a parte AUTOMATIZÁVEL daquele §5: impulso, DC,
// silêncio, subgrave, senos perto de Nyquist, NaN/Inf, blocos irregulares,
// troca de sample rate no meio, automação rápida sem clique, e o downmix
// mono. Não substitui escuta — o próprio documento diz que assinatura
// dourada não substitui aprovação auditiva humana. O que ele faz é
// garantir que nenhuma dessas entradas produza algo que não deveria
// chegar a um alto-falante.
//
// Fora do escopo deste arquivo, e registrado como pendente:
// medição BS.1770/LUFS, taps nomeados de gravação e exportação
// PCM24/float.

#include "core/SignalGraph.hpp"
#include "dsp/Master.hpp"
#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const what) {
    if (!c) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr float kCeil = rasgo::panel::SinkOut::kCeiling;

// A cadeia real de saída: MASTER (proteção de excelência) seguido do sink
// (guarda de segurança). É este par que chega ao dispositivo.
struct Chain {
    Master master;
    rasgo::panel::SinkOut sink;
    std::vector<AudioBlock> mOut{2, AudioBlock(kSr, 2, 64)};
    std::vector<AudioBlock> sOut{1, AudioBlock(kSr, 2, 64)};

    void prepare(const float sr, const std::size_t frames) {
        master.prepare(sr, frames);
        sink.prepare(sr, frames);
        mOut.assign(2, AudioBlock(sr, 2, frames));
        sOut.assign(1, AudioBlock(sr, 2, frames));
    }
    // devolve o bloco final (o que iria pro dispositivo)
    const AudioBlock& run(const AudioBlock& in) {
        std::vector<const AudioBlock*> mi{&in};
        master.process(mi, mOut);
        std::vector<const AudioBlock*> si{&mOut[0]};
        sink.process(si, sOut);
        return sOut[0];
    }
};

struct Stats { float peak = 0.0f; float maxStep = 0.0f; bool finite = true; };

Stats inspect(const AudioBlock& b, float* carryLast = nullptr) {
    Stats s;
    float last = carryLast ? *carryLast : b.at(0, 0);
    for (std::size_t c = 0; c < b.channels(); ++c)
        for (std::size_t i = 0; i < b.frames(); ++i) {
            const float v = b.at(c, i);
            if (!std::isfinite(v)) { s.finite = false; continue; }
            s.peak = std::max(s.peak, std::fabs(v));
            if (c == 0) {
                s.maxStep = std::max(s.maxStep, std::fabs(v - last));
                last = v;
            }
        }
    if (carryLast) *carryLast = last;
    return s;
}

void fill(AudioBlock& b, const float l, const float r) {
    for (std::size_t i = 0; i < b.frames(); ++i) { b.at(0, i) = l; b.at(1, i) = r; }
}

// ---- 1. sinais patológicos -------------------------------------------------

void testPathologicalInputs() {
    struct Case { const char* name; float l, r; };
    const Case cases[] = {
        {"silêncio",        0.0f,   0.0f},
        {"DC positivo",     0.9f,   0.9f},
        {"DC negativo",    -0.9f,  -0.9f},
        {"muito alto",     40.0f, -40.0f},
        {"denormal",        1.0e-38f, -1.0e-38f},
    };
    for (const auto& k : cases) {
        Chain ch;
        ch.prepare(kSr, 64);
        AudioBlock in(kSr, 2, 64);
        fill(in, k.l, k.r);
        Stats st;
        for (int n = 0; n < 32; ++n) st = inspect(ch.run(in));
        check(st.finite, k.name);
        check(st.peak <= kCeil + 1.0e-6f, k.name);
    }
}

void testImpulse() {
    Chain ch;
    ch.prepare(kSr, 64);
    AudioBlock in(kSr, 2, 64);
    bool ok = true;
    for (int n = 0; n < 16; ++n) {
        fill(in, 0.0f, 0.0f);
        if (n == 4) { in.at(0, 0) = 1.0f; in.at(1, 0) = -1.0f; }  // impulso
        const Stats st = inspect(ch.run(in));
        if (!st.finite || st.peak > kCeil + 1.0e-6f) ok = false;
    }
    check(ok, "impulso não estoura nem contamina os blocos seguintes");
}

// Subgrave e senos junto de Nyquist: as duas pontas onde filtro de DC e
// limitador costumam se comportar mal.
void testExtremeFrequencies() {
    const float freqs[] = {5.0f, 20.0f, kSr * 0.45f, kSr * 0.499f};
    for (const float f : freqs) {
        Chain ch;
        ch.prepare(kSr, 64);
        ch.master.setParameter("gain", 0.0f);
        AudioBlock in(kSr, 2, 64);
        double ph = 0.0;
        const double dp = 2.0 * M_PI * static_cast<double>(f) / kSr;
        Stats st;
        for (int n = 0; n < 64; ++n) {
            for (std::size_t i = 0; i < in.frames(); ++i, ph += dp) {
                const float v = 0.95f * static_cast<float>(std::sin(ph));
                in.at(0, i) = v;
                in.at(1, i) = -v;      // antifase: pior caso pro mono
            }
            st = inspect(ch.run(in));
        }
        check(st.finite, "seno extremo: saída finita");
        check(st.peak <= kCeil + 1.0e-6f, "seno extremo: dentro do teto");
    }
}

void testNonFiniteNeverEscapes() {
    Chain ch;
    ch.prepare(kSr, 64);
    AudioBlock in(kSr, 2, 64);
    fill(in, 0.3f, 0.3f);
    in.at(0, 5) = std::numeric_limits<float>::quiet_NaN();
    in.at(1, 9) = std::numeric_limits<float>::infinity();
    in.at(0, 20) = -std::numeric_limits<float>::infinity();

    bool ok = true;
    for (int n = 0; n < 8; ++n) {
        const Stats st = inspect(ch.run(in));
        if (!st.finite || st.peak > kCeil + 1.0e-6f) ok = false;
        fill(in, 0.3f, 0.3f);   // só o primeiro bloco tem lixo
    }
    check(ok, "NaN/Inf não escapam nem envenenam os blocos seguintes");
}

// ---- 2. blocos irregulares e troca de sample rate --------------------------

void testIrregularBlockSizes() {
    Chain ch;
    const std::size_t sizes[] = {1, 7, 64, 3, 128, 33, 256, 2};
    bool ok = true;
    for (const std::size_t n : sizes) {
        ch.prepare(kSr, n);
        AudioBlock in(kSr, 2, n);
        fill(in, 0.5f, -0.5f);
        const Stats st = inspect(ch.run(in));
        if (!st.finite || st.peak > kCeil + 1.0e-6f) ok = false;
    }
    check(ok, "blocos de tamanho irregular não quebram a cadeia");
}

void testSampleRateChange() {
    const float rates[] = {44100.0f, 48000.0f, 96000.0f, 22050.0f, 192000.0f};
    Chain ch;
    bool ok = true;
    for (const float sr : rates) {
        ch.prepare(sr, 64);
        AudioBlock in(sr, 2, 64);
        fill(in, 0.7f, -0.7f);
        Stats st;
        for (int n = 0; n < 16; ++n) st = inspect(ch.run(in));
        if (!st.finite || st.peak > kCeil + 1.0e-6f) ok = false;
    }
    check(ok, "trocar a taxa de amostragem não deixa estado inválido");
}

// ---- 3. automação rápida sem clique ----------------------------------------
//
// O critério é o DEGRAU entre amostras vizinhas. Um parâmetro trocado de
// um extremo ao outro entre blocos não pode virar descontinuidade na onda:
// é isso que se ouve como clique.

void testFastAutomationNoClick(const char* param, float a, float b,
                               const char* what) {
    Chain ch;
    ch.prepare(kSr, 64);
    ch.master.setParameter("gain", 0.0f);
    AudioBlock in(kSr, 2, 64);
    double ph = 0.0;
    const double dp = 2.0 * M_PI * 220.0 / kSr;
    float last = 0.0f;
    float worst = 0.0f;
    for (int n = 0; n < 48; ++n) {
        ch.master.setParameter(param, (n % 2) ? a : b);   // extremo a extremo
        for (std::size_t i = 0; i < in.frames(); ++i, ph += dp) {
            const float v = 0.5f * static_cast<float>(std::sin(ph));
            in.at(0, i) = v;
            in.at(1, i) = v * 0.5f;
        }
        if (n > 2) worst = std::max(worst, inspect(ch.run(in), &last).maxStep);
        else       inspect(ch.run(in), &last);
    }
    // referência: o próprio seno anda ~0,014 por amostra a 220 Hz. Um
    // degrau de meia escala seria ~0,5. O limite de 0,12 pega o degrau
    // duro sem exigir uma rampa mais lenta do que o instrumento usa.
    check(worst < 0.12f, what);
}

void testAutomation() {
    testFastAutomationNoClick("mute", 0.0f, 1.0f, "MUTE alterna sem clique");
    testFastAutomationNoClick("gain", -60.0f, 0.0f, "GAIN varre sem clique");
    testFastAutomationNoClick("width", 0.0f, 2.0f, "WIDTH varre sem clique");
}

// ---- 4. downmix mono e correlação ------------------------------------------

void testMonoDownmix() {
    // Um par em ANTIFASE somado a mono cancela — é física, não bug. O que
    // o instrumento precisa garantir é que o botão MONO faça o somatório
    // ele mesmo, de forma previsível, em vez de deixar a surpresa pra
    // sala do ouvinte.
    Chain ch;
    ch.prepare(kSr, 64);
    ch.master.setParameter("gain", 0.0f);
    ch.master.setParameter("dc_block", 0.0f);
    ch.master.setParameter("mono", 1.0f);
    AudioBlock in(kSr, 2, 64);
    fill(in, 0.5f, -0.5f);                 // antifase perfeita
    const AudioBlock* out = nullptr;
    for (int n = 0; n < 24; ++n) out = &ch.run(in);
    check(std::fabs(out->at(0, 32)) < 0.02f,
          "MONO soma a antifase e o resultado é silêncio previsível");
    check(std::fabs(out->at(0, 32) - out->at(1, 32)) < 1.0e-5f,
          "MONO deixa os dois canais idênticos");

    // E em fase, MONO não pode atenuar o conteúdo comum
    Chain ch2;
    ch2.prepare(kSr, 64);
    ch2.master.setParameter("gain", 0.0f);
    ch2.master.setParameter("dc_block", 0.0f);
    ch2.master.setParameter("mono", 1.0f);
    AudioBlock in2(kSr, 2, 64);
    fill(in2, 0.4f, 0.4f);
    const AudioBlock* o2 = nullptr;
    for (int n = 0; n < 24; ++n) o2 = &ch2.run(in2);
    check(std::fabs(o2->at(0, 32) - 0.4f) < 0.02f,
          "MONO não atenua o que já era comum aos dois canais");
}

}  // namespace

int main() {
    testPathologicalInputs();
    testImpulse();
    testExtremeFrequencies();
    testNonFiniteNeverEscapes();
    testIrregularBlockSizes();
    testSampleRateChange();
    testAutomation();
    testMonoDownmix();
    if (g_failures == 0) std::puts("test_output_excellence: OK");
    return g_failures == 0 ? 0 : 1;
}
