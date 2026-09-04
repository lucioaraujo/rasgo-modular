# Dossiê — Módulo 24: Modelador de timbre (`SHAPE`)

**Família:** TRANSFORM
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Shape.hpp`, `tests/test_shape.cpp`
**Candidato registrado:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

A relação `Fold`/`RingMod` do `Cable` é um começo, mas um módulo
**visível** torna o timbre por dobras legível pra quem não descobre a
relação de cabo. Uma cadeia de modelagem num módulo só, com VCA de saída.

Cadeia em série (por amostra):
- **ring mod** — `x·mod` misturado a seco por `ring` (precisa do `mod`
  conectado; sem ele, `ring` não faz nada);
- **wavefolder** — dobra triangular fechada (`ph = u/4 + ¼` ;
  `4·|ph − round(ph)| − 1` — ~linear perto de 0, reflete a cada ±2),
  profundidade por `fold` (drive 1→7×). `symmetry` (−1…+1) injeta um
  bias DC **antes** da dobra → dobras assimétricas = harmônicos pares
  (o controle de "timbre" do Buchla 259/258);
- **wrap** — `wrap` mistura *soft-clip* (`tanh`) com *wrap-around*
  (`x − 2·round(x/2)`, dente-de-serra que reentra em ±1) — de contido a
  destruído;
- **saturação** — `tanh` normalizada, drive por `sat`;
- **VCA de saída** — `level` (0–1).
- desvio **`drift`** — oscilação lenta minúscula (±~4 %) no drive da
  dobra, xorshift semeado. `drift = 0` → determinístico.

Sem alocação / lock / IO em `process()`.

**Testes (11 funções, Debug + Release — `tests/test_shape.cpp`):**
`ring = 1` + `mod` senoidal → saída = produto (bandas soma/diferença,
sem a fundamental); `ring = 0` → passa; `fold` alto num seno → mais
energia harmônica alta (dobras) e a saída fica limitada a ~±1;
`symmetry ≠ 0` → aparece 2º harmônico (energia em 2·f) que não existia;
`wrap = 1` num sinal forte → descontinuidades (wrap-around) vs `wrap = 0`
suave; `sat` aumenta o achatamento; `level` escala a saída; `drift`
altera < 6 % e `drift = 0` é determinístico; tudo finito e ≤ ~1,05;
dois renders byte-idênticos; grafo `OSC → SHAPE → FILTER`.

**Antialiasing (feito — 2026-09-04):** dobra + wrap + sat rodam a **2×**
(`src/dsp/Oversampler.hpp` — upsample linear + FIR meia-banda de 13 taps)
**e** com **ADAA de 1ª ordem** dentro do laço 2×: a antiderivada fechada
de `m(u)` (`∫folded = 8·w·|w| − 4·w`, `∫wrapped = 2·s²` — ambas contínuas
em todo `u`). Atraso de grupo ~2,5 amostras. Medido (vs. a matemática a
16×): erro-vs-ideal cai de ~1,4 pra ~0,55 nos casos moderados; piso de
alias em bins não-harmônicos < 1% da fundamental com `fold` alto; uso
"limpo" (`fold`/`wrap` = 0) fica em **THD 0,02%** (transparente). Caso
extremo `f0` grave + `fold` no talo ainda passa (precisaria de 4×).

**Pendências (candidatos):** 4× opcional pro `fold` no talo com nota
grave; `fold` com número de dobras explícito; segunda entrada de `mod`
pra cross-mod; `spread` entre estágios (Warps/Atlas §39).

---

## 1. Problema musical e papel no fluxo

O Rasgo tem filtro (subtrativo), EQ (`PARAMETRIC`) e o `MATTER`/`STRING`
(físico). Falta a **síntese por distorção controlada** — pegar um seno
pobre e enriquecer com dobras e ring-mod, o gesto da costa oeste
(Buchla/Serge). A relação de cabo faz `fold`/`ring`, mas escondido e sem
os controles de caráter (`symmetry`, `wrap`, `sat`).

Papel: transformação, depois de uma fonte simples. `OSC.sine → SHAPE →
FILTER` (dá corpo antes de filtrar); `OSC → SHAPE` com `mod` de um 2º
`OSC` (ring-mod clássico, sinos e metais); `SHAPE` no meio de um patch
como "sujador" com VCA.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Buchla 259/258 "Timbre"** | wavefolder + `symmetry` (bias assimétrico = harmônicos pares) | conceito (hardware) |
| **Serge Wave Multipliers / VCM** | dobras em série, cada uma some harmônicos | conceito |
| **Ring modulator clássico** | multiplicador de 4 quadrantes: `x·y` → bandas soma/diferença | teoria pública |
| **Dobra triangular fechada** (`4·\|x/4 − round(x/4)\| − 1`) | folding sem iteração, R → [−1,1] | teoria pública (onda triangular) |
| **Warps (Mutable) / Atlas §39** | "a relação/estágio É o processo" — cadeia de modeladores | conceito |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
x = in
if mod conectado:  x = x + (x·mod − x)·ring          # ring dry/wet
x = x + symmetry·0.5                                  # bias DC (assimetria)
drive = 1 + fold·6·(1 + driftCur)
u = x·drive
ph = u/4 + ¼ ; folded = 4·|ph − round(ph)| − 1        # dobra (linear perto de 0)
wrapped = u − 2·round(u/2)                            # wrap-around ±1 (dente de serra)
y = folded + (wrapped − folded)·wrap                  # dobra suave ↔ wrap seco
y = y + (tanh(y·1.5) − y)·sat                         # sat: 0 = transparente
out = y·level
```
`driftCur` desliza pra um alvo xorshift novo a ~8 Hz (±`drift`·0,04).

