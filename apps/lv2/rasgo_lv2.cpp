// Os módulos do Rasgo Modular como plugins LV2, um plugin por módulo, sem
// JUCE (24/11/2026; dossies/ESTUDO_outras_plataformas.md, itens 3 e 4).
//
// É a mesma ideia da casca do VCV Rack (`rasgo-modular-vcv`, `Casca.hpp`):
// cada módulo é um `Signal` do motor, rodado em blocos de 16 amostras
// (16 amostras de atraso, 0,33 ms a 48 kHz). Parâmetros e portas vêm do
// próprio módulo: nada copiado à mão. Um só binário guarda todos os
// plugins; o mesmo arquivo, compilado com RASGO_LV2_TTL, escreve os .ttl.
//
// - Todas as portas são lv2:AudioPort, nos valores do Rasgo (áudio e CV
//   ±1, gate 0–1, 1 V/oct em oitavas). AudioPort funciona em todo
//   hospedeiro (Ardour, Carla, MOD); CVPort não.
// - Hospedeiros LV2 ligam toda porta (a uma entrada muda, se nada está
//   ligado). O módulo, porém, muda de comportamento quando uma entrada não
//   está ligada (o CLOCK usa o relógio interno sem EXT, por exemplo).
//   Então uma entrada conta como ligada enquanto traz sinal, e por 1 s
//   depois do último valor diferente de zero.
// - Fora: SIGNAL-IN, POLY-IN, NOTE-OUT, MIXER e MASTER, os conectores do
//   aplicativo (o hospedeiro tem os seus), como no VCV.
//
// A ABI do LV2 (lv2/core/lv2.h, ISC) é pequena e estável; as declarações
// abaixo seguem a especificação, para não depender do SDK no build.

#include "core/SignalGraph.hpp"
#include "escalas/BancoEscalas.hpp"
#include "panel/ModuleCatalog.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

extern "C" {
typedef void* LV2_Handle;
typedef struct { const char* URI; void* data; } LV2_Feature;
typedef struct LV2_Descriptor {
    const char* URI;
    LV2_Handle (*instantiate)(const struct LV2_Descriptor*, double, const char*,
                              const LV2_Feature* const*);
    void (*connect_port)(LV2_Handle, uint32_t, void*);
    void (*activate)(LV2_Handle);
    void (*run)(LV2_Handle, uint32_t);
    void (*deactivate)(LV2_Handle);
    void (*cleanup)(LV2_Handle);
    const void* (*extension_data)(const char*);
} LV2_Descriptor;
}

namespace rasgo_lv2 {

using rasgo::modular::AudioBlock;
using rasgo::modular::Signal;

constexpr const char* kUriBase = "https://lucioaraujo.github.io/rasgo-modular/lv2/";
constexpr std::size_t kBloco = 16;

inline bool foraDoLv2(const std::string& t) {
    return t == "SIGNAL-IN" || t == "POLY-IN" || t == "NOTE-OUT" || t == "MIXER" || t == "MASTER";
}

struct Tipo { std::string tipo, familia, uri; };

inline const std::vector<Tipo>& tipos() {
    static const std::vector<Tipo> v = [] {
        std::vector<Tipo> r;
        for (const auto& g : rasgo::panel::moduleCatalog())
            for (const char* t : g.types)
                if (!foraDoLv2(t)) r.push_back({t, g.family, std::string(kUriBase) + t});
        return r;
    }();
    return v;
}

// os tempos do ENVELOPE andam em escala logarítmica, como no app
inline bool logaritmico(const std::string& tipo, const std::string& id) {
    return tipo == "ENVELOPE" && (id == "attack" || id == "decay" || id == "release");
}

struct Instancia {
    std::unique_ptr<Signal> dsp;
    std::vector<const float*> in;
    std::vector<float*> out;
    std::vector<const float*> ctl;
    std::vector<float> ctlVisto;
    std::vector<std::string> ids;
    std::vector<AudioBlock> entra, sai, pronto;
    std::vector<const AudioBlock*> ptrs;
    std::vector<int> segura;           // blocos que a entrada ainda conta como ligada
    std::vector<char> temSinal;
    int seguraBlocos = 3000;
    std::size_t pos = 0;
    float taxa = 48000.0f;

    void preparar() {
        dsp->prepare(taxa, kBloco);
        entra.assign(dsp->inputCount(), AudioBlock(taxa, 2, kBloco));
        sai.assign(dsp->outputCount(), AudioBlock(taxa, 2, kBloco));
        pronto.assign(dsp->outputCount(), AudioBlock(taxa, 2, kBloco));
        ptrs.assign(dsp->inputCount(), nullptr);
        segura.assign(dsp->inputCount(), 0);
        temSinal.assign(dsp->inputCount(), 0);
        seguraBlocos = std::max(1, static_cast<int>(taxa / static_cast<float>(kBloco)));
        pos = 0;
    }

