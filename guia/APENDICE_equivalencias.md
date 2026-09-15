# Apêndice — "se você conhece o Eurorack"

Uma tabela de tradução pra quem chega do Eurorack (hardware ou VCV Rack):
qual módulo do Rasgo faz o papel de qual módulo conhecido, e onde o Rasgo
diverge de propósito.

Isto é um mapa de **papéis**, não de clones. O Rasgo estudou o conceito
público desses módulos (teoria, fichas técnicas, patentes expiradas) e
escreveu o próprio código — nunca copiou. A proveniência módulo a módulo
está na seção 7 de cada página do guia e, em uma linha, em
[`../RASGO_MODULAR.md §36.3`](../RASGO_MODULAR.md).

**Este apêndice cresce junto com o guia.** Hoje traz as equivalências mais
pedidas; cada família preenchida adiciona as suas.

---

## Vozes e osciladores

| Se você usa… | No Rasgo é… | Diferença deliberada |
|---|---|---|
| VCO analógico (Doepfer A-110, DPO) | **`OSC`** | 5 formas ao mesmo tempo em saídas separadas; FM linear through-zero de verdade; sub embutido; `drift` orgânico |
| Mutable Plaits (modelo *chord*) | **`CHORD`** | parafônico com condução de vozes (`voicing`) na troca de acorde; casa com o `HARMONY` pra progressões |
| Mutable Rings / Elements (voz) | **`MATTER`** | 24 modos; `structure` corda→sino, `position` = onde a excitação bate |
| Karplus-Strong / corda física | **`STRING`** | `tanh` no laço → arco vira ciclo-limite estável, não estoura |
| Xaoc Odessa (aditivo) | **`ADDITIVE`** | 64 parciais; `drift` é cintilância *determinística* (sem RNG) |
| Yamaha DX / 4-op FM (Akemie's Castle) | **`OPERATOR`** | 4 operadores, 8 algoritmos, feedback DX7; razões quantizadas à tabela harmônica/inarmônica |
| oscilador de wavetable (Piston Honda) | **`WAVETABLE`** | 16 quadros gerados por receita espectral (sem arquivo); captura um ciclo ao vivo do `SIGNAL-IN` |
| Rossum Panharmonium | **`SPECTRA`** | ouve `in`, acha os parciais e re-oscila seguindo; `freeze` = pad infinito de qualquer som |
| Curtis Roads / síntese pulsar | **`PULSAR`** | altura e timbre em duas frequências independentes; `mask` faz ritmo por subtração |
| Noise (Doepfer A-118), S&H (Wogglebug) | **`NOISE`** | 8 saídas ao mesmo tempo (branco→violeta + S&H + random suave); modo `poisson` = contador Geiger |

## Filtros e modeladores de timbre

| Se você usa… | No Rasgo é… | Diferença deliberada |
|---|---|---|
| VCF SVF (Cytomic, Ripples) | **`FILTER`** | 3 SVF na mesma frequência; `spread` abre a relação entre eles = formante; auto-oscila |
| EDP Wasp filter, Schlappi 100 Grit | **`WASP`** | inversores CMOS 4069 que ceifam duro e assimétrico; ceifa também *depois* do filtro |
| Buchla 259 Timbre, Serge Wave Multipliers | **`SHAPE`** | cadeia ring-mod → fold → wrap → sat → VCA num módulo; antialias 2× + ADAA |
| Make Noise Optomix, Buchla 292 (LPG) | **`LPG`** | modelo de vactrol com `bounce` (overshoot); `mode` cruza filtro↔VCA |
| EQ paramétrico (VCV Parametra) | **`PARAMETRIC`** | 4 estágios RBJ; `sweep` move as bandas como um grupo |
| Frap Fumana, 4ms SMR (banco de formantes) | **`FORMANT`** | 5 passa-faixas paralelos varrendo A→E→I→O→U; `mode` vocoder de 5 bandas |
| Bode/Moog frequency shifter | **`SHIFTER`** | banda única, saídas `up`/`down` simultâneas; `feedback` = barber-pole de Risset |
| vocoder (EMS 5000, Roland VP-330) | **`VOCODER`** | 4–20 bandas; `sibilance` para as fricativas, `freeze` = pad falado |
| Rings (modo ressoador) / Three Sisters | **`RESONATOR`** | banco de modos afinados batido de fora; `tilt` cruza as saídas low↔high |
| Schlappi 100 Grit, OTO Biscuit (lo-fi) | **`CRUSH`** | 5 vetores de dano digital, todos *semeados* → reproduzível |
| VCA lin/exp (Doepfer A-131/132) | **`VCA`** (duplo) · **`VCA4`** (banco) | CV atenuvertida *soma* ao knob (porta de verdade); `sum` = mini-mixer |
| Portamento (Minimoog), slide do TB-303 | **`GLIDE`** | portamento *por nota*: modos sempre / slide-gated / legato |
| atenuversor + offset (Maths ch1/ch4) | **`CONTROL`** · **`MULT`** | `CONTROL` = utilidade dupla de CV; `MULT` = uma fonte → 4 versões prontas |

## Modulação e tempo

| Se você usa… | No Rasgo é… | Diferença deliberada |
|---|---|---|
| Mutable Tides, Make Noise Maths (função) | **`FUNCTION`** | uma rampa que é envelope/LFO/oscilador conforme a taxa; `drift` |
| ADSR (Doepfer A-140), Maths (envelope) | **`ENVELOPE`** | A/D/(S)/R com VCA embutido; curva côncava↔convexa |
| Mutable Stages, Rossum Control Forge | **`STAGES`** | N segmentos; `hold` faz virar envelope OU sequência sem trocar de tipo |
| Sample & Hold (Doepfer A-148), Marbles | **`SH`** | duplo, com `correlation` entre os dois acasos (gêmeos↔espelho) |
| Batumi, ochd (multi-LFO), Wogglebug | **`DRIFT`** | move-se em escala de *minutos*, com memória e correlação; faz o patch de seed evoluir sozinho |
| Ian Fritz chaos, Wogglebug | **`CHAOS`** | poço duplo; o chute aleatório é o que atravessa de um poço pro outro |
| Pamela's New Workout, 4ms QCD (clock) | **`CLOCK`** | euclidiano O(1) + acento por AND/OR de divisores + drift no andamento |
| Music Thing Turing Machine | **`TURING`** | `lock` de acaso → laço travado; o par improvisado do `SEQUENCE` |
| Pamela's (÷/×), Kinks (lógica booleana) | **`LOGIC`** | divisor + multiplicador + and/or/xor + flip-flop + gate delay num módulo |
| Metropolix, René (sequenciador) | **`SEQUENCE`** | 8 passos editáveis × 5 modos de leitura; o par escrito do `TURING` |
| Mutable Grids, vpme Euclidean Circles | **`TRIGSEQ`** | gerador, não editor: `map` morfa entre 4 grooves; soa ao carregar |

## Decisão, roteamento, espaço, saída

| Se você usa… | No Rasgo é… | Diferença deliberada |
|---|---|---|
| Mutable Branches / Marbles (aleatório) | **`DECISION`** | gate de Bernoulli + CV uniforme→sino + déjà-vu (loop-lock) |
| quantizador (Scales, uO_C) | **`QUANTIZER`** | 12 escalas curadas; histerese contra a indecisão; glide |
| Noise Engineering Numeric Repetitor | **`ABACUS`** | aritmética de CV como *número*: soma/resto/bit a bit + contador binário → ritmo |
| AI Synthesis AI250 BXR (boxcar) | **`BOXCAR`** | `scan` reconstrói a onda toda; `geiger` = trem de Poisson livre |
| Doepfer A-151/A-152 (chave sequencial) | **`SWITCH`** | mux N→1 *e* demux 1→N; endereço por clock ou por CV |
| EMS Synthi / Doepfer A-138m (matriz) | **`MATRIX`** | grade 4×4 clicável; `ring` faz a coluna virar ring-mod de 4 quadrantes |
| Doepfer A-180, Intellijel Buff Mult | **`MULT`** | cada saída tem atenuversor + offset próprios; ocioso vira 4 fontes de tensão |
| Intellijel Planar 2, joystick Buchla 208 | **`PLANAR`** | morph vetorial XY; o gesto é gravável e *sai* como CV pra dirigir o patch |
| Mutable Clouds, arbhar (granular) | **`MEMORY`** | buffer de 3 s + freeze — a cicatriz do cabo virada módulo |
| reverb de rack (Rings reverb, FX Aid) | **`SPACE`** (multitap) · **`HALL`** (FDN) | `SPACE` = de eco a cauda num contínuo; `HALL` = 8 linhas, denso e estéreo |
| Magneto, Mimeophon, RE-201 (delay/fita) | **`LOOPER`** | delay de linha com `hold`/`reverse`/`age`/`heads` (multi-cabeça Space Echo) |
| chorus/flanger/phaser (Roland Dimension) | **`SWIRL`** | os quatro num módulo; `age` = caráter BBD que faz o flanger cantar sozinho |
| toca-fatias (MPC, Akai S-series) | **`SAMPLER`** | grava ao vivo ou de arquivo; `wear` = desgaste *por disparo*, determinístico |
| — (não há prato de DJ no Eurorack) | **`TURNTABLE`** | o buffer lido por um prato com massa e inércia; beatmatch é gesto, sem quantizar BPM |
| mixer de performance (WMD/SSF) | **`MIXER`** | 4 canais, pan de potência constante; isento da Motion Engine (o balanço é seu) |
| Mordax DATA, ALM MUM M8 (scope) | **`SCOPE`** | as medições *saem como CV* (pitch YIN, envelope, brilho, onset) — o patch se ouve |
| saída MIDI (Expert Sleepers, Hermod) | **`NOTE-OUT`** | converte gate + 1 V/oct em MIDI e grava a partitura; contraparte do `SIGNAL-IN` |
| entrada de áudio / MIDI (Ears, µMIDI) | **`SIGNAL-IN`** | áudio ao vivo + MIDI num adaptador só; voz mono last-note |

---

## O que o Rasgo tem e o Eurorack normalmente não

- **`drift` em quase todo módulo** — deriva orgânica *semeada*: um
  passeio lento e correlacionado (não ruído bruto) que imita a
  imperfeição de qualquer sistema físico real — mas, por ser sorteado a
  partir de uma seed, dois renders da mesma seed são byte-idênticos. Num
  módulo de hardware, esse tipo de deriva viria de tolerância de
  componente — aqui é escolhida e reprodutível (ver
  `COMO_PENSAR.md` §5).
- **medição que vira CV** (`SCOPE`, `BOXCAR`) — num scope de hardware a
  tela é um beco sem saída, a leitura fica presa ali. Aqui, o que o
  módulo mede (altura, brilho, nível, ataque) sai por um **jack** —
  qualquer coisa que já mede o som pode também **comandar** outra parte
  do patch (ver `29_scope.md`/`51_boxcar.md` pro método de medição sem
  FFT).
- **a relação entre saídas como processo** — `FILTER.spread`,
  `RESONATOR.tilt`, `SHIFTER` up/down: várias saídas são **leituras
  diferentes do mesmo evento**, não sinais independentes — o gesto é
  *cruzar* entre elas (redistribuir energia), não girar um knob de mix
  entre duas coisas separadas (ver `COMO_PENSAR.md` §4).
- **soa ao carregar** — todo módulo tem um modo autônomo, geralmente uma
  fonte de excitação interna de baixo nível que nunca deixa o módulo
  ficar mudo por falta de entrada; um patch faz som sem teclado nem
  entrada nenhuma (ver `COMO_PENSAR.md` §1).
- **a relação de cabo** (`RingMod`/`Fold`/`Difference`, ruptura/
  cicatriz, condução probabilística) — no Eurorack, combinar dois sinais
  exige um módulo dedicado (um Warps, por exemplo). No Rasgo, o **cabo
  em si** pode processar, porque ele já é um objeto com estado (ganho,
  condução) — não precisa de um HP a mais (clique no corpo de um cabo
  no painel pra usar; ver `RELACAO_DE_CABO.md`).
- **a Motion Engine** — o patch de seed pode evoluir sozinho ao longo de
  minutos (`DRIFT`, `MotionEngine`), coisa que um rack físico não faz
  sem um sequenciador externo dedicado a isso.
