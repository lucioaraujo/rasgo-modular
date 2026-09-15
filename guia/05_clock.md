# CLOCK — relógio euclidiano

**Família:** TIME · **Módulo 5**
**Essência:** o ritmo como um **campo** de onde padrões emergem — o
euclidiano (distribuir k pulsos em n passos) gera quase toda célula
rítmica do mundo a partir de dois números.
**Dossiê técnico:** [`../dossies/05_clock.md`](../dossies/05_clock.md)
· **Fonte:** `src/dsp/EuclidClock.hpp`

---

## A ideia

Numa lógica generativa o ritmo não devia ser um *piano-roll*. O
euclidiano — distribuir `FILL` pulsos em `LENGTH` passos o mais
uniformemente possível — gera quase todas as células rítmicas
tradicionais a partir de dois números. O AND/OR de dois divisores de
acento dá polirritmia sem programar nada. E o `DRIFT` faz o andamento
respirar. É a fonte de tempo do patch.

## Por dentro

**O que é um ritmo "euclidiano", em uma frase:** distribuir `FILL`
disparos em `LENGTH` passos **o mais uniformemente possível** — em vez
de agrupá-los de qualquer jeito, o algoritmo (Bjorklund/Toussaint,
público) espalha os disparos de modo que os espaços entre eles sejam o
mais parecidos possível uns com os outros. O fato notável (e o porquê
disso vale a pena): quase toda célula rítmica tradicional do mundo —
o *tresillo* cubano (3 em 8), o *son clave* (5 em 16), ritmos de vários
continentes — **é** uma distribuição euclidiana de algum `FILL`/`LENGTH`.
Não é coincidência: espalhar o mais uniforme possível é, em muitos
casos, exatamente o que soa "certo" ao ouvido humano — daí dois números
gerarem tanta variedade reconhecível.

Um relógio interno (`BPM × MULT`) produz o `CLK` — o pulso de passo
estável sobre o qual a grade euclidiana é calculada, virando o `EUC`.

**`ROTATE`, o que muda de fato:** o mesmo padrão euclidiano (a mesma
`FILL`/`LENGTH`, então a mesma **densidade** de disparos) pode começar
em qualquer um dos `LENGTH` passos — girar o padrão é só decidir
**qual** passo é o "passo 0". A forma do padrão (as distâncias entre os
disparos) não muda, só onde ele começa a ser lido — a mesma ideia de
"inversão" de um acorde (`CHORD`, #26), aplicada ao tempo em vez de à
altura.

**O acento (`ACC`), como funciona de fato:** a cada passo, o módulo
checa se o número do passo é **múltiplo** de `ACC-A` (a cada 4 passos,
por exemplo) e/ou de `ACC-B`. A chave `AND` decide se precisa ser
múltiplo dos **dois ao mesmo tempo** (um acento raro, só onde os dois
"encaixam") ou de **qualquer um deles** (mais frequente). Usar dois
divisores diferentes que não são múltiplos um do outro (3 e 4, por
exemplo) produz **polirritmia** de graça: o acento cai num ciclo que só
se repete idêntico depois de `A×B` passos, então o padrão de acentos
parece "flutuar" sobre o pulso principal em vez de se alinhar
regularmente com ele.

`FEEL` muda **em quantas partes** cada tempo se subdivide antes de
distribuir os passos: reto divide em potências de 2 (a subdivisão
"quadrada" padrão); tercina divide cada tempo em 3 partes iguais (a
sensação rítmica de "swing"/valsa em vez de reta); quintina, septina
etc. seguem o mesmo princípio com outros números. "Glitch" abandona uma
subdivisão fixa e **sorteia** um fator de tempo diferente a cada passo
— o groove deixa de ter uma grade fixa por trás.

## Os jacks, um a um

### Entradas

- **`EXT`** (ext_clock) (controle, disparo) — clock externo. Conectado,
  o `CLOCK` segue as bordas dele em vez do `BPM` interno. **Plugue
  aqui:** outro `CLOCK`, um `LOGIC`, um pulso de fora — pra escravizar
  este relógio.
- **`RST`** (reset) (controle, disparo) — zera a posição do passo
  euclidiano. **Plugue aqui:** um `CLOCK` de compasso, o `EOS` de um
  `SEQUENCE`, a tecla de um transporte.
- **`BPM`** (bpm_mod) (controle) — CV que soma ao `BPM`. **Plugue
  aqui:** um `FUNCTION` bem lento (accelerando/ritardando), `DRIFT`.

### Saídas (todas gate)

- **`CLK`** — pulso de passo estável em `bpm·mult`. **Plugue em:** o
  `clock` de um `SEQUENCE`/`TURING`/`LOGIC`, `SH.trig`.
- **`EUC`** — o gate euclidiano (`FILL` de `LENGTH`). **Plugue em:**
  `ENVELOPE.gate`, `MATTER.HIT`, `DRUM.GATE`, `DECISION.trig` — é a
  saída rítmica principal.
- **`ACC`** — o acento (AND/OR de `ACC-A` e `ACC-B`). **Plugue em:**
  `DRUM.ACC`, `VCA.cv`, qualquer `_mod` — dá peso a certos passos.

## Os controles, um a um

**BPM** (20–300) — o andamento.

**MULT** (0,25–8×) — quantos passos de `CLK` cabem numa batida.

**LEN** (length, 1–32) — o nº de passos da grade euclidiana.

**FILL** (0–32) — quantos dos `LEN` passos disparam em `EUC`. Baixo (3 de
16) = esparso; perto de `LEN` = quase contínuo.

**ROT** (rotate, 0–31) — gira o padrão euclidiano (mesma densidade, fase
diferente).

**SWING** (0–1) — atrasa os passos ímpares dentro da própria fatia de
tempo.

**DRIFT** (0–1) — passeio lento no andamento efetivo (±~12%). 0 =
determinístico.

**GATE** (gate_len, 0,05–0,95) — a fração de cada passo em que o pulso
fica alto.

**ACC-A** / **ACC-B** (1–16) — os dois divisores do acento.

**AND** (accent_mode, chave) — ligado: o acento exige múltiplo de A **e**
de B; desligado: A **ou** B (polirritmia de graça).

**FEEL** (0–6) — o agrupamento: reto / tercina / quintina / septina /
nonina / undecina / glitch.

## Como cabear

**O motor rítmico do patch:**
```
CLOCK (EUC) → ENVELOPE (gate)      LEN 16, FILL 5
CLOCK (CLK) → SEQUENCE (clock)     (a melodia anda no passo)
CLOCK (ACC) → DRUM (ACC)           ACC-A 4
```

## Potencializar

- **Polirritmia:** `ACC-A` = 3, `ACC-B` = 4, `AND` desligado — o acento
  cai num padrão de 12 contra a grade principal.
- **Ritmo que respira:** `DRIFT` ~0,3 + `SWING` ~0,15 — o groove nunca
  fica metronômico.
- **Tercina virando reto:** automatize o `FEEL` com um `SEQUENCE` de CV
  — seções que mudam de subdivisão.
- **Reset generativo:** `DECISION.gate → RST` com baixa probabilidade —
  de vez em quando o padrão "reancora" e o groove reinicia.

## Se você conhece o Eurorack

Faz o papel de um clock euclidiano (Pamela's New Workout, vpme Euclidean
Circles) + um gerador de acento por divisão. A base é o algoritmo de
Toussaint/Bjorklund (público). Para recombinar clocks (÷/×, lógica),
veja o `LOGIC` (#22).
