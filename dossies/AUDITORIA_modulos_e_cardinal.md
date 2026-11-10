# Auditoria — os 69 módulos, suas lacunas e o que o Cardinal complementa

Data: 10/11/2026. Pedido do autor: "verifique módulo a módulo do rasgo
modular e verifique as lacunas, ou pontos fracos, investigue no cardinal o
que pode complementar, para implementação e melhoria de todos os módulos" —
e a questão anterior: "ou permitir abrir o mesmo módulo mais vezes, ou criar
módulos similares com especificidades".

Estado: **diagnóstico e proposta**. Só a correção técnica do MATTER (§2.1),
que é defeito, entra direto; o resto espera a ordem do autor.

## 0. Método (o que é medido e o que é inferido)

- **Levantamento mecânico** dos 69 módulos em `src/dsp/` (tamanho, painel,
  parâmetros, portas, testes, dossiê; marcas de sobreamostragem, polyBLEP,
  ADAA, suavização, guarda de finitude, alocação no `process`).
- **Três bancadas medidas** (programas no scratchpad, não no repositório):
  1. *degraus de parâmetro como o VARIA* — cada parâmetro de cada módulo de
     áudio varrido em degraus a cada ~33 ms entre 30 % e 70 % do curso,
     contra o mesmo módulo parado; reporta o maior salto entre amostras;
  2. *FM em taxa de áudio* (±3 oitavas a 331 Hz) na entrada de altura de 10
     módulos ressoadores/filtros: a saída fica finita?
  3. *(já existentes)* a régua de qualidade das seeds e a bateria de saída.
- **Cardinal 26.02**: as categorias de módulos por etiqueta e descrição
  (estudos de 08/11, §4.12 do estudo da família, pesquisa do grave de hoje).
- O que está marcado "a medir" é suspeita fundada no código, **não**
  achado confirmado.

## 1. A questão estrutural: um de cada tipo

**Hoje** o rack tem um módulo de cada tipo. Medido no código: o **núcleo e o
formato de patch já são por instância** (`node <id> <tipo>`, cabos e
modulações por id) — a restrição mora na **seed** (`PatchSeed`: ~60 buscas
`firstT`/`hasT` por tipo) e no **mapa `byType`** do app (15 usos), mais
textos e partitura que nomeiam por tipo.

| Caminho | A favor | Contra |
|---|---|---|
| **A. Instâncias** (o mesmo módulo várias vezes) | É a gramática do modular: dois OSC desafinados (espessura), dois ENVELOPE (VCA + filtro, o baixo), vários LFOs, polifonia. Nenhum código de DSP novo. | Refatorar seed e app para "tipo + número"; a seed ganha a decisão "quantas cópias"; a partitura e o VARIA passam a dizer "OSC 2"; o rack cresce na tela. |
| **B. Módulos parecidos com especificidade** | Sem refatoração; cada um tem identidade própria. | Duplica código e catálogo; o músico não entende por que "OSC-B" difere de "OSC"; vira atalho técnico com cara de módulo. |

**Recomendação**: **A, com limite e por família** — instâncias para fontes,
envelopes/funções, VCAs, filtros e utilitários (até 2–4 de cada); únicos
os do rack inteiro (MIXER, MASTER, CLOCK, SIGNAL-IN, LISTEN, NOTE-OUT). B
só quando a diferença for musical (o LPG não é "um segundo ENVELOPE").
Ordem segura: (1) app — adicionar uma 2ª instância à mão; (2) partitura e
VARIA com "TIPO n"; (3) só então a seed aprender a pedir cópias, com a
régua de qualidade vigiando. **Decisão do autor.**

## 2. Achados medidos que valem para todo o sistema

### 2.1 MATTER produz NaN/Inf com FM de áudio — **defeito** (corrigir já)

Bancada 2: FM ±3 oitavas a 331 Hz na entrada `freq_mod` → saída **não
finita**. RESONATOR, STRING, FORMANT, FILTER, WASP, LPG, PULSAR, ADDITIVE e
WAVETABLE ficaram finitos. MASTER e saída barram o NaN antes do
alto-falante, mas ele envenena o estado de qualquer módulo depois do MATTER
no patch (fica mudo). É o mesmo achado do Aquorbium na cópia (ressoador de
dois polos re-afinado a cada amostra não conserva energia).

