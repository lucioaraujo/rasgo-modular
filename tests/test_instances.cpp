// Instâncias: o mesmo módulo mais de uma vez no rack (11/11/2026, decisão do
// autor). Regras em `apps/panel/Instances.hpp`.

#include "panel/Instances.hpp"
#include "panel/ModuleCatalog.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using namespace rasgo::modular;
using namespace rasgo::panel;

namespace {
int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}
}  // namespace

int main() {
    SignalGraph g;
    std::vector<std::size_t> shown;
    auto add = [&](const char* t) { const auto id = g.add(makeModule(t)); shown.push_back(id); return id; };
    const auto osc1 = add("OSC");
    add("MIXER");
    add("MASTER");
    add("CLOCK");

    // os do rack inteiro não duplicam
    for (const char* t : {"MIXER", "MASTER", "CLOCK"})
        check(canAddInstance(g, shown, t) == AddRefusal::singleton, "MIXER/MASTER/CLOCK são únicos");
    check(canAddInstance(g, shown, "SIGNAL-IN") == AddRefusal::none, "um primeiro SIGNAL-IN entra");

    // fontes: até 4 cópias
    check(instanceNumber(g, shown, osc1) == 0 && instanceLabel(g, shown, osc1) == "OSC",
          "com uma cópia só, o nome não leva número");
    const auto osc2 = add("OSC");
    check(instanceLabel(g, shown, osc1) == "OSC 1" && instanceLabel(g, shown, osc2) == "OSC 2",
          "com duas cópias: OSC 1 e OSC 2");
    add("OSC");
    check(canAddInstance(g, shown, "OSC") == AddRefusal::none, "a 4ª cópia ainda entra");
    const auto osc4 = add("OSC");
    check(instanceLabel(g, shown, osc4) == "OSC 4", "a 4ª cópia é OSC 4");
    check(canAddInstance(g, shown, "OSC") == AddRefusal::tooMany, "a 5ª cópia é recusada");

    // removido do rack (o app esconde; o nó fica no grafo) não conta
    shown.erase(std::remove(shown.begin(), shown.end(), osc2), shown.end());
    check(canAddInstance(g, shown, "OSC") == AddRefusal::none, "removido do rack libera a vaga");
    check(instanceLabel(g, shown, osc4) == "OSC 3", "a numeração segue o que está no rack");

    // um patch com duas cópias sobrevive a salvar/abrir (o núcleo é por id)
    {
        SignalGraph p;
        const auto a = p.add(makeModule("OSC"));
        const auto b = p.add(makeModule("OSC"));
        const auto mx = p.add(makeModule("MIXER"));
        p.node(a).setParameter("freq", 110.0f);
        p.node(b).setParameter("freq", 111.5f);
        p.connect(a, 2, mx, 0);
        p.connect(b, 2, mx, 1);
        auto r = SignalGraph::deserialize(p.serialize(), [](const std::string& t) { return makeModule(t.c_str()); });
        check(r.nodeCount() == 3 && r.node(0).type() == "OSC" && r.node(1).type() == "OSC",
              "o patch volta com as duas cópias");
        check(r.node(1).parameterValue("freq") == 111.5f, "cada cópia guarda o seu valor");
        check(r.cableCount() == 2, "os cabos de cada cópia voltam");
    }

    if (g_failures == 0) std::printf("test_instances: OK\n");
    return g_failures == 0 ? 0 : 1;
}
