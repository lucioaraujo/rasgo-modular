# Pesquisa — o Cardinal no grave (baixo, osciladores, LFO, envelope, VCF, VCA)

Data: 10/11/2026. Pedido do autor: "preciso que pesquise o que o cardinal tem
relacionado a baixo (oscs, lfos, envelope, vcfs, vcas, etc); também verifique
os diferentes osciladores". Estado: **pesquisa, nada implementado**.

Fonte: Cardinal 26.02 local (`/media/luc/2tb_interno/cardinal-26.02/plugins`),
92 coleções. Método: nome, descrição e etiquetas de cada módulo nos
`plugin.json`, e o código-fonte quando a descrição não bastava. Conceito
estudado; nenhum código copiado. As licenças citadas são as das coleções
(maioria GPL-3.0-or-later, compatível com a AGPL do Modular — mas a regra
RASGO continua: conceito, não código).

## 1. Panorama

| Categoria (etiqueta) | Módulos no Cardinal | No Modular |
|---|---|---|
| Oscilador / VCO | 123 | OSC, FUNCTION, WAVETABLE, ADDITIVE, OPERATOR, PULSAR, CHORD, PLL, NOISE, CHAOS, MATTER, STRING, DRUM, SAMPLER… |
| LFO | 69 | sem módulo "LFO": FUNCTION (rampa que vira LFO), DRIFT, CHAOS, PENDULUM, BOUNCE, MOUSSE |
| Envelope | 51 | ENVELOPE (AD/ASR + VCA), STAGES, FUNCTION, LPG |
| Filtro / VCF | 56 | FILTER (SVF três irmãs), WASP, LPG, PARAMETRIC, FORMANT, RESONATOR |
| VCA | 28 | VCA (2 canais, curva), VCA4, ENVELOPE (VCA embutido), LPG |
| Baixo dedicado | 2 vozes (Autinn Bass, Open303) + 6 bumbos | nenhuma voz de baixo dedicada; DRUM para o bumbo |

## 2. Vozes de baixo dedicadas

- **Autinn / Bass** ("Bass and Acid synth", GPL-3) — não é oscilador: é a
  **cadeia 303** que processa um oscilador de fora. Lido no código: envelope
  de **VCF separado do VCA**; **acento** que encurta o decaimento do filtro
  (200 ms), sobe o VCA ~60 % e **empilha** acentos seguidos (o "stacking"
  da Devilfish); ENV MOD em até 5 oitavas (como a 303); decaimento do VCF
  0,2–2 s, do VCA ~2,5 s; ataque do VCA de 3 ms só como "declicker"; cada
  constante tem a fonte anotada no código (manuais, Devilfish, NAM).
- **BaconPlugs / Open303** — invólucro do *open303* de Robin Schmidt (a
  emulação de referência da 303).
- **Bumbos**: Befaco Kickall (envelopes de altura e de volume), ChowDSP
  ChowPulse (o modelador de pulso do circuito do bumbo da 808), WSTD Bass
  Drum 9 (909), Synthetic/Marionette, Extratone Opabinia (bumbo primitivo
  que também distorce áudio de entrada).

**No Modular**: a gramática da 303 é patchável — OSC → FILTER
(`cutoff_mod`) → ENVELOPE, GLIDE com `slide` (o deslize da 303 já existe) —
mas faltam duas peças: **um segundo envelope independente para o filtro**
(o rack tem um ENVELOPE; FUNCTION ou STAGES podem servir, sem acento) e o
**acento** que mexe ao mesmo tempo em filtro, VCA e decaimento (é o ACCENT
proposto do Antitotem, §4.5 do estudo; o Rasgo Synth Studio tem a
`AcidBasslineVoice` como precedente da família).

## 3. Filtros que importam no grave