### 2.2 Chaves e parâmetros discretos estalam quando mudam — **confirmado**

Bancada 1. Parâmetros contínuos **não** estalaram (melhor do que eu supunha:
nenhum knob contínuo deu salto relevante). O que estala é a troca de
chave/estado, que o VARIA faz (dwell mínimo de 5–25 s) e a mão também:

| Módulo · parâmetro | maior salto parado → trocando |
|---|---|
| OPERATOR `algo` | 0,24 → **1,29** |
| SWIRL `type` | 0,02 → 0,16 |
| LOOPER `reverse` / `hold` | 0,01 → 0,16 / 0,13 |
| BOXCAR `scan` | 0,00 → 0,14 |
| FUNCTION `slope` | 0,00 → 0,20 |
| MIXER `mute1..4` | 0,00 → 0,18 (o VARIA não toca; a mão sim) |
| MEMORY `freeze` | 0,00 → 0,08 |

Correção comum: **crossfade curto (5–10 ms)** na troca, dentro de cada
módulo; o MUTE do MIXER com rampa como o PAN já tem. É também uma causa de
estalo com o VARIA ligado, ao lado dos xruns já anotados.

### 2.3 Não há suavização de parâmetro no núcleo

38 módulos leem os parâmetros uma vez por bloco, sem rampa. A bancada 1
mostrou que, para os contínuos, isso não estala hoje — mas o risco volta com
blocos maiores ou mudanças bruscas (preset, MUDA/CRUZA). Proposta leve: o
`Signal` guardar o valor do bloco anterior e oferecer `parameterRamp(id)`
(início/fim do bloco) para os módulos de áudio que quiserem; sem mudar os
outros.

### 2.4 Não-linearidades sem antialiasing — **a medir**

Com saturação/dobra e sem sobreamostragem nem ADAA no código: DRUM
(`drive`), MATTER, STRING (`drive` no laço), RESONATOR, OPERATOR
(realimentação), PLL, PULSAR, PLANAR, SPACE, HALL, LOOPER, SWIRL, SHIFTER,
SPECTRA, VCA/VCA4 (curva), CHAOS (em taxa de áudio). Têm OS ou ADAA: SHAPE
(OS+ADAA), FILTER, WASP, LPG (OS). Próximo passo: medir o aliasing de cada
um (seno agudo + drive, energia fora dos harmônicos), como o Aquorbium
mediu −18,6 dB a 7 kHz no DRIVE; corrigir com ADAA de 1ª ordem (Parker,
Zavalishin & Le Bivic, DAFx 2016) onde passar do limite.

### 2.5 Outros

- **Testes**: todos os módulos do build têm teste; só MAGNET e ORBIT (lote
  2, fora do build) não.
- **Alocação**: nenhuma no `process`; VCA4 e MIXER montam nomes de
  parâmetro (`"gain"+n`) por bloco — sem heap (string curta), mas é busca
  por texto a cada bloco; trocar por índices é limpeza, não urgência.
- **Feito hoje** (correções aprovadas): denormais no núcleo, STRING
  afinada, loudness em 10 ms e sem log10 no áudio, Lagrange em SPRING e
  PRISM, FORMANT no nível, MASTER com pico verdadeiro e latência na bateria.

## 3. Módulo a módulo, por família

Colunas: o que é; **fraqueza/lacuna** (M = medido, C = lido no código,
E = escuta/ergonomia); **no Cardinal**; **proposta**.

