# RASGO Modular — pesquisa de módulos

**Estado:** vivo — reconstruído em 2026-09-01
**Finalidade:** a base de pesquisa por módulo. Para cada módulo que o Rasgo
Modular vai fazer: de que módulo(s)/artigo(s) real(is) ele parte, que
conceitos e tecnologias aplica, a licença da fonte, e o **desvio Rasgo**
(o que o torna original, não uma cópia). Processo do Atlas §49:
*módulo original → princípio → algoritmo → abstração → deslocamento →
comportamento Rasgo*.

> **Onde estava a pesquisa.** O autor pesquisou o ModularGrid Top 100 por
> dias no início de agosto/2026. A triagem completa **foi registrada** em
> `RASGO/AQUORBIUM/aquorbium-arquitetura.md` seção 9, "Pesquisa ModularGrid
> Top 100 — triagem e apropriação original" (commit `560857c`, 9/ago/2026).
> Está lá porque o RASGO Modular só nasceu como projeto em 16/ago, depois
> da pesquisa - e o trabalho foi feito no contexto do Aquorbium. As
> "apropriações originais" daquele doc são amarradas ao Bioma do Aquorbium
> (Organism/Desire/Environment/AquariumIntegrity); o que serve ao Rasgo
> Modular é a coluna **módulo real → conceito/ficha técnica verificada**.
> Este documento consolida isso e passa a ser o lugar único do Modular.

---

## 1. Método de desenvolvimento de módulo

O Rasgo Modular adota o **`MODULE_DEVELOPMENT_STANDARD.md` do Aquorbium**
(código do autor). Resumo:

**Dossiê antes do código** (`dossies/00_indice.md` — 27 feitos):
problema musical e papel no fluxo; fontes primárias e quais conceitos
foram apropriados (sem copiar código); equações, estados, comportamento
nos extremos, estratégia antialias/estabilidade; entradas/saídas/
feedback/limites de ganho/CPU/memória; três modos obrigatórios;
alternativas descartadas e por quê; critérios técnicos e perguntas de
escuta.

**Oito portões:** 1 arquitetura (API pequena, core portátil, integração
removível) · 2 correção (extremos, seed, reset, invariância de bloco, não
finitos) · 3 tempo real (nada de alocação/lock/espera/IO no callback;
pior caso conhecido) · 4 áudio (saída limitada, DC/descontinuidade
medidas, espectro quando há não linearidade) · 5 **generativo** (percurso
coerente sem eventos externos; não cai em repetição curta nem
aleatoriedade sem memória) · 6 performance (controles com nome e efeito
audível, suavização, sem salto destrutivo) · 7 escuta (render
reproduzível, A/B, registro honesto - diferença não é melhoria) · 8
orçamento (benchmark antes/depois, decisão explícita).

**Regra de profundidade:** o primeiro marco pode ser pequeno, mas
completo dentro do que promete. Função futura = candidata, nunca controle
fictício.

**Licença:** o Rasgo Modular é **AGPLv3-or-later** (decisão do autor,
2026-09-01 — `RASGO_MODULAR.md §29`). Código de terceiros só entra sob
licença compatível com AGPLv3 (MIT/BSD/ISC/Apache-2.0/LGPL/GPL/AGPL);
CC-NC/ND não. Mesmo o que é compatível: **fontes são ESTUDADAS**
(algoritmo, princípio) e reescritas do zero na arquitetura `SignalGraph`
— a intenção da v1 é partir de *conceitos*, não de código alheio; copiar
é decisão consciente por arquivo, com origem/alterações registradas.
Casos concretos: `RASGO_SYNTH/Scales.hpp` (Módulo 12) e
`HarmonicWanderer.hpp` (Módulo 14) — só as tabelas de intervalos / a
lógica de movimento, que são fato musical de domínio público, não o
código.

**Encontrar algo interessante que NÃO é livre** (plugin pago, código
fechado, esquema de cliente) **não é beco sem saída** — é ponto de
partida. O procedimento (regra do autor, 2026-09-02):

1. **rastrear a origem do conceito.** O que é fechado quase sempre é uma
   *implementação* de uma ideia mais antiga e pública. Ex.: VCV Doepfer
   A-124 (fechado) → Doepfer A-124 (esquema de cliente) → **filtro do EDP
   Wasp, 1978** (documentado à exaustão no DIY). Vai-se até a camada
   pública.
2. **aproveitar o CONCEITO** (topologia, princípio, comportamento), nunca
   o código nem o netlist do esquema fechado.
3. **adaptar ao Rasgo com originalidade** — o *deslocamento* do Atlas §49
   é obrigatório: não-linearidade no laço, `drift` seeded, a relação
   entre saídas como processo, determinismo. Se não há desvio, não é
   módulo Rasgo — é cópia.

O que fica registrado no dossiê: a fonte fechada só como *referência
funcional* ("de onde veio a ideia"), a fonte pública real de onde se
estudou, e qual é o desvio.

---

## 2. Ordem de execução (música generativa primeiro)

A ordem do Atlas §48 (Marbles, Branches, Clouds, Warps...) prioriza
decisão/memória. Mas pra **ouvir música** o Rasgo Modular precisa antes de
uma FONTE. Ordem adotada, cada um com dossiê próprio quando começar:

| # | Módulo | Família | Parte de | Estado |
|---|---|---|---|---|
| 0 | `SignalGraph` + `Cable` + `ControlSnapshot` | fundação | — | **feito** (2026-09-01) |
| 1 | **`FUNCTION` — gerador de função** | SOURCE / TIME | Tides/Stages (função emergente: LFO/env/osc conforme a taxa) + PolyBLEP + EMW VC Wavetable LFO. Desvio: `drift`. | **feito, marco 1** — `dossies/01_gerador_de_funcao.md` |
| 2 | **`FILTER` — filtro das três irmãs** | TRANSFORM | SVF TPT (Cytomic/Simper) + Three Sisters (`spread`: relação entre `low`/`center`/`high` = formante) + Ripples (MIT). Auto-oscilação por não-linearidade no laço. | **feito, marco 1** — `dossies/02_filtro.md` |
| 3 | **Comportamento de `Cable`: relação** | RELATION | Warps (ring-mod, waveshaping, cross-mod NA conexão) — `RingMod`/`Fold`/`Difference` com companion do bloco anterior + `amount` seco/molhado; serializado no patch | **feito, marco 1** — `dossies/03_relacao_de_cabo.md` |
| 4 | **`DECISION` — decisão / probabilidade** | DECISION | Branches (Bernoulli `bias`) + Marbles (déjà-vu / loop-lock, `spread`, quantização) + Sapèl (distribuição uniforme→sino via `shape`) + slew (S&H suave). Avança por trigger externo ou clock interno. | **feito, marco 1** — `dossies/04_decisao.md` |
| 5 | **`CLOCK` — clock euclidiano** | TIME | Toussaint/Bjorklund (E(k,n) por fórmula de Bresenham) + vpme Euclidean Circles (AND/OR de divisores → acento polirrítmico) + Pamela's (swing, `drift` no andamento) + Grids (ritmo como campo). Interno ou escravo de `ext_clock`. | **feito, marco 1** — `dossies/05_clock.md` |
| 6 | **`ENVELOPE` — envelope + VCA** | UTILITY / TIME | Maths (envelope-função, curva côncava↔convexa contínua, canal = gerador+processador) + Just Friends (transient/sustain) + Contour (AD/ASR). VCA embutido (`vca_depth`). | **feito, marco 1** — `dossies/06_envelope.md` |
| — | **`peca_generativa`** — peça de 40 s com os 6 módulos + fundação, determinística por seed, sem repetição, `.wav` | — | — | **feito** — `examples/peca_generativa.cpp` → `validation-output/peca_generativa.wav` |
| 7 | **`MEMORY` — buffer granular** | MEMORY | Clouds (buffer + textura granular + `freeze`) + arbhar (autoescuta) + Roads *Microsound* (grão = janela Hann; nuvem = grãos assíncronos). Pool de 16 grãos, `pitch`/`spray`/`feedback`/`blend`. A cicatriz do `Cable` como módulo. | **feito, marco 1** — `dossies/07_memory.md` |
| 8 | **`TURING` — laço de registrador** | SEQUENCE | Music Thing Turing Machine (shift-register, `lock` = P(preservar o laço), expansores Volts/Pulses) + Marbles `déjà-vu`. Sequência que emerge do acaso e cristaliza. Clock externo ou interno. | **feito, marco 1** — `dossies/08_turing.md` |
| M | **Modelo de conexão — matriz + constelação + semântico** | CONNECTION | (1) matriz: `matrixSources/matrixSlots/matrixCell/matrixToText`. (2) constelação: `setNodePosition/couplingFromDistance/applyConstellation`, `Cable::constellationGain`. (3) **semântico**: `contributeQuality`/`followQuality`/`qualityValue` — conectar por SIGNIFICADO (Energy/Brightness/Density/Tension/Motion), padrão `EnergyControlBus` do TRIOIO. As três camadas do modelo de conexão. | **feito, marco 2** — `test_signal_graph.cpp` |
| — | **`peca_generativa_2`** — peça de 50 s com os 8 módulos + matriz + constelação (MEMORY respira no campo), determinística, `.wav` | — | — | **feito** — `examples/peca_generativa_2.cpp` → `validation-output/peca_generativa_2.wav` |
| — | **`peca_generativa_3`** — peça de 55 s: melodia de corda numa escala (TURING→QUANTIZER→STRING) sobre baixo modal (MATTER), EQ + espaço, **barramento semântico** (Motion→SPACE.feedback, Energy→PARAMETRIC.gain3). Determinística, `.wav` | — | — | **feito** — `examples/peca_generativa_3.cpp` → `validation-output/peca_generativa_3.wav` |
| 9 | **`MATTER` — ressoador modal** | MATTER | Rings/Elements (24 modos, `structure` harmônico→esticado = corda→sino, `position` = onde bate, `damping` = T60) + síntese modal clássica (Cook/Adrien) + ressoador de 2 polos (J.O. Smith). Golpe interno por `strike`. | **feito, marco 2** — `dossies/09_matter.md` |
| 10 | **`SPACE` — espaço multitap** | SPACE | Schroeder/Moorer/Dattorro (comb + all-pass, damping no laço) + Rainmaker (N tomadas como material) + chorus (LFO na leitura). De eco rítmico a cauda pelos mesmos controles. | **feito, marco 2** — `dossies/10_space.md` |
| 11 | **`STRING` — corda por guia-de-onda** | MATTER | Karplus-Strong + Jaffe & Smith (all-pass fracionário pra afinar, posição de pinça) + J.O. Smith. Não-linearidade no laço (`tanh`) → arco = ciclo-limite estável. A outra face do `MATTER` (modal × guia-de-onda). | **feito, marco 2** — `dossies/11_string.md` |
| 12 | **`QUANTIZER` — quantizador de escala** | DECISION / PERCEPTION | 12 escalas curadas das tabelas pesquisadas de `RASGO_SYNTH/.../Scales.hpp` + histerese (hardware) + glide exponencial (theremin). Saída em oitavas (1 V/oct). Fecha "acaso → música". | **feito, marco 2** — `dossies/12_quantizer.md` |
| 13 | **`PARAMETRIC` — EQ paramétrico** | TRANSFORM / UTILITY | Fórmulas RBJ (Audio EQ Cookbook — LP/HP/shelf/peak) + ficha de recursos do VCV Parametra (N biquads CV, VCA, soft-clip; **código fechado, não consultado**). 4 estágios em série, 6 tipos, Q contínuo. Desvio: `sweep` move todas as bandas como grupo. | **feito, marco 2** — `dossies/13_parametric.md` |
| 14 | **`HARMONY` — movimento harmônico** | DECISION / INFERENCE | 6 técnicas reais de modulação (Coltrane/Giant Steps, sub tritônica+ii-V-I, mediante cromática/Jobim, intercâmbio modal, jazz modal/Miles, backdoor ii-V) — só a lógica de intervalos de `RASGO_SYNTH/HarmonicWanderer.hpp` (fato musical). Dirige `root`/`scale` do `QUANTIZER`. | **feito, marco 2** — `dossies/14_harmony.md` |
| 15 | **`SEQUENCE` — sequenciador de passos** | SEQUENCE | Hexen §119 (sequenciador = família de comportamentos de leitura) + René/Metropolix (direção como parâmetro) + Grids (browniano). Padrão de 8 passos editável (altura+gate por passo) × 5 modos de leitura (forward/backward/pingpong/random/brownian). O par escrito do `TURING`. | **feito, marco 2** — `dossies/15_sequence.md` |
| 16 | **`MIXER` — mixer de 4 canais** | MIX | prática de mesa: 4 entradas mono → soma; `gain` (−60..+12 dB), `pan` (potência constante), `mute` por canal; `out_gain` global; saída estéreo (grafo mono → (L+R)/2). Sem estado, sem alocação. | **feito, marco 2** — `dossies/16_mixer.md` |
| 17 | **`MASTER` — barramento de saída** | MIX / METER | matriz mid/side (Blumlein — `width` 0–2) + soma `mono` + bloqueio de DC (passa-alta 1 polo, J. Smith) + limitador suave de segurança (`tanh`, transparente até ±0,9) + `gain` + saída de VU (`level`, pico com decaimento 300 ms). O último nó de todo patch. | **feito, marco 2** — `dossies/17_master.md` |
| 18 | **`OSC` — oscilador subtrativo** | SOURCE | PolyBLEP (Välimäki/Finke — antialias na descontinuidade) + hard sync clássico + FM linear through-zero (Buchla 259) + sub por divisão (Juno/Moog). 5 formas ao mesmo tempo (seno/tri/serra/pulso/sub), 1 V/oct, PWM, `drift`. A voz "neutra" pra `SEQUENCE`→`QUANTIZER`→`OSC`→`FILTER`. | **feito, marco 3** (2026-09-02) — `dossies/18_oscilador.md` |
| 19 | **`NOISE` — ruído e aleatório** | SOURCE / UTILITY | filtro de ruído rosa de Paul Kellet (domínio público) + S&H clássico + Buchla 266 smooth random. Saídas branco/rosa/brown/S&H/smooth; `spread` uniforme→sino (acaso estruturado, padrão `shape` do `DECISION`). Trigger externo ou relógio interno. | **feito, marco 3** (2026-09-02) — `dossies/19_ruido.md` |
| 20 | **`VCA` — amplificador duplo** | TRANSFORM / UTILITY | VCA lin/exp (Doepfer A-131/132) + Quad VCA como mixer + atenuverter na CV (Maths). 2 canais; a CV atenuvertida SOMA ao knob (modulação por porta — o knob fica vivo); `response` lin→exp; softSat; `sum` = mini-mixer; desvio `drift`. | **feito, marco 3** (2026-09-02) — `dossies/20_vca.md` |
| 21 | **`CONTROL` — utilidades de CV** | UTILITY | Maths (atenuversor + offset + slew + somador) + Serge DUSG (retificação) + seguidor RC. Duplo: `scale` (−2..2), `offset`, `rectify` (lerp x→\|x\|), `slew`+`curve` (linear↔RC), `sum` (soma/média); `scale=0` = fonte de CV; `rectify`+`slew` = seguidor de envelope. | **feito, marco 3** (2026-09-02) — `dossies/21_control.md` |
| 22 | **`LOGIC` — lógica e utilidades de clock** | TIME / UTILITY | Pamela's (÷/×) + Kinks/Boolean (AND/OR/XOR) + flip-flop T + A-160 (contador módulo-N). Divisor ÷1–32, multiplicador ×1–8 (período medido), `gate_len`, `delay` (anel 0–200 ms), `flip`, `rate` interno se `clock` livre. Fecha o rack de partida. | **feito, marco 3** (2026-09-02) — `dossies/22_logic.md` |
| 23 | **`SH` — sample & hold duplo** | UTILITY | S&H clássico (Buchla 265/266) + smooth random (266) + Marbles `X` (correlação). 2 canais; `trackN`, `slewN` (glide), `slope` (subida ≠ descida), `spread` (uniforme→sino), `correlation` −1..1 (gêmeos↔espelho). Trigger externo ou relógio interno. Primeiro candidato do §2.2 feito. | **feito, marco 3** (2026-09-03) — `dossies/23_sample_hold.md` |
| 24 | **`SHAPE` — modelador de timbre** | TRANSFORM | ring-mod → wavefolder triangular + `symmetry` (harmônicos pares) → `wrap` (dobra↔wrap seco) → `sat` → VCA. Síntese por distorção da costa oeste (Buchla/Serge) como módulo visível. Desvio `drift`. Antialias: 2× + ADAA (2026-09-04). | **feito, marco 3** (2026-09-03) — `dossies/24_shape.md` |
| 25 | **`LPG` — low-pass gate a vactrol** | TRANSFORM / UTILITY | Buchla 292 / Optomix + modelo de fotocélula (LDR). Vactrol = seguidor não-linear assimétrico (sobe rápido, cauda longa); `mode` 0..1 (filtro↔VCA, 0.5=os dois), `response` (tempo da cauda), `offset`, `resonance`, `bounce` (overshoot do vactrol). O timbre plucky da costa oeste. Desvio `drift`. | **feito, marco 3** (2026-09-03) — `dossies/25_lpg.md` |
| 26 | **`CHORD` — VCO parafônico** | SOURCE | Plaits (modelo "chord") + Harmonaig + super-saw + PolyBLEP. 2–4 vozes empilhadas de uma base 1 V/oct; 10 formatos de acorde (por `chord` ou `chord_cv`), `inversion`, `voicing` (condução de vozes), `detune` (coro), `wave` (serra↔pulso↔tri). Casável com `HARMONY` no controle. Desvio: `drift` por voz. | **feito, marco 3** (2026-09-03) — `dossies/26_chord.md` |
| 27 | **`DRIFT` — campo de deriva** | DECISION / UTILITY | ANTITOTEM `deriveFromMemory`/`CRI-DRF-001` (momentum: velocidade acumula e retroalimenta a intensidade; cadência por loop) + AQUORBIUM `BiomaBrain::correlatedValues` (LFSR compartilhado, 4 leituras ponderadas distintas + LFO próprio). CV que se move em MINUTOS; `stride` (juntas↔separadas), `anchor` (memória de topologia — a deriva orbita marcos, 2026-09-04), `advance` (cadência por compasso), `event`. Faz o patch de seed evoluir sozinho. | **feito, marco 3** (2026-09-03) — `dossies/27_drift.md` |
| 28 | **`SWITCH` — chave sequencial** | ROUTE / UTILITY | Doepfer A-151/A-152 (chave sequencial/endereçada) + 4ms SISM (slew na troca) + CD4051 (teoria). 4 entradas → 1 saída (mux N→1); endereço avança no `clock`, zera no `reset`, ou vem da CV `addr`; `mode` (forward/pingpong/random semeado/só-`addr`), `glide` (crossfade na troca) + slew de 1 ms; saída `step`. O roteador controlado — variação de roteamento no fluxo autônomo. | **feito, marco 3** (2026-09-03) — `dossies/28_switch.md` |
| 29 | **`SCOPE` — osciloscópio + análise** | METER / UTILITY | osciloscópio de bancada (trigger nível/borda/histerese) + Mordax DATA/ALM MUM M8 + centroide espectral por Parseval + ZCR. Desvio Rasgo: medições saem como CV — `thru` limpo, `trig` (comparador c/ histerese), `level` (pico), `bright` (centroide sem FFT), `pitch` (v/oct por período, trava após 3 períodos), `hold`. Espectro desenhado / XY = pendência do painel. | **feito, marco 3** (2026-09-03) — `dossies/29_scope.md` |
| 30 | **`TRIGSEQ` — grade de trigs / percussão** | SEQUENCE / TIME | Mutable Grids (mapa rítmico + limiar de densidade — conceito, tabelas próprias) + TR-808/909 + Pamela's/randomRHYTHM. GERADOR (não editor): `map` morfa 4 caracteres (straight/broken/shuffle/sparse), `density1..4` por linha, `swing`, `chaos` (prob., não flip), `ratchet`, `fill` (entrada) + `fill_amt`, `drift`. Saídas `t1..t4` + `accent` (≥2 coincidem) + `any`. Determinístico. | **feito, marco 3** (2026-09-03) — `dossies/30_trigseq.md` |
| 31 | **`ABACUS` — aritmética binária de CV** | LOGIC / UTILITY | Noise Engineering Numeric Repetitor + retificador clássico + aritmética modular. `math` (`a`⊕`b`: soma/sub/mul/resto), `quant` (degraus + slew), `rect` (retificador dedicado: meia +/− / completa / sinal). Contador binário: `clock` soma `count_step`, `c = count mod modulus` → `p1`/`p2` (bits) + `carry` (overflow→ritmo). Sem `a` → fonte = rampa do contador (toca sozinho). Sem RNG. | **feito, marco 3** (2026-09-04) — `dossies/31_abacus.md` |
| 32 | **`WASP` — filtro áspero** | TRANSFORM | circuito do EDP Wasp (análises independentes — NÃO Doepfer/VCV) + inversor CMOS 4069 + SVF TPT não-linear. 12 dB com grão: SVF TPT (2 polos) + ceifador agressivo no laço + estágio de saída (buzz). `grit`, `bias` (pares + DC block), `mode` (LP↔BP↔HP), `drive` (waveshaper), corte a 24 kHz, `drift`. Auto-oscila. Contraponto sujo do `FILTER`. | **feito, marco 3** (2026-09-04) — `dossies/32_wasp.md` |

Feito desde então: movimento harmônico (`HARMONY`), inclinações de
24/48 dB no `PARAMETRIC` (cascata de biquads, 2026-09-02), sequenciador
editável (`SEQUENCE`), camada semântica de conexão (`followQuality`),
barramento de saída (`MIXER` + `MASTER`), painel gráfico de teste
(`apps/panel/`).

Ainda pendente: reverb por FDN (modo do `SPACE`), decaimento dependente
de frequência na `STRING`, camada de patch (cabos desenhados), sugestão
de patch por seed, front-ends de produção (JUCE + web/WASM).

### 2.1 Módulos essenciais que ainda faltam para o funcionamento básico

Mapeando o acervo contra o "rack de partida" clássico (VCO · VCF · VCA · EG ·
LFO · clock · seq · random · quantizer · mixer · saída): temos `FILTER`
(VCF), `ENVELOPE` (EG + VCA embutido), `FUNCTION` (LFO/função), `CLOCK`,
`SEQUENCE`/`TURING`, `QUANTIZER`, `HARMONY`, `MIXER`, `MASTER`, mais as
vozes de caráter (`MATTER`, `STRING`, `MEMORY`) e a camada de decisão
(`DECISION`). Faltavam **cinco** para fechar a base — **todos feitos**
(marco 3, 2026-09-02): `OSC`, `NOISE`, `VCA`, `CONTROL`, `LOGIC`. Cada um
com dossiê, partindo de conceito (não de código alheio):

| Prio | Módulo | Família | Por que é essencial | Parte de (a estudar) |
|---|---|---|---|---|
| ~~1~~ | ~~**`OSC` — oscilador**~~ | SOURCE | **FEITO — marco 3 (2026-09-02), `dossies/18_oscilador.md`.** 5 formas simultâneas (seno/tri/serra/pulso/sub) antialias PolyBLEP, 1 V/oct, PWM, hard sync, FM linear through-zero, sub-oitava, `drift`. `spread`/super-saw fica como desvio futuro | PolyBLEP (Välimäki/Finke), hard sync, TZFM (Buchla 259), sub por divisão |
| ~~1~~ | ~~**`NOISE` — ruído e aleatório**~~ | SOURCE + UTILITY | **FEITO — marco 3 (2026-09-02), `dossies/19_ruido.md`.** Branco/rosa (Kellet)/brown + S&H + smooth random (Buchla 266); `spread` uniforme→sino. Trigger externo ou interno | filtro de ruído rosa de Paul Kellet, S&H clássico, Buchla 266 smooth |
| ~~1~~ | ~~**`VCA` — amplificador**~~ | TRANSFORM / UTILITY | **FEITO — marco 3 (2026-09-02), `dossies/20_vca.md`.** Duplo, lin/exp, CV atenuvertida soma ao knob, softSat, `sum` mixer, `drift`. | VCA lin/exp (Doepfer), Quad VCA mixer, atenuverter (Maths) |
| ~~1~~ | ~~**`CONTROL` — utilidades de CV**~~ | UTILITY | **FEITO — marco 3 (2026-09-02), `dossies/21_control.md`.** Duplo: atenuversor (`scale` −2..2), `offset`, `rectify` contínuo (meia/onda-completa), `slew` + `curve` (linear↔RC), saída `sum` (soma/média), desvio `drift`. `scale=0` → fonte de CV. `rectify`+`slew` = seguidor de envelope. | Maths (atenuversor/offset/slew/somador), Serge DUSG, seguidor RC |
| ~~2~~ | ~~**`LOGIC` — lógica e utilidades de clock**~~ | TIME / UTILITY | **FEITO — marco 3 (2026-09-02), `dossies/22_logic.md`.** Divisor (÷1–32) + multiplicador (×1–8, período medido) de clock; `and`/`or`/`xor` de dois gates (saídas simultâneas); flip-flop T (`flip`); `gate_len` (duty) + `delay` (0–200 ms, anel); `rate` = relógio interno se `clock` livre. | Pamela's (÷/×), Kinks/Boolean (lógica), flip-flop T, A-160 (contador módulo-N) |

