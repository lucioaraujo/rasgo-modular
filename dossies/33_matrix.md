# Dossiê — Módulo 33: Matriz de roteamento (`MATRIX`)

**Família:** ROUTE
**Estado:** **implementado — marco 3** (2026-09-04)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Matrix.hpp`, `tests/test_matrix.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `MIXER` soma 4 canais → 1 saída. Faltava a **matriz** — 4 entradas ×
4 saídas onde CADA cruzamento é um ganho (atenuversor), como as matrizes
de pinos do EMS Synthi / ARP 2500 ou o Doepfer A-138m. Patch denso sem
espaguete: liga 4 fontes e 4 destinos com 8 cabos e controla os 16
roteamentos nos knobs.

- **`in1..in4`** (áudio/CV) → **`out1..out4`**; `out_k = level · sat(Σ_j
  in_j · g_jk)`;
- **`g11..g44`** (16 células, −1..+1) = ganho de cada cruzamento;
  atenuversor (negativo = inverte). Padrão = **identidade** (g11=g22=
  g33=g44=1, resto 0) → passa-direto ao carregar;
- **`level`** (0..2) = escala geral da saída;
- **`norm`** (0..1) = normalização por coluna — `out_k /= max(1, Σ_j
  |g_jk|)` interpolado por `norm`; 0 = soma crua (mais células abertas =
  mais alto), 1 = nível constante independente de quantas células abrem;
- **`sat`** (0..1) = saturação suave na saída (`lerp(x, tanh-ish, sat)`)
  — mantém a matriz utilizável como realimentação sem estourar;
- **`drift`** (0..1) = **desvio lento dos ganhos** (assinatura RASGO): 
  cada célula ganha um wobble senoidal próprio (±0,12, escala de dezenas
  de segundos) — o roteamento "respira".

Desvio Rasgo: `drift` (a matriz que se move); `norm` por coluna; `sat`
que torna a matriz segura em laço (as saídas podem voltar às entradas
por cabo → rede de realimentação, trabalho do grafo, não do módulo).

Sem alocação / lock / IO em `process()`. Determinístico (o `drift` é
soma de senos, sem RNG).

**Testes (13 funções, Debug + Release):** `ring=1` na coluna 1 com
`g11=g21=1` + 2 senoides → energia nas bandas soma/diferença, pouca nas
fundamentais (ring-mod de 4 quadrantes); `ring=0` → soma linear (as
fundamentais passam); identidade (padrão) → `out_k
== in_k`; `g21=1` (resto 0) → `out1 == in2`; célula negativa → inverte;
soma: `g11=g21=0.5` → `out1 == 0.5·(in1+in2)`; `level=2` → dobra;
`norm=1` com 2 células a 1 numa coluna → `out` = média, não soma (nível
constante vs `norm=0`); `sat` alto + entrada forte → limita (|out| <
entrada), `sat=0` → transparente; `drift`=0 → dois renders idênticos e
os ganhos não se mexem; `drift` alto → os ganhos efetivos variam ao
longo do tempo (medido); cada saída é soma SÓ da sua coluna (mudar
`g_j2` não afeta `out1`); nada conectado → todas as saídas 0; tudo
finito; grafo `OSC/NOISE → MATRIX → 2× FILTER` com routing cruzado.

**Ring-mod (feito — 2026-09-04):** `ring` (0–1) cruza cada coluna entre a
soma linear e o **produto** das entradas ponderado pelo ganho — célula
`g~0` = fator unitário (bypass), `g~±1` = ±entrada. Dois ganhos em 1 →
ring-mod de 4 quadrantes. `ring=0` = idêntico ao antigo. `prod` clampado
a ±4 antes do `level`/`invn`/`sat`.

**Grade clicável (feito — 2026-09-04):** o painel gráfico (`panel_main`)
desenha a matriz 4×4 como grade clicável, os 16 knobs não são desenhados
(seguem no `panel()` pro ASCII). Ver `RM-PANEL-DISPLAYS`.

**Pendências (candidatos):** grade N×M configurável (6×4, 8×8);
`slew` nos ganhos (transição suave ao mexer no knob ao vivo);
expor a **matriz do motor** (`matrixCell`) como
alternativa a este módulo auto-contido.

