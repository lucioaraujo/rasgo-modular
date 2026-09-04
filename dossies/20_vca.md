# Dossiê — Módulo 20: Amplificador (`VCA`)

**Família:** TRANSFORM / UTILITY
**Estado:** **implementado — marco 3** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Vca.hpp`, `tests/test_vca.cpp`

## Estado da implementação (marco 3)

Feito: o **VCA avulso** que faltava. O `ENVELOPE` embute um VCA, mas não
dá pra pôr um VCA no meio da cadeia de modulação, fazer AM em áudio, um
tremolo, ou usar o VCA como somador/atenuador de CV. `VCA` é **duplo**
(2 canais independentes) e serve de mixer de 2 entradas de brinde.

Por canal (×2):
- **`in`** (áudio ou CV) × **ganho**;
- **ganho = `level` (knob) + `cv_amount` · CV** — a entrada `cv` é
  **atenuvertida** (`cv_amount` −1..1) e **soma** ao knob. Sem CV
  conectada, o knob sozinho manda (o VCA também é atenuador manual).
  É modulação por **porta de verdade** — o knob fica vivo (ao contrário
  do `connectToParameter`);
- **`response`** (0 linear → 1 exponencial): `ganho_final = g^(1+3·resp)`
  — linear pra somar CV, exponencial ("dB-linear") pra volume de áudio;
- ganho suavizado (1 polo ~1,5 ms) — sem zipper noise;
- **saturação suave** perto do teto (`tanh` acima de ~0,9) — um VCA real
  tem um som, não é um multiplicador perfeito.

**Global:** `drift` (0–1) — desvio Rasgo: uma oscilação lenta e
correlacionada nos dois ganhos (xorshift semeado), ±~3 % no máximo. "O
VCA respira." `drift = 0` → saída determinística.

**Saídas:** `out1`, `out2`, `sum` (= out1 + out2, com teto suave) — o
`sum` faz o `VCA` valer como um mini-mixer de 2 canais.

Sem alocação, sem lock/IO em `process()`.

**Testes (11/11, Debug + Release):** ganho `level` bate (0,5 → metade);
`level = 0` → silêncio; CV atenuvertida soma ao knob (`cv_amount = 1`,
CV = 0,5, knob = 0 → ganho 0,5; `cv_amount = −1` inverte); `response`
alto encurva (ganho a meio-caminho < linear); suavização — degrau de
ganho não estala (derivada limitada); saturação segura AM forte perto de
±1; `sum` = out1 + out2; `drift` altera a saída mas fica < 5 %; dois
renders byte-idênticos; canais independentes; integração
(`NOISE.pink → VCA` · `ENVELOPE.env → VCA.cv` = voz com amplitude).

**Pendências (candidatos, não controles fictícios):** over-unity
(ganho > 1, como Intellijel Quad VCA); modo bipolar/ring (mas o `Cable`
RingMod já faz); `response` contínuo com curva de crossfade real (hoje é
potência); CV logarítmica de verdade (tabela) em vez da aproximação por
potência; 4 canais (Quad).

---

## 1. Problema musical e papel no fluxo

O `VCA` é o verbo "multiplicar" da gramática (`§20`). Sem ele: não dá
pra fazer um envelope *externo* controlar o volume de uma voz que não
tem VCA embutido; não dá tremolo (LFO × áudio); não dá AM; não dá pra
escalar uma CV antes de mandar pra um parâmetro; não dá pra fazer
ducking. O `ENVELOPE` resolve o caso "envelope → própria saída", mas
`SEQUENCE`, `DECISION`, `NOISE.smooth`, um `FUNCTION` lento — nenhum tem
VCA. É o utilitário mais usado de qualquer rack.

Papel: fica no meio. `voz → VCA` com `ENVELOPE.env`/`NOISE`/`FUNCTION` no
`cv`. Ou `CV → VCA` como atenuador. Ou 2 fontes → `sum` como mixer.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **VCA linear vs exponencial** (Doepfer A-132 lin / A-131 exp; qualquer texto) | linear pra CV/somar, exponencial (dB-linear) pra volume percebido | prática |
| **Quad VCA como mixer** (Intellijel, 4ms; ficha) | saídas normalizadas somam → um VCA também é um mixer pequeno | prática |
| **Atenuverter na entrada de CV** (Maths, Frames) | escalar ± a CV antes de aplicar = o knob é a base, a CV soma | conceito |
| **Saturação de VCA analógico** | ganho alto não é linear perfeito — o transistor/OTA satura | conceito |
| **`drift`** (`FUNCTION`/`OSC` do Rasgo) | oscilação lenta seeded = "não é digital-morto" | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra, canal `k` ∈ {1,2}:
```
g = level_k + cv_amount_k · (cv_k conectada ? cv_k[frame] : 0)
g = clamp(g, 0, 1)
g_shaped = g^(1 + 3·response_k)
g_final = g_shaped · (1 + driftState)          driftState ∈ [−0.03, 0.03]
g_smooth_k += (g_final − g_smooth_k) · coef    (coef ~ 1,5 ms)
y = in_k[frame] · g_smooth_k
out_k = softSat(y)                              (tanh acima de ±0,9)
```
`sum = softSat(out1 + out2)`.

drift: a cada ~1200 amostras, `driftState += ruído · 0,0004 · drift²`,
clamp ±0,03.

**Estados:** `gSmooth_[2]`, `driftState_`, `driftCounter_`, `rng_`. Sem
alocação, sem RNG fora do `drift`.

**Extremos.** `cv_amount` extremo + CV grande → `g` clampado a [0,1]
(não há over-unity nesta versão). `response = 1` com `g` pequeno → ganho
quase zero (curva bem convexa) — correto pra volume. `in` já perto de
±1 com ganho 1 → `softSat` segura. `drift` com `in` DC → o DC ganha a
oscilação (esperado). Reset → ganhos suavizados a 0? Não — a `gSmooth_`
começa no `level` (sem clique no arranque). `driftState` zerado, RNG
re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** sem CV, `level` fixo = atenuador manual (2 canais). Com
  `drift` pequeno, o volume tem uma vida sutil.
- **Performance:** `level` ao vivo é o fader; `sum` mixa 2 fontes;
  `response` muda a "pegada" do fade (linear = abrupto, exp = suave).
- **Híbrida:** `ENVELOPE.env → cv1` (voz com envelope externo);
  `FUNCTION` lento → `cv2` (tremolo); `NOISE.smooth → cv` (volume que
  deriva); `DECISION.gate → cv` (VCA como gate).

## 5. Portas, parâmetros, limites

**Entradas:** `in1`, `in2` (Audio), `cv1`, `cv2` (Control).
**Saídas:** `out1`, `out2`, `sum` (Audio).
**Parâmetros:** `level1`/`level2` (0–1, def 0/0 — **começa fechado**,
como um VCA de verdade), `cv1_amount`/`cv2_amount` (−1..1, def 1),
`response1`/`response2` (0–1, def 0), `drift` (0–1, def 0).
**Limites:** saída em ~[−1,1] (softSat). CPU por amostra: 2× (1 `pow` +
1 `tanh` + aritmética). Sem alocação.

## 6. Alternativas descartadas

- **Over-unity no marco 1:** ganho > 1 é útil mas perigoso num sandbox
  de cabeamento livre; fica como 2ª camada com o `OutputStage` de rede
  de segurança.
- **Ring/bipolar:** a relação `RingMod` do `Cable` (Módulo 3) já faz
  x·companion. O VCA é ganho unipolar de propósito.
- **CV logarítmica por tabela:** a aproximação `g^(1+3·resp)` dá o
  contorno certo (quiet no fundo, range no topo) sem tabela. Tabela real
  é 2ª camada.
- **VCA de canal único com mais recursos:** um rack precisa de **pelo
  menos 2** VCAs quase sempre (voz + modulação). Duplo desde o marco 1.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `level` linear bate (0,5 → −6 dB); `level = 0` → −∞;
`cv_amount` atenuvertida soma corretamente ao knob e inverte com sinal
negativo; `response` alto → ganho a meio-caminho menor que linear;
degrau de ganho suavizado (sem descontinuidade audível — derivada por
amostra limitada); `softSat` segura AM a ganho 1 com entrada ±1;
`sum` = out1 + out2 (com teto); `drift` fica < 5 %; dois renders
byte-idênticos; canais não vazam um no outro.

**Escuta:** o fade linear soa "abrupto no fim" e o exponencial "suave"?
o tremolo (LFO → cv) soa limpo ou tem clique? a AM (áudio → cv) gera as
bandas laterais esperadas? o `drift` num pad sustentado dá "vida" ou
soa como falha? o `sum` como mixer tem headroom?

## 8. Integração e painel

Classe `Vca` (`type()` = `"VCA"`), 4 entradas, 3 saídas, 7 parâmetros.
`panel()` próprio (10 HP: 2 tiras de canal — knob LEVEL + knob CV
(atenuverter) + knob RESP —, knob DRIFT, jacks IN/CV in e OUT/SUM out).
Testado isolado (ganho, atenuverter, curva, suavização, saturação,
drift, independência) antes do patch. Cadeias canônicas:
`OSC → VCA.in1` · `ENVELOPE.env → VCA.cv1` (voz com amplitude);
`NOISE.smooth → VCA.cv2` (volume que deriva). Adicionado ao catálogo
(`apps/panel/ModuleCatalog.hpp`, família TRANSFORM).
