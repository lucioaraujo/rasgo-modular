# SIGNAL-IN — a entrada do mundo

**Família:** SOURCE · **Módulo 49**
**Essência:** áudio ao vivo + MIDI num adaptador só. Um teclado, um
controlador ou outro instrumento **toca o Rasgo**; e o áudio da mesma
entrada fica disponível pra processar.
**Dossiê técnico:** [`../dossies/49_signal_in.md`](../dossies/49_signal_in.md)
· **Fonte:** `src/dsp/SignalIn.hpp`

---

## A ideia

O `NOTE-OUT` deixa o Rasgo **tocar** um sequenciador externo. O
`SIGNAL-IN` é o inverso: alguém de fora toca o Rasgo. `SIGNAL-IN.pitch →
OSC.1V/O`, `SIGNAL-IN.gate → ENVELOPE.gate`, `SIGNAL-IN.cc →
FILTER.cutoff` — o patch generativo vira um instrumento tocável. E o
áudio da entrada (`L`/`R`) continua disponível pra o Rasgo processar som
de fora (era o antigo `AUDIO-IN`).

É um **adaptador**: converte o mundo (ALSA, MIDI) para os sinais do
Rasgo. O núcleo do instrumento nunca sabe o que é ALSA — isso vive só no
painel.

## Por dentro

**Por que existe um "anel" (ring buffer) em vez de ler a placa de som
direto:** capturar áudio de hardware acontece numa **thread própria**,
que roda no próprio ritmo da placa de som — não é sincronizada, amostra
a amostra, com o cálculo do resto do patch. Se as duas partes
tentassem se comunicar diretamente, uma teria que **esperar** a outra
de vez em quando, e essa espera é exatamente o tipo de coisa que causa
estalos/glitches em áudio ao vivo. Um **anel** resolve isso: é uma fila
circular onde a thread de captura só **escreve** (nunca espera por
quem lê) e o motor de áudio só **lê** (nunca espera por quem escreve)
— as duas seguem seus próprios ritmos sem nunca travar uma a outra. O
MIDI usa a mesma ideia, num segundo anel.

**Voz monofônica com pilha (*last-note priority*), o que significa na
prática:** o `SIGNAL-IN` só toca **uma** nota de altura por vez (não é
polifônico) — mas guarda uma **pilha** de todas as teclas que estão
fisicamente pressionadas no momento, na ordem em que foram apertadas. A
saída `1V/O` sempre reflete a nota **mais recente ainda pressionada**.
Isso é o que permite o gesto clássico de teclado monofônico: segure
duas notas, solte a de cima — a saída volta pra nota de baixo (que
ainda está pressionada), sem precisar apertá-la de novo. É a mesma
lógica de um Minimoog/MS-20.

**Por que silêncio determinístico sem hardware capturando:** sem
nenhuma placa de som real conectada capturando áudio, as saídas `L`/`R`
ficam em silêncio absoluto — nunca lixo de memória, nunca ruído
aleatório do sistema. Isso importa além de "não estourar": um patch
que usa `SIGNAL-IN` continua **testável e reproduzível** mesmo sem
hardware nenhum plugado (útil pra abrir o painel numa máquina sem
microfone/interface e o patch ainda se comportar de um jeito previsível).

## Os jacks, um a um

**Não tem entradas** — o "de fora" entra pela placa de som e pela porta
MIDI, não por cabo de painel. **Só saídas:**

### Áudio

- **`L`** / **`R`** (áudio) — o áudio da entrada do sistema, canais
  esquerdo e direito, escalados por `GAIN`. Sem nada capturando,
  silêncio (nunca trava, nunca lê lixo). **Plugue em:** `FILTER.in`,
  `SPACE.in`, `MATTER.in`, `WAVETABLE.CAP` — ou direto num canal do
  `MIXER`.

### MIDI → CV

- **`1V/O`** (controle, altura) — a nota MIDI tocada como 1 V/oct (nota
  60 = 0 V), com o pitch-bend somado (× `BEND`). **Plugue em:**
  `OSC.1V/O`, `MATTER.1V/O`, `QUANTIZER.cv`, qualquer entrada de altura.
- **`GATE`** (controle, gate) — alto enquanto alguma tecla está
  pressionada (rampa de 1 ms). **Plugue em:** `ENVELOPE.gate`,
  `MATTER.HIT`, `DRUM.GATE`.
- **`VEL`** (controle) — a *velocity* da última nota (0–1), segurada.
  **Plugue em:** `VCA.cv`, `FILTER.cutoff`, `OPERATOR.IDX` — o toque
  mais forte soa mais alto / mais brilhante.
- **`CC`** (controle) — o valor do Control Change nº `CC#` (0–1),
  segurado. **Plugue em:** qualquer `_mod` — a *mod wheel* ou um fader
  do controlador comanda um parâmetro.

## Os controles, um a um

**GAIN** (0–2) — o ganho aplicado ao áudio da entrada.

**BEND** (0–24 st) — o alcance do pitch-bend do MIDI, em semitons. Afeta
a saída `1V/O`.

**CC#** (cc_num, 0–127) — qual Control Change a saída `CC` segue.
1 = *mod wheel*.

## Como cabear

**Tocar o patch com um teclado:**
```
SIGNAL-IN (1V/O) → OSC (1V/O)
SIGNAL-IN (GATE) → ENVELOPE (gate)
SIGNAL-IN (VEL)  → VCA (cv)          (ou → FILTER cutoff)
SIGNAL-IN (CC)   → FILTER (cutoff)   (a mod wheel abre o filtro)
OSC (SAW) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)
```

**Processar áudio de fora:**
```
SIGNAL-IN (L) → FILTER (in) → MIXER (ch1)
SIGNAL-IN (R) → SPACE (in)  → MIXER (ch2)
```

## Potencializar

- **Metade tocado, metade generativo:** o teclado toca a melodia
  (`1V/O`/`GATE`) enquanto o `CLOCK`+`TRIGSEQ` seguem a percussão
  sozinhos.
- **A voz de fora reafinando o Rasgo:** `SIGNAL-IN.1V/O → QUANTIZER.root`
  — o que você toca define a tônica da parte generativa.
- **Congelar o que entra:** `L → SPECTRA.in`, `FRZ` disparado por um
  pedal (`CC` acima de um limiar via `ABACUS`) — um pad do instante.
- **Wavetable do vivo:** `L → WAVETABLE.CAP`, `GATE → GRAB` — cada nota
  que você toca captura um ciclo do áudio de entrada.

## Se você conhece o Eurorack

Faz o papel de uma interface MIDI-CV (Doepfer A-190, Hermod, Expert
Sleepers) **mais** uma entrada de áudio (Ears, Doepfer A-119). Num só
módulo, com voz mono *last-note* (a lógica do Minimoog/MS-20). É a
contraparte de entrada do `NOTE-OUT` (#38).
