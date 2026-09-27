#pragma once

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
    // nenhuma familia traduzida ainda
}

}  // namespace rasgo::panel::detail
