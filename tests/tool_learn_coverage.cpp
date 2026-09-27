// Cobertura de tradução do LEARN, por família.
//
// Existe porque a tradução dos 803 verbetes de widget é feita por lotes, e
// sem um medidor o progresso fica invisível — e trabalho invisível é
// trabalho que se abandona pela metade sem ninguém notar.
//
// Mede o que IMPORTA: um verbete só conta como traduzido se TODOS os
// campos preenchidos no português também estiverem preenchidos no outro
// idioma. Contar por campo mostraria 80% quando metade das caixas ainda
// apareceria bilíngue.
#include "core/SignalGraph.hpp"
#include "panel/LearnCatalog.hpp"
#include "panel/ModuleCatalog.hpp"

#include <cstdio>
#include <map>
#include <string>
#include <vector>

using namespace rasgo::modular;
using rasgo::panel::Lang;

namespace {

struct Conta { long total = 0, feitos = 0, carFaltando = 0; };

bool traduzido(const char* tipo, const std::string& bind, Lang l) {
    const auto* pt = rasgo::panel::lookupLearn(tipo, bind, Lang::pt);
    const auto* x  = rasgo::panel::lookupLearn(tipo, bind, l);
    if (pt == nullptr || x == nullptr) return false;
    if (x == pt) return false;                       // caiu no português
    // todo campo preenchido no pt tem de estar preenchido aqui
    if (!pt->quick.empty()      && x->quick.empty())      return false;
    if (!pt->understand.empty() && x->understand.empty()) return false;
    if (!pt->explore.empty()    && x->explore.empty())    return false;
    return true;
}


// Escapa uma string para JSON. Só o necessário: aspas, contrabarra e os
// controles. UTF-8 passa cru — o arquivo é lido por `json.load` em modo
// UTF-8 e o acento não precisa de \u.
std::string json(const std::string& s) {
    std::string o = "\"";
    for (const unsigned char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += (char)c; }
        else if (c == '\n') o += "\\n";
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", c); o += b; }
        else o += (char)c;
    }
    return o + "\"";
}

// Despeja o PORTUGUÊS de uma família como JSON, para servir de base ao
// lote de tradução. Existe porque os `bind` e a ordem vêm do código, não
// da minha lembrança: um bind digitado errado no JSON entra como verbete
// órfão, nunca é consultado, e o medidor continua mostrando 0% sem dizer
// por quê.
void despejar(const char* familia) {
    std::printf("[\n");
    bool primeiro = true;
    for (const auto& grp : rasgo::panel::moduleCatalog()) {
        if (std::string(grp.family) != familia) continue;
        for (const char* t : grp.types) {
            auto n = rasgo::panel::makeModule(t);
            if (!n) continue;
            for (const auto& w : n->panel().widgets) {
                if (w.bind.empty()) continue;
                const auto* pt = rasgo::panel::lookupLearn(t, w.bind, Lang::pt);
                if (pt == nullptr) continue;
                if (!primeiro) std::printf(",\n");
                primeiro = false;
                std::printf(" [%s, %s, [%s, %s, %s]]",
                            json(t).c_str(), json(w.bind).c_str(),
                            json(pt->quick).c_str(),
                            json(pt->understand).c_str(),
                            json(pt->explore).c_str());
            }
        }
    }
    std::printf("\n]\n");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 2 && std::string(argv[1]) == "--despejar") { despejar(argv[2]); return 0; }

    const char* nomes[3] = {"en", "fr", "es"};
    const Lang langs[3] = {Lang::en, Lang::fr, Lang::es};

    std::printf("COBERTURA DE TRADUÇÃO DO LEARN — verbetes de WIDGET\n\n");
    std::printf("%-12s %7s", "família", "total");
    for (const char* n : nomes) std::printf(" %10s", n);
    std::printf("\n");

    std::map<std::string, Conta> porFam[3];
    Conta geral[3];
    std::vector<std::string> pendentes;

    for (const auto& grp : rasgo::panel::moduleCatalog()) {
        long total = 0;
        long feitos[3] = {0,0,0};
        long falta[3] = {0,0,0};
        for (const char* t : grp.types) {
            auto n = rasgo::panel::makeModule(t);
            if (!n) continue;
            for (const auto& w : n->panel().widgets) {
                if (w.bind.empty()) continue;
                const auto* pt = rasgo::panel::lookupLearn(t, w.bind, Lang::pt);
                if (pt == nullptr) continue;
                ++total;
                for (int i = 0; i < 3; ++i) {
                    if (traduzido(t, w.bind, langs[i])) ++feitos[i];
                    else falta[i] += (long)(pt->quick.size()
                                          + pt->understand.size()
                                          + pt->explore.size());
                }
            }
        }
        std::printf("%-12s %7ld", grp.family, total);
        for (int i = 0; i < 3; ++i) {
            std::printf(" %9.0f%%", total ? 100.0*feitos[i]/total : 100.0);
            geral[i].total += total;
            geral[i].feitos += feitos[i];
            geral[i].carFaltando += falta[i];
        }
        std::printf("\n");
        if (feitos[0] < total) pendentes.push_back(grp.family);
    }

    std::printf("\n%-12s %7ld", "TOTAL", geral[0].total);
    for (int i = 0; i < 3; ++i)
        std::printf(" %9.0f%%", geral[i].total
                    ? 100.0*geral[i].feitos/geral[i].total : 100.0);
    std::printf("\n\n");
    for (int i = 0; i < 3; ++i)
        std::printf("  falta escrever em %s: %ld caracteres\n",
                    nomes[i], geral[i].carFaltando);
    if (!pendentes.empty()) {
        std::printf("\nfamílias pendentes: ");
        for (const auto& f : pendentes) std::printf("%s ", f.c_str());
        std::printf("\n");
    } else {
        std::printf("\nTodas as famílias traduzidas.\n");
    }
    return 0;
}