    void bloco() {
        for (std::size_t p = 0; p < ids.size(); ++p) {
            const float v = ctl[p] ? *ctl[p] : ctlVisto[p];
            if (v != ctlVisto[p]) { ctlVisto[p] = v; dsp->setParameter(ids[p], v); }
        }
        for (std::size_t i = 0; i < ptrs.size(); ++i) {
            if (temSinal[i]) segura[i] = seguraBlocos;
            else if (segura[i] > 0) --segura[i];
            temSinal[i] = 0;
            ptrs[i] = segura[i] > 0 ? &entra[i] : nullptr;
        }
        dsp->process(ptrs, sai);
        pronto.swap(sai);
    }

    void rodar(const uint32_t n) {
        for (uint32_t k = 0; k < n; ++k) {
            for (std::size_t i = 0; i < entra.size(); ++i) {
                const float x = in[i] ? in[i][k] : 0.0f;
                entra[i].at(0, pos) = x;
                entra[i].at(1, pos) = x;
                if (x != 0.0f) temSinal[i] = 1;
            }
            for (std::size_t o = 0; o < pronto.size(); ++o)
                if (out[o]) out[o][k] = pronto[o].at(0, pos);
            if (++pos == kBloco) { pos = 0; bloco(); }
        }
    }
};

// porta → (tipo, índice): entradas, saídas, controles, nessa ordem
LV2_Handle instanciar(const LV2_Descriptor* d, double rate, const char*, const LV2_Feature* const*) {
    rasgo::escalas::BancoEscalas::ligarAoQuantizer();
    const auto& ts = tipos();
    const auto it = std::find_if(ts.begin(), ts.end(), [&](const Tipo& t) { return t.uri == d->URI; });
    if (it == ts.end()) return nullptr;
    auto dsp = rasgo::panel::makeModule(it->tipo);
    if (!dsp) return nullptr;
    auto* x = new Instancia;
    x->dsp = std::move(dsp);
    x->taxa = static_cast<float>(rate);
    x->in.assign(x->dsp->inputCount(), nullptr);
    x->out.assign(x->dsp->outputCount(), nullptr);
    for (const auto& p : x->dsp->parameters()) {
        x->ids.push_back(p.descriptor.id);
        x->ctlVisto.push_back(p.descriptor.defaultValue);
    }
    x->ctl.assign(x->ids.size(), nullptr);
    x->preparar();
    return x;
}

void ligar(LV2_Handle h, uint32_t porta, void* dados) {
    auto* x = static_cast<Instancia*>(h);
    std::size_t p = porta;
    if (p < x->in.size()) { x->in[p] = static_cast<const float*>(dados); return; }
    p -= x->in.size();
    if (p < x->out.size()) { x->out[p] = static_cast<float*>(dados); return; }
    p -= x->out.size();
    if (p < x->ctl.size()) x->ctl[p] = static_cast<const float*>(dados);
}

void ativar(LV2_Handle h) { static_cast<Instancia*>(h)->preparar(); }
void rodar(LV2_Handle h, uint32_t n) { static_cast<Instancia*>(h)->rodar(n); }
void limpar(LV2_Handle h) { delete static_cast<Instancia*>(h); }
const void* extensao(const char*) { return nullptr; }

// ---- os .ttl (o mesmo catálogo, escrito uma vez no build) ----------------

inline std::string simbolo(std::string s) {
    for (auto& c : s)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) c = '_';
    if (s.empty() || (s[0] >= '0' && s[0] <= '9')) s = "p" + s;
    return s;
}

inline const char* classe(const std::string& fam) {
    if (fam == "SOURCE") return "lv2:GeneratorPlugin";
    if (fam == "TRANSFORM") return "lv2:FilterPlugin";
    if (fam == "MODULATE") return "lv2:ModulatorPlugin";
    if (fam == "SPACE") return "lv2:SpatialPlugin";
    return "lv2:UtilityPlugin";
}

inline std::string texto(const std::string& s) {
    std::string r;
    for (char c : s) { if (c == '"' || c == '\\') r += '\\'; r += c; }
    return r;
}