Com os cinco feitos, o **rack de partida está completo**: fonte (`OSC`),
ruído/acaso (`NOISE`), amplificação (`VCA`), utilidades de CV (`CONTROL`)
e de tempo/lógica (`LOGIC`) — mais as vozes de caráter, a camada
generativa e o estágio de saída.

### 2.2 Candidatos (depois do rack de partida)

> **Verificação de fontes Doepfer (2026-09-02):**
> - **GitHub:** a Doepfer Musikelektronik **não tem presença oficial** —
>   nenhuma org/user da empresa. Só trabalho de DIY da comunidade
>   (painéis, ferramentas 3D, editores dos controladores MIDI), licenças
>   variadas.
> - **`doepfer.de`:** **NÃO publica os esquemas completos.** O service
>   manual do A-100 (esquemas + silk + BOM) é **"available only for A-100
>   customers"** — documento de cliente, não aberto. A página DIY tem só
>   teoria de circuito, alguns PDFs de *modificação* (A-128/A-155/A-163/
>   A-165/A-188-1) e dicas soltas (ex.: A-124 auto-oscila com um 10k em
>   paralelo com R13). **Sem aviso de licença → todos os direitos
>   reservados.** Correção de uma afirmação anterior otimista demais.
> - **VCV "Doepfer":** plugin **proprietário, autoria "VCV"**
>   (`VCVRack/library/manifests/Doepfer.json` → `"license":
>   "proprietary"`), sem repo de fonte — **VERMELHO**.
>
> **Conclusão:** não usar esquema da Doepfer nem código do VCV. A fonte
> pública legítima do "Wasp" é o **circuito do EDP Wasp (1978)** — um dos
> filtros mais clonados do DIY, com esquemas independentes e análises
> abundantes (CMOS 4069 como elemento de ganho num filtro Sallen-Key de
> 12 dB). A Doepfer A-124 é ela mesma um clone do Wasp. RASGO parte do
> *conceito/topologia* e reescreve no `SignalGraph` (§49), como sempre.

| Módulo | Família | Ideia | Parte de (a estudar — do circuito/teoria pública, não de código fechado) |
|---|---|---|---|
| ~~**`WASP`** (ou modo do `FILTER`)~~ | TRANSFORM | **FEITO — marco 3 (2026-09-04), `dossies/32_wasp.md`, `src/dsp/Wasp.hpp`.** 12 dB/oitava com GRÃO: núcleo SVF TPT (2 polos, igual ao `FILTER`) com ceifador muito mais agressivo no laço + **estágio de saída** que ceifa DEPOIS do filtro (buzz reedy, não re-filtrado). `grit` (joelho do ceifador), `bias` (teto assimétrico → harmônicos pares + bloqueador de DC), `mode` (LP↔BP↔HP), `drive` (waveshaper com corte), corte a 24 kHz, `drift`. Auto-oscila perto de `resonance`=1. Antialias: núcleo a 2× (2026-09-04). Pendências: modelo de inversor CMOS mais fiel (auto-osc francamente palhetada), `fold` no laço. | circuito do EDP Wasp (1978, análises independentes — René Schmitz/DIY, Befaco Sallen-Key parente); inversor CMOS 4069 (teoria); SVF TPT não-linear (Zavalishin/Simper-Cytomic). **NÃO** service manual Doepfer (cliente-only) nem VCV (proprietário). |
| ~~**`SHAPE`**~~ (ex-`WAVEFOLDER`/`SHAPER`) | TRANSFORM | **FEITO — marco 3 (2026-09-03), `dossies/24_shape.md`, `src/dsp/Shape.hpp`.** Cadeia: ring-mod (`x·mod`, dry/wet) → wavefolder triangular fechado (`fold`) com `symmetry` (bias = harmônicos pares) → `wrap` (dobra suave↔wrap seco) → `sat` (tanh progressivo) → VCA (`level`). Desvio `drift` no drive da dobra. Antialias: 2× + ADAA de 1ª ordem (2026-09-04). | Buchla 259/258 timbre (fold + symmetry), Serge VCM/Wave Multipliers, ring-mod clássico, dobra triangular fechada |
| ~~**`SH`** — dual sample & hold~~ | UTILITY | **FEITO — marco 3 (2026-09-03), `dossies/23_sample_hold.md`, `src/dsp/SampleHold.hpp`.** 2 canais; segura `inN` ou o acaso interno no pulso de `trigN`/relógio interno; `trackN` (track & hold), `slewN` (glide Buchla 266), **`slope`** (−1..1: subida ≠ descida — 2026-09-04), `spread` (uniforme→sino), **`correlation`** (−1..1: gêmeos↔espelho, Marbles `X`). | Buchla 265/266, Doepfer A-148, Marbles `X`, track & hold, `shape` do `DECISION` |
| ~~**`SCOPE`** — osciloscópio + analisador~~ | METER / UTILITY | **FEITO — marco 3 (2026-09-03), `dossies/29_scope.md`, `src/dsp/Scope.hpp`.** Desvio Rasgo: as medições **saem como CV**. `in`→`thru` limpo (a saída 0 = o que o Display desenha); `trig` = comparador com histerese (`reject`) contra `trigger`/`edge` (trigger do scope + disparador utilitário); `level` (seguidor de pico); `bright` (centroide espectral pelo diferenciador — Parseval, **sem FFT**); `pitch` (v/oct, período entre cruzamentos de zero, trava após 3 períodos consistentes); `hold` congela. **Espectro desenhado (barras Goertzel — 2026-09-04):** clicar no Display alterna onda ↔ espectro (feature do painel, não porta). Pendências: modo XY/Lissajous, autocorrelação p/ pitch. | osciloscópios de bancada (trigger de nível/borda/histerese), Mordax DATA, ALM MUM M8, centroide espectral por Parseval, ZCR (detecção de pitch por período) |
| ~~**`CHORD`** — VCO parafônico~~ | SOURCE | **FEITO — marco 3 (2026-09-03), `dossies/26_chord.md`, `src/dsp/Chord.hpp`.** 2–4 vozes de uma base 1 V/oct; 10 formatos (`chord`/`chord_cv`), `inversion` (0–3), **`voicing`** (condução de vozes na troca de acorde — mínimo movimento + glide, 2026-09-04), `detune` (coro), `wave` (serra↔pulso↔tri, PolyBLEP), `drift` por voz. Soma normalizada `1/√vozes`. | Plaits, Harmonaig, super-saw, PolyBLEP, tabelas de acorde |
| ~~**`LPG`** — low-pass gate a vactrol~~ | TRANSFORM / UTILITY | **FEITO — marco 3 (2026-09-03), `dossies/25_lpg.md`, `src/dsp/Lpg.hpp`.** Seguidor de vactrol assimétrico (sobe ~2 ms, cauda que freia perto de 0) controla um filtro de 2 polos E um VCA juntos; `mode` 0..1 (filtro↔VCA, 0.5 = os dois), `response` (tempo da cauda ~30 ms–2,5 s), `offset`, `resonance`, **`bounce`** (overshoot do vactrol pós-golpe — 2026-09-04); `strike` + `cv`. Desvio `drift`. Antialias: `fc` suavizado (o filtro é linear, não aliasa). | Buchla 292 / 200-series LPG, Make Noise Optomix / DPO (LPG), Mannequins Three Sisters (modo LPG), modelo de fotocélula (LDR: resposta exponencial assimétrica, "memória" do vactrol) |
| ~~**`SWITCH`** — chave sequencial / roteador controlado~~ | ROUTE / UTILITY | **FEITO — marco 3 (2026-09-03), `dossies/28_switch.md`, `src/dsp/Switch.hpp`.** 4 entradas `a`/`b`/`c`/`d` → 1 saída `out` (mux N→1); endereço avança em `clock` (borda ↑), zera em `reset`, ou vem da CV `addr` (se conectada, manda); `steps` 2–4, `mode` (forward/pingpong/random semeado/só-`addr`), `glide` (crossfade na troca) + slew de 1 ms anti-clique; saída `step`. **`dir`** (0/1, 2026-09-04): `1` = demux 1→N (`a` → `out`/`out_b`/`out_c`/`out_d`). Pendências: `hold` nas saídas não-selecionadas, 8 entradas, `spread` no `random`. | Doepfer A-151/A-152 (switch sequencial/endereçado), 4ms SISM, Erica Pico SEQ/S&H, multiplexador CD4051 (teoria) |
| ~~**`MATRIX`** — matriz de roteamento~~ | ROUTE / MIX | **FEITO — marco 3 (2026-09-04), `dossies/33_matrix.md`, `src/dsp/Matrix.hpp`.** Módulo auto-contido 4×4: cada cruzamento é um ganho (atenuversor), `out_k = level·sat(Σ_j in_j·g_jk)`. 16 células `g11..g44` (padrão identidade = passa-direto), `norm` (nível constante por coluna), **`ring`** (coluna vira produto = ring-mod de 4 quadrantes — 2026-09-04), `sat` (matriz segura em laço), `level`, `drift` (os ganhos respiram). No painel gráfico é uma **grade 4×4 clicável** (2026-09-04). Determinístico. Pendências: grade N×M configurável, `slew` nos ganhos, expor a `matrixCell` do motor como alternativa. | Doepfer A-138m / A-100 matrix, Serge / EMS Synthi (matriz de pinos), Befaco / Erica matrix mixer; camada matriz do `SignalGraph` (marco 2) |
| ~~**`TRIGSEQ`** — sequenciador de trigs (grade de bateria)~~ | SEQUENCE / TIME | **FEITO — marco 3 (2026-09-03), `dossies/30_trigseq.md`, `src/dsp/TrigSeq.hpp`.** 4 linhas de gate on/off (bumbo/caixa/chimbal/perc). **Gerador, não editor** (identidade RASGO): `map` (0–1) morfa 4 caracteres arquetípicos interpolando o peso de cada passo; `density1..4` = limiar sobre o peso (Grids); `swing` (passos ímpares), `chaos` (fantasma/queda por probabilidade), `ratchet` (rajada de 3), `fill` (entrada) + `fill_amt`, `drift` (passeio lento). Saídas `t1..t4` + `accent` (≥2 linhas) + `any`. `length` recorta, `rate` = relógio interno. Determinístico (xorshift semeado). No painel gráfico o Display mostra as **4 lanes de gate rolando** (2026-09-04). Pendências: overlay editável de toggles (precisa de máscara de passos no módulo), 6–8 linhas, `prob` por passo explícito, saída de velocity. | TR-808/909 (grade), Pamela's PRO Workout, Vermona randomRHYTHM, Grids (mapa rítmico — conceito público, tabelas reescritas) |
| ~~**`ABACUS`** — lógica e aritmética binária de CV~~ | LOGIC / UTILITY | **FEITO — marco 3 (2026-09-04), `dossies/31_abacus.md`, `src/dsp/Abacus.hpp`.** `math` = `a` ⊕ `b` por `op` 0–7: soma/subtração/multiplicação/resto · **bit a bit AND/OR/XOR/NAND** (inteiros de 5 bits — Lunetta, 2026-09-04); `quant` = fonte em `steps` degraus + `slew`; `rect` = **retificador dedicado** (meia +/− · onda completa · sinal). Contador binário: `clock` soma `count_step` (pode ser negativo), `c = count mod modulus` → `p1` (bit `bitA`), `p2` (XOR de bits adjacentes), `carry` (overflow → ritmo). **Sem `a` → fonte = rampa do contador** (toca sozinho). Determinístico, sem RNG. Pendências: `shift`/rotação como `op` 8+, `modulus`/`steps` por CV, euclidiano do contador. | Noise Engineering Numeric Repetitor / Bin Seq (contador + máscara), retificador clássico (Maths / Serge DUSG), aritmética modular (teoria), divisor binário / Gray code (teoria) |
| ~~**`MULT`** — múltiplo bufferizado~~ | UTILITY | **FEITO — marco 3 (2026-09-04), `dossies/34_mult.md`, `src/dsp/Mult.hpp`.** Múltiplo PROCESSADO: 1 entrada → 4 saídas, cada uma com atenuversor (`scale` ±2, negativo = inverte) + `offset` (±1) próprios — o mini-`CONTROL` por tomada que a nota pedia. `slew` compartilhado. **`dual`** + `in2` (2026-09-04): out1/2 ← `in`, out3/4 ← `in2` (A-180-2). Ocioso (sem `in`) → 4 fontes de tensão manual (`out_k = offset_k`). Sem `drift` (utilidade de precisão). Pendências: `slew` por tomada (rise/fall), saída de soma, barras animadas no painel. | múltiplo bufferizado clássico (Doepfer A-180, Intellijel Buff Mult), atenuversor+offset (Maths/Serge), voltage spreader (Frap/Doepfer) |
| **`SAMPLER` / `TURNTABLE` / `TAPE`** — áudio gravado como matéria | SOURCE / MEMORY / GESTURE | sampler (buffer de disco ou ao vivo, varispeed, slice, `wear`), toca-discos de DJ (prato com inércia, scratch, crossfader), fita cassete (wow&flutter, saturação magnética, `age`, modo echo). **Nó opcional** — o painel abre e soa sem arquivo nenhum. | **Ver o estudo à parte: [`dossies/ESTUDO_audio_sampling.md`](dossies/ESTUDO_audio_sampling.md)** — decisão de biblioteca (`dr_wav.h`, domínio público/MIT-0, header-only), risco de determinismo, e (revisão 2026-09-06) **§2: o `NAVALHA2_JUCE` já tem `SlicePlayer` + `HeritagePitch` + `SliceBank` portáveis** (GPL-3.0-or-later, crédito a Glerm Soares + Lúcio Araújo) e o vocabulário GAP/STUTTER/BURST/MICRO/BLADE já articulado. |
| voz de **percussão** | MATTER/SOURCE | `MATTER` + `NOISE` + `ENVELOPE` já montam bumbo/caixa/prato; um módulo dedicado empacota | 909/808, Rings percussivo |
| ~~**`AUDIO-IN`** — entrada de áudio ao vivo~~ | SOURCE | **FEITO — marco 3 (2026-09-04), `dossies/35_audio_in.md`, `src/dsp/AudioIn.hpp` + `apps/panel/AlsaSource.hpp`.** Nó adaptador OPCIONAL (nunca dependência — `§36.8`): qualquer fonte de áudio externa rodando no sistema (outro instrumento RASGO, entrada de linha, microfone) vira matéria-prima dentro do grafo. Anel circular SPSC lock-free entre a captura ALSA (thread própria, dedicada — não compartilha temporização com a reprodução) e `process()`; sem `AUDIO-IN` no patch, a captura nem abre (nunca pega o microfone à toa); sem alimentação, saída em silêncio (nunca lixo, nunca trava). `gain` (0–2×). Pendências: seleção de dispositivo (hoje só "default"), medidor de nível no painel, mono-sum opcional. | ANTITOTEM/NAVALHA2 (`setAudioChannels`), `AlsaSink.hpp` (mesmo padrão, contraparte de captura); pergunta do autor "como podemos conectar o antitotem no rasgo modular?" |
| **adaptadores** `MIDI`/`CV` | INPUT / GESTURE | opcionais, nunca dependência (`§36.8`) | — |