| Família | No Cardinal | No Modular |
|---|---|---|
| **Ladder de transistor (Moog, 24 dB)** | Autinn Flora, kocmoc LADR/uLADR, MockbaModular Filtah, Bidoo lIMbO (com **ganho que compensa** a perda de grave da ressonância) | **não há** |
| **Ladder de diodo (303)** | kocmoc DIOD (LP4/LP2, normalização de ganho, CV linear e exponencial) | **não há** |
| Sallen-Key | kocmoc SKF | não há |
| SVF | Fundamental, Bogaudio, Venom, Valley Feline, Surge, kocmoc SVF | FILTER (SVF TPT de Simper), WASP |
| Grave ressonante dedicado | rackwindows **Holt** (Airwindows: passa-baixa ressonante "focado no grave", 1–4 polos progressivos) | — |
| Físico | Sapphire Elastika (bolas e molas), Tube Unit (guia de onda) | MATTER, STRING, RESONATOR, SPRING |
| Vactrol / LPG | AudibleInstruments Ripples, Bogaudio VISH (envelope "vactrol-ish") | LPG |

**A maior lacuna do Modular no grave é o ladder.** O SVF do FILTER é limpo
e estável, mas o caráter do baixo subtrativo (o grave que "afunda" quando a
ressonância sobe, a saturação dentro do laço, o diodo ácido) é do ladder.
Detalhe que vários resolvem e é o crivo: **compensar o ganho do grave**
quando a ressonância sobe (lIMbO, DIOD), senão o baixo some justamente
quando o filtro canta.

## 4. Envelopes

- ADSR com **gate por estágio** (Befaco ADSR), DADSR(H) com atraso e hold
  (Bogaudio), **curvas moldáveis** por estágio (MSM ADSR), envelope
  percussivo (Befaco Percall), MSEG/LFO complexo (MindMeld ShapeMaster),
  **envelope "vactrol"** (Bogaudio VISH), seis envelopes relacionados
  (CVfunk Envelope Array), **decaimentos que se propagam por uma rede de 24
  nós** (CVfunk Impulse Controller), estágios avulsos encadeáveis (DHE Stage/
  Booster/Hostage), EG×VCA juntos (Surge).
- **No Modular**: ENVELOPE (AD/ASR, curva, VCA embutido), STAGES, FUNCTION.
  Faltam: LOOP e pulso de fim de ataque (já na fila, §4.12), DADSR/hold, e
  um envelope de filtro separado para o baixo (ver §2).

## 5. VCAs

Curva log↔lin↔exp contínua (Befaco Hex Mix VCA), **VCA com saturação suave**
(ZZC SC-VCA), VCA de 12 dB de ganho (Bogaudio VCAMP), tremolo/velocidade
(VELO). **No Modular**: VCA com `response` (curva) e `drift` — cobre o
essencial; a saturação suave no VCA é o que falta (e o Polivoks VCA do
Studio, §4.8, é o mesmo território: tanh de par diferencial).

## 6. LFOs

- Famílias de fase (Bogaudio 4FO/8FO, CVfunk Hex Mod com reset por fase),
  **sincronizados ao relógio com "wonk"** (CVfunk Wonk), LFO dobrado
  (AnimatedCircuits LFold), wavetable LFO, **atratores caóticos** como LFO
  (Sapphire Frolic/Glee/Lark/Zoo — Rucklidge, Aizawa, Dequan Li; NLC Sloth,
  lentíssimos, de Don Cross), MSEG.
- **No Modular** não há um módulo "LFO": FUNCTION, DRIFT, CHAOS, PENDULUM,
  BOUNCE e MOUSSE fazem o papel, cada um com caráter. Para o baixo, o que o
  Batruc decidiu vale aqui (§4.3b): taxa livre ou em divisões do compasso,
  **reset de fase como escolha** (livre, no compasso, no ataque), destinos
  rotulados. Conferir se o FUNCTION já sincroniza ao CLOCK com essas opções.

## 7. Os osciladores, por família

