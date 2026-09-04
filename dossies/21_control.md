# Dossiê — Módulo 21: Utilidades de CV (`CONTROL`)

**Família:** UTILITY
**Estado:** **implementado — marco 3** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Control.hpp`, `tests/test_control.cpp`

## Estado da implementação (marco 3)

Quarto dos essenciais (`PESQUISA_MODULOS.md §2.1`). A "gramática do
sistema" — escalar, deslocar, somar e atrasar CV — estava espalhada em
lugar nenhum: o `gain` do `Cable` escala, o `connectToParameter` tem
`depth`+`offset`, mas **não há um módulo visível** que o músico plugue
pra dizer "inverte esse LFO, tira metade, soma um offset, e faz colar".
Sem isso o patch fica ilegível (a transformação some dentro do cabo).

- **duplo** (2 canais independentes de processamento de CV);
- **`scale`** (−2…+2) — **atenuversor**: `<0` inverte, `|·|<1` atenua,
  `>1` amplifica. `scale = 0` → o canal vira **fonte de tensão** (só o
  offset);
- **`offset`** (−1…+1) — soma uma constante (depois do `scale`);
- **`rectify`** (0…1) — 0 passa, 0,5 meia-onda (corta o negativo), 1
  onda-completa (`|x|`). Retificar + `slew` = **seguidor de envelope**;
- **`slew`** (0…1) — tempo de deslizamento (0 = instantâneo; 1 ≈ 2 s).
  Glide/portamento, gate→rampa, suavizar S&H;
- **`curve`** (0…1) — 0 = **slew linear** (inclinação constante — o
  portamento "de verdade", independe do tamanho do salto); 1 = **lag
  exponencial** (RC — o seguidor de envelope). Contínuo entre os dois
  (padrão `curve` do `ENVELOPE`);
- **`sum`** (saída) — `out1` + `out2`, com `sum_mode` 0 = soma (com teto)
  / 1 = média. Mixer/somador de CV de brinde;
- **desvio `drift`** (0…1) — passeio lento minúsculo somado ao offset
  (xorshift semeado, ±~1,5 %). `drift = 0` → determinístico. Precisão é o
  ponto deste módulo; o `drift` é opt-in pra "humanizar" um offset parado.

Um stream xorshift semeado em `prepare()` (só pro `drift`) →
determinístico (dois renders byte-idênticos). Sem alocação, sem lock/IO
em `process()`.

**Testes (14 funções, Debug + Release — `tests/test_control.cpp`):**
`scale` bate (ganho medido);
`scale < 0` inverte; `scale = 0` → só offset (fonte de tensão constante);
`offset` soma a constante certa; `rectify = 1` → `|x|`; `rectify = 0.5`
→ negativo zerado, positivo intacto; `slew = 0` → passa na hora; `slew`
alto + degrau → rampa (derivada por amostra pequena, sem overshoot);
`curve = 0` linear (inclinação ~constante em saltos de tamanhos
diferentes) vs `curve = 1` exponencial (inclinação decai); `rectify +
slew` num seno de áudio → segue a amplitude (envelope); `sum` = soma /
média conforme `sum_mode`; `drift` altera < 3 % e `drift = 0` é
determinístico; tudo finito; dois renders byte-idênticos; integração no
grafo (`SEQUENCE.pitch → CONTROL → QUANTIZER` com portamento).

**Pendências (candidatos, não controles fictícios):** slew assimétrico
(rise ≠ fall — Maths/Serge DUSG); `slew_cv` (CV sobre o tempo de slew);
detector de inclinação / comparador (gate quando `in` cruza um limiar);
"track & hold" (segura enquanto um gate está alto); quantização a
inteiros (fica no `ABACUS`); um terceiro canal só de soma.

---

## 1. Problema musical e papel no fluxo

Todo sistema modular precisa da camada chata e essencial: **pegar uma
tensão e mexer nela**. Um LFO que oscila 0…1 mas você quer −0,3…+0,7. Um
envelope que precisa abrir o filtro *pra baixo*. Duas modulações que
deveriam somar antes de entrar num destino. Uma melodia de sequenciador
que precisa de *glide* entre as notas. Um sinal de áudio cuja **amplitude**
deveria virar CV pra abrir um VCA (seguidor de envelope).

Sem `CONTROL` isso ou não dá pra fazer, ou fica escondido: o `Cable.gain`
atenua mas não inverte nem soma; o `connectToParameter` tem `depth` e
`offset` (e desde 2026-09-04 é aditivo sobre o knob, §36.2), mas não
aparece no painel — o músico não *vê* a transformação.

Papel: utilidade, no meio de qualquer cadeia de modulação. `LFO →
CONTROL → destino` (escala/inverte/desloca); `SEQUENCE.pitch → CONTROL →
QUANTIZER` (portamento); `áudio → CONTROL (rectify+slew) → VCA.cv`
(seguidor de envelope); `CONTROL` sozinho = fonte de CV manual (offset)
ou lentamente à deriva (`drift`).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Make Noise Maths** (canais 1 e 4: atenuversor + somador; canais 2/3: slew/função) | um canal de CV = atenuverter + offset + slew, e a soma dos canais numa saída dedicada | conceito (hardware) |
| **Serge DUSG / "Smooth & Stepped Generator"** | slew limiter com rise/fall separados; retificação; a mesma célula faz glide, LFO, seguidor | conceito |
| **Seguidor de envelope RC clássico** (qualquer texto de síntese) | retifica + filtra passa-baixa de 1 polo = envoltória da amplitude | teoria pública |
| **Slew limiter de inclinação constante** (portamento de sintetizador mono, MS-20/Minimoog) | glide linear: taxa fixa de V/s, independe do tamanho do intervalo — soa diferente do RC | prática |
| **`curve` do `ENVELOPE` do Rasgo** | um knob contínuo côncavo↔convexo em vez de um seletor lin/exp | código do autor |
| **`shape`/`spread` do `DECISION`** | precisão é o padrão; o "humano" (`drift`) é opt-in e pequeno | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra, canal *k* ∈ {1, 2}:
```
x   = in_k                                   (0 se não conectado)
xr  = lerp(x, |x|, rectify_k) = x + (|x| − x)·rectify_k
      # rectify=0 → x ; rectify=0.5 → 0.5x + 0.5|x| = max(x,0) ; rectify=1 → |x|
