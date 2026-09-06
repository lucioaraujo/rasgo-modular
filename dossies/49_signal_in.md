# Dossiê — Módulo 49: Adaptador de entrada áudio + MIDI + CV (`SIGNAL-IN`)

**Família:** SOURCE (adaptador — na taxonomia §4.2, o verbo GESTO/ENTRADA
vive em SOURCE)
**Estado:** **implementado — Onda D** (2026-09-06) — módulo pronto; a
thread ALSA-seq do painel é o passo seguinte (como o `AlsaSource` foi pro
`AUDIO-IN`)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/SignalIn.hpp`, `src/dsp/AudioIn.hpp` (alias),
`tests/test_signal_in.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda D, #49)

## Estado da implementação

**Decisão do autor (2026-09-06):** em vez de módulos `MIDI-IN` e `CV-IN`
separados, o `AUDIO-IN` (#35) **cresce** pra um adaptador único —
áudio + MIDI num só. Contraparte de **entrada** do `NOTE-OUT` (#38).
Identidade RASGO: nó adaptador OPCIONAL, nunca dependência; o
`rasgo_modular_core` não sabe o que é ALSA nem ALSA-seq.

Dois anéis SPSC lock-free, alimentados de threads externas (`apps/panel/`):

- **ÁUDIO** — `pushSamples(interleavedLR, frames)`, idêntico ao
  `AUDIO-IN` (mesma lógica de anel, resync em estouro, silêncio em
  underrun).
- **MIDI** — `pushMidi(status, d1, d2)`: eventos empacotados num anel de
  1024. `process()` drena tudo por bloco e resolve uma voz **monofônica**
  (last-note com pilha de 16: solta a nota de cima e a de baixo volta).

**Saídas** (6): `out` (áudio L — **nome preservado** pra não quebrar
conexão de `.rmp` antigo), `r` (áudio R), `pitch` (1 V/oct, nota 60 =
0 V, + pitch-bend × `bend`), `gate` (rampa de 1 ms), `vel`, `cc` (o CC
nº `cc_num`).

**Parâmetros:** `gain` (0–2), `bend` (0–24 st — alcance do pitch-bend),
`cc_num` (0–127, def 1 = mod wheel).

**Migração:** `type()` devolve `"SIGNAL-IN"`. `makeModule("AUDIO-IN")`
**também** constrói este módulo (alias no `ModuleCatalog`), e re-salvar
um `.rmp` antigo escreve `"SIGNAL-IN"` — a migração acontece sozinha.
`src/dsp/AudioIn.hpp` virou `using AudioIn = SignalIn;` (compat de
`#include` e do nome no painel).

**Sem nada alimentando** (rodando fora do painel, os testes/exemplos, ou
painel sem MIDI/áudio conectado): tudo em silêncio — `pitch` = 0 V
(nota 60), `gate` = 0, determinístico. **Testável sem hardware.**

**Testes (Debug + Release):** áudio — herdados do `AUDIO-IN`
(round-trip, gain, underrun→silêncio, estouro→resync). MIDI — note-on
sobe `gate`/`pitch`/`vel`; last-note priority (C4, G4 por cima, solta G4
→ volta a C4, `gate` segue alto); note-on vel 0 = note-off; pitch-bend
±0,5 fundo de escala × `bend`; `cc` só segue o `cc_num` escolhido;
determinismo byte a byte com áudio + 3 eventos MIDI; `type() ==
"SIGNAL-IN"`.

**Pendências:**
- **thread ALSA-seq no painel** — abrir uma porta `snd_seq` de entrada,
  numa thread dedicada (como o `AlsaSource` do `AUDIO-IN`), e chamar
  `pushMidi()`. É o único pedaço que falta pra o MIDI funcionar ao vivo;
  precisa de um teclado pra validar.
- **CV bruto** — a "interface DC-coupled" (canais de áudio extras como
  CV) fica pra quando houver caso; hoje `SIGNAL-IN` é áudio + MIDI.
- polifonia (v1 é mono); saída de aftertouch / clock MIDI;
  `bend`/`glide`/`retrig` como no `GLIDE`.

---

## 1. Problema musical e papel no fluxo

O `NOTE-OUT` deixa o Rasgo **tocar** um sequenciador externo (contrato
`NOTE` do `MUSICAL SCORE`). `SIGNAL-IN` é o inverso: um teclado, um
controlador, outro instrumento MIDI **toca o Rasgo**. `SIGNAL-IN.pitch →
OSC.1V/O`, `SIGNAL-IN.gate → ENVELOPE.gate`, `SIGNAL-IN.cc → FILTER.cutoff`
— o patch generativo vira um instrumento tocável. E o áudio da mesma
entrada (`out`/`r`) continua disponível pra processar (era o `AUDIO-IN`).

## 2. Fontes ESTUDADAS

- **`AUDIO-IN` (#35)** — o anel SPSC de áudio (`pushSamples`/`process`,
  resync, underrun). Reusado tal e qual.
- **ALSA sequencer** (`snd_seq`) — API padrão do sistema pra I/O MIDI no
  Linux; a thread do painel a usará (pendência).
- **Voz monofônica de teclado** — last-note priority com pilha de notas
  (o comportamento clássico de sintetizador mono: Minimoog, MS-20).
  Fato de design, domínio público.
- **`NOTE-OUT` (#38)** — a contraparte; mesma ideia de adaptador sobre um
  contrato, sem mexer na interface de nenhum outro módulo.

**Desvio Rasgo:** o adaptador único (áudio + MIDI num módulo, não três) e
a integração — as saídas de MIDI são CV como qualquer outra, então um
teclado e um `LFO` são intercambiáveis no destino.

## 3. Modelo

**Anel de áudio:** igual ao `AUDIO-IN` (`dossies/35_audio_in.md`).

**Anel de MIDI:** `pushMidi` empacota `(status<<16 | d1<<8 | d2)` num
`array<uint32,1024>`; `midiWrite_` atômico (release). `drainMidi()` lê de
`midiRead_` até `midiWrite_` (acquire):
```
nibble alto de status:
  0x90 (note-on):  d2>0 → noteOn(d1,d2) ; d2==0 → noteOff(d1)
  0x80 (note-off): noteOff(d1)
  0xB0 (CC):       d1==cc_num → ccVal_ = d2/127
  0xE0 (bend):     bendNorm_ = ((d2<<7 | d1) − 8192) / 8192   ∈ [−1,1)
noteOn(n,v):  empilha n ; curNote_=n ; curVel_=v/127 ; gateHigh_=true
noteOff(n):   remove n da pilha ; pilha vazia ? gate off : curNote_ = topo
```

**Por amostra:**
```
pitch = (curNote_ − 60)/12 + bendNorm_ · bend/12          (oitavas)
gateRamp_ += (gateHigh_?1:0 − gateRamp_) · (1 − e^(−1/(0,001·sr)))
saídas: out=L·gain  r=R·gain  pitch  gate=gateRamp_  vel=curVel_  cc=ccVal_
```

**Extremos:** anel MIDI cheio (produtor muito à frente) → os eventos
antigos são sobrescritos (`& kMidiMask`), `drainMidi` lê a janela mais
recente — 1024 eventos = folga de sobra pra a granularidade de bloco.
Nenhuma nota → `pitch` = 0 V (nota 60), `gate` = 0. `bend` = 0 →
pitch-bend inerte. Todos os CC ignorados exceto `cc_num`.

## 4. Três modos obrigatórios

- **autônoma:** sem MIDI nem áudio conectado → silêncio total,
  determinístico (o painel abre e soa por outros módulos).
- **performance:** um teclado toca `pitch`/`gate`/`vel`; um controlador
  manda `cc`; o áudio de entrada passa por `out`/`r`.
- **híbrida:** `SIGNAL-IN.pitch → QUANTIZER → OSC` (teclado quantizado à
  escala do patch), `SIGNAL-IN.cc → MotionEngine`-style destino,
  `SIGNAL-IN.out → SAMPLER.in` (samplear o que entra).

## 5. Portas, parâmetros, limites

**Entradas:** nenhuma (adaptador — recebe de fora do grafo).
**Saídas:** `out` (Audio L), `r` (Audio R), `pitch` (Control v/oct),
`gate` (Control), `vel` (Control), `cc` (Control).
**Parâmetros:** `gain` (0–2, def 1), `bend` (0–24 st, def 2),
`cc_num` (0–127, def 1).
**Métodos externos (só do painel, fora do RT):**
`pushSamples(const float*, size_t)`, `pushMidi(uint8, uint8, uint8)`.
**Limites:** `out`/`r` em ~[−2,2] (áudio de entrada × `gain` até 2×).
CPU: por bloco drena os eventos MIDI (O(n eventos)); por amostra ~6
escritas + a rampa do gate. `prepare` aloca o anel de áudio (~0,5 MB) +
4 KB de anel MIDI.

## 6. Alternativas descartadas

- **`MIDI-IN` e `CV-IN` separados** — decisão do autor: um adaptador só.
  Menos módulos, e as saídas são CV genérica (um teclado e um LFO são
  intercambiáveis no destino).
- **manter `AUDIO-IN` e adicionar `MIDI-IN`** — o `AUDIO-IN` "cresce",
  não coexiste; a migração por alias no desserializador é barata.
- **polifonia na v1** — mono (last-note) cobre teclado tocado como
  sintetizador mono; poli precisa de N vozes e alocação, fica pra
  `SIGNAL-IN` poli ou um `POLY` dedicado.
- **parsear o MIDI no `process()` amostra a amostra** — a granularidade
  de bloco (~5 ms) é suficiente pra controle; drenar tudo no topo do
  bloco é mais simples e RT-safe.

## 7. Integração e painel

6 HP, família **SOURCE**. Display do nível de entrada. Knobs `GAIN`/
`BEND`/`CC#`; jacks `L`/`R` (áudio) + `1V/O`/`GATE`/`VEL`/`CC` (MIDI→CV).

No painel: `syncAudioIn` (renomear pra `syncSignalIn` — pendência
cosmética) já abre o `AlsaSource` quando há um nó `SIGNAL-IN`. Falta a
thread `snd_seq` que chama `pushMidi()` — mesmo padrão da thread de
áudio. `ModuleCatalog::makeModule` aceita `"AUDIO-IN"` como alias.

Cadeias canônicas: `SIGNAL-IN.pitch → OSC.1V/O`,
`SIGNAL-IN.gate → ENVELOPE.gate`, `SIGNAL-IN.cc → FILTER.cutoff`;
`SIGNAL-IN.out → FILTER/SHAPE/SAMPLER` (o áudio de entrada, como o
`AUDIO-IN`).
