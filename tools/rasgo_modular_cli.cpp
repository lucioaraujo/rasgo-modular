// rasgo_modular_cli — o Rasgo Modular pela linha de comando, sem janela nem
// placa de som (21/11/2026; ideia registrada no TAREFAS depois do comando
// `infinite` do Infinite, e "faça as implementações que faltam").
//
//   rasgo_modular_cli render --seed N [--dur S] [--sr HZ] saida.wav
//   rasgo_modular_cli render-patch patch.rmp [--dur S] [--sr HZ] saida.wav
//   rasgo_modular_cli validate patch.rmp
//   rasgo_modular_cli info [TIPO]
//   rasgo_modular_cli demo-patch TIPO,TIPO,… saida.rmp
//
// `render` faz o mesmo que o app ao abrir uma seed: o rack de fábrica, a
// seed (`seedPatch`) e o equilíbrio de volume (`balanceSeedLevel`); depois
// toca S segundos para um WAV de 24 bits. Sem o VARIA (que no app anda pela
// interface) — é a seed como ela nasce, determinística: a mesma seed dá o
// mesmo arquivo. `render-patch` toca um `.rmp` salvo como ele está.
// O teto de saída é o do `SinkOut` (−1 dBFS), como na régua das seeds.

#include "core/SignalGraph.hpp"
#include "escalas/BancoEscalas.hpp"
#include "io/WavWriter.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace rasgo::modular;

namespace {

int usage() {
    std::fprintf(stderr,
        "rasgo_modular_cli — Rasgo Modular sem janela\n\n"
        "  render --seed N [--dur S] [--sr HZ] saida.wav   uma seed para WAV (24 bits)\n"
        "  render-patch patch.rmp [--dur S] [--sr HZ] saida.wav\n"
        "  validate patch.rmp                              confere um patch salvo\n"
        "  info [TIPO]                                     os módulos; com TIPO, portas e parâmetros\n"
        "  demo-patch T1,T2,… saida.rmp                    patch de demonstração para os painéis\n");
    return 2;
}

std::unique_ptr<Signal> factory(const std::string& t) {
    if (t == "OUT") return std::make_unique<rasgo::panel::SinkOut>();
    return rasgo::panel::makeModule(t);
}

bool readFile(const std::string& path, std::string& text) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream ss; ss << in.rdbuf(); text = ss.str();
    return true;
}

std::size_t findSink(SignalGraph& g) {
    for (std::size_t i = 0; i < g.nodeCount(); ++i)
        if (g.node(i).type() == "OUT") return i;
    return static_cast<std::size_t>(-1);
}

int renderGraph(SignalGraph& g, const std::size_t sink, const double seconds,
                const float sr, const std::string& out) {
    constexpr std::size_t kB = 256;
    AudioBlock block(sr, 2, kB);
    std::vector<float> pcm;
    const long blocks = static_cast<long>(std::ceil(seconds * sr / kB));
    pcm.reserve(static_cast<std::size_t>(blocks) * kB * 2);
    double peak = 0.0;
    for (long b = 0; b < blocks; ++b) {
        g.process(block, sink, 0);
        for (std::size_t i = 0; i < kB; ++i) {
            float l = block.at(0, i), r = block.at(1, i);
            if (!std::isfinite(l)) l = 0.0f;
            if (!std::isfinite(r)) r = 0.0f;
            pcm.push_back(l); pcm.push_back(r);
            peak = std::max(peak, static_cast<double>(std::max(std::fabs(l), std::fabs(r))));
        }
    }
    if (!writeWav24(out, pcm, static_cast<std::uint32_t>(sr), 2)) {
        std::fprintf(stderr, "não consegui escrever %s\n", out.c_str());
        return 1;
    }
    std::printf("%s: %.1f s, %.0f Hz, pico %.1f dBFS\n", out.c_str(), seconds, sr,
                20.0 * std::log10(std::max(1e-9, peak)));
    return 0;
}

struct Opts { double dur = 30.0; float sr = 48000.0f; std::uint64_t seed = 0; bool hasSeed = false;
              std::vector<std::string> pos; bool ok = true; };