tgt = scale_k · xr + offset_k + driftCur_k
```
`driftOffset_k`: se `drift > 0`, passeio xorshift ±(drift·0,015),
atualizado a ~8 Hz e interpolado; senão 0.

Slew até `tgt` a partir de `y_k` (estado):
```
dt        = 1/sr
slewTime  = slew_k² · 2.0            [s]      (slew_k=0 → 0)
se slewTime < dt:  y_k = tgt                  (instantâneo)
senão:
  # componente LINEAR (inclinação constante)
  step   = dt / slewTime                       (fração do range 2.0 por amostra)
  lin    = y_k + clamp(tgt - y_k, -step·2, +step·2)
  # componente EXPONENCIAL (RC)
  a      = 1 - exp(-dt / slewTime)
  expo   = y_k + (tgt - y_k)·a
  y_k    = lerp(lin, expo, curve_k)
out_k = y_k
```
Saída de soma:
```
s   = out1 + out2
sum = sum_mode < 0.5 ? clamp(s, -1, 1) : s · 0.5
```

**Nota retificação.** O lerp `x→|x|` passa por `0.5·x + 0.5·|x|` em
`rectify_k = 0.5`, que é **exatamente** `max(x, 0)` (meia-onda). O modelo
é contínuo e o ponto médio já é meia-onda — sem ramo especial.

**Estados:** `y_[2]` (saída pós-slew), `driftPhase_`, `driftCur_[2]`,
`driftTgt_[2]`, `rng_`. Sem alocação.

**Extremos.** `scale = 0` → `out = offset` (+ drift): fonte de CV, útil e
correto. `slew = 1` + `in` mudando rápido → `y` mal acompanha (glide
lento) — vira quase DC no valor médio. `rectify = 1` + `scale < 0` →
envoltória **invertida** (abre o filtro pra baixo). `offset` no teto +
`scale·x` estourando → `out` pode passar de ±1; **não** é clampado por
canal (CV pode passar de ±1 no Rasgo — 1.0 = 1 oitava/unidade, não um
teto); só o `sum` no modo soma é clampado. `in` de áudio com `slew = 0`
e `rectify = 0` → o áudio passa direto (o `CONTROL` é transparente).
Reset → `y_`, drift zerados, RNG re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado; `scale = 0`, `offset` ajusta uma CV
  constante (fonte de tensão manual — transpõe um oscilador, abre um
  filtro). `drift > 0` → a constante deriva devagar sozinha. `sum`
  combina as duas.
- **Performance:** `scale`/`offset` ao vivo = transpor / inverter /
  deslocar uma modulação existente sem repatch; `slew` = quanto a
  mudança "cola" (de degraus a glide preguiçoso). `sum_mode` alterna
  soma↔média sem estalo.
- **Híbrida:** `SEQUENCE.pitch → in1`, `scale = 1`, `slew` médio →
  portamento na melodia; `LFO → in2` atenuvertido, e `sum` leva a
  soma pra um destino; `áudio → in1`, `rectify = 1`, `slew` médio,
  `curve = 1` → seguidor de envelope que abre um `VCA`/`FILTER`.

## 5. Portas, parâmetros, limites

**Entradas:** `in1`, `in2` (ambas Audio — CV e áudio compartilham o tipo
no Rasgo).
**Saídas:** `out1`, `out2`, `sum` (todas Audio).
**Parâmetros (por canal, k = 1,2):** `scale_k` (−2…+2, def 1),
`offset_k` (−1…+1, def 0), `rectify_k` (0…1, def 0), `slew_k` (0…1, def
0), `curve_k` (0…1, def 0).
**Parâmetros globais:** `sum_mode` (0…1, def 0 — soma/média),
`drift` (0…1, def 0).
**Limites:** CV não é clampada por canal (só `sum` no modo soma). CPU por
amostra: 1 `exp` + aritmética por canal (o `exp` do slew pode ser
tabelado numa 2ª camada se pesar). Sem alocação.

## 6. Alternativas descartadas

- **Canal único (como um atenuverter avulso):** dois canais + soma cobre
  atenuversor, offset, mixer de CV e média num módulo só — é o que o
  Maths faz e o que um patch quer.
- **Seletor lin/exp pro slew:** o knob `curve` contínuo (padrão do
  `ENVELOPE`) dá os intermediários e é coerente com o resto do Rasgo.
- **`slew` em segundos (ms…s) direto:** o mapeamento `slew²·2 s` dá
  resolução fina no começo (onde mora o portamento musical) e alcança 2 s
  no teto; um knob 0…1 é mais legível no painel.
- **Retificação como toggle (off/half/full):** o knob contínuo `rectify`
  permite "quase meia-onda" e casa com a estética de knobs do Rasgo; o
  ponto médio já é meia-onda exata (ver §3).
- **`drift` maior / sempre ligado:** este módulo é a régua do sistema.
  Precisão é o padrão; o `drift` fica pequeno e opt-in.
- **Clampar a CV por canal em ±1:** no Rasgo 1.0 é uma unidade (1 V/oct),
  não um teto; clampar quebraria transposições legítimas.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `scale` — ganho medido bate com o knob (±); `scale < 0`
inverte a fase; `scale = 0` → saída = `offset` (±tolerância do drift);
`offset` soma a constante certa; `rectify = 1` → `|x|` (energia dobra a
frequência); `rectify = 0.5` → metade negativa zerada, positiva intacta;
`slew = 0` → saída = alvo amostra a amostra; `slew` alto + degrau →
sobe monotônico sem overshoot, derivada por amostra pequena; `curve = 0`
→ inclinação ~igual pra saltos de tamanhos diferentes (linear); `curve =
1` → inclinação decai ao se aproximar (exponencial); `rectify + slew` num
seno de 200 Hz → saída ≈ amplitude do seno (envelope), sem ripple grande;
`sum` = soma (clampada) ou média conforme `sum_mode`; `drift = 0` →
dois renders byte-idênticos; tudo finito.

**Escuta:** o portamento (`in` = sequência, `slew` médio, `curve = 0`)
soa como um sintetizador mono "de verdade" (glide de tempo constante) ou
"elástico" demais? Com `curve = 1` o glide soa mais "natural/vocal"? O
seguidor de envelope abre o filtro *junto* com a batida ou atrasado
demais? `scale` negativo num LFO triangular — o movimento inverte de
forma óbvia? `drift` num offset parado — dá "vida" ou só instabiliza?

## 8. Integração e painel

Classe `Control` (`type()` = `"CONTROL"`), 2 entradas, 3 saídas, 12
parâmetros. `panel()` próprio (~10 HP): Display (dois valores de saída
como barras), duas fileiras de knobs (SCALE/OFFSET/RECT/SLEW/CURVE por
canal), toggle SUM (soma/média), knob DRIFT, jacks IN1/IN2 · OUT1/OUT2/
SUM. Testado isolado (ganho, inversão, offset, retificação, slew lin/exp,
seguidor de envelope, soma/média, drift, determinismo) antes do patch.
Cadeias canônicas: `LFO → CONTROL → *_mod` (escala/inverte/desloca);
`SEQUENCE.pitch → CONTROL(slew) → QUANTIZER` (portamento);
`voz de áudio → CONTROL(rectify+slew) → VCA.cv` (seguidor de envelope);
`CONTROL` isolado = fonte de CV manual. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família UTILITY — junto de `VCA`).