| Família | Exemplos no Cardinal | No Modular |
|---|---|---|
| Analógico clássico / polyBLEP | Fundamental VCO, Bogaudio VCO/XCO, 21kHz PalmLoop (polyBLEP/BLAMP), **MUS-X Oscillators** (sobreamostragem maciça, sync, FM, ring), Surge Classic/Modern | OSC (polyBLEP; polyBLAMP no triângulo é 2ª camada) |
| "Sabor" de sintetizador | MockbaModular Maug* (Moog: saw, square .48/.29/.17, shark), CZ* (Casio CZ: saw, pulse, reso1–3, dblsine) | — |
| **Phase distortion (Casio CZ)** | Bidoo TiARE, MockbaModular CZ*, HetrickCV Feedback Sine Chaos | **não há** |
| Supersaw / unísono | Surge Classic (unison), sonusmodular Chainsaw | não há (CHORD tem detune entre vozes) |
| Wavetable | Fundamental, Surge, Valley **Terrorform**, Bidoo liMonADe, sonusmodular Neurosc | WAVETABLE |
| FM / PM / operador | Bogaudio FM-OP, kocmoc OP, Valley Dexter (4 op), Venom XM-OP (FM/PM/RM + feedback), Autinn Excavi (TZFM) | OPERATOR, OSC (FM through-zero) |
| Through-zero + wavefold | Befaco **PonyVCO** (TZFM + folder + VCA), MSM OSCiX | OSC + SHAPE |
| Aditiva | Bogaudio ADDITATOR, sonusmodular Addiction/Tropicana, Befaco Octaves, Even VCO | ADDITIVE |
| Formante | Bidoo ForK | FORMANT (filtro), PULSAR |
| Macro (Mutable) | Braids, Plaits, Tides, Frames | PLANAR, FUNCTION (Tides é referência do FUNCTION) |
| Caos / Benjolin | Venom Benjolin, 21kHz TachyonEntangler (sync caótico), ZetaCarinae RosslerRustler, Firefly (osciladores acoplados por fase) | CHAOS, PLL |
| Física | KarplusStrongPoly, ChowModal, Surge String | STRING, MATTER, RESONATOR |
| Chip / 8-bit | BaconPlugs ChipWaves (NES), ChipYourWave (desenhado) | CRUSH (processador) |
| Curvas e geometria | Axioma **Rhodonea** (rosáceas), Bogaudio **CHIRP** (seno varrido), CVfunk Ouros (injeção de fase, estéreo) | — |
| Ruído / granular | Surge S&H Noise, E340 Cloud | NOISE, MEMORY |
| Drones | mscHack Dronez, sonusmodular Ladrone | — |
| Bytebeat | voxglitch Byte Beat | **território do Risco** (§4.11 do estudo) — não trazer |

## 8. O que isso pede ao Modular (proposta, por prioridade)

1. **LADDER** — o filtro que falta ao grave: transistor (Moog, 24 dB) e
   diodo (303) como modos; LP4/LP2; saturação no laço com antialiasing por
   antiderivada (a pendência ADAA do estudo, §4.1/Aquorbium);
   **compensação do grave** quando a ressonância sobe, visível como knob.
   Entra na seed como mais um processador da cadeia.
2. **A gramática do baixo** sem criar uma "voz de baixo pronta": o ACCENT
   (já aprovado, do Antitotem) com saídas para filtro, VCA e decaimento ao
   mesmo tempo, e o segundo envelope (FUNCTION/STAGES) documentado como
   receita na fase didática. O GLIDE já faz o deslize.
3. **OSC**: modos que faltam e são baratos — **phase distortion** (CZ),
   **unísono/supersaw** (vozes desafinadas com espalhamento), e a
   **sub-harmônica por razão** (o OSC tem SUB e SUB 2; a ideia do
   Subharmonicon — subdivisões 1/2…1/16 — falta; o Venom HQ quantiza para
   parciais e sub-parciais).
4. **VCA com saturação suave** (curva tanh, como o par diferencial).
5. **LFO para o baixo**: conferir FUNCTION com sincronia ao CLOCK e reset de
   fase como escolha antes de pensar em módulo novo.

Fora (outra personalidade ou repetição): imitações de marca (Maug*, CZ*,
303 como réplica), bytebeat (Risco), mais um SVF.

Cada item, se aprovado, passa pelo portão conceitual (estudo §4.8.1) num
dossiê próprio antes do código.
