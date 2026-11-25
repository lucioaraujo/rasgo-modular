// O LEARN do Rasgo Modular no navegador (25/11/2026, F4a): os verbetes do
// app (`LearnCatalog.hpp`, `HeaderHelp.hpp`) para a caixa LEARN da página.
//
// Arquivo à parte, compilado SEM otimização (ver build.sh): o catálogo são
// milhares de textos em quatro línguas montados em funções gigantes, e o
// otimizador levava mais de 15 minutos nelas. Aqui só há busca em tabela,
// que não precisa de velocidade; o motor de som segue em -O2.

#include "core/SignalGraph.hpp"
#include "panel/HeaderHelp.hpp"
#include "panel/LearnCatalog.hpp"
#include "panel/LearnPresentation.hpp"

#include <algorithm>
#include <string>
#include <vector>

using rasgo::modular::SignalGraph;

extern "C" {
const std::vector<char>& rw_web_entrada();
SignalGraph* rw_web_grafo();

// Os verbetes são tabelas fixas: a página consulta a sua própria instância
// do motor. `rw_entrada` traz "TIPO\tbind" (widget), "TIPO" (módulo) ou a
// chave do botão do cabeçalho; `lingua` = 0 en, 1 pt, 2 fr, 3 es. Devolve
// {"t":título,"q":…,"u":…,"e":…} ou "" sem verbete.
namespace {
std::string g_learn;
// a mesma escrita de texto JSON do motor
void json(std::string& o, const std::string& s) {
    o += '"';
    for (const char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (c == '\n') o += "\\n";   // as quebras de linha dos textos ficam
        else if (static_cast<unsigned char>(c) < 0x20) o += ' ';
        else o += c;
    }
    o += '"';
}
void learnJson(const rasgo::panel::LearnEntry* e, const std::string& titulo) {
    g_learn.clear();
    if (!e) return;
    g_learn = "{\"t\":";
    json(g_learn, titulo);
    g_learn += ",\"q\":"; json(g_learn, e->quick);
    g_learn += ",\"u\":"; json(g_learn, e->understand);
    g_learn += ",\"e\":"; json(g_learn, e->explore);
    g_learn += '}';
}
rasgo::panel::Lang lingua(int l) { return static_cast<rasgo::panel::Lang>(std::clamp(l, 0, 3)); }
std::string entrada(int tamanho) {
    const auto& e = rw_web_entrada();
    const auto n = static_cast<std::size_t>(std::max(0, tamanho));
    return n < e.size() ? std::string(e.data(), n) : std::string{};
}
}  // namespace

__attribute__((used, visibility("default")))
const char* rw_learn_widget(int tamanho, int l) {
    const std::string t = entrada(tamanho);
    const auto tab = t.find('\t');
    g_learn.clear();
    if (tab == std::string::npos) return g_learn.c_str();
    learnJson(rasgo::panel::lookupLearn(t.substr(0, tab), t.substr(tab + 1), lingua(l)), "");
    return g_learn.c_str();
}

__attribute__((used, visibility("default")))
const char* rw_learn_modulo(int tamanho, int l) {
    const std::string t = entrada(tamanho);
    learnJson(rasgo::panel::lookupLearnModule(t, lingua(l)), t);
    return g_learn.c_str();
}

__attribute__((used, visibility("default")))
const char* rw_learn_cabo(int l) {
    learnJson(&rasgo::panel::learnCable(lingua(l)),
              rasgo::panel::tr(rasgo::panel::strings::learnCableHead, lingua(l)));
    return g_learn.c_str();
}

// o botão do cabeçalho pela chave (`HeaderHelpRow::key`: seed, restore, …)
__attribute__((used, visibility("default")))
const char* rw_learn_cabecalho(int tamanho, int l) {
    const std::string k = entrada(tamanho);
    g_learn.clear();
    for (const auto& row : rasgo::panel::headerHelpRows())
        if (k == row.key) {
            learnJson(&row.entries[static_cast<std::size_t>(std::clamp(l, 0, 3))],
                      rasgo::panel::tr(row.title, lingua(l)));
            break;
        }
    return g_learn.c_str();
}

// os textos fixos da caixa (`learnIdle`)
__attribute__((used, visibility("default")))
const char* rw_learn_ocioso(int l) {
    g_learn = rasgo::panel::tr(rasgo::panel::strings::learnIdle, lingua(l));
    return g_learn.c_str();
}

// o valor do controle sob o mouse, ao vivo (este é do motor que toca):
// "BASE x unidade\nDSP y unidade", ou "" se o bind não é parâmetro
__attribute__((used, visibility("default")))
const char* rw_learn_valor(int id, int tamanho) {
    g_learn.clear();
    SignalGraph* g_grafo = rw_web_grafo();
    if (!g_grafo || id < 0 || static_cast<std::size_t>(id) >= g_grafo->nodeCount() || tamanho <= 0)
        return g_learn.c_str();
    const std::string bind = entrada(tamanho);
    auto& n = g_grafo->node(static_cast<std::size_t>(id));
    for (const auto& par : n.parameters())
        if (par.descriptor.id == bind) {
            try {
                g_learn = "BASE " + rasgo::panel::formatLearnValue(
                              g_grafo->parameterUserValue(static_cast<std::size_t>(id), bind), par.descriptor.unit)
                        + "\nDSP " + rasgo::panel::formatLearnValue(n.parameterValue(bind), par.descriptor.unit);
            } catch (...) { g_learn.clear(); }
            break;
        }
    return g_learn.c_str();
}

}  // extern "C"