Opts parse(int argc, char** argv, int from) {
    Opts o;
    for (int i = from; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&](const char* what) -> const char* {
            if (i + 1 >= argc) { std::fprintf(stderr, "falta o valor de %s\n", what); o.ok = false; return "0"; }
            return argv[++i];
        };
        if (a == "--dur") o.dur = std::atof(next("--dur"));
        else if (a == "--sr") o.sr = static_cast<float>(std::atof(next("--sr")));
        else if (a == "--seed") { o.seed = std::strtoull(next("--seed"), nullptr, 10); o.hasSeed = true; }
        else o.pos.push_back(a);
    }
    if (!(o.dur > 0.0 && o.dur <= 3600.0)) { std::fprintf(stderr, "--dur entre 0 e 3600 s\n"); o.ok = false; }
    if (!(o.sr >= 22050.0f && o.sr <= 192000.0f)) { std::fprintf(stderr, "--sr entre 22050 e 192000\n"); o.ok = false; }
    return o;
}

int cmdRender(int argc, char** argv) {
    const Opts o = parse(argc, argv, 2);
    if (!o.ok || !o.hasSeed || o.seed == 0 || o.pos.size() != 1) return usage();
    SignalGraph g;
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g.add(std::move(n));
    const auto sink = g.add(std::make_unique<rasgo::panel::SinkOut>());
    rasgo::panel::seedPatch(g, o.seed, 0, factory);
    g.prepare(o.sr, 2, 256);
    g.setActiveOutput(sink);
    rasgo::panel::balanceSeedLevel(g, o.seed, o.sr, 256);
    std::printf("seed %llu: %zu cabos\n", static_cast<unsigned long long>(o.seed), g.cableCount());
    return renderGraph(g, sink, o.dur, o.sr, o.pos[0]);
}

