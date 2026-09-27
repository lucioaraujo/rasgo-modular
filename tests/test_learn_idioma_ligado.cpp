// O LEARN traduzido está LIGADO nos front-ends?
//
// Este arquivo existe por um erro cometido em 27 set. 2026. Depois de
// traduzir os 803 verbetes de widget e ver o medidor reportar 100% nos três
// idiomas, as duas chamadas nos front-ends ainda eram
// `lookupLearn(tipo, bind)` — sem idioma. O parâmetro tem padrão
// `Lang::pt`, então tudo COMPILAVA, todos os testes PASSAVAM, o medidor
// dizia 100%, e quem rodasse o instrumento em inglês continuaria lendo
// português. A tradução estava escrita e invisível.
//
// Nada avisava, e é isso que torna o caso perigoso:
//
//   - o compilador não avisa: o padrão é comportamento PEDIDO (um verbete
//     faltando numa língua deve mostrar o texto certo em outra, não uma
//     caixa vazia);
//   - a interface não avisa: a caixa LEARN aparece normalmente, só na
//     língua errada;
//   - o medidor não avisa: ele mede o CATÁLOGO, não quem o consulta.
//
// A verificação é por VARREDURA DE FONTE, e isso é deliberado. Testar o
// caminho de verdade exigiria instanciar o front-end — janela, contexto
// gráfico, loop de eventos — que é justamente o que os testes deste
// projeto não fazem. Ler o texto da chamada é grosseiro, mas pega o erro
// que aconteceu, e pega no `ctest` em vez de numa sessão de escuta.

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int falhas = 0;

// Devolve o conteúdo do arquivo, ou vazio se não abrir.
std::string ler(const std::string& caminho) {
    std::ifstream f(caminho, std::ios::binary);
    if (!f) return {};
    std::ostringstream o;
    o << f.rdbuf();
    return o.str();
}

// Extrai a lista de argumentos de cada chamada de `nome(` — contando
// parênteses para não parar no primeiro `)` de uma expressão interna.
std::vector<std::string> chamadas(const std::string& fonte,
                                  const std::string& nome) {
    std::vector<std::string> saida;
    const std::string agulha = nome + "(";
    std::size_t i = 0;
    while ((i = fonte.find(agulha, i)) != std::string::npos) {
        // `lookupLearn(` casaria dentro de `lookupLearnModule(`; exigir que
        // o caractere anterior não seja de identificador resolve o inverso,
        // e a checagem abaixo resolve este.
        const std::size_t depois = i + nome.size();
        if (fonte[depois] != '(') { i = depois; continue; }
        std::size_t j = depois;      // no '('
        int nivel = 0;
        std::string args;
        for (; j < fonte.size(); ++j) {
            const char c = fonte[j];
            if (c == '(') { ++nivel; if (nivel == 1) continue; }
            if (c == ')') { --nivel; if (nivel == 0) break; }
            args += c;
        }
        saida.push_back(args);
        i = j;
    }
    return saida;
}

// Quantos argumentos de TOPO tem a lista (vírgulas fora de parênteses).
std::size_t quantosArgs(const std::string& args) {
    if (args.find_first_not_of(" \t\n") == std::string::npos) return 0;
    std::size_t n = 1, nivel = 0;
    for (const char c : args) {
        if (c == '(' || c == '[' || c == '<') ++nivel;
        else if (c == ')' || c == ']' || c == '>') { if (nivel) --nivel; }
        else if (c == ',' && nivel == 0) ++n;
    }
    return n;
}

void conferir(const char* arquivo, const char* funcao, std::size_t minArgs) {
    const std::string caminho = std::string(RASGO_MODULAR_DIR) + "/" + arquivo;
    const std::string fonte = ler(caminho);
    if (fonte.empty()) {
        std::printf("  FALHOU: não consegui ler %s\n", caminho.c_str());
        ++falhas;
        return;
    }
    const auto cs = chamadas(fonte, funcao);
    if (cs.empty()) {
        // Não é erro: um front-end pode deixar de consultar. Mas é AVISO,
        // porque o caso provável é a função ter sido renomeada e este teste
        // ter deixado de verificar o que dizia verificar.
        std::printf("  aviso: nenhuma chamada de %s em %s\n", funcao, arquivo);
        return;
    }
    for (const auto& args : cs) {
        if (quantosArgs(args) < minArgs) {
            std::printf("  FALHOU: %s chama %s sem idioma: %s(%s)\n",
                        arquivo, funcao, funcao, args.c_str());
            ++falhas;
        }
    }
}

}  // namespace

int main() {
    // `lookupLearn(tipo, bind, lang)` e `lookupLearnModule(tipo, lang)`:
    // 3 e 2 argumentos. Menos que isso é o padrão silencioso do português.
    for (const char* f : {"apps/panel/panel_main.cpp",
                          "apps/juce/RasgoModularApp.cpp"}) {
        conferir(f, "lookupLearn", 3);
        conferir(f, "lookupLearnModule", 2);
    }
    if (falhas == 0) std::puts("test_learn_idioma_ligado: OK");
    return falhas == 0 ? 0 : 1;
}
