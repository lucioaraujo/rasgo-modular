# DYNAMICS — compressor e gate

**Família:** TRANSFORM · **Módulo 63**
**Essência:** abaixa o que passa do limiar (ou fecha o que fica abaixo) e
devolve, como sinal, o que ouviu e quanto abaixou.
**Dossiê técnico:** [`../dossies/63_dynamics.md`](../dossies/63_dynamics.md)
· **Fonte:** `src/dsp/Dynamics.hpp`

---

## A ideia

Um compressor faz duas coisas que mudam a música: segura os picos (o som
fica mais junto, mais "colado") e deixa um som abaixar outro — o pad que
respira quando o bumbo bate. No Modular ele faz uma terceira: o que ele
ouve e quanto ele abaixa saem como sinais (ENV e GR), e viram modulação
para qualquer outro módulo.

## Por dentro

Um **detector** mede o nível (o pico de cada amostra, ou a energia média
de ~10 ms). Uma **curva** em dB decide quanto abaixar: com RATIO 4, cada
4 dB acima do limiar viram 1; com o joelho (KNEE), a passagem é gradual.
O ganho não muda de uma vez: sobe em ATK e volta em REL. No modo GATE a
curva se inverte — o que fica abaixo do limiar é fechado.

Dois desvios Rasgo mexem no tempo: **BRTH** faz o release seguir o
intervalo entre os golpes que ele ouve (a compressão respira no andamento),
e **DRIFT** varia ataque e release a cada golpe, por um acaso semeado.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o som a comprimir, mono ou estéreo. Sem cabo, o
  módulo fica calado.
- **`SIDE`** (qualquer sinal) — o que o detector deve ouvir. **Plugue
  aqui:** um `DRUM`, o bumbo de um `TRIGSEQ` + `DRUM`, uma voz.
- **`THR`** (controle) — CV no limiar (±24 dB). **Plugue aqui:** um LFO,
  um `ENVELOPE`.

### Saídas

- **`OUT`** (áudio) — a entrada comprimida.
- **`ENV`** (controle, 0–1) — o nível que o detector ouve, liso: um
  seguidor de envelope. **Plugue em:** `CUT` de um `FILTER`, um `VCA`.
- **`GR`** (controle, 0–1) — quanto ele abaixa agora. **Plugue em:** um
  `VCA` invertido noutro som, o `DRIFT` de qualquer módulo.

## Os controles, um a um

**THRES** (−48..0 dB) — o limiar. **RATIO** (1–20; 20 = limitador) —
quanto abaixa acima dele. **KNEE** (0–12 dB) — de uma vez ou aos poucos.
**ATK** (0,1–200 ms) e **REL** (5 ms–2 s) — o tempo de descer e de voltar.
**GAIN** (0–24 dB) — devolve o nível. **SIDE** (0–1) — o detector ouve
IN (0) ou SIDE (1). **RMS** — energia média em vez de pico. **GATE** —
vira noise gate. **BRTH** (0–1) e **DRIFT** (0–1) — os desvios Rasgo.

## Receitas

1. **O pad que respira.** `TRIGSEQ` → `DRUM` → `DYNAMICS.SIDE`;
   `CHORD` → `DYNAMICS.IN`; SIDE = 1, RATIO 6, ATK 5 ms, REL 200 ms. O
   acorde abaixa a cada bumbo. Com BRTH = 1, o REL acompanha o ritmo.
2. **Segurar a mistura.** `MIXER` → `DYNAMICS.IN` → `MASTER`; RMS ligado,
   RATIO 2–3, KNEE 8, GAIN até o nível voltar.
3. **Um ouvido que move o patch.** Só o `SIDE` ligado (uma voz, um
   `LOOPER`); `ENV` → `CUT` de um `FILTER` noutro som: o filtro abre com a
   força do que o detector ouve.
4. **Gate rítmico.** GATE ligado, um `NOISE` → IN, um LFO lento → THR: o
   ruído abre e fecha conforme o limiar passa por ele.