### 2.3 Candidatos — levantamento ANTITOTEM (2026-09-04)

Pedido do autor: "o antitotem tem vários módulos interessantes [...]
precisa vasculhar, o que temos na lista de módulos interessantes que
ainda não foi feito". Leitura de `ANTITOTEM/src/core/{CmosVoice,
ChaosSources, NoiseFields, SimpleSequencer}.h` (código do autor,
GPLv3/AGPLv3 — compatível; conceito, não código, como sempre). Primeiro,
o que o autor lembrou e **já está feito** — pra não redescobrir:

- **"a parte de deriva é interessante"** → já é o **`DRIFT`** (Módulo 27,
  `dossies/27_drift.md`), explicitamente derivado de
  `ANTITOTEM/deriveFromMemory`/`CRI-DRF-001` — momentum, `anchor`
  (memória de topologia), 4 saídas correlacionadas. **Feito.**
- **"o noise é rico"** → o `NOISE` do Rasgo já dá branco/rosa/brown
  **simultâneos** (saídas paralelas, não um seletor de 1 cor como o
  `NoisePalette` do Antitotem) + S&H + smooth random. Mais rico nesse
  eixo específico (patcheável, não precisa escolher 1 cor por vez). O
  que falta ver abaixo (`azul`/`violeta`/`bit`).
- **scanner direction** (`ScannerDirection`: forward/reverse/pendulum) →
  já coberto pelo `dir` do `SWITCH` (Módulo 28).
- **step rules** (`mutate`/`ratchet`) → já cobertos por `TURING.mutate` e
  `TRIGSEQ.ratchet`/`.chaos`.

O que **não está feito** — candidatos reais:

| Candidato | Família | Ideia | Parte de (a estudar — conceito, não código) |
|---|---|---|---|
| ~~**cores de ruído extras** (azul, violeta, bit)~~ | SOURCE | **FEITO (2026-09-05).** `NOISE` ganhou 3 saídas simultâneas a mais: `blue` (branco diferenciado 1×), `violet` (branco diferenciado 2× — testar "azul − rosa" como a NAVALHA fazia primeiro não deu um violeta confiavelmente mais agudo com o filtro de rosa de 7 polos deste projeto; dupla diferenciação funciona por construção), `bit` (1 bit bipolar do gerador a taxa de áudio). Painel alargado de 12 pra 20 HP pra caber os 8 jacks de saída. | `ANTITOTEM/src/core/NoiseFields.h::NoisePalette` (paleta de 6 cores; teoria de ruído colorido é domínio público — diferenciação/integração de ruído branco) |
| ~~**oscilador PLL / soft-sync**~~ | SOURCE | **FEITO (2026-09-05) — `PLL`, Módulo 37** (não um modo dentro do `OSC` — módulo dedicado, a pedido do autor: "faz o b, porém será um oscilador sofisticado, com itens que o primeiro não contém ainda"). `dossies/37_pll.md`, `src/dsp/Pll.hpp`. Detector de fase compara contra uma referência externa e CURVA a própria taxa em vez de resetar duro; toca livre sem referência (é um segundo VCO de verdade). `ratio` generaliza pra além de 1:1 (desvio Rasgo), `lock_gain` exposto, alcance de captura limitado (±0,9) medido e documentado — mesma limitação de um PLL analógico real. | `ANTITOTEM/src/core/CmosVoice.h` — OSC5, estudo de 4046 PLL/VCO (detector de fase + ganho de malha, `pllLockGain`) |
| ~~**campo caótico (double-well)**~~ | DECISION | **FEITO (2026-09-05) — `CHAOS`, Módulo 36.** `dossies/36_chaos.md`, `src/dsp/Chaos.hpp`. Dois integradores perseguem uma força restauradora não-linear (`x − x³`) com dois poços estáveis; `drive`/`damping` decidem se assenta, oscila ou "caça"; o chute periódico aleatório é o que deixa o sistema atravessar de um poço pro outro (confirmado por teste: sem ele fica preso). `rate` de CV lenta a textura de áudio, `freeze`, `reseed`. | `ANTITOTEM/src/core/ChaosSources.h::ChaosField` — estudo de caos de poço duplo (Ian Fritz, 2007, `PESQUISA_CMOS_LUNETTA.md` do próprio Antitotem — não é port de circuito, é estudo digital original do autor) |
| ~~**feel rítmico não-binário**~~ | TIME | **FEITO (2026-09-05).** `CLOCK` ganhou `feel` (reto/tercina/quintina/septina/nonina/undecina/glitch) — multiplica `MULT` por 1/3/5/7/9/11 (quantizado e nomeado; `mult` continua controlando velocidade fina por cima); `glitch` sorteia um fator de tempo por passo (~0,7–1,4×) em vez de multiplicar por um número fixo. `swing` do Antitotem ficou de fora — o `CLOCK` já tem um `swing` contínuo próprio, duplicar como posição discreta só confundiria. | `ANTITOTEM/src/core/SimpleSequencer.h::ClockFeel` (enum) |
| ~~**proximidade por oscilador**~~ (metade — órbita ficou de fora) | SOURCE (depth) | **FEITO (2026-09-05).** `OSC` ganhou `prox` — mistura as 5 saídas com uma versão passada por passa-baixa de 1 polo bem suave de si mesmas (0,06), `prox=0` é exatamente a saída crua (no-op determinístico, testado). **`órbita` (LFO de pitch autônomo) ficou de fora de propósito**: o `OSC` já tem `drift` fazendo exatamente esse papel (passeio lento correlacionado na afinação) — adicionar `órbita` seria duplicar, não contribuir. | `ANTITOTEM/src/core/CmosVoice.h` — `oscillatorProximity`/`oscillatorOrbit` (Y/Z, comentário explícito: "no-op em 0", "não sincronizado entre osciladores") |
| ~~**rede de feedback com tipo selecionável**~~ | SOURCE (depth) | **FEITO (2026-09-05) — dentro do `PLL`, Módulo 37**, não do `OSC` (evitou o redesenho do `fm_amount` existente que tinha adiado isso antes). `feedback_type` (6 posições: direto/retificado/capacitivo/pulso/"transistor"/refluxo) modula a FASE antes de ler a forma. | `ANTITOTEM/src/core/CmosVoice.h::feedbackSample()` / `FeedbackSignal` (enum) |

**Atualização (2026-09-05): os 6 candidatos desta lista estão feitos.**
Ordem em que foram implementados, do mais simples ao mais arriscado:
cores de ruído extras → feel rítmico não-binário → campo caótico como
módulo novo → proximidade por oscilador (metade — órbita ficou de fora
por decisão, não por dificuldade) → PLL/soft-sync + rede de feedback
selecionável (os dois juntos, dentro do `PLL`, Módulo 37 — um segundo
oscilador dedicado e sofisticado, não modos bolt-on no `OSC`). As três
decisões de arquitetura que ficaram de fora desta lista de candidatos de
módulo — `CROSS` (`PatchGenetics.hpp`, alinhamento por tipo), o
contrato `NOTE` do `MUSICAL SCORE` (`NOTE-OUT`, Módulo 38, adaptador
observador sem mudar interface de nenhum outro módulo) e a gramática
explícita do `Seed` (§1.1 do `ESTUDO_seed_composicao_generativa.md`,
`apps/panel/SeedGrammar.hpp`) — **também estão feitas, desde
2026-09-05.**

### 2.4 Próxima leva — roadmap de continuidade (2026-09-06)

Cruzando os 38 módulos feitos com esta pesquisa (§2.2 aberto, §6 Polivoks,
§7 Top-100 ★, §4 Aquorbium, §5 técnicas): o que **ainda não foi
contemplado**, em ondas do mais seguro/completador ao mais arriscado/novo.
Cada um vira dossiê antes do código (método §1).

**Onda A — fecha lacunas óbvias do rack (baixo risco): COMPLETA (2026-09-06).**

| # | Módulo | Família | Ideia | Parte de (conceito público) |
|---|---|---|---|---|
| ~~39~~ | ~~**`GLIDE`**~~ — portamento por nota | TRANSFORM / PITCH | **FEITO — 2026-09-06, `dossies/39_glide.md`, `src/dsp/Glide.hpp`.** 3 modos (sempre / slide-gated 303 / legato), `time` de subida + `fall` (assimetria descida = `time·6^fall`), `curve` linear↔RC, saídas `moving`/`done`. | TB-303 slide, portamento MS-20/Minimoog (rate const.) vs RC, Bela Gliss / EMW glide processor |
| ~~40~~ | ~~**`WAVETABLE`**~~ — oscilador de tabela | SOURCE | **FEITO — 2026-09-06, `dossies/40_wavetable.md`, `src/dsp/Wavetable.hpp`.** 16 quadros procedurais (serra→quadrada→formante→seno, SEM arquivo — opção A), 10 mip-maps band-limited; `warp` = distorção de fase CZ / WAVE CUT; **captura de ciclo ao vivo** (`capture`+`grab`) — tabela do que o `AUDIO-IN` ouve. `drift` = varredura autônoma. | tutorial JUCE, WolfSound; **EMW WAVE-6** (hardware do autor); Casio CZ phase distortion; série de Fourier band-limited |
| ~~41~~ | ~~**`LOOPER`**~~ — delay com HOLD / REVERSE / tape | SPACE | **FEITO — 2026-09-06, `dossies/41_looper.md`, `src/dsp/Looper.hpp`.** Delay de linha (buffer ~2,2 s) com `hold` (ancora a janela e repete infinito, sem realimentação nova), `reverse` (2 grãos Hann em crossfade — sem clique) e caráter de fita/BBD num knob `age` (passa-baixa no laço + wow&flutter ~0,9/6,5 Hz + tanh + chiado semeado). Gates `freeze`/`rev`. Distinto do `SPACE` (reverb) e do `MEMORY` (granular). | §6 (tape/digital delay com hold e reverse; BBD/flanger); 4ms DLD / Tapographic; Make Noise Mimeophon |
| — | **`heads`** no `LOOPER` (2026-09-06) — eco de fita multi-cabeça (Space Echo) + Frippertronics; o `TAPE` NÃO virou módulo (`ESTUDO_audio_sampling §4`) | SPACE | 1–4 cabeças lendo frações do `time`, somadas; a realimentação regenera todas | Roland RE-201; Frippertronics |
| — | **`peca_generativa_5`** — peça que exercita a Onda A | — | melodia acid (SEQUENCE→QUANTIZER→OSC→FILTER com GLIDE) + LOOPER congelando frases | — |

**Onda B — territórios de síntese novos (risco médio): COMPLETA (2026-09-06).**

| # | Módulo | Família | Ideia | Parte de |
|---|---|---|---|---|
| ~~42~~ | ~~**`ADDITIVE`**~~ — oscilador aditivo / espectral | SOURCE | **FEITO — 2026-09-06, `dossies/42_additive.md`, `src/dsp/Additive.hpp`.** 64 parciais somados (acumuladores de fase + LUT); envelope espectral por 4 knobs: `tilt` (brilho, `k^-e` de 2,6 a 0,15), `odd` (−1..1, ímpar/par), `stretch` (−1..1, inarmonicidade `k + s·c·k(k−1)` monotônica), `comb` (pente `cos` sobre o índice). Corte de Nyquist por parcial com fade. `drift` = cintilância **determinística** (senóides incomensuráveis, sem RNG). Seguidor de ganho + `tanh` na saída. | Xaoc Odessa ★★ (§7 #85 — só o conceito dos macros), síntese aditiva clássica (§5), inarmonicidade de cordas/barras (acústica) |
| ~~43~~ | ~~**`PLANAR`**~~ — morph vetorial XY | MIX (morph) | **FEITO — 2026-09-06, `dossies/43_planar.md`, `src/dsp/Planar.hpp`.** 4 fontes nos cantos de um quadrado, ponto `x`/`y` interpola (bilinear); `curve` linear ↔ potência constante; `smooth` (glide no ponto); GESTO gravável no gate `gesture` (grava enquanto alto, toca em loop na descida) + saídas `x_out`/`y_out` de CV (o gesto dirige o resto do patch — "a relação é o processo"); `drift` = passeio 2D determinístico (Lissajous de 3 senos incomensuráveis). | Intellijel Planar 2 ★ (§7 #33 — só o conjunto de gestos), Buchla 208 joystick, síntese vetorial Prophet VS / Wavestation (domínio público) |
| ~~44~~ | ~~**`OPERATOR`**~~ — voz FM multi-operador | SOURCE | **FEITO — 2026-09-06, `dossies/44_operator.md`, `src/dsp/Operator.hpp`.** 4 operadores (senóides), 8 algoritmos (série → aditivo), razões `ratio_b/c/d` quantizadas à tabela musical `{0,5;1;1,5;2;2,5;3;4;5;7;9}`, `index` global (Bessel/Chowning), `feedback` no operador A (auto-FM → serra, média de 2 amostras à la DX7). Ordem A→B→C→D fixa (nenhum algoritmo tem laço). `drift` = desafino determinístico por operador. Sem EGs por op (fica pra `OPERATOR+`). | John Chowning 1973 (teoria, domínio público); DX7/DX21/TX81Z (4-op + 8 algoritmos — conceito, patente expirada); Akemie's Castle / YM2151 ★ (§7 #88); Bastl Pizza (§7 #98) |
| ~~45~~ | ~~**`FORMANT`**~~ — ressoador espectral multibanda | TRANSFORM | **FEITO — 2026-09-06, `dossies/45_formant.md`, `src/dsp/Formant.hpp`.** 5 passa-faixas em PARALELO (SVF TPT, não-linearidade no laço); `vowel` (0–1, +CV) varre a sequência A→E→I→O→U (5 formantes/vogal — freq log, ganho dB, banda linear; dados fonéticos de voz de baixo, `constexpr`); `shift` = escala todas as frequências (2^(shift·1,5), trato vocal); `res` = estreita as bandas (canta/apita); `mix` seco↔ressoado; `drift` = wobble determinístico por formante. Sem entrada → silêncio (TRANSFORM). | Fant 1960 (teoria fonte-filtro, pública); tabelas de formante Csound `fof`/`fmnt` (fato fonético); Frap Fumana ★ (§7 #19); Random*Source Serge ResEQ ★ (§7 #51); 4ms SMR |

**Onda C — espaço / caráter (risco médio): COMPLETA (2026-09-06).**

| # | Módulo | Família | Ideia | Parte de |
|---|---|---|---|---|
| ~~46~~ | ~~**`HALL`** — reverb FDN~~ | SPACE | **FEITO — 2026-09-06, `dossies/46_hall.md`, `src/dsp/Hall.hpp`.** Módulo NOVO (não modo do `SPACE` — topologia diferente, `SPACE` já entregue). 8 linhas de atraso + **matriz de Householder** (`y = x − (2/N)Σx`, ortogonal → estável pra g≤1). `size` escala as linhas (0,3×–1,7×), `decay` = RT60 `0,2·75^decay` (g por linha), `damp` = passa-baixa de 1 polo no laço, `mod` = modulação determinística das linhas (chorus, quebra o ringing), `pre` = pré-atraso, `mix`. Gate `freeze` → g=1 (cauda infinita, *lossless*) + entrada→0. Saídas estéreo `l`/`r` descorrelacionadas. | Jot & Chaigne 1991 (FDN — teoria pública); Householder/Hadamard (DSP clássico); Dattorro 1997 (damping no laço); NE Desmodus Versio ★ (§7 #72), Strymon StarLab ★ (§7 #95) |
| ~~47~~ | ~~**`DRUM`**~~ — voz de percussão | SOURCE | **FEITO — 2026-09-06, `dossies/47_drum.md`, `src/dsp/Drum.hpp`.** Um gate → um golpe. 3 camadas: CORPO (senóide com envelope de altura — o pitch-sweep do 808; `map` mistura com `tanh` → clique do 909), ESTALO (ruído por passa-alta cujo corte sobe com `map`, envelope curto — `snap`), ENVELOPE de amplitude (`decay` ~20 ms–2 s). `tone` (+CV v/oct), `bend`, `drive` (saturação), `roll` (auto-disparo ~2–40 Hz = modo autônomo), `drift` (humanização por golpe — xorshift semeado NO disparo, determinístico). `accent` CV. | TR-808/909 (topologia da voz de bumbo — circuitos DIY públicos); Chowning/Roads (percussão sintética); vpme QD ★ (§7 #24); §2.2 |

**Onda D — grande / opt-in / arriscado:**

| # | Módulo | Família | Ideia | Parte de |
|---|---|---|---|---|
| ~~48~~ | ~~**`SAMPLER`**~~ | SPACE | **FEITO — 2026-09-06, `dossies/48_sampler.md`, `src/dsp/Sampler.hpp`.** Toca-fatias: `trig` → um golpe de um trecho gravado (gate `rec`) ou de arquivo (painel via `setBuffer()` — `dr_wav` na camada `io/`, fora do core). `speed` varispeed bipolar (±0,25×–±4×, negativo = reverso), `slices` 1–16 + `pos` (CV), `repitch` (0 varispeed / 1 pitch-shifter), `wear` (desgaste determinístico por disparo — jitter + hold + bit-crush), `loop`. De-click adaptativo. **Porte do `NAVALHA2_JUCE`** (`SlicePlayer` + `HeritagePitch` = `dsp/PitchShift.hpp` = `G09.pitchshift.pd`) — GPL-3.0-or-later, crédito Glerm Soares + Lúcio Araújo (`RASGO_MODULAR.md §29.1`). | **`dossies/ESTUDO_audio_sampling.md`** §2 (Navalha 2 prior art) / §3 (`dr_wav`); Akai/E-mu varispeed; MPC chop; Puckette G09 |
| ~~50~~ | ~~**`TURNTABLE`**~~ | SPACE | **FEITO — 2026-09-06, `dossies/50_turntable.md`, `src/dsp/Turntable.hpp`.** O mesmo buffer do `SAMPLER` lido por um **prato com massa**: `readPos` é a integral de uma velocidade angular com inércia (EDO de 1ª ordem). `speed` (alvo ±0,5×–±2×, negativo = reverso), `torque` (força do motor → *wow* de partida), `friction` (coasting no `brake` + retorno pós-scratch), `grab` (firmeza da mão na CV `scratch`), `start`, `wear` (estalos determinísticos), `loop`. `trig` põe a agulha e liga o motor. Acoplamento AC na saída. **Desvio Rasgo** (`ESTUDO §4.2`): o Navalha 2 rejeita a metáfora de DJ; aqui diverge, mas com o modelo físico do prato, SEM quantização de BPM (beatmatch é gesto). `TAPE` NÃO virou módulo — virou `heads` no `LOOPER`. | **`dossies/ESTUDO_audio_sampling.md`** §4.2; Technics SL-1200 (referência funcional); técnica de scratch de DJ; `SAMPLER` (#48) |
| ~~49~~ | ~~**`SIGNAL-IN`**~~ — o `AUDIO-IN` cresceu | SOURCE | **FEITO — 2026-09-06, `dossies/49_signal_in.md`, `src/dsp/SignalIn.hpp`.** Áudio + MIDI num adaptador só (não módulos separados). Anel SPSC de áudio (do `AUDIO-IN`) + anel de MIDI (`pushMidi`); voz MONOFÔNICA last-note (pilha de 16). Saídas: `out`/`r` (áudio) + `pitch` (1 V/oct, nota 60 = 0 V, + pitch-bend × `bend`) + `gate` (rampa 1 ms) + `vel` + `cc` (o CC `cc_num`). `type()` = "SIGNAL-IN"; `makeModule("AUDIO-IN")` = alias de migração; `AudioIn` = `using SignalIn`. Contraparte de entrada do `NOTE-OUT`. Thread ALSA-seq no painel feita (`apps/panel/AlsaMidi.hpp` — porta virtual, `aconnect`); falta só validar ao vivo com teclado. CV bruto DC-coupled fica pra quando houver caso. | `AUDIO-IN` (#35 — anel de áudio); ALSA seq (MIDI); voz mono last-note (Minimoog/MS-20); `NOTE-OUT` (#38, a contraparte) |

**Fora de onda — conceitual, decidir se vira módulo:**
- **`BOXCAR` (#51 — FEITO 2026-09-06, `dossies/51_boxcar.md`, `src/dsp/Boxcar.hpp`)** — *boxcar averager* / integrador de porta (gatilho + delay de abertura + *aperture* + média de N capturas; *scanning* reconstrói a forma de onda). `mode` 0 follower (S&H de janela, com `average` alto = S&H sem tremor) · 1 reconstruct · 2 oscillator (relê o buffer que ele mesmo montou). Auto-trigger por limiar (`thresh`), período medido normaliza os bins (reconstrói em rubato), piso de ruído interno no modo autônomo. Saída `geiger` = trigger de Poisson livre; **o `NOISE` (#19) ganhou o param `poisson`** (mesmo processo, no relógio interno do S&H/smooth). Família **DECISION**, `in`/`out` Audio+Control. Origem: AI Synthesis AI250 BXR → boxcar de bancada (domínio público, SR200 NIM); o firmware do AI250 publica só `.bin`, nome `BOXCAR` sem impedimento (termo genérico de DSP);
- **síntese pulsar** (`BiomaPulsar`, §4) — pode ser modo do `ADDITIVE` ou voz própria;
- **LFO múltiplo livre** (Batumi/ochd, §7 #61/#31) — talvez modo do `FUNCTION` (N saídas defasadas) em vez de módulo;
- **memória de estados / keyframes** (Frames, §3) — é **feature de painel** (navegar entre snapshots do patch), não módulo DSP;
- **plataforma polimórfica** (Ornament & Crime, §6) — ambicioso demais pra agora; o espírito já vive no `SWITCH`/`MATRIX`.

> **Nota (2026-09-06):** o autor **não tem conta no ModularGrid** — a
> triagem do §7/§9 saiu de um fetch analisado na época, não de uma
> wishlist pessoal. Não há entrada externa pendente; a fila abaixo segue
> como está até nova pesquisa dirigida.

### 2.5 Revisão — o que ainda NÃO foi codado (2026-09-07)

Cruzamento dos **51 módulos feitos** contra §2.2/§3/§4/§6/§7/§8. Nada
disto está aprovado — é a revisão da lista pedida pelo autor. Cada um,
se for adiante, vira dossiê antes do código (método §1). Ordenado por
"tapa um buraco real do rack" → "amplia alcance" → "provável modo, não
módulo".

**Tier 1 — buracos reais na paleta de efeitos/utilidade.** O que JÁ
existe: reverb (`SPACE` multitap + `HALL` FDN), delay de linha (`LOOPER`
com hold/reverse/fita/multi-cabeça), eco granular (`MEMORY`),
distorção/fold (`SHAPE`, `WASP`), EQ (`PARAMETRIC`), LFO/função
(`FUNCTION`, 0,01 Hz–12 kHz). O que FALTA: a família de **modulação**
(atrasos CURTOS modulados) e um destruidor lo-fi dedicado.

| Cand. | Família | O que é / por que falta | Parte de (conceito público) |
|---|---|---|---|
| ~~**`SWIRL`** (chorus / flanger / ensemble / **phaser**)~~ | SPACE | **FEITO — 2026-09-07, `dossies/52_swirl.md`, `src/dsp/Swirl.hpp` (Módulo 52).** Os quatro num módulo (`type`): chorus/flanger/ensemble = atrasos CURTOS modulados por LFO triangular; phaser = **6 all-pass de 1ª ordem TPT** (o `PHASER` da linha abaixo entrou aqui). `feedback` (−1..1, `tanh` no laço; flanger auto-oscila passando da unidade), `spread` (LFO de R defasado → estéreo), `tone` (1 polo no molhado), `age` (**desvio Rasgo** — caráter BBD: companding + ruído semeado + wobble; daí a auto-oscilação sem entrada), `mix`. `mix=0` bypass exato. | BBD/bucket-brigade (teoria); Roland Dimension/CE-1, Juno/Solina; phaser Bode/Small Stone (all-pass, Zölzer DAFX); §6 |
| ~~**`STAGES`**~~ (gerador de segmentos configuráveis) | MODULATE | **FEITO — 2026-09-07, `dossies/54_stages.md`, `src/dsp/Stages.hpp` (Módulo 54).** `segments` (2–8), `contour`/`tilt`/`hold`/`curve` esculpem a forma por macros (gerador, não editor); `hold` faz virar envelope (0, rampa) ↔ sequência (1, degrau); `loop` corre ↔ dispara; `jitter` (desvio Rasgo — passeio semeado). Saídas `out`/`eoc`/`step`. | Mutable Stages (MIT — conceito); Rossum Control Forge ★ (§7 #43); Blukač Fractalist ★ (§7 #4) |
| ~~**`CRUSH`**~~ (destruidor lo-fi / decimador) | TRANSFORM | **FEITO — 2026-09-07, `dossies/53_crush.md`, `src/dsp/Crush.hpp` (Módulo 53).** `rate` (S&H sem anti-alias), `bits`, `drive`, `wrap` (clipa↔enrola — overflow de inteiro), `glitch` (travada/dropout/repique), `jitter` (wow digital), `tone`, `mix`. Tudo semeado → dano REPRODUTÍVEL. É onde mora o verbo **DAMAGE**. | decimator/bitcrusher (teoria); Schlappi 100 Grit ★ (§7 #13); Atlas §16/§17 |

**Tier 2 — vozes / geradores que ampliam o alcance:**

| Cand. | Família | O que é / por que | Parte de |
|---|---|---|---|
| ~~**`RESONATOR`**~~ | TRANSFORM | **FEITO — 2026-09-07, `dossies/55_resonator.md`, `src/dsp/Resonator.hpp` (Módulo 55).** Banco de ≤ 24 modos afinados, excitado de fora; `structure` harmônico↔esticado, `tilt` cruza as saídas `low`/`high` (Three Sisters), `strike` = exciter embutido, modo autônomo por ruído interno. Distinto do `MATTER` (voz fechada). | Mutable Rings/Elements (MIT — conceito, §3); Mannequins Three Sisters ★★ (§7 #50); `BiomaModalResonator` (§4) |
| ~~**`PULSAR`**~~ | SOURCE | **FEITO — 2026-09-07, `dossies/56_pulsar.md`, `src/dsp/Pulsar.hpp` (Módulo 56).** Trem de *pulsarets* + silêncio; `freq` = altura (taxa de repetição), `formant` = timbre (freq interna do pulsaret), **independentes** — o pente harmônico fica preso a `freq`, `formant` só move o envelope espectral. `shape`, `window` (Tukey→Hann→expodec), `jitter`/`mask` (semeados; *masking* de Roads), `spread` (estéreo por granulação). Nem `ADDITIVE`/`OPERATOR`/`MEMORY` fazem isso. | Curtis Roads, *Microsound* (teoria pública); `BiomaPulsar` (§4) |
| **`SWARM`** (multi-LFO orgânico) | MODULATE | N (4–8) LFOs com relação de fase e uma "dispersão orgânica" — do quad travado (Batumi) ao cardume que deriva junto mas nunca idêntico (ochd). O `DRIFT` é escala de MINUTOS; o `FUNCTION` é um. Isto é sub-áudio, várias saídas, para animar um patch inteiro. **Forte candidato a `mode` do `FUNCTION`** (N saídas defasadas) em vez de módulo — decisão do autor. | DivKid ochd ★ (§7 #31); Xaoc Batumi ★ (§7 #61); IME Kermit ★ (§7 #70) |

**Tier 3 — provavelmente MODO, não módulo (o `feedback_generative_design_light_touch` pede modo antes de módulo):**

- **sequenciador melódico euclidiano** — Bjorklund + acento por AND/OR de divisores (vpme Euclidean Circles ★ §7 #46) → `mode` do `TRIGSEQ` (saída de pitch) ou do `SEQUENCE`.
- **melodia generativa** (contorno/densidade/registro em vez de passos desenhados — Bard Quartet ★ §7 #38, meloDICER ★ §7 #78) → `mode` do `SEQUENCE` ou do `DECISION` + `QUANTIZER`.
- **envelope quádruplo** (Klavis Quadigy ★ §7 #74) — utilitário; baixa prioridade.
- **delay de pente rítmico** (Rainmaker ★ §7 #42) — 16 taps afinados; nichado, avaliar depois.
- **oscilador caótico como voz** (Orbit 3 ★ §7 #60, Clank Chaos ★★ §7 #82) — o `CHAOS` já vai a 400 Hz; um `mode` "voz" se faltar corpo.
- **striker granular** (rajada de grãos no disparo — `BiomaGranularStriker` §4) → `mode` do `MEMORY`.

**Parados de propósito (não viram módulo):**

- **memória de estados / keyframes** (Frames, §3) — feature de painel (navegar snapshots do patch), não DSP.
- **plataforma polimórfica** (Ornament & Crime, §6) — amplo demais; o espírito vive no `SWITCH`/`MATRIX`/`STAGES`.
- **spatializer / multitap de posição** (`AquariumSpatializer` §4) — a família SPACE (`SPACE`+`HALL`+`LOOPER`+`MEMORY`) já cobre; reavaliar só se o autor pedir imagem estéreo posicional.
- **CV bruto DC-coupled in/out** (Expert Sleepers) — `SIGNAL-IN` já traz MIDI+áudio; fica "pra quando houver caso" (§2.2).
- **`OrganismVoiceEngine` / `Ecosystem`** (§4) — arquitetura (despacho tipado; a cicatriz do `Cable`), não módulo de catálogo.

**Recomendação de ordem, se o autor quiser uma "Onda E":**
`SWIRL` → `CRUSH` → `PHASER` (ou dobrar no `SWIRL`) → `STAGES` → `RESONATOR` → `PULSAR`. **Onda E COMPLETA (2026-09-07): `SWIRL`, `CRUSH`, `STAGES`, `RESONATOR`, `PULSAR` — o `PHASER` entrou no `SWIRL`.** `STAGES` é o mais rico conceitualmente; `RESONATOR`/`PULSAR` ampliam as vozes.

---

### 2.6 Revisão cruzada com o ranking de POPULARIDADE do ModularGrid (2026-09-07)

O `§7` é o Top 100 por **avaliação** (satisfação, consultado 2026-09-01).
Esta revisão cruza os **56 módulos feitos** contra o Top 100 por
**popularidade** — quantos racks contêm o módulo (o *module finder*
ordenado por `order=popular`; a lista é conhecida e muito estável entre
anos). O objetivo é achar buracos que só aparecem quando se olha o que a
comunidade de fato usa, não só o que avalia bem.

**Conclusão do cruzamento:** ~85 % do Top 100 por popularidade já tem
casa no RASGO. O ranking é dominado por **utilidades** (VCA, atenuversor,
mult, mixer, mudança de oitava, S&H — ~40 %) e **vozes/filtros de
trabalho** (VCO analógico, VCF, ADSR — ~30 %); as duas faixas o RASGO
cobre inteiras (`VCA`, `CONTROL`, `ABACUS`, `MULT`, `MATRIX`, `SWITCH`,
`MIXER`, `SH`, `LOGIC` · `OSC`, `FILTER`, `ENVELOPE`, `FUNCTION`, `LPG`).
O terço de "caráter" (granular, físico, caos, efeito digital) está quase
todo coberto (`MEMORY`, `MATTER`, `STRING`, `RESONATOR`, `CHAOS`,
`SWIRL`, `CRUSH`, `HALL`, `SPACE`, `LOOPER`, `SAMPLER`, `TURNTABLE`).

**O que o cruzamento expôs de novo (não estava no `§2.5`):** a lacuna
**análise → síntese**. O RASGO tem `ADDITIVE`/`FORMANT` (constroem
espectro) e `SCOPE` (mede áudio → CV), mas nada que **ouve um sinal e
o re-sintetiza**. Nada digno de nota está approvado — cada linha, se for
adiante, vira dossiê antes do código (método `§1`). Nomes provisórios.

**Tier 1 — buracos reais (dossiê próprio).**

| Cand. | Família | O que é / por que falta | Parte de (conceito público) |
|---|---|---|---|
| ~~**`SHIFTER`** (deslocador de frequência)~~ | TRANSFORM | **FEITO — 2026-09-07, `dossies/58_shifter.md`, `src/dsp/Shifter.hpp` (Módulo 58).** Virou MÓDULO próprio (não `mode` do `SHAPE` — o conceito é distinto do waveshaper). Move o espectro por Δf fixo em Hz (SSB por Hilbert FIR de 255 taps + atraso casado). Saídas `up`/`down` simultâneas; `feedback` = barber pole (a saída `up` volta pra entrada, `tanh` no laço); `drift` semeado; `tone`; `mix`. Rejeição de imagem > 50 dB acima de ~250 Hz. Distinto do ring-mod do `SHAPE` (bandas simétricas). | Harald Bode / Bode-Moog frequency shifter (SSB — teoria pública, anos 1960); Hartley/Weaver SSB; transformada de Hilbert (FIR); Zölzer *DAFX*; barber-pole / Shepard-Risset (Shepard 1964, Risset 1969) |
| ~~**`SPECTRA`** (resíntese espectral)~~ | SOURCE | **FEITO — 2026-09-07, `dossies/57_spectra.md`, `src/dsp/Spectra.hpp` (Módulo 57).** Banco de 64 passa-faixas ressonantes log (35 Hz–14 kHz) + seguidor de pico → a cada ~6 ms pega os `voices` picos, interpolação parabólica; `voices` (2–24) senóides de fase contínua que deslizam pros picos herdados. `blur` (velocidade do rastreio), `shift`/`stretch` transpõem a re-síntese, `tone`, `jitter` semeado, **`freeze`** (para a análise = *spectral freeze* / pad infinito), `mix`. `in` livre → ruído + 2 parciais fantasma semeados → drone autônomo. `process()` não aloca. Pendências: FFT real, *partial tracking* com continuidade. | phase vocoder (Flanagan & Golden 1966); SMS (Xavier Serra 1989); McAulay–Quatieri (1986); *spectral freeze* (técnica pública); Q constante (Brown 1991); Panharmonium / Rainmaker spectral (**conceito, não código**) |

**Tier 2 — `mode` de um módulo existente (o `feedback_generative_design_light_touch` pede modo antes de módulo).**

| Cand. | Onde | O que é / por que | Parte de (conceito público) |
|---|---|---|---|
| ~~**vocoder de análise/síntese**~~ | `mode` do `FORMANT` | **FEITO — 2026-09-07, `mode` do `FORMANT` (Módulo 45).** Entrada `mod` (índice 3, apensa — patches existentes intactos) + param `vocoder` (0–1). `vocoder>0` + `mod` cabeado → +5 SVF de análise no modulador + 5 seguidores de envelope (~12 ms) → os ganhos das 5 bandas de formante seguem a energia do modulador em cada frequência, em vez da tabela de vogal. Vocoder de 5 bandas (grosso mas vocálico); `vowel` escolhe QUAIS 5 frequências vocodar. `vocoder=0` ou sem `mod` → FORMANT byte-idêntico. 4 testes novos em `test_formant.cpp`. | Homer Dudley, *The Vocoder* (Bell Labs, 1938 — domínio público); banco de análise + seguidores RC + banco de síntese (teoria clássica) |
| **análise de áudio → CV: ~~pitch por autocorrelação~~ + onset/transiente** | estende o `SCOPE` | **PITCH FEITO — 2026-09-07: `SCOPE.pitch` trocou o ZCR por autocorrelação YIN** (decimado 3×, janela 320, lags 16–300, a cada ~12 ms) — robusto a harmônicos (serra/quadrada/acorde: erro < 1 %; ruído → 0). Falta ainda uma saída `onset` (pulso no ataque — fluxo espectral / derivada da envoltória) pra fechar o "seguidor de áudio" tipo Maths ch2/3. | YIN (de Cheveigné & Kawahara, 2002 — artigo público); detecção de onset por fluxo espectral (Bello et al., 2005 — *tutorial* público) |

**Tier 3 — utilidade que falta, cerimônia mínima.**

| Cand. | Onde | O que é / por que | Parte de |
|---|---|---|---|
| ~~**banco de VCA de 4 canais** (`VCA4`)~~ | MÓDULO próprio | **FEITO — 2026-09-07, `dossies/59_vca4.md`, `src/dsp/Vca4.hpp` (Módulo 59).** 4 canais (`levelN` + `cvN_amt` atenuvertido), `curve` linear/exp **compartilhado**, `mix_gain` (0–2 — a soma dos 4 com teto suave), `drift` semeado nos 4 ganhos. Entradas `in1..4`/`cv1..4`, saídas `out1..4` + `mix`. Mesmo núcleo do `VCA` (ganho suavizado + `softSat`). Módulo próprio — o `VCA` #20 continua duplo. 8 testes. | Doepfer A-131/132 (VCA exp/linear — teoria pública); Mutable Veils (curva — ficha pública); Quad VCA como mixer (Intellijel/4ms) |

**O ranking de popularidade RE-CONFIRMA candidatos do `§2.5`:**

- **`SWARM`** (multi-LFO orgânico) — Xaoc Batumi (`§7 #61`), DivKid ochd
  (`§7 #31`), Eowave Quadrantid Swarm e Frap Tools 333 estão todos alto
  na popularidade. `§2.5` Tier 2, forte candidato a `mode` do `FUNCTION`.
