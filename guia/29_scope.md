# SCOPE — osciloscópio que devolve como CV

**Família:** OUT · **Módulo 29**
**Essência:** ver, medir e — o ponto — **realimentar**. As medições
(altura, brilho, nível, ataque) saem por jacks, como CV. O instrumento
que escuta a si mesmo.
**Dossiê técnico:** [`../dossies/29_scope.md`](../dossies/29_scope.md)
· **Fonte:** `src/dsp/Scope.hpp`

---

## A ideia

Num scope de hardware a tela é um beco sem saída. Aqui o que o módulo
mede sai por um jack: o brilho médio da mistura abre e fecha um filtro;
a altura da voz principal afina um drone; o nível dispara um envelope de
*sidechain*; o `ONSET` sincroniza um envelope ao ataque do áudio. É uma
forma de composição — o sistema fechando o próprio laço.

## Por dentro

`IN` é copiado limpo em `THRU` (o que o Display do painel desenha). Um
comparador com histerese (`REJECT` — a mesma zona-morta explicada no
`QUANTIZER`, #12, aqui evitando que ruído perto do nível `TRIGGER`
dispare o `TRIG` várias vezes seguidas por engano) contra `TRIGGER`,
na borda `EDGE`, produz o pulso `TRIG`.

**`BRIGHT`, como medir "brilho" sem calcular um espectro completo:**
diferenciar um sinal (medir a **variação** entre uma amostra e a
anterior — a mesma operação do "Por dentro" do `Difference` de cabo,
`RELACAO_DE_CABO.md` §1.3) reforça as frequências **altas** muito mais
que as baixas — quanto mais rápida a oscilação, maior a diferença
amostra a amostra. Comparando a energia do sinal **diferenciado** com a
energia do sinal **original**, dá pra estimar, de forma barata (sem
nenhuma análise de frequência completa tipo FFT), se a energia do som
está concentrada mais nos agudos ou nos graves — é essa proporção que
vira a saída `BRIGHT` (o "centroide espectral", ou seja, o "centro de
massa" das frequências presentes).

**`PITCH`, como achar a altura por autocorrelação (o método YIN):** em
vez de procurar "qual é a frequência mais forte" (o que pode enganar —
um harmônico às vezes é mais forte que a própria fundamental),
autocorrelação compara o sinal com **cópias dele mesmo, deslocadas no
tempo**, procurando o deslocamento em que elas mais **se parecem** — e
esse deslocamento **é** o período de repetição do som, o que dá a
altura diretamente, sem depender de qual harmônico está mais alto. É
mais robusto contra timbres com harmônicos fortes disputando com a
fundamental do que simplesmente procurar o pico de energia.

**`ONSET`, como detectar um ataque:** dois seguidores de envelope
correm em paralelo sobre o mesmo `IN` — um **rápido** (reage quase
instantaneamente a qualquer subida de energia) e um **lento** (reage
com atraso, seguindo a tendência geral). Num trecho estável, os dois
ficam parecidos. No instante exato de um ataque/transiente, o rápido
**dispara na frente** do lento, e a diferença entre os dois cresce de
repente — é esse pico de diferença, comparado contra `SENS`, que gera
o pulso `ONS`. `SENS` alto aceita diferenças pequenas (dispara em
qualquer subida); baixo exige uma diferença grande (só ataques fortes
o suficiente contam).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a analisar/exibir. **Plugue aqui:** a soma
  do `MIXER`, uma voz, uma linha de bateria.
- **`EXT`** (controle) — fonte externa de disparo. Sem cabo, `TRIG` usa
  o próprio `IN`.

### Saídas

- **`THRU`** (áudio) — `IN` copiado sem alteração. **Plugue em:** um
  canal do `MIXER` — o `SCOPE` fica no caminho e não muda o som.
- **`TRIG`** (controle, gate) — pulso quando `IN` cruza `TRIGGER` na
  direção de `EDGE`. **Plugue em:** `FUNCTION.sync` (um LFO trava na
  fase da voz), `ENVELOPE.gate`.
- **`LVL`** (level) (controle) — o pico do sinal como CV. **Plugue em:**
  `VCA.cv` invertido (ducking), `SPACE.mix_mod`.
- **`BRT`** (bright) (controle) — o centroide espectral (grave↔agudo).
  **Plugue em:** `FILTER.FC` (o filtro segue o brilho da entrada),
  `HALL.damp`.
- **`PIT`** (pitch) (controle, 1 V/oct) — a altura detectada. **Plugue
  em:** `OSC.1V/O` — um segundo oscilador afina pela altura da entrada.
- **`ONS`** (onset) (controle, gate) — um pulso (~2 ms) a cada
  ataque/transiente. **Plugue em:** `ENVELOPE.gate`, `LPG.strike`,
  `SAMPLER.trig` — o patch reage aos ataques do áudio.

## Os controles, um a um

**TRIG** (trigger, −1..1) — o nível do comparador de disparo.

**EDGE** (0/1) — dispara na borda de descida (1) ou subida (0).

**REJ** (reject, 0–1) — a histerese ao redor do nível — evita disparo
múltiplo por ruído.

**RESP** (response, 0–1) — a velocidade dos seguidores de `LVL`/`BRT`/
`PIT`. Ataque sempre mais rápido que o release.

**HOLD** (0–1) — congela `LVL`/`BRT`/`PIT` no valor atual.

**SENS** (sens, 0–1) — a sensibilidade do detector de `ONSET`. Alto =
dispara em qualquer subida; baixo = só ataques fortes.

## Como cabear

**O instrumento se ouvindo:**
```
MIXER (L+R) → SCOPE (IN)
SCOPE (THRU) → MASTER (IN)          (o SCOPE não muda o som)
SCOPE (BRT) → FILTER (FC)           (o filtro abre quando a mistura fica brilhante)
SCOPE (LVL) → VCA (cv) invertido de um pad     (ducking)
```

**Sincronizar ao áudio de fora:**
```
SIGNAL-IN (L) → SCOPE (IN)
SCOPE (ONSET) → ENVELOPE (gate)     (dispara no compasso do áudio de entrada)
SCOPE (PIT) → OSC (1V/O)            (o Rasgo afina pelo que entra)
```

## Potencializar

- **Auto-sidechain:** `SCOPE.onset` de uma linha de bumbo → um
  `ENVELOPE` invertido no `VCA` do pad — o pad "bombeia" com o bumbo.
- **LFO travado na voz:** `SCOPE.trig` no `FUNCTION.sync` — a modulação
  fica em fase com a fundamental da voz.
- **Filtro que segue o timbre:** `BRT → FILTER.FC` com um `CONTROL`
  invertido — o filtro *fecha* quando o som fica agudo (compensação).
- **Melodia que ecoa a melodia:** `PIT → QUANTIZER → OSC` — um segundo
  oscilador toca "de ouvido" a linha principal.

## Se você conhece o Eurorack

Faz o papel de um scope de rack (Mordax DATA, ALM MUM M8) **mais** um
extrator de pitch/envelope/onset (Doepfer A-119, um envelope follower).
A base: trigger de nível/borda/histerese; centroide espectral por
Parseval; **YIN** (de Cheveigné & Kawahara, 2002); detector de
transiente por dois seguidores. O parente é o `BOXCAR` (#51) — mede pra
reconstruir; o `SCOPE` mede pra escalar.