### FONTES (SOURCE)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| OSC | (C) polyBLAMP do triângulo pendente; um só no rack (§1) | MUS-X Oscillators (sobreamostragem maciça), PalmLoop (BLEP/BLAMP), Surge Classic (unísono), CZ/TiARE (phase distortion), Venom HQ (sub-harmônicos) | unísono, phase distortion, sub-harmônica por razão (aprovados); polyBLAMP |
| WAVETABLE | (E) poucas tabelas próprias? conferir | Terrorform (deforma a tabela), Surge WT, liMonADe | modos de deformação da tabela (dobra, espelho, "rasgo" da tabela) |
| ADDITIVE | lacuna: FIBRA (aprovada) | ADDITATOR, Addiction, Octaves | FIBRA (parciais que rasgam) |
| OPERATOR | (M) `algo` estala na troca; (C) realimentação sem antialias | Dexter (4 op + wavetable), Bogaudio FM-OP, Venom XM-OP | crossfade na troca de algoritmo; medir aliasing da realimentação |
| PULSAR | (C) não-linear sem antialias — a medir | Bidoo ForK (formante) | medir |
| SPECTRA | (C) idem | Befaco Bandit (espectral) | medir |
| PLL | (C) idem | — (raro no Cardinal) | medir |
| CHORD | (E) acordes fixos por tabela | D'acchord (6 vozes) | ligar ao HARMONY/TOM (já é por CV); unísono por voz |
| NOISE | ok (6 cores, S&H) | Noise Plethora (multitimbral), Crackle | modo "estalos" (Crackle/Noise Cornucopia do Studio) |
| MATTER | (M) **NaN com FM de áudio** | ChowModal, Elastika | corrigir já (§2.1); conferir conservação de energia |
| STRING | corrigida hoje (afinação) | KarplusStrongPoly, Surge String | arco contínuo já existe; medir o `drive` no laço |
| DRUM | (C) `drive` sem antialias | Kickall (env de altura e volume), ChowPulse (808), WSTD 909 | conferir envelope de altura; medir drive |
| SIGNAL-IN | (E) sem escolha de dispositivo (usa o padrão do sistema) | — | seletor de entrada no app (se o autor quiser) |

### TRANSFORMAÇÃO (TRANSFORM)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| FILTER | lacuna: sem ladder | Flora, LADR, DIOD, lIMbO (compensação de grave), Sallen-Key | LADDER (aprovado) |
| FORMANT | corrigido hoje (nível) | Prism Rainbow (SMR), Bogaudio FFB | — |
| VOCODER | (C) a medir | Bogaudio PEQ14 + XF | medir |
| RESONATOR | ok na FM (finito) | surgext Resonator/Combulator | afinado "fora da série" como receita didática (Trioio) |
| WASP | ok | — | — |
| LPG | ok (OS) | Ripples, VISH (envelope vactrol) | — |
| VCA / VCA4 | lacuna: saturação suave | SC-VCA (soft clip), Hex Mix VCA (curva) | VCA com saturação (aprovado) |
| SHAPE | ok (OS+ADAA) | re-fold, Wave Multiplier (A-137-2) | FractureFolder (dobra com ponto de reflexão irregular, Studio) |
| SHIFTER | (C) a medir | — | medir |
| CRUSH | ok (`wrap` = overflow) | Bitulator | — |
| PARAMETRIC | ok | PEQ6/14 | — |
| GLIDE | ok (slide) | SLEW, SLN | — |
| CONTROL | ok | — | — |
| DYNAMICS | (E) sem lookahead (por desenho) | Pressed Duck (ducking), ZamAudio Compressor | — |

### MODULAÇÃO (MODULATE)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| ENVELOPE | lacuna: LOOP, EOA, hold, DADSR | Befaco ADSR (gate por estágio), DADSR(H), MSM ADSR (curvas), ShapeMaster | LOOP + EOA (aprovado); DADSR como modo |
| FUNCTION | (M) `slope` estala ao trocar; papel de LFO do baixo a conferir | Tides, Rampage, LFO 4FO/8FO, Wonk | crossfade; sincronia ao CLOCK + reset de fase como escolha |
| STAGES | ok | Segment Generator (Stages), DHE Stage | — |
| DRIFT | aperfeiçoamentos aprovados (CAPTURE, resíduo, elasticidade) | — | (na fila) |
| CHAOS | (C) só o poço duplo; sem antialias em áudio | Sapphire Frolic/Glee/Lark/Zoo (atratores), NLC Sloth, Benjolin | mais atratores como modos (Rössler, Aizawa); medir aliasing |
| SH | ok | — | — |
| BOUNCE, MOUSSE, PENDULUM | validados, aguardam escuta | Bouncy Balls, Elastika | — |