**Estados:** `driftCur_`, `driftTgt_`, `driftCounter_`, `rng_`. Sem
alocação. Não tem memória de amostra (é sem estado além do drift) — cada
amostra só depende da entrada e dos parâmetros.

**Extremos.** `mod` ausente → `ring` inerte (correto). `fold = 0`,
`wrap = 0`, `sat = 0`, `symmetry = 0` → `out = tanh(in)·level` (quase
transparente até ±0,7). `fold = 1` num seno de amplitude 1 → muitas
dobras, timbre "elétrico", saída em ±1. `wrap = 1` + sinal > ±1 → saltos
bruscos (é o ponto — som digital agressivo). `in` DC → `out` DC modelado
(sem oscilação — folder não auto-oscila). `level = 0` → silêncio. Reset
→ drift zerado, RNG re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** sem entrada, `out = 0` (é processador — não gera).
  Aceitável: o `SHAPE` no rack de partida sempre tem uma fonte antes.
  (Um `drift` audível sem entrada seria controle fictício.)
- **Performance:** `fold`/`symmetry`/`wrap`/`sat` ao vivo varrem de
  "seno limpo" a "quebrado" com transição contínua; `ring` abre a
  modulação em anel; `level` é o VCA.
- **Híbrida:** `mod` de um 2º oscilador (ring-mod dependente de altura);
  `fold_mod` de um envelope (as dobras abrem no ataque — timbre dinâmico);
  `symmetry` de um LFO lento (o brilho respira).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `mod` (Audio — carrier do ring-mod),
`fold_mod` (Control — CV sobre o `fold`).
**Saídas:** `out` (Audio).
**Parâmetros:** `ring` (0–1, def 0), `fold` (0–1, def 0), `symmetry`
(−1…+1, def 0), `wrap` (0–1, def 0), `sat` (0–1, def 0,15), `level`
(0–1, def 0,8), `drift` (0–1, def 0).
**Limites:** saída ≤ ~1,05 (a saturação final segura). CPU: o núcleo
(dobra + wrap + ADAA + `tanh`) roda **2× por amostra** pelo oversampler
(~2 `tanh` + FIR de 6 MACs por sub-amostra). Sem alocação, sem RNG no
caminho do antialias — determinístico. Atraso de grupo ~2,5 amostras.

## 6. Alternativas descartadas

- **Wavefolder iterativo (reflete N vezes):** a fórmula triangular
  fechada dá o mesmo espectro sem loop e é O(1).
- **Ring-mod com sub-oscilador interno** (sem precisar de `mod`): tira a
  patchabilidade; um cabo de um `OSC` é o idioma modular.
- **Oversampling 4× já no marco 1:** a dobra alias, sim; mas a 1ª
  entrega foi "completa no que promete" (modelar timbre); o antialias
  entrou na 2ª camada (2026-09-04) como **2× + ADAA** — o 2× sozinho
  deixa resíduo entre fs/2 e fs, o ADAA sozinho não basta pra nota grave
  com `fold` no talo; juntos cobrem quase tudo pelo custo de um 2×.
- **Oversampler compartilhado pra tudo (sem ADAA):** só 2× levaria a
  ~metade da rejeição; o ADAA fechado do folder é barato (é sem memória)
  e some com o alias na fonte — vale o par.
- **`fold` e `sat` como um knob só ("drive"):** são gestos tímbricos
  diferentes (dobra = espelho/harmônicos ímpares fortes; sat =
  achatamento/compressão). Separados dão mais espaço.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `ring = 1` com `in` e `mod` senos de f1, f2 → energia em
|f1±f2|, ~nada em f1/f2; `ring = 0` → passa; `fold` alto → razão
energia-alta/energia-baixa sobe e |out| ≤ ~1; `symmetry ≠ 0` → aparece
componente em 2·f que era ~0; `wrap = 1` → nº de descontinuidades
grandes por ciclo > `wrap = 0`; `sat` sobe → fator de crista cai;
`level` escala linear; `drift = 0` → dois renders byte-idênticos; tudo
finito.

**Escuta:** o folder soa "Buchla" (elétrico, vocálico) ou "digital
sujo"? `symmetry` muda o caráter de "oco" pra "encorpado"? o ring-mod
dá sino/metal ou só chiado? `wrap` no talo é usável ou só barulho? dá
pra ir de transparente a destruído girando um knob só de cada vez?

## 8. Integração e painel

Classe `Shape` (`type()` = `"SHAPE"`), 3 entradas, 1 saída, 7
parâmetros. `panel()` próprio (~10 HP): knobs RING/FOLD/SYM/WRAP/SAT/
LEVEL/DRIFT, jacks IN/MOD/FCV in, OUT, Display (a curva de transferência
ou a forma de onda de saída). Testado isolado (ring, fold, simetria,
wrap, sat, level, drift, determinismo) antes do patch. Cadeias
canônicas: `OSC.sine → SHAPE → FILTER`; `OSC → SHAPE` + `OSC2 →
SHAPE.mod` (ring-mod); `ENVELOPE.env → SHAPE.fold_mod` (dobras
dinâmicas). Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família TRANSFORM).
