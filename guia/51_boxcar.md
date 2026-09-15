# BOXCAR — integrador de janela

**Família:** DECISION · **Módulo 51**
**Essência:** um S&H que mede uma **fatia**, não um instante — e que,
empilhando capturas, revela um sinal enterrado no ruído ou reconstrói a
onda toda. Do equipamento de teste nuclear.
**Dossiê técnico:** [`../dossies/51_boxcar.md`](../dossies/51_boxcar.md)
· **Fonte:** `src/dsp/Boxcar.hpp`

---

## A ideia

O *boxcar averager* (integrador de porta) trava numa referência
repetitiva, abre uma janela num ponto do período, integra o conteúdo e
**empilha** com as capturas anteriores. O ruído descorrelacionado some
(~1/√N); o sinal coerente emerge. Três gestos que o Rasgo não tinha:

- **revelar um sinal no ruído** — ao longo de segundos, um tom fraco
  sincronizado ao `TRIG` "se desenha";
- **um S&H de fatia** — a média de uma janela é robusta a transientes
  (um seguidor de envelope que não treme);
- **varrer uma cabeça de leitura** por uma forma de onda capturada.

## Por dentro

**Por que empilhar capturas revela um sinal escondido no ruído:** se um
evento se **repete** de forma confiável (travado num `TRIG`), o
conteúdo real dele é **igual** (ou muito parecido) a cada repetição —
mas o ruído junto é **diferente** a cada vez, sem relação de uma
repetição pra outra. Somar (ou tirar a média de) N capturas faz o
conteúdo real **se reforçar** a cada soma (ele é sempre o mesmo,
então N cópias somadas continuam com a mesma forma, só N vezes mais
"alto"), enquanto o ruído, por ser descorrelacionado, tende a se
**cancelar parcialmente** — cresce muito mais devagar que o sinal
(proporcional à raiz quadrada de N, não a N). O resultado prático: cada
vez que você dobra `AVERAGE`, a relação sinal/ruído melhora — um tom
fraco demais pra ouvir dentro do ruído, ao longo de segundos de médias
acumuladas, literalmente **emerge** e "se desenha". É a mesma técnica
usada em instrumentação científica (EEG, osciloscópios de bancada) pra
extrair sinais fracos e repetitivos de dentro de ruído.

**O que é a "janela" (`APERTURE`) e por que ela não é um instante só:**
um sample-and-hold comum captura **um ponto** no tempo. O `BOXCAR`
captura uma **fatia** — todo o conteúdo dentro de um intervalo de
tempo, dentro de cada período — e tira a **média** desse intervalo
inteiro, não só de um ponto. `APERTURE` estreita ou alarga essa fatia:
bem estreita, ele se comporta quase como um S&H comum, pontual;
larga, ele "borra" um arco inteiro da onda numa média só — o que faz
dele um seguidor de envelope robusto a transientes (um pico rápido
isolado não desloca a média de uma janela larga tanto quanto
deslocaria uma leitura pontual).

**Como `DELAY` e `SCAN` reconstroem a onda inteira:** `DELAY` decide
**em que ponto**, dentro de cada período medido, a janela abre — uma
"cabeça de leitura" que você pode posicionar em qualquer fase do
ciclo. `SCAN` faz essa posição **variar sozinha**, varrendo o período
inteiro ao longo do tempo, capturando e mediando um pedacinho de cada
vez — ponto por ponto, período após período, até o formato **inteiro**
da onda ter sido visitado e reconstruído (a mesma lógica de um
osciloscópio antigo de tubo, desenhando a imagem linha por linha).
`MODE` decide o que fazer com esse resultado: `follower` só entrega a
média da última janela capturada (um número que segue o sinal
devagar); `reconstruct` toca de volta a onda inteira já reconstruída,
sincronizada ao `TRIG`; `oscillator` relê esse buffer reconstruído
livremente, na sua própria `RATE`, virando uma wavetable feita do que
o módulo mediu (a mesma ideia da captura ao vivo do `WAVETABLE`, #40,
só que aqui o "ciclo gravado" já vem filtrado pela média, não é uma
captura bruta de um instante só).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio, ou CV) — o sinal a analisar. Desconectado, um piso de
  ruído interno de −34 dB dá o que reconstruir (soa ao carregar).
  **Plugue aqui:** ruído + um tom fraco (pra revelar), uma voz, um LFO.
