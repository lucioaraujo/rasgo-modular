# OPERATOR — voz FM de 4 operadores

**Família:** SOURCE · **Módulo 44**
**Essência:** o motor DX — 4 operadores senoidais, 8 algoritmos, feedback.
Baixo elétrico, e-piano, sino, metal, o "pluck" digital dos anos 80.
**Dossiê técnico:** [`../dossies/44_operator.md`](../dossies/44_operator.md)
· **Fonte:** `src/dsp/Operator.hpp`

---

## A ideia

O `OSC` faz FM linear de **um par** — o metálico simples. Toda a paleta
FM de verdade — o e-piano, o sino, o baixo *slap*, o metal — precisa
empilhar operadores e escolher **como** eles se modulam. O `OPERATOR`
põe o motor DX no patch: entra 1 V/oct, sai áudio, e `ALGO`/`INDEX`/as
razões moldam o timbre. Todos aceitam CV — `ENVELOPE → INDEX` é o ataque
FM clássico.

## Por dentro

**O que "um operador modula outro" quer dizer, de fato:** cada
operador é uma senoide (a mesma engrenagem de sempre, o acumulador de
fase). "A modula B" significa que a **saída de A**, a cada instante, é
somada à **fase** de B antes de B ler sua própria tabela de seno — ou
seja, A empurra "onde no ciclo" B está lendo, instante a instante. Isso
é literalmente a mesma modulação de frequência do `OSC` (#18) ou do
`PLL` (#37), só que aqui o modulador **também** é um oscilador
completo, não uma entrada externa — e você pode empilhar essa
modulação em cadeia (A modula B, que modula C, que modula D).

**Por que isso cria harmônicos novos, e por que `INDEX` controla
"quantos":** modular a fase de uma senoide com outra senoide não produz
só a frequência original — produz uma **família inteira** de novas
frequências, espaçadas pela frequência do modulador pra cima e pra
baixo da portadora (as "bandas laterais" de Chowning, descritas por
funções de Bessel — o detalhe matemático não importa pra tocar, o que
importa é o efeito: **quanto mais forte a modulação, mais pares de
bandas laterais aparecem, e mais elas carregam energia**). `INDEX` é
exatamente essa força — em 0, cada operador é uma senoide pura (sem
bandas laterais, nenhuma modulação); subindo, cada vez mais bandas
aparecem, o timbre fica mais brilhante e mais denso.

`ALGO` (0–7) escolhe a **topologia** dessa cadeia: em 0, é uma cadeia
longa A→B→C→D (cada operador modula o próximo — o espectro fica mais
rico e mais imprevisível, porque cada modulação já modulada empilha
mais bandas laterais em cima das anteriores); em 7, os quatro tocam
**em paralelo**, sem se modular — cada um é uma senoide simples, e a
soma delas é síntese aditiva pura (som de órgão). A ordem de cálculo é
sempre A→B→C→D e só A tem `FB` (feedback), então **nunca existe um
laço** entre operadores diferentes — cada topologia se resolve numa
passada só, sem precisar de nenhum truque de bloco anterior (diferente
do feedback de cabo, ver `CABEAMENTO.md` §1).

**Por que as razões são quantizadas a uma tabela, e não livres:** a
razão entre a frequência de um operador modulador e a do portador
decide **onde** as bandas laterais caem. Se a razão for um número
**inteiro** (1, 2, 3…), as bandas caem exatamente em cima de
harmônicos da fundamental — o resultado soa **afinado**, como um
instrumento comum. Se a razão for **fracionária** (2,5, 7, 9…), as
bandas caem **fora** da série harmônica — inarmônico, metálico, de
sino. A tabela do `OPERATOR` restringe as razões a um conjunto
escolhido de valores "sempre musicalmente úteis" em vez de deixar
qualquer número (que produziria muito mais resultados feios do que
bons) — de propósito.

**`FB` (feedback do operador A) sozinho:** um operador modulando **a
própria fase** com sua própria saída é uma automodulação — sem
depender de nenhum outro operador, isso já basta pra transformar uma
senoide pura, progressivamente, num dente-de-serra rico em harmônicos
(quanto mais `FB`, mais a forma se afasta do seno puro) — uma fonte de
harmônicos "de graça" que não precisa ocupar um segundo operador.

## Os jacks, um a um

### Entradas

- **`1V/O`** (controle, altura) — a nota do operador A (os outros
  seguem pelas razões). **Plugue aqui:** `QUANTIZER.pitch`,
  `SEQUENCE.pitch`, `TURING.cv`.
- **`IDX`** (controle) — soma ao knob `INDEX` (a profundidade de
  modulação). **É a entrada principal.** **Plugue aqui:** `ENVELOPE.env`
  — o ataque brilhante que escurece na cauda, o "som DX". Ou um LFO — o
  timbre pulsa.

### Saída

- **`OUT`** (áudio) — a soma das portadoras do algoritmo (÷ nº de
  portadoras + *softclip*). Vai ao `MIXER`, geralmente direto (o timbre
  já é o timbre; um filtro é opcional).

## Os controles, um a um

**FREQ** (8–8000 Hz) — a frequência do operador A / ponto de partida.

**FINE** (±100 cents) — afinação fina.

**ALGO** (0–7) — qual operador modula qual. 0 = série (mais "FM"); 7 =
paralelo (aditivo/órgão). Gire devagar com `INDEX` médio e o espectro se
reorganiza a cada passo.

**RB / RC / RD** (ratio_b/c/d, 0–9) — a razão de frequência de B/C/D
vs A, quantizada à tabela. Inteira = harmônico; 2,5 = sino; 7 ou 9 =
metal áspero.

**INDEX** (0–1) — profundidade global de modulação. 0 = os 4 operadores
são senoides puras; 1 ≈ 6 ciclos de desvio de fase (bem brilhante).

**FB** (feedback, 0–1) — o operador A modula a própria fase. Sozinho já
leva a senoide de A a um dente-de-serra — uma fonte de harmônicos sem
precisar de outro operador.

**DRIFT** (0–1) — micro-desafino lento e independente por operador.
Determinístico. 0 = sem desafino.

## Como cabear

**E-piano / pluck:**
```
SEQUENCE → QUANTIZER → OPERATOR (1V/O)
ENVELOPE (env) → OPERATOR (IDX)       (brilhante no ataque, escuro na cauda)
OPERATOR (OUT) → MIXER (ch1)
CLOCK (euclid) → ENVELOPE (gate)
```

**Baixo:** `ALGO` baixo, `RB` = 1, `INDEX` moderado, `DECAY` médio no
envelope de `IDX`. `FB` pequeno pra encorpar.

## Potencializar

- **O algoritmo como gesto:** um `FUNCTION` bem lento ou um `SEQUENCE`
  de CV no `ALGO` (via `CONTROL`) — o timbre muda de "personalidade" ao
  longo da peça.
- **Razão viva:** um `SH` no `RC` — a cada nota, uma cor inarmônica
  diferente (mas sempre da tabela, então sempre "musical").
- **Sino com esticamento:** `RB` = 2, `RC` = 5, `RD` = 7, `INDEX` médio,
  `DECAY` longo — badalar metálico.
- **Cruze com o `FILTER`:** um `OPERATOR` já rico + `FILTER` com
  `resonance` — o filtro esculpe as bandas laterais.

## Se você conhece o Eurorack

Faz o papel de Akemie's Castle (YM2151/OPM) ou Bastl Pizza — FM de
4 operadores com algoritmos. A base é Chowning (1973) e o DX7/DX21/TX81Z
(conceito, patente expirada). Sem EGs por operador nesta versão
(`ENVELOPE → INDEX` cobre o ataque FM); o aliasing de banda lateral é
aceito, como no DX. Distinto do `ADDITIVE` (parcial a parcial), do `OSC`
(TZFM de 1 par) e do `WAVETABLE` (forma varrida).
