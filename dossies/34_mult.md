# Dossiê — Módulo 34: Múltiplo processado (`MULT`)

**Família:** UTILITY
**Estado:** **implementado — marco 3** (2026-09-04)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Mult.hpp`, `tests/test_mult.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

No grafo digital o fan-out já é livre e sem perda — um `MULT` que só
copia não faz nada que um cabo não faça. Então este `MULT` **processa
cada saída**: 1 entrada → 4 saídas, cada uma com atenuversor + offset
próprios (um mini-`CONTROL` por tomada). É o distribuidor de CV — mandar
a mesma modulação pra 4 destinos com quantidades e polaridades
diferentes.

- **`in`** → **`out1..out4`**; `out_k = slew(scale_k · in + offset_k)`;
- **`scale1..4`** (−2..+2) = atenuversor por tomada (negativo = inverte);
- **`offset1..4`** (−1..+1) = tensão somada por tomada;
- **`slew`** (0–1) = glide compartilhado em todas as tomadas (0 = direto);
- **fonte quádrupla de tensão:** sem `in` conectado, `out_k = offset_k`
  — o `MULT` sozinho vira 4 fontes de CV manual.

Desvio Rasgo: o mini-`CONTROL` por tomada (o que o `PESQUISA §2.2`
pedia); a fonte quádrupla quando ocioso. Sem `drift` — é utilidade,
precisão importa.

Sem alocação / lock / IO em `process()`. Determinístico (sem RNG).

**Testes (11 funções, Debug + Release):** `dual` on → `out1`/`out2`
seguem `in`, `out3`/`out4` seguem `in2`; off → os 4 seguem `in`;
`scale=1`/`offset=0` → `out_k
== in`; `scale2=−1` → `out2 == −in`; `offset3=0.4` → `out3 == in + 0.4`;
`scale4=0.5`+`offset4=−0.2` → `out4 == 0.5·in − 0.2`; tomadas
independentes (mexer `scale1` não muda `out2`); sem `in` → `out_k ==
offset_k` (fonte de CV); `slew=0` → degrau segue a entrada amostra a
amostra; `slew` alto → glide (derivada por amostra pequena num degrau
de `in`); tudo finito; dois renders byte-idênticos; grafo `LFO →
MULT` · `out1 → FILTER.cutoff` · `out2 → VCA.cv` (mesma fonte, dois
destinos, quantidades diferentes).

**Modo dual (feito — 2026-09-04):** toggle `dual` + 2ª entrada `in2`.
Ligado, `in` alimenta `out1`/`out2` e `in2` alimenta `out3`/`out4` —
dois múltiplos de 1→2 num painel (Doepfer A-180-2). Desligado = 1→4 como
antes. `out3`/`out4` sem `in2` conectado → só o `offset` (fonte de CV).

**Pendências (candidatos):** `slew` por tomada (assimétrico rise/fall);
saída de **soma** das tomadas processadas; apresentação no painel da
distribuição como barras animadas (o valor de cada saída).

---

## 1. Problema musical e papel no fluxo

Uma decisão comum: "esse LFO vai pro filtro E pro VCA, mas com pesos
diferentes, e no VCA invertido". Com cabo puro isso exige um atenuversor
em cada ponta. O `MULT` junta a distribuição e o ajuste: uma fonte, 4
versões, cada uma pronta pro seu destino. Ocioso, é um banco de 4
tensões — útil pra afinar drones, dar bias a filtros, testar.

Papel: utilidade de distribuição de CV. `ENVELOPE.env → MULT` +
`out1 → FILTER.cutoff` (+full) + `out2 → SHAPE.fold` (metade) +
`out3 → SPACE.mix` (invertido) + `out4 → PARAMETRIC.sweep`. No
`seedPatch` v2: destino de `slow`/`mod`, fonte de `mod`/`slow` pras 4
saídas — espalha uma modulação pelo patch.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Múltiplo bufferizado clássico** (Doepfer A-180, Intellijel Buff Mult) | 1 entrada → N saídas com buffer; sem "afundar" o pitch (problema do múltiplo passivo — não se aplica ao digital) | prática de domínio público |
| **Atenuversor + offset** (Maths ch1/ch4, Serge, `CONTROL` do Rasgo) | ganho ±2 e tensão somada por canal | código do autor / prática |
| **"Voltage spreader"** (Frap Tools, Doepfer A-138s "spread") | uma fonte distribuída com pesos progressivos | ficha/conceito |

## 3. Modelo — matemática, estados, extremos

`slewCoef = slew<=0 ? 1 : 1 − exp(−dt / (slew²·0.5))` (padrão SampleHold).