- **`TRIG`** (controle, disparo) — a **referência de repetição**: o
  evento a que as capturas se travam. Sem cabo: cruzamentos do limiar
  `THRSH` ou o `RATE` interno. **Plugue aqui:** `CLOCK.euclid`, o gate
  de uma voz cíclica.
- **`SWP`** (sweep) (controle) — CV que soma ao `DELAY` — varre a janela
  pela onda. **Plugue aqui:** um `SEQUENCE` de CV (uma frase de
  posições), um LFO.
- **`THR`** (controle) — CV que soma ao limiar `THRSH` do auto-trigger.

### Saídas

- **`OUT`** (áudio, ou CV) — o sinal seguido / reconstruído, misturado
  com o seco por `BLEND`. **Plugue em:** um canal do `MIXER` (no modo
  OSC, é uma voz), `VCA.cv` (no modo follower, um seguidor de envelope),
  `OSC.1V/O`.
- **`GEIG`** (geiger) (controle, gate) — um trem de gates de **Poisson
  livre** (não preso a clock). **Plugue em:** `DECISION.trig`,
  `ENVELOPE.gate` — eventos esparsos e irregulares.

## Os controles, um a um

**DLY** (delay, 0–1) — onde a janela abre, como fração do período. Gira
uma "cabeça de leitura" por dentro da onda capturada.

**APER** (aperture, 0–1) — a largura da janela. →0 = amostra pontual
(vira um S&H de fase fixa); larga = a média borra um arco.

**AVG** (average, 1–64) — a profundidade N da média. Alto = o ruído some
(~1/√N), o coerente fica.

**SCAN** (−1..1) — a velocidade e direção com que o `DELAY` varre o
período sozinho. 0 = estático. ≠0 = reconstrói a onda toda.

**MODE** (0–2) — 0 follower · 1 reconstruct · 2 oscillator.

**RATE** (0,05–40 Hz) — o relógio interno (se `TRIG` livre) e a
frequência de releitura no modo OSC.

**THRSH** (−1..1) — o limiar do auto-trigger (edge trigger de
osciloscópio).

**GEI** (geiger, 0–1) — a densidade do trem de Poisson na saída `GEIG`.
0 = saída muda.

**BLEND** (0–1) — seco ↔ processado. 0 = bypass.

## Como cabear

**Revelar um tom no ruído:**
```
NOISE (white) + OSC (sine fraco) → MIXER → BOXCAR (IN)
CLOCK (na frequência do tom) → BOXCAR (TRIG)     MODE 0, AVG alto
BOXCAR (OUT) → MIXER (ch1)
```
Ao longo de segundos, o tom "se desenha".

**Wavetable do que ele ouviu:**
```
voz → BOXCAR (IN)
BOXCAR (TRIG) ← o gate da voz
BOXCAR (OUT) → MIXER     MODE 2, SCAN ~0,3, RATE na faixa de áudio
```

**Seguidor de envelope estável:**
```
voz → BOXCAR (IN)     MODE 0, APER larga, AVG médio
BOXCAR (OUT) → VCA (cv)
```

## Potencializar

- **Scan como frase:** `SEQUENCE.pitch → SWP` — uma melodia de
  *posições* dentro da onda capturada.
- **Geiger no groove:** `GEIG → TRIGSEQ.fill` — viradas em tempos
  aleatórios.
- **De follower a oscilador:** automatize o `MODE` — o módulo mede uma
  voz e depois "vira" essa voz.

## Se você conhece o Eurorack

Inspirado no AI Synthesis AI250 BXR. A base é o *boxcar averager* de
bancada (Stanford Research SR200/SR250 — domínio público), o edge
trigger de osciloscópio, e o processo de Poisson. O `NOISE` (#19) tem o
mesmo modo Poisson (param `poisson`). Distinto do `SH`/`SCOPE`.
