# SWITCH — chave sequencial

**Família:** ROUTE · **Módulo 28**
**Essência:** faz a **variação de roteamento** virar parte do fluxo
autônomo — a mesma linha tocada ora por um oscilador, ora por uma corda,
sem repatch. Mux N→1 ou demux 1→N.
**Dossiê técnico:** [`../dossies/28_switch.md`](../dossies/28_switch.md)
· **Fonte:** `src/dsp/Switch.hpp`

---

## A ideia

Patch generativo interessante troca de material: o filtro alimentado ora
pelo ruído, ora pelo acorde; um S&H que amostra de fontes diferentes a
cada compasso. Sem `SWITCH` isso exige repatch manual. O `SWITCH` roteia:
`DEMUX` desligado, é um **mux** (`A`/`B`/`C`/`D` → uma saída); ligado, é
um **demux** (`A` → uma das saídas). O endereço avança num clock, num
reset, ou vem direto de uma CV.

## Por dentro

**Mux e demux, pra quem não conhece os termos:** um **multiplexador**
("mux") é um roteador N→1 — tem várias entradas, mas só deixa
**uma** por vez passar pra uma única saída, decidida por um número de
endereço. Um **demultiplexador** ("demux") é o espelho: uma entrada só,
que é jogada pra **uma** de várias saídas possíveis, também decidida
por um endereço. `DEMUX` desligado é a primeira forma (`A`/`B`/`C`/`D`
→ uma saída); ligado, é a segunda (`A` → uma das quatro saídas).

Um contador de endereço (0..`STEPS`−1) decide qual entrada/saída está
ativa agora. `MODE` controla como esse contador avança: pra frente e
ping-pong seguem uma ordem fixa (a mesma ideia do `MODE` do
`SEQUENCE`, #15); aleatório sorteia a próxima posição sem padrão fixo
(semeado — reprodutível); só-`ADDR` desliga o contador e deixa uma CV
externa decidir a posição diretamente, sem depender de clock nenhum
(se `ADR` estiver conectado, ele **sempre** tem prioridade sobre
qualquer `MODE`).

**Por que `GLIDE` importa mais em áudio do que em CV:** trocar
instantaneamente de fonte, no meio de uma forma de onda de áudio,
produz um **salto abrupto** de valor que o ouvido escuta como um
clique/estalo — porque uma descontinuidade brusca numa onda de áudio
contém energia espalhada por todas as frequências (a mesma lógica do
"quebra abrupta gera aliasing" do `OSC`, #18, só que aqui é
percebido diretamente como estalo, não como afinação errada). `GLIDE`
faz as duas fontes se **sobrepor** brevemente no instante da troca
(um crossfade) em vez de cortar seco, evitando esse salto. Numa CV
lenta, uma troca abrupta raramente é audível como clique (o destino
já está "ouvindo" a CV como um controle, não como som direto) — por
isso `GLIDE`=0 costuma bastar pra CV, mas não pra áudio. O `SLEW` de
1 ms embutido continua ativo sempre, como uma rede de segurança mínima
mesmo com `GLIDE`=0.

## Os jacks, um a um

### Entradas

- **`A`** / **`B`** / **`C`** / **`D`** (áudio) — no modo mux, as 4
  fontes; no modo demux, só `A` é usada (a fonte a distribuir).
  **Plugue aqui:** vozes, processadores, LFOs.
- **`CLK`** (clock) (controle, disparo) — avança o endereço na borda de
  subida. **Plugue aqui:** `CLOCK.euclid`, `SEQUENCE.eos`,
  `LOGIC.flip`.
- **`RST`** (reset) (controle, disparo) — zera o endereço.
- **`ADR`** (addr) (controle) — endereço direto por CV (se conectado,
  manda). **Plugue aqui:** `SEQUENCE` de CV, `DECISION.x`,
  `NOISE.smooth` — a fonte muda por CV, não por passo.

### Saídas

- **`OA`** (out) (áudio) — no mux, a fonte selecionada; no demux, a
  saída 0.
- **`OB`** / **`OC`** / **`OD`** (áudio) — no demux, as outras saídas.
- **`STP`** (step) (controle) — a posição atual como CV. **Plugue em:**
  qualquer `_mod` — pra o resto do patch saber qual fonte está tocando.

## Os controles, um a um

**STEP** (steps, 2–4) — quantas posições o endereço percorre.

**MODE** (0–3) — pra frente / ping-pong / aleatório / só-`ADDR`.

**GLID** (glide, 0–1) — o *crossfade* no ponto de troca. Alto = as
fontes se sobrepõem na transição (bom pra áudio); 0 = corte seco.

**SLEW** (0–1) — suaviza a troca (anti-clique). Sempre um pouco.

**DEMUX** (dir, chave) — desligado: mux N→1; ligado: demux 1→N.

## Como cabear

**A voz alterna entre timbres:**
```
OSC (SAW) → SWITCH (A)
MATTER (OUT) → SWITCH (B)
CLOCK (via LOGIC.div, N=4) → SWITCH (CLK)     STEP = 2
SWITCH (OA) → FILTER (in) → MIXER (ch1)
```

**Distribuir uma voz por 4 destinos (demux):**
```
voz → SWITCH (A)     DEMUX ligado
SWITCH (OA/OB/OC/OD) → 4 processadores diferentes → MIXER
DRIFT (a) → SWITCH (ADR)     (o destino da voz deriva sozinho)
```

## Potencializar

- **Roteamento generativo:** `DECISION.x → ADR`, `MODE` = 3 — a fonte
  (ou o destino) muda a cada decisão.
- **Timbre que "corta":** `GLIDE` = 0, troca no contratempo — a voz
  muda de caráter bruscamente, como um edit.
- **`STP` avisa o patch:** `STP → FILTER.cutoff` — cada fonte toca com
  um corte diferente automaticamente.
- **Ping-pong estéreo:** demux para 2 saídas com pans opostos, `MODE` =
  1 — a voz "quica" no palco.

## Se você conhece o Eurorack

Faz o papel de uma chave sequencial/endereçada (Doepfer A-151/A-152) com
slew na troca (4ms SISM). A base é o multiplexador CD4051 e os modos de
leitura do `SEQUENCE` do Rasgo. Distinto do `MATRIX` (todas as conexões
de uma vez) e do `PLANAR` (morph contínuo, não comutação).