- **sequenciador euclidiano melódico** — Pamela's, vpme Euclidean Circles.
  `§2.5` Tier 3 → `mode` do `TRIGSEQ`/`SEQUENCE`.
- **melodia generativa por contorno/densidade** — Vermona meloDICER
  (`§7 #78`), Shakmat Bard Quartet (`§7 #38`), Mimetic Digitalis. `§2.5`
  Tier 3.

**Parados de propósito (o cruzamento não muda a decisão do `§2.5`):**
canivete polimórfico (Disting / O&C — vive em `SWITCH`/`MATRIX`/`STAGES`);
spatializer posicional (a família SPACE cobre); performance mixer com
sends/mutes/cue (fora do escopo de instrumento de composição por ora).

**Ordem sugerida, se virar "Onda F":** `SPECTRA` → `SHIFTER` (ou `mode`
do `SHAPE`) → vocoder (`mode` do `FORMANT`) → VCA 4ch. `SPECTRA` é o mais
rico e o único genuinamente novo no catálogo. **Onda F COMPLETA
(2026-09-07): `SPECTRA` ✓ (#57) · `SHIFTER` ✓ (#58, módulo próprio) ·
vocoder ✓ (`mode` do `FORMANT` #45) · `VCA4` ✓ (#59, módulo próprio).**

---

### 2.7 Nota — a fase didática (site + PDF)

Depois de fechada a construção de módulos, começa a **etapa didática**:
como usar cada módulo (portas, controles, o que cada knob faz de
verdade), como cabear, e — como a tabela "módulo popular → equivalente
RASGO" acima — **como chegar a resultados interessantes conectando um
conjunto de módulos**, com exemplos passo a passo. Material para o site
que será criado e para uma publicação (PDF).

Insumos que já existem para isso:
- o `LEARN` do painel (`apps/panel/LearnCatalog.hpp`) — 3 níveis por
  bind (*rápido* / *entender* / *explorar*) + a definição de cada
  módulo; é a semente do texto de referência;
- os dossiês (`dossies/NN_*.md`) — problema, fontes, modelo, testes de
  cada módulo;
- as 5 peças de exemplo (`examples/peca_generativa*`) — patches completos
  comentados;
- este `PESQUISA_MODULOS.md` — a proveniência conceitual (de que a
  publicação precisa para citar fontes corretamente).

A tabela de equivalências ModularGrid → RASGO (feita nesta revisão) entra
como apêndice: "se você conhece o módulo X do Eurorack, no RASGO é o Y".

---

## 3. Matriz Mutable Instruments → Rasgo (Atlas §36)

Licença: geração **STM32F** (Plaits, Marbles, Rings, Stages, Tides, Warps,
Clouds) = **MIT**; geração **AVR** (Braids, Branches, Grids, Frames, Peaks)
= **GPL-3.0**; hardware = CC BY-SA 3.0. Repo: `pichenettes/eurorack` (não
está local).

| Módulo | Conceito | Desvio Rasgo |
|---|---|---|
| **Clouds** | granular, buffer, freeze, textura | cicatriz sonora - a conexão continua soando depois de rompida (`Cable`) |
| **Marbles** | aleatoriedade estruturada, probabilidade, correlação, déjà-vu | cabo probabilístico - decide se transmite, desvia ou rompe |
| **Warps** | cross-mod, waveshaping, **a relação é o processo** | comportamento da conexão, não módulo |
| **Rings / Elements** | ressonador, excitação → matéria | cabo/objeto com rigidez, tensão, densidade, desgaste |
| **Plaits** | vários modelos de síntese, morfável | identidade mutável - o módulo muda de função na performance |
| **Tides** | função contínua deformável (env/LFO/osc) | vida útil de uma conexão; temporalidade orgânica |
| **Frames** | keyframes + interpolação de **estados** | memória do patch; navegar entre estados complexos, não automatizar knobs |
| **Stages** | segmentos reconfiguráveis, **função emergente** | fragmento que vira envelope/seq/osc/ruído conforme quem conecta |
| **Grids** | mapa rítmico + probabilidade (ritmo como campo) | ritmo emergente sem sequenciador linear |
| **Branches** | Bernoulli gate | decisão probabilística, bifurcação, risco |
| **Streams** | dinâmica dependente do sinal | conexão que reage ao que transporta (fraco→estável, extremo→ruptura) |

---

## 4. Inventário DSP do Aquorbium (AGPL-3.0, código do autor)

Estudar o algoritmo; reescrever na arquitetura do Rasgo Modular. Não
importar (copyleft + licença do Modular indefinida).

| Classe | O que é | Candidato a família |
|---|---|---|
| `BiomaModalResonator` | ressonador modal | MATTER |
| `BiomaPulsar` | síntese pulsar | SOURCE |
| `BiomaPhysicalString` | corda física (Karplus estendido / waveguide) | MATTER / SOURCE |
| `BiomaGranularStriker` | striker granular + granular cloud | SOURCE / DAMAGE |
| `MultiTapSpace` / `AquariumSpatializer` | espaço multitap, reflexão, campo | SPACE |
| `OrganismVoiceEngine` | despacho tipado entre famílias de síntese | ROUTE / arquitetura |
| `Ecosystem` | presença, estratégia, integridade — ecologia e cicatriz reais | MEMORY / DAMAGE / REPAIR |
| `GeometricSequencer` | Bjorklund/euclidiano, step com acento próprio | SEQUENCE / TIME |
| `Numeric` | saneamento numérico, proteção | UTILITY |
| `FixedMpscQueue` | fila lock-free GUI/MIDI → áudio | infraestrutura |

---

## 5. Técnicas verificadas — `AQUORBIUM/biome.odt` (5/ago/2026)

Pesquisa com fontes primárias conferidas (era pro Aquorbium; as
técnicas servem):

- **Wavetable** (JUCE): lookup table pré-preenchida + interpolação entre
  amostras. Ref: tutorial oficial JUCE `tutorial_wavetable_synth`, WolfSound.
- **Thru-zero FM:** quando o modulador levaria o carrier a frequência
  negativa, deixa o valor ir negativo (subtrai da fase em vez de somar) -
  não perde metade da modulação como o FM comum. Ref: navs.modular.lab,
  Learning Modular.
- **Wavefolder:** `if x>1: x=2-x` funciona mas alia forte; produção usa
  `sin(gain·x)` (sine shaping, método Serge) + oversampling ou polyBLAMP.
  Ref: **paper DAFx23** "Antialiasing Piecewise Polynomial Waveshapers".
- **Ritmo euclidiano:** algoritmo de **Bjorklund**. Ref: Wikipedia,
  gist `unohee/d4f32b3222b42de84a5f` (já há implementação no Aquorbium).
- **Lorenz como modulador:** equações de Lorenz (σ, ρ, β, dT) geram CVs
  caóticas mas suaves. Ref: Cherry Audio Lorenz Attractor, "Chaotic Sound
  Synthesis".
- **Granular** (JUCE): projeto GRNLR (`github.com/passivist/GRNLR`) com
  tutorial passo a passo.
- **Mastering em tempo real:** limiter com lookahead (SimpleCompressor de
  Daniel Rudrich), compressão multibanda com crossover Linkwitz-Riley
  (`marcossrivas/Multiband_Crossover`), medição LUFS ITU-R BS.1770
  (K-weighting → quadrado → gate → integração).

---

## 6. Linha Polivoks / Erica Synths — lacunas do Rasgo Synth (3-5/ago/2026)

Análise numa sessão do Rasgo Synth (transcrição `f01ec51f`). Genuinamente
ausentes no motor do Synth, candidatos reais - alguns também interessam ao
Modular:

- **VCO com hard sync** (um oscilador reinicia a fase forçado por outro) -
  técnica distinta de cross-FM;
- **Bassline acid (TB-303):** filtro ressonante + envelope de acento +
  **slide/portamento** entre notas consecutivas;
- **BBD delay/flanger** (banda limitada, companding, degradação
  característica) - diferente de delay genérico;
- **Tape/digital delay com hold** (congelar/repetir infinito) **e reverse**
  como modos reais;
- **Ornament & Crime (O_C):** plataforma polimórfica (gerador de CV,
  quantizador, LFOs complexos) - código aberto e hackeável.

---

## 7. ModularGrid — Top 100 por avaliação (consultado 2026-09-01)

**A triagem conceitual completa deste Top 100 está em
`AQUORBIUM/aquorbium-arquitetura.md §9`** - módulo a módulo, com ficha
técnica verificada. Módulos que a triagem destacou por princípio (não
utilidade pura), relevantes ao Rasgo Modular:

- **Turing Machine Mk II** - registrador de deslocamento com realimentação;
  trimmer "Lock" = chance de mutação de cada bit por ciclo. → `Cable`
  probabilístico / SEQUENCE.
- **Frap Tools SAPÈL** - random domada, **distribuição de probabilidade
  configurável** (uniforme → gaussiana) por potenciômetro. → DECISION.
- **mylar melodies RANDOM8 / Bastl Déjà Vu** - máquina de estado da CV
  aleatória: aleatório → looping → evolving → travado. → DECISION/MEMORY.
- **Blukač Fractalist** - um `FractalCore` único em taxa de controle E de
  áudio (mesma lógica, escalas de tempo diferentes). → princípio Stages.
- **Rossum Control Forge** - gerador de função programável, endereçamento
  duplo (alcançar um ponto do estado sem quebrar sistema fechado). → TIME.
- **Make Noise Maths / Mannequins Just Friends / Frap Tools Falistri** -
  gerador de função (env/LFO/osc). → módulo 1 (fonte/rampa).
- **Mannequins Three Sisters** - filtro onde a **relação entre as saídas**
  é o processo (LOW/CENTER/HIGH que se cruzam). → módulo 2 (filtro).
- **Xaoc Belgrad / Mutable Blades / Rossum Evolution / Vult Freak** -
  filtro como "algo além de filtro". → módulo 2.
- **Befaco Noise Plethora / Nonlinearcircuits Triple Sloths** - ruído e
  caos lento como material. → DAMAGE / modulação.
- **vpme Euclidean Circles / QD** - euclidiano (Bjorklund) + AND/OR entre
  divisores pra acento polirrítmico de graça. → módulo 5 (clock).
- **Instruō arbhar** - granular com captura por onset do próprio mix
  (autoescuta vira material). → MEMORY / a cicatriz.
- **Buchla 281t/258t, Odessa (aditivo), Akemie's Castle (FM), Plinky
  (plucked)** - o balde "vozes" é o genuinamente diverso; terceira rodada.

Ranking bruto por satisfação (≥30 avaliações), referência cruzada. `★` =
conceitualmente rico pro Rasgo Modular.

```
 1 ALM Pamela's PRO Workout ★(clock/deriva)   2 Joranalogue Step 8
 3 Joranalogue Link 2        4 Xaoc Praga      5 Music Thing Turing Machine Mk II ★(shift-register random)
 6 Tiptop Buchla 281t ★(função)  7 Joranalogue Contour 1 ★(env)   8 Cosmotronic Delta-V
 9 Eowave Quadrantid Swarm  10 Intellijel Metropolix ★(seq)
11 Erica Fusion VCO2       12 Xaoc Belgrad ★(filtro)  13 Schlappi 100 Grit ★(distorção/relação)
14 Bela Gliss              15 Make Noise Maths ★★(função generativa)   16 Happy Nerding 3x MIA
17 Frap Tools Falistri ★(função)  18 knob.farm Hyrlo  19 Frap Tools Fumana ★(formante)
20 Intellijel Sealegs      21 ThreeTom MS-22   22 Frap Tools Sapèl ★(random correlacionado)
23 Joranalogue Compare 2   24 vpme QD Quad Drum  25 3x MIA black  26 Joranalogue Select 2
27 ACL Sinfonion ★(harmonia)  28 ALM Pamela's NEW Workout  29 Cosmotronic Messor  30 Intellijel Buff Mult
31 DivKid ochd ★(LFO orgânico)  32 Frap Tools Brenso ★(complex osc)  33 Intellijel Planar 2 ★(vetor/morph)
34 Xaoc Sarajewo           35 Frap Tools 333   36 Mannequins Just Friends ★★(função)
37 WMD MSCL                38 Shakmat Bard Quartet ★(quantizador generativo)  39 Tiptop Buchla 258t ★(complex osc)
40 Quadratt 1U  41 DROID P2B8  42 Intellijel Rainmaker ★(delay/pitch)  43 Rossum Control Forge ★(função programável)
44 Steppy 1U  45 Joranalogue Filter 8 ★  46 vpme Euclidean Circles v2 ★(euclidiano)
47 Instruō arbhar ★(granular)  48 Westlicht PER|FORMER ★(seq)  49 IME Piston Honda MK III ★(wavetable)
50 Mannequins Three Sisters ★★(filtro-relação)
51 Random*Source Serge ResEQ ★  52 Shakmat Knight's Gallop ★(seq generativo)  53 Vult Freak ★(filtro)
54 NANO ONA  55 Just Friends  56 WMD C4RBN ★(filtro)  57 DROID G8  58 Intellijel Bifold ★(wavefolder)
59 AJH Transistor Ladder ★  60 Joranalogue Orbit 3 ★(oscilador/caos)  61 Xaoc Batumi ★(LFO quad)
62 DROID  63 Maths white  64 Hyrlo  65 DROID P10  66 DUAL VCA 1U  67 DROID X7
68 Shakmat Dual Dagger  69 Doepfer A-140-2 ADSR  70 IME Kermit MK III ★(LFO/mod)  71 Schippmann VCF-02 ★
72 Noise Engineering Desmodus Versio ★(reverb)  73 Make Noise 0-Coast ★(west coast)  74 Klavis Quadigy ★(env quad)
75 Shakmat Four Bricks Rook  76 Befaco Noise Plethora ★★(ruído como material)  77 Xaoc Sofia ★(osc)
78 Vermona meloDICER ★(seq randômico)  79 Thonk Plinky ★(plucked/físico)  80 Rabid Elephant Natural Gate ★(LPG)
81 Mutable Ripples (2020) ★(filtro MIT)  82 Clank Chaos ★★(caos)  83 Rossum Evolution ★(filtro ladder variável)
84 Joranalogue Generate 3 ★(complex osc)  85 Xaoc Odessa ★★(aditivo/spectral)  86 Shakmat SumDif
87 Befaco Oneiroi ★(looper/granular)  88 ALM Akemie's Castle ★(FM YM2151)  89 Shakmat Time Wizard ★(clock/probabilidade)
90 Erica Sample Drum  91 Acid Rain Maestro  92 Headphones 1U  93 WMD SSM ★(matriz de switch)
94 Nonlinearcircuits Triple Sloths V2 ★★(caos lento)  95 Strymon StarLab ★(reverb)  96 Befaco A*B+C ★(utilitário)
97 Shakmat Mod Medusa ★(mod)  98 Bastl Pizza ★(FM/PD)  99 Mutable Tides ★★  100 Mutable Marbles ★★
```

---

## 8. Onde pesquisar

**Sites especializados em módulos (links diretos, passados pelo autor):**

- ModularGrid — browser de módulos com filtro por função:
  `https://modulargrid.net/e/modules/browser` (o autor filtrou por
  `SearchFunction` = 70, 29, 16, 42, 30, 35, 37 em 31/jul)
- ModularGrid — Top 100 por avaliação:
  `https://modulargrid.net/e/modules/evaluationlists` (fonte da triagem
  do Aquorbium §9-10)
- **VCV Rack** — `https://vcvrack.com/` · biblioteca:
  `https://library.vcvrack.com/` · código: `https://github.com/VCVRack`
  (`VCVRack/Rack` = host + API; `VCVRack/Fundamental` = módulos base:
  VCO/VCF/VCA/env/seq). Referência de arquitetura (host/DSP/UI,
  convenções Eurorack) e catálogo navegável por função/fabricante.
  **Licenças (auditadas 2026-09-01, detalhe no Atlas §4.2):** código do
  Rack e do Fundamental = **GPLv3-or-later** (livre, copyleft = AMARELO;
  **compatível** com a AGPLv3-or-later que o Rasgo Modular adotou);
  gráficos/painéis/visual = **CC BY-NC / BY-NC-ND** (não-livre =
  VERMELHO, incompatível). **Uso no Rasgo:** estudar arquitetura e
  conceitos e reimplementar (o que já fazemos); copiar código é
  juridicamente possível mas é decisão consciente por arquivo (registrar
  origem/alterações, preservar avisos); **nunca** copiar arte/identidade.
- **patcher.xyz** — browser de módulos, mais moderno, com entradas/saídas
  e patches: `https://patcher.xyz/modules/browser`
- **SchneidersLaden** (Berlim) — catálogo Eurorack 3U, 111 páginas:
  `https://schneidersladen.de/en/eurorack-modular-3u?order=new_release_sorting`
  (páginas 1-15 varridas na pesquisa do Aquorbium §12)
- **Electronic Music Works (EMW)** — fabricante brasileiro, ~80 módulos,
  hardware que o autor usa: `https://www.electronicmusicworks.com/eurorack.html`
- **4ms Company** — `https://4mscompany.com/modules.php` (autor: "tem
  módulos bem interessantes" + "os layouts dos módulos também são
  bacanas" - referência de PAINEL/LAYOUT, não só de DSP; ver
  `RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md §3.1`). Firmware
  aberto em vários; Meta Module (modular-dentro-de-módulo, roda plugins),
  SMR (spectral multiband resonator), Ensemble Oscillator, Tapographic
  Delay, DLD (dual looping delay), PEG (pingable envelope), Stereo
  Triggered Sampler. Exemplo de painel/setup:
  `https://4mscompany.com/Images/Eurorack/setup.jpg` (referência de
  layout - avaliar quando chegar a UI de rack, passo 8 do roadmap).
- **Intellijel** — `https://intellijel.com/` (vários no Top 100:
  Metropolix, Planar 2, Rainmaker, Steppy, Sealegs, Bifold, Quadratt,
  DUAL VCA). Referência de sistema coeso (1U + 3U, case, utilitários) e de
  design de painel/UX; documentação e manuais fortes.
- **Synth Anatomy** — novidades quase diárias:
  `https://synthanatomy.com/category/music-tech-news/eurorack`
- **synths.pw / WEBRACK** — `https://synths.pw/` (autor: "um bom site pra
  vasculhar" + captura de tela como "ideia de layout"). Modular no
  navegador + timeline de DAW embaixo. Observado da captura:
  - duas fileiras de módulos ~12HP, título + × pra fechar;
  - **paleta categorizada** à esquerda (Oscillators / Modulators /
    Filters / Sequencing & Clock / Effects), cada item com "?" de ajuda -
    princípio Hexen, casa com a taxonomia RASGO;
  - **jack acende (anel) quando conectado** - regra "cabo só aparece
    quando explica relação real" (design doc §3.1);
  - displays por módulo (nome do preset, escala do quantizador + teclado);
  - DSP% no topo; SAVE / FORK / EXAMPLES (patch versionável); menu LOOK;
  - **atenuverter em toda entrada de modulação** (profundidade sempre à mão);
  - **timeline embaixo** (faixas, clipes, BPM, grade, REC, Pencil/Erase/
    Snap) - modular + composição linear juntos. Relevante pra "criação de
    música original": gravar/arranjar o que o modular gera.
  - cabos = espaguete Eurorack colorido - o que o autor quer evitar (as
    3 camadas matriz/constelação/semântico resolvem). O RESTO vale trazer.
- **"The Book of Bad Ideas" V2** (Infinitesimal) —
  `https://www.infinitesimal.eu/modules/images/5/5e/The_book_of_bad_ideas_V2.pdf`
  (autor: "várias coisas interessantes nele"). Coleção de ideias de
  módulo DIY/experimentais - "ideias ruins" que rendem. Casa direto com
  o Atlas §11 (ruptura), §16 (erro como material), §17 (degradação).
  **Também anotado no Atlas.**

**Da lista do ChatGPT (`0_BRAINSTORM/links_websites_synths.odt`):**
diretório da **Superbooth** (fabricantes pequenos, protótipos) ·
**Mod Wiggler** (fórum profundo, firmware alternativo, circuitos
obscuros) · **Lines / llllllll.co** (música experimental, generativo,
Monome/Norns/Teletype) · **Thonk** (DIY, open source, Serge, Lunetta) ·
**GitHub tópico Eurorack** (EuroPi, O_C, Raspberry Pi Pico) ·
**Perfect Circuit / Sonicstate** (guias comparativos, demos em feira). A
ODT recomenda a ordem: Synth Anatomy → Superbooth → Lines → Mod Wiggler →
Thonk → GitHub (revela melhor sampling experimental, rearranjo temporal,
probabilidades, performance gestual, morphing, sistemas generativos).

---

## 9. Fontes recuperadas (2026-09-01) — TUDO está no doc do Aquorbium

A pesquisa de módulos que o autor lembrava está em
**`AQUORBIUM/aquorbium-arquitetura.md`**, seções 9 a 12 (commit `560857c`,
9/ago/2026), feita no contexto do Aquorbium porque o RASGO Modular ainda
não existia. Estrutura:

- **§9** — triagem ModularGrid Top 100: ~16 módulos (1ª rodada) + ~20
  (2ª rodada), com ficha técnica verificada e "apropriação original"
  (esta amarrada ao Bioma do Aquorbium).
- **§10** — panorama completo do Top 100 (o que seria usar tudo).
- **§11** — questões em aberto pra próxima rodada.
- **§12** — **pesquisa EMW (Electronic Music Works) — módulos que o autor
  JÁ USOU** (hardware brasileiro, crivo = "já provou valor sonoro na
  mão"): WAVE-6 Polyphonic Osc (wavetable 6 vozes + WAVE CUT tipo PWM),
  POT ACTION RECORDER, RESONANT FILTER SEQUENCER, FIXED FILTER BANK,
  NOISE CINEMA, MIX SEQUENCER, ENVELOPE FOLLOWER, VC WAVETABLE LFO, RING
  MODULATOR, GLIDE PROCESSOR. + varredura SchneidersLaden (páginas 1-15
  de 111): ADDAC Cracklebox / VC Relabi Generator, New Systems Discrete
  Map, Hieroglyphic Plume Pulsar, Emute Labs uSeq, Joranalogue×Hainbach
  Collide 4, Robaux DCSN3, RYK Vector Wave, Tiptop Z-8000, Xaoc Sofia,
  Neuzeit Quasar, Shakmat Harlequin's Context, u-he CVilization, E-RM
  Polygogo.

Outras fontes: `AQUORBIUM/biome.odt` (técnicas, seção 5) ·
`AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md` (processo, seção 1) · sessão
`f01ec51f` (linha Polivoks, seção 6) · `links_websites_synths.odt`
(onde pesquisar, seção 8).

**A fazer:** quando um módulo do Rasgo Modular partir de um item dessas
seções, copiar pra cá só a linha *módulo → conceito/ficha* + o desvio
Rasgo (não as apropriações Aquorbium). ~~Conferir se há rack/wishlist na
conta ModularGrid do autor~~ — **o autor não tem conta no ModularGrid**
(2026-09-06); a triagem do Top-100 saiu de um fetch analisado na época.