bool loadPatch(const std::string& path, SignalGraph& g, std::string& err) {
    std::string text;
    if (!readFile(path, text)) { err = "não consegui ler " + path; return false; }
    try {
        g = SignalGraph::deserialize(text, factory);
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
    return true;
}

int cmdRenderPatch(int argc, char** argv) {
    const Opts o = parse(argc, argv, 2);
    if (!o.ok || o.pos.size() != 2) return usage();
    SignalGraph g; std::string err;
    if (!loadPatch(o.pos[0], g, err)) { std::fprintf(stderr, "patch inválido: %s\n", err.c_str()); return 1; }
    const auto sink = findSink(g);
    if (sink == static_cast<std::size_t>(-1)) { std::fprintf(stderr, "o patch não tem saída (OUT)\n"); return 1; }
    g.prepare(o.sr, 2, 256);
    g.setActiveOutput(sink);
    return renderGraph(g, sink, o.dur, o.sr, o.pos[1]);
}

int cmdValidate(int argc, char** argv) {
    if (argc != 3) return usage();
    SignalGraph g; std::string err;
    if (!loadPatch(argv[2], g, err)) { std::fprintf(stderr, "inválido: %s\n", err.c_str()); return 1; }
    const bool hasOut = findSink(g) != static_cast<std::size_t>(-1);
    try { g.prepare(48000.0f, 2, 256); } catch (const std::exception& e) {
        std::fprintf(stderr, "inválido ao preparar: %s\n", e.what()); return 1;
    }
    std::printf("válido: %zu módulos, %zu cabos%s\n", g.nodeCount(), g.cableCount(),
                hasOut ? "" : " (sem saída OUT)");
    return 0;
}

int cmdInfo(int argc, char** argv) {
    if (argc == 2) {
        int total = 0;
        for (const auto& f : rasgo::panel::moduleCatalog()) {
            std::printf("%s:", f.family);
            for (const char* t : f.types) { std::printf(" %s", t); ++total; }
            std::printf("\n");
        }
        std::printf("%d módulos\n", total);
        return 0;
    }
    auto m = rasgo::panel::makeModule(argv[2]);
    if (!m) { std::fprintf(stderr, "módulo desconhecido: %s\n", argv[2]); return 1; }
    std::printf("%s\n entradas:", m->type().c_str());
    for (std::size_t i = 0; i < m->inputCount(); ++i) std::printf(" %s", m->inputDescriptor(i).name.c_str());
    std::printf("\n saídas:");
    for (std::size_t i = 0; i < m->outputCount(); ++i) std::printf(" %s", m->outputDescriptor(i).name.c_str());
    std::printf("\n parâmetros:\n");
    for (const auto& p : m->parameters())
        std::printf("  %-14s %g … %g (padrão %g)%s%s\n", p.descriptor.id.c_str(), p.descriptor.minimum,
                    p.descriptor.maximum, p.descriptor.defaultValue,
                    p.descriptor.unit.empty() ? "" : " ", p.descriptor.unit.c_str());
    return 0;
}

// O patch de demonstração das imagens de painel do site (21/11/2026): o rack
// de fábrica, e cada entrada livre dos módulos pedidos recebe uma fonte de
// teste — áudio ← a serra do OSC, gate/trigger ← o CLK do CLOCK, controle ← o
// BI do FUNCTION —, para o visor mostrar o módulo funcionando. É o mesmo
// critério da exportação por seed (`RASGO_EXPORTAR_PAINEIS`), num arquivo,
// para `website/exportar_patch.sh` (sem áudio nem MIDI).
// entradas de pulso que não declaram a unidade (o CLOCK do TRIGSEQ, o KICK
// do PENDULUM…): reconhecidas pelo nome
bool isPulseName(const std::string& n) {
    for (const char* k : {"clock", "gate", "trig", "kick", "strike", "pluck", "fill"})
        if (n.find(k) != std::string::npos) return true;
    return false;
}

int cmdDemoPatch(int argc, char** argv) {
    if (argc != 4) return usage();
    std::vector<std::string> want;
    { std::stringstream ss(argv[2]); std::string tok;
      while (std::getline(ss, tok, ',')) if (!tok.empty()) want.push_back(tok); }
    SignalGraph g;
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g.add(std::move(n));
    g.add(std::make_unique<rasgo::panel::SinkOut>());
    auto first = [&](const char* t) {
        for (std::size_t i = 0; i < g.nodeCount(); ++i) if (g.node(i).type() == t) return i;
        return static_cast<std::size_t>(-1);
    };
    auto outPort = [&](std::size_t n, const char* nm) {
        for (std::size_t p = 0; p < g.node(n).outputCount(); ++p)
            if (g.node(n).outputDescriptor(p).name == nm) return p;
        return static_cast<std::size_t>(-1);
    };
    const std::size_t osc = first("OSC"), clk = first("CLOCK"), fun = first("FUNCTION");
    const std::size_t saw = outPort(osc, "saw"), ck = outPort(clk, "clock"), bi = outPort(fun, "bi");
    // altura (v/oct) ← a melodia do QUANTIZER (o KEY precisa de notas)
    const std::size_t qz = first("QUANTIZER");
    const std::size_t qp = qz == static_cast<std::size_t>(-1) ? qz : outPort(qz, "pitch");
    int wired = 0, missing = 0;
    for (const auto& t : want) {
        const std::size_t n = first(t.c_str());
        if (n == static_cast<std::size_t>(-1)) { std::fprintf(stderr, "módulo desconhecido: %s\n", t.c_str()); ++missing; continue; }
        if (n == osc || n == clk || n == fun) continue;
        for (std::size_t p = 0; p < g.node(n).inputCount(); ++p) {
            const auto& d = g.node(n).inputDescriptor(p);
            // reset fica livre: o mesmo pulso no RST zera o módulo a cada
            // passo (o TRIGSEQ, o KEY e o VOLTA saíam com o visor parado)
            if (d.name.find("reset") != std::string::npos || d.name == "rst") continue;
            std::size_t sn = fun, sp = bi;
            if (d.kind == PortKind::Audio) { sn = osc; sp = saw; }
            else if (d.unit == "trig" || d.unit == "gate" || isPulseName(d.name)) { sn = clk; sp = ck; }
            else if (d.unit == "v/oct" && qz != static_cast<std::size_t>(-1) && n != qz
                     && qp != static_cast<std::size_t>(-1)) { sn = qz; sp = qp; }
            try { g.connect(sn, sp, n, p); ++wired; }
            catch (...) { try { g.connect(sn, sp, n, p, true); ++wired; } catch (...) {} }
        }
    }
    std::string text = g.serialize() + "panel shown";
    for (std::size_t i = 0; i + 1 < g.nodeCount(); ++i) text += ' ' + std::to_string(i);
    text += '\n';
    std::ofstream out(argv[3], std::ios::binary);
    if (!out) { std::fprintf(stderr, "não consegui escrever %s\n", argv[3]); return 1; }
    out << text;
    std::printf("%s: %zu módulos, %d cabos de demonstração\n", argv[3], g.nodeCount(), wired);
    return missing > 0 ? 1 : 0;
}

}  // namespace

int main(int argc, char** argv) {
    rasgo::escalas::BancoEscalas::ligarAoQuantizer();   // SCALE = BANCO nos patches
    if (argc < 2) return usage();
    const std::string cmd = argv[1];
    if (cmd == "render") return cmdRender(argc, argv);
    if (cmd == "render-patch") return cmdRenderPatch(argc, argv);
    if (cmd == "validate") return cmdValidate(argc, argv);
    if (cmd == "info") return cmdInfo(argc, argv);
    if (cmd == "demo-patch") return cmdDemoPatch(argc, argv);
    return usage();
}