int escreverTtl(const char* pasta, const char* binario) {
    const std::string dir(pasta);
    FILE* m = std::fopen((dir + "/manifest.ttl").c_str(), "w");
    FILE* t = std::fopen((dir + "/rasgo-modular.ttl").c_str(), "w");
    if (!m || !t) { std::fprintf(stderr, "não consegui escrever em %s\n", pasta); return 1; }
    std::fprintf(m, "@prefix lv2: <http://lv2plug.in/ns/lv2core#> .\n"
                    "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n\n");
    std::fprintf(t, "@prefix doap: <http://usefulinc.com/ns/doap#> .\n"
                    "@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n"
                    "@prefix lv2: <http://lv2plug.in/ns/lv2core#> .\n"
                    "@prefix pprops: <http://lv2plug.in/ns/ext/port-props#> .\n"
                    "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n\n");
    for (const auto& ti : tipos()) {
        auto dsp = rasgo::panel::makeModule(ti.tipo);
        if (!dsp) continue;
        std::fprintf(m, "<%s>\n    a lv2:Plugin ;\n    lv2:binary <%s> ;\n    rdfs:seeAlso <rasgo-modular.ttl> .\n\n",
                     ti.uri.c_str(), binario);
        std::fprintf(t, "<%s>\n    a lv2:Plugin , %s ;\n    doap:name \"Rasgo %s\" ;\n"
                        "    doap:license <https://spdx.org/licenses/GPL-3.0-or-later> ;\n"
                        "    doap:maintainer [ foaf:name \"Lúcio de Araújo\" ; foaf:homepage <https://lucioaraujo.github.io/rasgo-modular/> ] ;\n"
                        "    rdfs:comment \"Rasgo Modular %s module (%s family). Same DSP as the Rasgo Modular application.\" ;\n"
                        "    lv2:optionalFeature lv2:hardRTCapable ;\n    lv2:port",
                     ti.uri.c_str(), classe(ti.familia), ti.tipo.c_str(), ti.tipo.c_str(), ti.familia.c_str());
        std::size_t idx = 0;
        std::vector<std::string> usados;
        const auto unico = [&](std::string s) {
            std::string b = s; int n = 2;
            while (std::find(usados.begin(), usados.end(), s) != usados.end()) s = b + "_" + std::to_string(n++);
            usados.push_back(s);
            return s;
        };
        const auto sep = [&] { std::fprintf(t, idx == 0 ? " [\n" : " , [\n"); };
        for (std::size_t i = 0; i < dsp->inputCount(); ++i, ++idx) {
            const auto& d = dsp->inputDescriptor(i);
            sep();
            std::fprintf(t, "        a lv2:InputPort , lv2:AudioPort ;\n        lv2:index %zu ;\n"
                            "        lv2:symbol \"%s\" ;\n        lv2:name \"%s%s\"\n    ]",
                         idx, unico("in_" + simbolo(d.name)).c_str(), texto(d.name).c_str(),
                         d.unit.empty() ? "" : (" (" + texto(d.unit) + ")").c_str());
        }
        for (std::size_t o = 0; o < dsp->outputCount(); ++o, ++idx) {
            const auto& d = dsp->outputDescriptor(o);
            sep();
            std::fprintf(t, "        a lv2:OutputPort , lv2:AudioPort ;\n        lv2:index %zu ;\n"
                            "        lv2:symbol \"%s\" ;\n        lv2:name \"%s%s\"\n    ]",
                         idx, unico("out_" + simbolo(d.name)).c_str(), texto(d.name).c_str(),
                         d.unit.empty() ? "" : (" (" + texto(d.unit) + ")").c_str());
        }
        for (const auto& p : dsp->parameters()) {
            const auto& d = p.descriptor;
            sep();
            std::fprintf(t, "        a lv2:InputPort , lv2:ControlPort ;\n        lv2:index %zu ;\n"
                            "        lv2:symbol \"%s\" ;\n        lv2:name \"%s\" ;\n"
                            "        lv2:default %g ;\n        lv2:minimum %g ;\n        lv2:maximum %g%s\n    ]",
                         idx, unico(simbolo(d.id)).c_str(), texto(d.id).c_str(),
                         static_cast<double>(d.defaultValue), static_cast<double>(d.minimum),
                         static_cast<double>(d.maximum),
                         logaritmico(ti.tipo, d.id) ? " ;\n        lv2:portProperty pprops:logarithmic" : "");
            ++idx;
        }
        std::fprintf(t, " .\n\n");
    }
    std::fclose(m);
    std::fclose(t);
    std::printf("%zu plugins LV2 descritos em %s\n", tipos().size(), pasta);
    return 0;
}

}  // namespace rasgo_lv2

#ifdef RASGO_LV2_TTL
int main(int argc, char** argv) {
    if (argc != 3) { std::fprintf(stderr, "uso: rasgo_lv2_ttl <pasta do bundle> <binario.so>\n"); return 2; }
    return rasgo_lv2::escreverTtl(argv[1], argv[2]);
}
#else
extern "C" __attribute__((visibility("default")))
const LV2_Descriptor* lv2_descriptor(uint32_t index) {
    static std::vector<LV2_Descriptor> ds = [] {
        std::vector<LV2_Descriptor> v;
        for (const auto& t : rasgo_lv2::tipos())
            v.push_back({t.uri.c_str(), rasgo_lv2::instanciar, rasgo_lv2::ligar, rasgo_lv2::ativar,
                         rasgo_lv2::rodar, nullptr, rasgo_lv2::limpar, rasgo_lv2::extensao});
        return v;
    }();
    return index < ds.size() ? &ds[index] : nullptr;
}
#endif