### TEMPO (TIME)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| CLOCK | lacuna: segunda saída defasada (phasing), saída de ACENTO métrico | Polymetric Phasors, Lilt, Wonk | phasing e força métrica (Studio/Batruc) |
| LOGIC | ok | BOOLs, Numberwang | — |
| TURING | aprovado: transformação reversível + MEMORY (Navalha) | Rungler, Turing clones | (na fila) |
| SEQUENCE | aprovado: cursor físico, fadiga | PhraseSeq, Topograph | (na fila) |
| TRIGSEQ | ok | Grids/Topograph | — |
| ARP | ok | — | contravoz da seed já usa |
| CELL | ok | Glitch Sequencer (autômato) | — |
| (falta) | **TAP** (tap tempo) | — | aprovado (Maravi) |

### DECISÃO (DECISION)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| QUANTIZER | lacuna: peneiras de Xenakis, afinações (justa, pitagórica, EDO) | Xen Quantizer (Scala), QuantMT, Microtonal Chords | peneiras + afinações (aprovado) |
| HARMONY | lacuna: neo-Riemann P/L/R | — | modo P/L/R |
| ABACUS, DECISION, BOXCAR | BOXCAR `scan` estala (M) | Bernoulli Gate | crossfade no BOXCAR |
| MARKOV | ok (SURP) | Rosenchance | — |
| PHRASE | ok | — | — |
| (faltam) | **COERÊNCIA, TOM, BLEFE, FRASE** | nenhum equivalente | aprovados (originais) |

### ROTA (ROUTE)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| SWITCH | aprovado: scanner fantasma | Scanner, Sequential Switch | (na fila) |
| MATRIX | lacuna: rotas com história (Método §4.0) | MATRIX44/88, INTERMIX (cenas) | modo "rota que alarga com o uso" |
| MULT | ok | — | — |
| PLANAR | (C) não-linear a medir | Vector Mix, Cross Fade 3D | medir |
| (falta) | **VIA** (portas do Antitotem), RELATE | — | aprovados |

### ESPAÇO (SPACE)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| SPACE, HALL | (C) a medir aliasing na realimentação | Valley Plateau, Surge reverbs | medir |
| LOOPER | (M) `reverse`/`hold` estalam | LilacLoop, Sapphire Echo | crossfade na troca |
| SWIRL | (M) `type` estala | — | crossfade |
| MEMORY | (M) `freeze` estala (leve) | Clouds/Beads | crossfade |
| SAMPLER, TURNTABLE | ok | voxglitch samplers | — |
| SPRING, PRISM | corrigidos hoje (Lagrange) | Coil (mola) | — |
| (falta) | **APÓCRIFO** (citar o passado) | só gagueira de passo | aprovado (original) |

### SAÍDA (OUT)

| Módulo | Fraqueza / lacuna | No Cardinal | Proposta |
|---|---|---|---|
| MIXER | (M) MUTE sem rampa; PALCO feito hoje | MixMaster, Pressed Duck | rampa no MUTE |
| MASTER | pico verdadeiro e latência agora na bateria | BassMaster, Console | — |
| SCOPE | ok (YIN, onset) | Sassy Scope | saliência harmônica para melodia na mistura (Trioio) |
| NOTE-OUT, LISTEN | ok | — | — |

## 4. Ordem proposta (técnico primeiro, depois a fila aprovada)

1. **MATTER finito com FM de áudio** (defeito; já).
2. **Crossfade nas trocas discretas** (OPERATOR, SWIRL, LOOPER, BOXCAR,
   FUNCTION, MEMORY) e **rampa no MUTE do MIXER** — técnico, um teste por
   módulo (o salto na troca fica ≤ o salto parado).
3. **Medir aliasing** dos 16 módulos de §2.4; ADAA onde passar do limite.
4. **Decisão do autor sobre instâncias** (§1).
5. Fila aprovada (§4.12 do estudo + pesquisa do grave): COERÊNCIA, TOM,
   BLEFE, APÓCRIFO, FIBRA, FRASE/TAP, REGRAS da seed, LADDER, gramática do
   baixo, OSC (PD/unísono/sub), VCA saturado, ENVELOPE LOOP/EOA, QUANTIZER
   (peneiras/afinações), e os aperfeiçoamentos de §4.7.