---

## 1. Problema musical e papel no fluxo

A matriz é o oposto do cabo: em vez de uma conexão de cada vez, todas
as conexões possíveis entre um conjunto de fontes e um de destinos,
cada uma com um botão. É como se pensa modulação em bloco — "essa fonte
um pouco em todo lugar", "troca a fonte de tudo de uma vez". Com `drift`,
vira uma rede de modulação que evolui sozinha.

Papel: roteamento e mistura, entre grupos. `OSC` + `NOISE` + `LFO` +
`ENVELOPE` nas 4 entradas; `FILTER.cutoff` + `VCA.cv` + `SHAPE.fold` +
`SPACE.mix` nos 4 destinos (via `out → *_mod`) → um painel de modulação.
Ou 4 vozes → 4 processadores com mistura cruzada. No `seedPatch` v2:
destino de várias fontes (`voice`/`bus`/`slow`), fonte de `bus`/`mod`
pras 4 saídas.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **EMS Synthi / ARP 2500** (matriz de pinos, teoria pública) | toda conexão fonte×destino disponível ao mesmo tempo; o pino é a conexão | prática de domínio público |
| **Doepfer A-138m / A-100 matrix; Befaco / Erica matrix mixer** | N×M com ganho (atenuversor) por cruzamento; soma por coluna | ficha/prática |
| **Serge / Buchla "matrix mixer"** | normalização por coluna; matriz como bloco de modulação | prática pública |
| **camada MATRIZ do `SignalGraph` do Rasgo** (`matrixSources`/`matrixSlots`/`matrixCell`, marco 2) | a mesma ideia já no motor — este módulo é a versão auto-contida | código do autor |

## 3. Modelo — matemática, estados, extremos

`drift` (1 vez por bloco): `driftPhase += 2π·0.03·(bloco/sr)` ;
`dw[j][k] = drift·0.12·sin(driftPhase·(1 + 0.3·j) + 1.7·k)`.

Por amostra:
```
para cada saída k (0..3):
  acc = 0 ; wsum = 0
  para cada entrada j (0..3):
    g = clamp(g_jk + dw[j][k], −1.5, 1.5)
    acc  += in_j · g
    wsum += |g|
  n   = 1 + norm·(max(1, wsum) − 1)         # divisor interpolado
  y   = level · (acc / n)
  out_k = y + (softclip(y) − y)·sat          # sat=0 transparente
```
`softclip(y) = y / (1 + |y|·0.7)` (aproximação de tanh, barata, ímpar).

**Estados:** `driftPhase`, `dw[4][4]` (recalculado por bloco). Sem
alocação. Os 16 ganhos vêm dos parâmetros.

**Extremos.** Nada conectado → todas as saídas 0 (é mixer). Padrão
(identidade) → `out_k = in_k`. Todas as 16 células a 1 → `out_k` = soma
das 4 entradas (com `norm=0`); `norm=1` → média. Célula a −1 → inverte.
`level=2` + soma de 4 entradas fortes + `sat=0` → pode passar de ±4 (o
destino que se cuide, ou liga o `sat`). `drift`=1 → os ganhos oscilam
±0,12 em torno do valor do knob; uma célula em 0 pode abrir levemente
(pretendido — a rede respira). `g_jk` fora de ±1 só via `dw` (o
`clamp(±1.5)` segura).

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → silêncio. Padrão identidade: se as 4
  entradas recebem sinal, as 4 saídas repetem — a matriz "não faz nada"
  até você mexer nas células (comportamento honesto de um patchbay).
- **Performance:** os 16 knobs são o instrumento — abrir/fechar
  cruzamentos ao vivo, inverter com o lado negativo; `level` e `norm`
  domam o nível; `drift` liga a evolução.
- **Híbrida:** `LFO`/`ENVELOPE`/`NOISE`/`DRIFT` nas entradas +
  `out1..4 → *_mod` de 4 módulos = bloco de modulação. `SEQUENCE.pitch`
  numa entrada + várias saídas → a mesma melodia espalhada com pesos
  diferentes.

## 5. Portas, parâmetros, limites

