# CONTROL — utilidades de CV

**Família:** TRANSFORM · **Módulo 21**
**Essência:** pegar uma tensão e mexer nela — escalar, inverter, deslocar,
retificar, deslizar, somar. A camada chata e essencial, agora visível
num painel.
**Dossiê técnico:** [`../dossies/21_control.md`](../dossies/21_control.md)
· **Fonte:** `src/dsp/Control.hpp`

---

## A ideia

Um LFO que oscila 0…1 mas você quer −0,3…+0,7. Um envelope que precisa
abrir o filtro *pra baixo*. Duas modulações que deviam somar antes de
entrar num destino. Um sinal de áudio cuja **amplitude** deveria virar
CV (seguidor de envelope).

Sem `CONTROL` isso ou não dá, ou fica escondido — o ganho de um cabo
atenua mas não inverte nem soma. O `CONTROL` põe a transformação **no
painel**: dois canais, cada um com atenuversor, offset, retificação,
slew, e uma saída de soma.

## Por dentro

Por canal, na ordem: `SCALE` (atenuversor −2..2, negativo inverte) →
`OFFSET` (constante) → `RECTIFY` (0 passa, 0,5 meia-onda = `max(x,0)`,
1 onda completa = `|x|`) → `SLEW` (deslize, 0–2 s) com `CURVE` (linear =
portamento, exponencial = seguidor RC). A saída `SUM` combina os dois
canais (soma ou média, `SUM MODE`).

**Por que `RECT` + `SLEW` exponencial forma um "seguidor de envelope"
clássico:** um sinal de áudio oscila rápido pra cima e pra baixo de
zero — sua "amplitude" (o quão alto está tocando, agora) não é
diretamente nenhum valor instantâneo dele, é uma noção que precisa
**emergir** ao longo de vários ciclos. A retificação (`onda completa`)
primeiro transforma tudo em positivo — em vez de balançar entre +1 e
−1, o sinal passa a pulsar entre 0 e 1, sempre subindo quando havia
qualquer energia, alto ou baixo. Um `SLEW` exponencial lento então
**suaviza** esses pulsos rápidos numa curva que só acompanha as
variações **lentas** de nível (sobe quando a energia média sobe, desce
quando cai) — filtrando o "tremor" rápido do próprio áudio e deixando
só o contorno de volume. É exatamente o circuito clássico (retificador
+ filtro RC) usado em compressores/sidechains analógicos pra "ler" o
volume de um sinal como uma CV.

## Os jacks, um a um

### Entradas

- **`IN1`** / **`IN2`** (áudio) — a tensão (ou áudio) a transformar.
  **Plugue aqui:** um LFO, um envelope, um `SEQUENCE`, `NOISE.smooth`,
  ou um sinal de áudio (pra retificar em seguidor de envelope).

### Saídas (todas áudio)

- **`O1`** / **`O2`** — cada canal transformado. **Plugue em:** qualquer
  entrada de parâmetro (`_mod`, `cutoff`, `1V/O`…).
- **`SUM`** — a soma (ou média) dos dois canais. **Plugue em:** um
  destino que deve receber duas modulações somadas.

## Os controles, um a um

**SCALE1** / **SCALE2** (−2..2) — o atenuversor. 1 = passa; 0,5 = pela
metade; **negativo = inverte**; 0 = a saída é só o offset (fonte de
tensão manual).

**OFF1** / **OFF2** (offset, −1..1) — constante somada depois do `SCALE`.

**RECT1** / **RECT2** (rectify, 0–1) — retificação contínua. 0 = passa;
0,5 = meia-onda (max(x,0)); 1 = onda completa (|x|).

**SLEW1** / **SLEW2** (0–1) — tempo de deslize (0–2 s). Suaviza degraus.

**CRV1** / **CRV2** (curve, 0–1) — a forma do slew. 0 = linear
(inclinação constante — portamento); 1 = exponencial (RC — seguidor de
envelope).

**SUM MODE** (0/1) — a saída `SUM` é soma (com teto) ou média dos dois
canais.

**DRIFT** (0–1) — passeio lento (opt-in) nos dois offsets — humaniza uma
tensão parada.

## Como cabear

**Inverter uma modulação:**
```
ENVELOPE (env) → CONTROL (IN1)      SCALE1 = −1
CONTROL (O1) → SPACE (mix_mod)      (a reverb fecha quando a nota abre)
```

**Seguidor de envelope:**
```
qualquer voz → CONTROL (IN1)        RECT1 = 1, SLEW1 alto, CRV1 = 1
CONTROL (O1) → VCA (CV)             (o volume de um sinal comanda outro)
```

**Somar duas modulações:**
```
LFO → CONTROL (IN1)                 SCALE1 = 0,3
ENVELOPE → CONTROL (IN2)            SCALE2 = 0,7
CONTROL (SUM) → FILTER (FC)
```

**Fonte de tensão manual:** nada no `IN1`, `SCALE1` = 0, `OFF1` no valor
que você quer — `O1` é uma tensão fixa ajustável.

## Potencializar

- **Bias de um LFO:** `SCALE` + `OFFSET` num LFO 0…1 → qualquer faixa
  que o destino precisar.
- **Rectify parcial:** `RECT` em 0,3 e um LFO triangular — meia-onda
  suave (uma forma nova que o `FUNCTION` não dá).
- **Portamento manual:** `SLEW` alto + `CRV` linear num `SEQUENCE.pitch`
  — glide entre as notas (ou use o `GLIDE`, que decide *quando*).

## Se você conhece o Eurorack

Faz o papel de Maths (atenuversor / offset / slew / somador) num só
módulo, duplo. Também Serge DUSG e um seguidor de envelope RC. O parente
de distribuição é o `MULT` (#34) — uma fonte, quatro cópias com pesos
próprios.