Por amostra:
```
x = in (ou 0 se não conectado)
para cada tomada k (0..3):
  tgt_k = clamp(scale_k · x + offset_k, −8, 8)
  y_k  += (tgt_k − y_k) · slewCoef
  out_k = y_k
```

**Estados:** `y[4]` (valor deslizado por tomada). Sem alocação.

**Extremos.** Sem `in` → `tgt_k = offset_k`, as saídas assentam nos
offsets (com `slew` levam um tempo). `scale_k=0` → `out_k = offset_k`
(ignora a entrada — tomada vira fonte de tensão). `scale_k=−2` + `in`
forte → pode passar de ±2; o `clamp(±8)` segura, o destino que se cuide
(é CV, não estágio de saída). `slew` alto + `in` de áudio → vira
passa-baixas de 1 polo (uso não-idiomático mas válido). Todas as tomadas
com o mesmo `scale`/`offset` → múltiplo comum.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → as 4 saídas = os 4 offsets (banco de
  tensão manual). Toca/serve ao carregar.
- **Performance:** `scale`/`offset` por tomada ao vivo = redistribuir a
  modulação sem repatch; inverter uma ponta com o lado negativo do
  `scale`; `slew` pra suavizar tudo de uma vez.
- **Híbrida:** `LFO`/`ENVELOPE`/`SEQUENCE.pitch`/`DRIFT` na entrada →
  4 destinos com pesos diferentes; `MULT` como pré-processador de uma
  fonte antes de espalhar.

## 5. Portas, parâmetros, limites

**Entradas:** `in`, `in2` (Control — `in2` só no modo `dual`).
**Saídas:** `out1`, `out2`, `out3`, `out4` (Control).
**Parâmetros:** `dual` (0/1, def 0), `scale1..4` (−2..2, def 1),
`offset1..4` (−1..1, def 0), `slew` (0–1, def 0).
**Limites:** saída clampada a ±8 (é CV). CPU: 4 MAC + 1 `exp` por bloco
(slew). Sem alocação, sem RNG, sem `sin`.

## 6. Alternativas descartadas

- **Múltiplo puro (só copiar):** inútil no grafo digital — o fan-out já
  é livre. O `PESQUISA §2.2` já dizia: só vira útil com `offset`/`invert`
  por saída.
- **Modo dual (2→2+2) no marco 1:** ficou pra 2ª camada (2026-09-04) —
  toggle `dual` + `in2`, out1/2 ← in, out3/4 ← in2. 1→4 com processamento
  por tomada segue como padrão (`dual` off).
- **`drift`:** é utilidade de precisão — um `MULT` que "erra" o valor
  sozinho é um defeito, não um recurso.
- **`slew` por tomada:** dobra os parâmetros; o `slew` compartilhado
  cobre o caso comum (suavizar a distribuição inteira).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `scale1=1`, `offset1=0`, `in` rampa → `out1[f] == in[f]`
exato; `scale2=−1` → `out2 == −in`; `offset3=0.4` → `out3 == in + 0.4`;
`scale4=0.5`, `offset4=−0.2` → `out4 == 0.5·in − 0.2`; mexer `scale1`
não muda `out2` (tomadas independentes); `in` livre → `out_k` assenta
em `offset_k`; `slew=0` → `out` acompanha `in` sem atraso; `slew=0.5` →
|Δ out por amostra| pequeno num degrau; dois renders byte-idênticos.

**Escuta:** distribuir um LFO por 4 destinos com pesos diferentes soa
como um patch mais rico sem virar bagunça? inverter uma ponta cria um
movimento contrário útil (filtro abre enquanto o volume fecha)? o
`slew` compartilhado amarra a modulação de forma musical? o banco de 4
tensões ocioso serve pra afinar/dar bias na hora?

## 8. Integração e painel

Classe `Mult` (`type()` = `"MULT"`), 2 entradas, 4 saídas, 10 parâmetros.
`panel()` próprio (10 HP): `Display` (as 4 saídas como barras), jacks
`IN`/`IN2` + toggle `DUAL` no topo, 4 pares de knobs `SCL k`/`OFF k`,
`SLEW`, 4 jacks de saída embaixo. Testado isolado (ganho, offset, inversão, independência,
fonte de tensão, slew) antes do patch. Cadeias canônicas: `LFO → MULT` ·
`out1 → FILTER.cutoff` · `out2 → VCA.cv` (invertido). Adicionado ao
catálogo do painel (`apps/panel/ModuleCatalog.hpp`, família TRANSFORM
junto do `CONTROL`/`VCA` — utilidades de CV).