**Entradas:** `in1`, `in2`, `in3`, `in4` (Audio).
**Saídas:** `out1`, `out2`, `out3`, `out4` (Audio).
**Parâmetros:** `g11`..`g44` (16, −1..1; diagonal def 1, resto def 0),
`level` (0–2, def 1), `norm` (0–1, def 0), `ring` (0–1, def 0),
`sat` (0–1, def 0),
`drift` (0–1, def 0).
**Limites:** saída não clampada dura (só o `sat` opcional); com `sat=0`
e ganhos altos pode exceder ±1 — é matriz, não estágio de saída. CPU:
16 multiplicações-acumulações + 1 `softclip` por amostra por saída; 16
`sin` por bloco (drift). Sem alocação, sem RNG.

## 6. Alternativas descartadas

- **Grade clicável no painel (widget novo):** feito (2026-09-04) sem
  `Widget::Kind` novo — o `panel_main` desenha a grade e trata o clique
  como um caso especial de `type()=="MATRIX"`, os 16 knobs seguem no
  `panel()` pro renderizador ASCII. Ver `RM-PANEL-DISPLAYS`.
- **Expor `matrixCell` do motor em vez de módulo:** a camada matriz do
  `SignalGraph` faz isso globalmente (todo nó × todo nó). Um módulo
  `MATRIX` auto-contido de 4×4 é mais legível pra um sub-patch e cabe
  no modelo "tudo é módulo". As duas coisas coexistem.
- **Células em ring-mod:** entrou (2026-09-04) como `ring` (0–1) que
  cruza cada coluna soma↔produto, sem quebrar a intuição de mixer em
  `ring=0`. Célula `g~0` = bypass unitário (não zera o produto).
- **N×M configurável já:** 4×4 é o tamanho clássico (Synthi tinha
  bancos assim) e cabe no painel; 8×8 pede repensar a apresentação.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** padrão → `out_k[f] == in_k[f]` exato; `g21=1` e resto 0 →
`out1 == in2`; `g11=−1` → `out1 == −in1`; `g11=g21=0.5` → `out1 ==
0.5·(in1+in2)`; `level=2` → saída ×2; `norm=1`, coluna 1 com `g11=g21=1`
→ `out1 == 0.5·(in1+in2)` (média), vs `norm=0` → `in1+in2` (soma);
`sat=0.9` + entrada ±2 → `|out| < 2`; `sat=0` → sem alteração; `drift=0`
→ dois renders byte-idênticos e `out1` estável com entrada DC;
`drift=0.8` → `out1` com entrada DC varia (a célula respira); mexer
`g32` não muda `out1` (colunas independentes); nada conectado → saída 0.

**Escuta:** os 16 knobs viram um instrumento de mistura de verdade
(abrir um cruzamento tem efeito claro e imediato)? `norm` mantém o
volume ao abrir mais células sem "achatar" o som? `drift` num bloco de
modulação faz o patch evoluir de forma orgânica (não aleatória)? a
matriz em laço (saída→entrada por cabo) com `sat` fica num
comportamento estável e musical em vez de estourar?

## 8. Integração e painel

Classe `Matrix` (`type()` = `"MATRIX"`), 4 entradas, 4 saídas, 21
parâmetros. `panel()` próprio (~20 HP): grade 4×4 (knobs `11`..`44` no
`panel()` / grade clicável no painel gráfico), jacks `IN1..4` na coluna
esquerda (alinhados às linhas), `OUT1..4` na fileira de baixo (alinhados
às colunas), macros LEVEL/NORM/RING/SAT/DRIFT à direita. Testado isolado (identidade, soma, inversão,
norm, sat, drift, independência de colunas) antes do patch. Cadeias
canônicas: `OSC`+`NOISE` → `in1`/`in2`, `out1`/`out2` → 2× `FILTER`
com mistura cruzada; `LFO`+`ENV` → entradas, `out → *_mod`. Adicionado
ao catálogo do painel (`apps/panel/ModuleCatalog.hpp`) — não há família
ROUTE no catálogo; entra em SEQUENCE junto do `SWITCH`
(roteamento), ou em MIX. Escolhido **MIX** (é mixer N×M).
