// Teste do binário LV2 (apps/lv2/rasgo_lv2.cpp), como um hospedeiro faria:
// carrega o .so, percorre `lv2_descriptor`, instancia cada plugin, liga as
// portas e roda meio segundo. Confere: todos os módulos estão lá, a saída é
// finita em todos, e o OSC soa sozinho (como no aplicativo).

#include "core/SignalGraph.hpp"
#include "escalas/BancoEscalas.hpp"
#include "panel/ModuleCatalog.hpp"

#include <dlfcn.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
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
typedef const LV2_Descriptor* (*Lv2Fn)(uint32_t);
}

int main(int argc, char** argv) {
    if (argc != 2) { std::fprintf(stderr, "uso: rasgo_modular_lv2_tests <rasgo-modular.so>\n"); return 2; }
    void* so = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!so) { std::fprintf(stderr, "FALHOU: dlopen: %s\n", dlerror()); return 1; }
    auto fn = reinterpret_cast<Lv2Fn>(dlsym(so, "lv2_descriptor"));
    if (!fn) { std::fprintf(stderr, "FALHOU: sem lv2_descriptor\n"); return 1; }
    rasgo::escalas::BancoEscalas::ligarAoQuantizer();

    int falhas = 0, n = 0;
    const std::string base = "https://lucioaraujo.github.io/rasgo-modular/lv2/";
    for (uint32_t i = 0; const LV2_Descriptor* d = fn(i); ++i, ++n) {
        const std::string tipo = std::string(d->URI).substr(base.size());
        auto modelo = rasgo::panel::makeModule(tipo);
        if (!modelo) { std::fprintf(stderr, "FALHOU: %s não existe no catálogo\n", tipo.c_str()); ++falhas; continue; }
        LV2_Handle h = d->instantiate(d, 48000.0, "", nullptr);
        if (!h) { std::fprintf(stderr, "FALHOU: %s não instancia\n", tipo.c_str()); ++falhas; continue; }
        const std::size_t ni = modelo->inputCount(), no = modelo->outputCount();
        const auto& ps = modelo->parameters();
        constexpr uint32_t kN = 256;
        std::vector<std::vector<float>> ins(ni, std::vector<float>(kN, 0.0f));
        std::vector<std::vector<float>> outs(no, std::vector<float>(kN, 0.0f));
        std::vector<float> ctl;
        for (const auto& p : ps) ctl.push_back(p.descriptor.defaultValue);
        uint32_t porta = 0;
        for (auto& b : ins) d->connect_port(h, porta++, b.data());
        for (auto& b : outs) d->connect_port(h, porta++, b.data());
        for (auto& c : ctl) d->connect_port(h, porta++, &c);
        d->activate(h);
        double soma = 0.0; bool finito = true;
        for (int k = 0; k < 94; ++k) {   // ~0,5 s
            d->run(h, kN);
            for (auto& b : outs) for (float x : b) if (!std::isfinite(x)) finito = false;
            if (no > 0) for (float x : outs[0]) soma += static_cast<double>(x) * x;
        }
        d->cleanup(h);
        if (!finito) { std::fprintf(stderr, "FALHOU: %s: saída não finita\n", tipo.c_str()); ++falhas; }
        if (tipo == "OSC") {
            const double rms = std::sqrt(soma / (94.0 * kN));
            std::printf("OSC: RMS %.3f\n", rms);
            if (rms < 1e-3) { std::fprintf(stderr, "FALHOU: o OSC não soa\n"); ++falhas; }
        }
    }
    std::printf("%d plugins LV2\n", n);
    if (n < 100) { std::fprintf(stderr, "FALHOU: esperava ao menos 100 plugins\n"); ++falhas; }
    dlclose(so);
    if (falhas == 0) std::printf("lv2: tudo certo\n");
    return falhas == 0 ? 0 : 1;
}
