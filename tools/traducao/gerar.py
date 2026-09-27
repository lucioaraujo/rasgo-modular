#!/usr/bin/env python3
"""Gera `apps/panel/LearnWidgetI18n.hpp` a partir dos JSON por família.

Existe porque a primeira tentativa montava o C++ por substituição de texto
e quebrou: eu procurava o `;` que fecha o bloco de cada idioma, e vários
textos traduzidos CONTÊM `;` — a inserção caiu no meio de uma string. O
gerador não tem esse problema porque não procura nada: escreve o arquivo
inteiro a partir dos dados.

Cada família é um JSON em `tools/traducao/<FAMILIA>.json`:

    [["MODULO", "bind",
      ["quick en", "understand en", "explore en"],
      ["quick fr", "...", "..."],
      ["quick es", "...", "..."]]]

Uso:  python3 tools/traducao/gerar.py
"""
import json, pathlib, sys

AQUI = pathlib.Path(__file__).parent
RAIZ = AQUI.parent.parent
SAIDA = RAIZ / "apps/panel/LearnWidgetI18n.hpp"

CABECALHO = '''#pragma once

// Tradução dos verbetes de WIDGET do LEARN, por LOTES.
//
// ARQUIVO GERADO por `tools/traducao/gerar.py` a partir dos JSON em
// `tools/traducao/`. Não editar à mão: edite o JSON da família e rode o
// gerador.
//
// ---- por que por lotes -------------------------------------------------
//
// Sao 803 verbetes, 986 campos preenchidos, 73.203 caracteres — ~220.000
// nos tres idiomas. Traduzir tudo de uma vez produziria uma mudanca
// impossivel de revisar e facil de abandonar pela metade. Por FAMILIA de
// modulos, cada lote e verificavel, commitavel, e o medidor
// (`rasgo_modular_learn_coverage`) mostra exatamente o que falta.
//
// ---- regras do lote ----------------------------------------------------
//
// 1. quando um verbete e traduzido, TODOS os campos preenchidos no
//    portugues vao juntos. A queda e por VERBETE, nao por campo: traduzir
//    so o `quick` daria uma caixa com duas linguas dentro, pior que uma
//    caixa numa lingua so;
// 2. nomes de modulo, siglas, rotulos de knob e unidades ficam como estao
//    (OSC, VCA, 1 V/oct, TORQ, S&H) — sao o vocabulario do modular em
//    qualquer idioma, e traduzi-los esconde o que o painel mostra;
// 3. a mao, nao por maquina. O conteudo descreve comportamento real de
//    cada parametro naquele modulo; traducao automatica produz texto que
//    PARECE explicacao sem ser, e num instrumento didatico isso e pior
//    que nao ter texto.

#include "panel/LearnCatalog.hpp"

namespace rasgo::panel::detail {

inline void registrarLotesTraduzidos() {
'''

RODAPE = '''}

}  // namespace rasgo::panel::detail
'''

def esc(t):
    return t.replace('\\', '\\\\').replace('"', '\\"')

def main():
    familias = sorted(AQUI.glob("*.json"))
    partes = [CABECALHO]
    total = 0
    if not familias:
        partes.append("    // nenhuma familia traduzida ainda\n")
    for f in familias:
        dados = json.loads(f.read_text(encoding="utf-8"))
        partes.append("    // ---- familia %s (%d verbetes) ----\n"
                      % (f.stem, len(dados)))
        for nome, idx in (("En", 0), ("Fr", 1), ("Es", 2)):
            partes.append("    {\n        LearnTable& t = learnTable%sMutable();\n" % nome)
            for item in dados:
                mod, bind = item[0], item[1]
                q, u, e = item[2 + idx]
                partes.append('        t["%s"]["%s"] = LearnEntry{"%s", "%s", "%s"};\n'
                              % (esc(mod), esc(bind), esc(q), esc(u), esc(e)))
            partes.append("    }\n")
        total += len(dados)
    partes.append(RODAPE)
    SAIDA.write_text("".join(partes), encoding="utf-8")
    print("%s: %d verbetes de %d familia(s)" % (SAIDA.name, total, len(familias)))
    return 0

if __name__ == "__main__":
    sys.exit(main())
