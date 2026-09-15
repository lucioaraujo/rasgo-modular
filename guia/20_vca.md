# VCA — amplificador controlado por tensão (duplo)

**Família:** TRANSFORM · **Módulo 20**
**Essência:** o verbo "multiplicar". Dois canais de `in × ganho`, com a
CV somando ao knob (o knob fica vivo). O utilitário mais usado de
qualquer rack.
**Dossiê técnico:** [`../dossies/20_vca.md`](../dossies/20_vca.md)
· **Fonte:** `src/dsp/Vca.hpp`

---

## A ideia

Sem `VCA` não dá pra um envelope **externo** controlar o volume de uma
voz que não tem VCA embutido; não dá tremolo (LFO × áudio); não dá AM;
não dá pra escalar uma CV antes de mandar pra um parâmetro; não dá
*ducking*. O `ENVELOPE` resolve "envelope → própria saída", mas
`SEQUENCE`, `DECISION`, `NOISE.smooth`, um `FUNCTION` lento — nenhum tem
VCA.

O `VCA` fica **no meio**: `voz → VCA` com um `ENVELOPE`/`NOISE`/`FUNCTION`
no `CV`. Ou `CV → VCA` como atenuador. Ou duas fontes → `SUM` como
mini-mixer.

## Por dentro

Por canal: `ganho = LVL + CV_AMOUNT · cv` — a CV **soma** ao knob (não
apaga; o knob continua vivo com o cabo plugado, ver `CABEAMENTO.md`
§4). `CV_AMOUNT` é um atenuversor (−1..1: positivo passa a CV normal,
negativo **inverte** o efeito dela — o volume cai quando a CV sobe, em
vez de subir).

**Por que `RESPONSE` faz diferença entre "somar CV" e "volume de
áudio":** multiplicar uma CV por outra CV, pra combinar controles, é
melhor feito de forma **linear** (dobrar o número dobra o resultado, de
forma previsível — bom pra fazer contas com CV). Mas o **ouvido**
percebe volume de forma **exponencial**, não linear (a mesma lógica de
1 V/oitava explicada em `CABEAMENTO.md` §4, só que pra amplitude em vez
de frequência — um decibel é sempre a mesma "distância percebida" de
volume, e decibéis são uma escala exponencial em amplitude linear).
`RESPONSE` em 0 usa a curva linear (ideal pra CV); em 1, usa uma curva
exponencial ("dB-linear" — cada passo igual do knob soa como um passo
igual de volume) — a curva certa pra controlar o volume real de uma
voz.

**Por que "AM/ring" aparece como truque nas receitas:** ligar um áudio
de verdade (não uma CV lenta) na entrada `CV`, com `RESPONSE` linear,
faz o `VCA` calcular literalmente `IN × CV` — a mesma operação de
multiplicar dois sinais descrita no `RingMod` de `RELACAO_DE_CABO.md`
§1.1 (só sem a parcela "seca" misturada por cima) — daí as mesmas
bandas soma/diferença, o mesmo timbre metálico quando os dois sinais
são áudio.

## Os jacks, um a um

### Entradas

- **`IN1`** / **`IN2`** (áudio) — o sinal a amplificar. **Plugue aqui:**
  uma voz, um processador, ou uma CV (o `VCA` também escala controle).
- **`CV1`** / **`CV2`** (controle) — o sinal que **comanda o ganho**.
  **Plugue aqui:** `ENVELOPE.env` (o volume segue a nota — o uso número
  1), um LFO (`FUNCTION` — tremolo), `NOISE.smooth`, `SEQUENCE` (nível
  por passo).

### Saídas (todas áudio)

- **`O1`** / **`O2`** — cada canal amplificado. Vão ao `MIXER`.
- **`SUM`** — os dois canais somados. Use como um mini-mixer de 2
  entradas.

## Os controles, um a um

**LVL1** / **LVL2** (level, 0–1) — o ganho manual de cada canal. A CV
soma aqui.

**CV1** / **CV2** (cv_amount, −1..1) — o atenuversor da entrada CV.
Negativo inverte (o volume *cai* quando a CV sobe).

**RSP1** / **RSP2** (response, 0–1) — 0 = linear (somar CV); 1 =
exponencial (volume de áudio percebido).

**DRIFT** (0–1) — oscilação lenta e correlacionada nos dois ganhos
(±~3%) — "o VCA respira".

## Como cabear

**Envelope de amplitude (a cadeia mínima):**
```
OSC (SAW) → FILTER (IN) → VCA (IN1) → MIXER (ch1)
ENVELOPE (env) → VCA (CV1)
CLOCK (euclid) → ENVELOPE (gate)
```

**Tremolo:** `FUNCTION` (LFO) → `VCA.CV1`, `RSP1` linear, `LVL1` médio.

**Atenuador de CV:** `LFO → VCA.IN1` (como controle), `LVL1` no valor
que você quer da amplitude, `O1` → o destino.

## Potencializar

- **Duplo com um único envelope:** `IN1` e `IN2` de duas vozes, o mesmo
  `ENVELOPE` nos dois `CV`, `CV_AMOUNT` diferentes — as vozes têm
  dinâmicas relacionadas mas não iguais.
- **AM / ring:** áudio em `IN1`, outro áudio em `CV1`, `RSP1` linear,
  `CV_AMOUNT` alto — modulação de amplitude (bandas laterais).
- **Ducking:** o kick num `ENVELOPE`, o `env` invertido (`CV_AMOUNT`
  negativo) no `VCA` do pad — o pad abaixa na batida.
- **`SUM` como submix:** duas vozes → `IN1`/`IN2`, `SUM` → um canal do
  `MIXER` (libera canais do mixer principal).

## Se você conhece o Eurorack

Faz o papel de um VCA lin/exp (Doepfer A-131/A-132), com atenuversor de
CV (Maths). A diferença: a CV **soma ao knob por porta** (não substitui)
— o knob fica vivo. Para bancos maiores há o `VCA4` (#59), 4 canais +
mixer.
