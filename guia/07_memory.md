# MEMORY — granular / freeze

**Família:** SPACE · **Módulo 7**
**Essência:** o patch grava a si mesmo e pode voltar a tocar o que
gravou — transposto, espalhado, congelado. A diferença entre um
instrumento que só avança e um que se lembra.
**Dossiê técnico:** [`../dossies/07_memory.md`](../dossies/07_memory.md)
· **Fonte:** `src/dsp/Memory.hpp`

---

## A ideia

A cicatriz do cabo já dizia: **romper não é apagar, é segurar o último
bloco** (ver [`COMO_PENSAR.md`](COMO_PENSAR.md)). O `MEMORY` é essa ideia
como módulo pleno — um *buffer* de ~3 s gravado continuamente, e uma
nuvem de grãos que relê esse buffer numa posição, num tamanho, numa
densidade e numa afinação que você controla. Com `FREEZE`, o buffer vira
uma textura fixa e só os grãos continuam. O que o sistema produziu vira
material do que ele vai produzir.

## Por dentro

**O que é um "grão", e por que uma nuvem deles não soa como cliques:**
um grão é um **pedacinho bem curto** de áudio (`GRAIN`, de 5 ms a
500 ms) lido de dentro do buffer, com um pequeno envelope próprio que
sobe e desce suavemente nas bordas (sem isso, cada grão teria uma
quebra abrupta no início/fim — a mesma questão de "descontinuidade
gera estalo" vista em vários outros módulos). Um grão sozinho, isolado,
soaria como um "tique" curto. A **nuvem** nasce de tocar **muitos**
grãos, sobrepostos, o tempo todo (`DENSITY` grãos por segundo) — em
densidade alta, os grãos se sobrepõem tanto que suas bordas suaves se
fundem numa textura contínua; em densidade baixa, eles ficam
espaçados o bastante pra soar como pulsos rítmicos separados.

**Por que `POSITION` funciona como "apontar pro passado":** o buffer
não é uma gravação de um trecho fixo — é uma janela **deslizante** dos
últimos ~3 segundos, sempre se atualizando (a cada instante, o que
entrou há 3 s atrás sai do buffer e o que acabou de chegar entra).
`POSITION`=0 pega grãos do que está entrando **agora mesmo**;
`POSITION`=1 pega grãos de **3 segundos atrás**. Variar `POSITION`
lentamente faz a nuvem "olhar" pra diferentes profundidades do passado
recente, revisitando o que já foi tocado. `SPRAY` acrescenta uma
dispersão aleatória em volta desse ponto (em vez de todos os grãos
começarem exatamente no mesmo instante, cada um começa um pouco antes
ou depois) — de leitura cirúrgica a nuvem borrada no tempo.

**Por que `PITCH` transpõe sem afetar o conteúdo gravado:** cada grão é
lido do buffer numa **velocidade** diferente da que foi gravado
(transposição, a mesma relação velocidade↔altura de um toca-discos) —
mas isso acontece só **na leitura de cada grão**, individualmente; o
que está guardado no buffer continua intacto, na afinação original. É
por isso que uma melodia no `PITCH` (via `QUANTIZER.pitch`) faz a
**nuvem inteira** tocar essa melodia, mesmo que o material-fonte no
buffer não tenha nenhuma nota definida.

`FREEZE` simplesmente **para de atualizar** a janela deslizante — o
buffer vira uma "fotografia" fixa dos últimos 3 s, e os grãos continuam
sendo lidos dessa fotografia parada em vez de um fluxo que muda. `FBK`
realimenta a própria **saída** (a nuvem já processada) de volta pro
buffer antes da próxima gravação — cada volta soma mais uma camada da
nuvem em cima de si mesma, degenerando/acumulando (cópia de cópia).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — gravado continuamente no buffer (exceto com
  `FREEZE`). **Plugue aqui:** `ENVELOPE.out` (a voz articulada),
  `FILTER.all`, qualquer voz.
- **`POS`** (position_mod) (controle) — CV que soma à `POSITION`.
  **Plugue aqui:** um LFO lento (os grãos "varrem" o passado),
  `DRIFT.a`, `SEQUENCE` de CV.
- **`PTCH`** (pitch_mod) (controle, 1 V/oct) — CV que soma ao `PITCH`
  (em oitavas). **Plugue aqui:** `QUANTIZER.pitch` (a nuvem toca uma
  melodia), um LFO (a nuvem cintila).
- **`FRZ`** (freeze_gate) (controle, gate) — congela o buffer enquanto
  alto (igual à chave `HOLD`). **Plugue aqui:** `CLOCK.euclid`,
  `DECISION.gate`, um pedal.

### Saída

- **`OUT`** (áudio) — a mistura entre o `IN` cru e a nuvem de grãos
  (`BLEND`). Vai ao `MIXER`.

## Os controles, um a um

**GRAIN** (0,005–0,5 s) — a duração de cada grão. Curto = textura
granular áspera; longo = quase o material contínuo.

**DENS** (density, 0,1–120 Hz) — quantos grãos novos por segundo. Baixo =
grãos esparsos (ritmo); alto = uma nuvem lisa.

**POS** (position, 0–1) — onde no buffer os grãos começam. A CV `POS`
soma aqui.

**SPRAY** (0–1) — espalha a posição de início de cada grão — de leitura
exata a nuvem borrada no tempo.

**PITCH** (−24..24 st) — transposição da leitura dos grãos.

**FBK** (feedback, 0–0,95) — realimenta a saída no buffer — o material se
acumula/degenera.

**BLEND** (0–1) — seco (entrada crua) ↔ molhado (nuvem).

**HOLD** (freeze, chave) — para a gravação; o buffer vira textura fixa.

## Como cabear

**Textura granular de uma voz:**
```
ENVELOPE (out) → MEMORY (IN)
MEMORY (OUT) → MIXER (ch1)     GRAIN ~0,1 s, DENS ~30 Hz, BLEND ~0,7
```

**Pad congelado:**
```
CHORD (OUT) → MEMORY (IN)
CLOCK (euclid) → MEMORY (FRZ)     (o acorde congela e descongela)
```

## Potencializar

- **A nuvem toca melodia:** `QUANTIZER.pitch → PTCH` — os grãos seguem a
  altura da parte melódica, uma oitava acima.
- **Varredura do passado:** um `FUNCTION` bem lento no `POS` — a nuvem
  "revisita" os últimos 3 segundos ciclicamente.
- **Time-stretch:** `FREEZE` + `POS` num LFO lento + `GRAIN` longo — o
  material se estende sem mudar de afinação.
- **Degradação:** `FBK` ~0,7 sem `FREEZE` — o material se sobrepõe a si
  mesmo e vira uma pasta (cópia de cópia).

## Se você conhece o Eurorack

Faz o papel de Mutable Clouds / Beads, do arbhar, do granular de Curtis
Roads. Distinto do `PULSAR` (#56 — sintetiza o grão, não o extrai de um
buffer), do `LOOPER` (#41 — delay de linha) e do `SAMPLER` (#48 —
toca-fatias).
