# Estudo à parte — áudio gravado como matéria: sampler, toca-discos, fita

**Estado:** estudo / decisão de arquitetura pendente — **não implementado**
**Pedido do autor (2026-09-02):** *"talvez já possamos inserir audio
library, para trabalhar os sinais desses áudio de alguma forma… samplers,
djs turntable, cassete mechanisms, etc."*
**Revisão (2026-09-06):** levantamento do que o **Navalha 2** já tem de
conceito e código pra esse trabalho (§2). Nenhum arquivo do Navalha 2 foi
tocado — é só leitura de prior art nossa.

Este documento levanta o problema e propõe um caminho. Nenhuma linha de
código ainda — a decisão de puxar uma dependência de leitura de áudio é
grande o suficiente pra merecer um "sim" explícito.

---

## 1. O que muda na identidade do RASGO Modular

A identidade (memória `project_rasgo_modular_identity`) diz: **autônomo
como um sintetizador — soa sozinho, sem entrada.** MIDI, áudio e
acoplamento a instrumento são **nós adaptadores opcionais**, nunca
dependência do motor.

Um `SAMPLER` / `TURNTABLE` / `TAPE` **não fere isso** desde que:

- o `rasgo_modular_core` continue **sem dependência** e fazendo som
  sozinho — o leitor de arquivo vive numa camada `io/` (como o
  `WavWriter.hpp` já vive), não no motor;
- o módulo funcione **sem arquivo**: sem sample carregado, um `SAMPLER`
  emite silêncio ou um buffer interno (ruído/click) — igual o `MEMORY`,
  que já é um buffer granular que **grava a própria entrada**;
- carregar um arquivo seja um **gesto de performance**, não um requisito
  de inicialização (o painel já abre fazendo som).

Ou seja: o `MEMORY` (Módulo 7) **já é 80% de um sampler** — buffer +
leitura granular + `freeze` + `pitch`/`spray`/`feedback`. Falta só a
porta de **carregar áudio de disco no buffer** em vez de só gravar a
entrada ao vivo.

## 2. Prior art nossa — Navalha 2

O **Navalha 2** é um subprojeto irmão (`RASGO/NAVALHA2_PD` — referência
Pure Data/web v0.28.1; `RASGO/NAVALHA2_JUCE` — reescrita JUCE/C++). É um
**instrumento de recorte de material gravado** — exatamente o problema
deste estudo — e está muito mais adiantado no conceito do que as
referências genéricas que este documento citava (Akai, Clouds, Space
Echo). O estudo original **não o mencionava**; esta seção corrige isso.

### 2.1 Licença e crédito

`RASGO/NAVALHA2_PD/docs/LICENSE_STATUS.md`: em julho/2026 **Glerm Soares**
(autor do Navalha original) autorizou o Navalha e a continuação Navalha 2
sob **GPL-3.0-or-later**, preservando a atribuição da autoria original e
identificando as contribuições posteriores (Navalha 2 = reescrita
JUCE/C++, evolução do conceito, por **Lúcio Araújo**) separadamente.

Consequência pra cá: GPL-3.0-**or-later** é compatível com o default da
família RASGO_MODULAR (AGPLv3-or-later — o "or later" permite combinar, e
a AGPLv3 §13 tem compatibilidade explícita com a GPLv3). **Mas há
autoria de terceiro** (Glerm Soares) — então portar código do Navalha 2
exige:

- crédito nominal a Glerm Soares (Navalha original) **e** a Lúcio Araújo
  (Navalha 2) no arquivo portado e num `THIRD_PARTY_NOTICES` / `CREDITS`;
- nota de que o algoritmo de *pitch* vem do exemplo `G09.pitchshift.pd`
  do Pure Data (Miller Puckette) — o próprio Navalha marca isso
  ("RIPPED FROM THE PD HELP FILES");
- registro individual, nunca dissolvido em lista genérica (é a regra do
  próprio Navalha 2, `NOVAS_PERSPECTIVAS.md §2`).

Não é caso de "traçar conceito à origem pública e desviar" (memória
`reference_rasgo_license_convention`) — isso é pra referência
fechada/paga. Aqui o código é livre e em parte do próprio autor: dá pra
**portar de fato**, com atribuição.

### 2.2 O conceito, já articulado

- **`CONCEPT_DECONSTRUCTION.md` / `DUAL_MATERIAL_v0.11.md`**: a tese —
  *"não é software de DJ, não são dois decks; são dois corpos de matéria
  gravada para corte, fragmentação e recombinação"*. Sem jog wheel, sem
  beatmatch, sem sync de deck. É o enquadramento do §1 deste estudo, só
  que já resolvido.
- **Vocabulário de roadmap** (`CONCEPT_DECONSTRUCTION.md`): GAP (silêncio
  como evento de sequenciador), STUTTER (repetição microscópica), BURST
  (sequência rápida de fragmentos diferentes), MICROSLICE, MEMORY
  (células/fragmentos protegidos), MUTATION (transformação controlada),
  EROSION (remoção/substituição progressiva), DECONSTRUCT (macro
  estrutural sobre várias probabilidades).
- **`NOVAS_PERSPECTIVAS.md`**: o Navalha 2 se pensa como "instrumento
  musical, ambiente de performance, laboratório de escuta e objeto de
  pesquisa sobre tempo, recorte, memória e recombinação" — não "sampler
  convencional". Mesma bússola do RASGO Modular.

### 2.3 O modelo de dados de recorte

`NAVALHA2_JUCE/src/core/SessionModel.h`:

- **`Slice { double start, end; }`** — limites normalizados 0–1 do buffer.
- **`SliceBank`** (até 128 slices): `divideRegion(start, end, count)`
  (divisão igual contígua), `setSlice(i, slice)`, `addBladeCut(pos01)` /
  `undoBladeCut()` (cortes manuais — a "navalha" literal: cada clique na
  waveform cria uma fronteira), `appendMicroSlices(i, divisões,
  durSegundos)` (subdivide um slice em micro-slices, com piso de ~2 ms).
- **`region [regionStart..regionEnd]`** — o usuário isola um trecho de uma
  gravação longa e só esse trecho vira banco de slices; **o arquivo nunca
  é reescrito** (`REGION_SLICER_v0.8.md`). Coordenadas sempre 0–1.

O fluxo (`REGION_SLICER_v0.8.md`):
`SOURCE AUDIO → SOURCE REGION → SLICE BANK (1..128) → PATTERN BANK (10×8)
→ 8-STEP SEQUENCER → engine`.

### 2.4 Os gestos de fragmentação

`FRAGMENTATION_v0.12.md` — quatro gestos não-destrutivos:

- **STUTTER ×4** — repete a célula focada 4× dentro de ~1 intervalo do
  sequenciador;
- **BURST ×8** — gesto de 8 eventos a partir de referências de slice do
  padrão (cai pro banco ativo se não houver slice tocável);
- **MICRO ×8** — subdivide o slice atual em até 8 menores e anexa ao
  banco (cap 128; material curtíssimo reduz a contagem pra não gerar
  micro-slice < ~2 ms);
- **REVERSE SLICE** — audição do slice atual pra trás.
- **De-click adaptativo**: reverse tem envelope de borda de no máx 5 ms,
  encurtado proporcional pra fragmento muito curto.

### 2.5 Código C++ reaproveitável (`NAVALHA2_JUCE/src/core/`)

| Arquivo | ~linhas | O que dá pra portar |
|---|---|---|
| **`SlicePlayer.h/.cpp`** | 66 + 178 | `StereoAudioBuffer` (interpolação linear, `interpolated(pos)`) + `SlicePlayer`: `trigger(Slice, reverse, playbackRate, attack, release)`, varispeed (`increment = bufSR/outSR · playbackRate`), reprodução reversa (`position -= increment`), envelope A/R por rampa, **de-click adaptativo** (`clamp(dur·0.24, 0.5 ms, 5 ms)` quando `attack/release` não são dados), `stop()` com fade de 5 ms, `normalizedPosition()`. É uma **voz de SAMPLER quase pronta.** |
| **`HeritagePitch.h/.cpp`** (`LegacyPitchChannel`) | ~117 | Pitch-shift do `G09.pitchshift.pd`: linha de delay de 10 ms, **duas leituras janeladas por `sin` defasadas meia fase** (evita o clique do salto de cabeça), HP de 5 Hz na saída, interpolação polinomial de 4 pontos idêntica ao `vd~` do Pd, faixa −12..+11 st (`ratio = exp(st · 0.05776)`), crossfade seco↔molhado (`setMode`). É o "repitch preservando duração" que o §3.1 abaixo só gesticulava. |
| **`SessionModel.h::SliceBank`** | — | O modelo de region/BLADE/micro-slice de §2.3 — informa o painel de um `SAMPLER` com edição de trecho. |
| **`AudioEngine.h/.cpp`** (orquestração) | — | `voicesPerSource = 2`, alocação **round-robin** com crossfade entre voz velha e nova (`nextVoice`), gate de comandos por callback. Modelo simples de polifonia + *voice stealing* pra STUTTER/BURST. |
| **Source mixer** (`STEREO_SOURCE_MIXER_v0.21.md`) | — | Por fonte: LEVEL (0–1.25), PAN (center-preserving), WIDTH (0 % mono … 200 %, mid/side), MUTE, SOLO + A/B BALANCE. Rampas de 15 ms. Reusa o pan do nosso `MIXER`. |

### 2.6 Abstrações Pure Data (`NAVALHA2_PD/core/`)

Referência de topologia (não portáveis direto, mas o desenho é claro):

- **`navalha_player.pd`** — `soundfiler` (load `-resize` pra arrays L/R),
  `tabplay~`, de-click com `vline~` (`0 5, 1 5 5` = 5 ms de subida),
  `gain`, `normalize`, `reverse` (delega ao reverse-reader);
- **`navalha_reverse_reader~.pd`** — lê os arrays do player pra trás com
  `tabread4~`, de-click adaptativo (máx 5 ms, proporcional);
- **`navalha_pitchshift_legacy~.pd`** — a versão Pd do Heritage Pitch;
- **`navalha_voice~.pd`** — voz virtual: arquivo/slice/pitch/envelope/
  nível/pan independentes antes do pitch mestre compartilhado;
- **`navalha_source_mixer~.pd`** — o mixer de §2.5.

### 2.7 O loop de re-alimentação

`RESAMPLE_FEEDBACK_LOOP_v0.17.3.md` /
`VIRTUAL_VOICES_v0.24.0.md`: PERFORM → REC (grava o master pós-pitch) →
TAKE → biblioteca → **vira SOURCE A/B** → REGION/BLADE → PERFORM. Uma
performance gravada vira matéria nova sem transformar o instrumento em
editor destrutivo. É o mesmo espírito da **captura ao vivo do
`WAVETABLE`** (Módulo 40) e da cicatriz do `Cable`/`MEMORY` — e sugere
que um `SAMPLER` RASGO deve poder gravar a saída do próprio grafo num
buffer (porta `in` + `rec`), não só ler disco.

## 3. A biblioteca de áudio — o que puxar (e o que não)

Restrições da família (memória `reference_rasgo_license_convention`):
compatível com AGPLv3, **sem** CC-NC/ND, **sem** cabeçalhos SPDX.
E o valor local: **header-only, zero dependência de sistema.**

| Opção | Formato | Licença | Veredito |
|---|---|---|---|
| **`dr_wav.h` / `dr_flac.h` / `dr_mp3.h`** (David Reid, "dr_libs") | WAV / FLAC / MP3 | domínio público **ou** MIT-0 (dual) | **VERDE** — single-header, zero-dep, mesma pegada do nosso `WavWriter`. Escolha default. |
| **`stb_vorbis.c`** (Sean Barrett) | OGG Vorbis | domínio público / MIT | VERDE — se quisermos OGG. Um `.c`, não header-only puro, mas trivial. |
| **`miniaudio.h`** (David Reid) | decodifica **e** toca (device I/O) | domínio público / MIT-0 | ÂMBAR — resolve decode **e** a saída ALSA do painel de uma vez, mas é ~90k linhas; talvez pesado demais pra "só ler um WAV". Reavaliar se um dia quisermos backend de áudio portável (Win/Mac). |
| **libsndfile** | tudo | **LGPL** | VERMELHO pra embutir — LGPL + link dinâmico dá trabalho e fere o "zero-dep". Não. |
| **FFmpeg / libav** | tudo | LGPL/GPL | VERMELHO — peso e licença. Não. |

**Recomendação:** começar com **`dr_wav.h`** só (WAV PCM/float — o que o
próprio `WavWriter` gera, então round-trip garantido). Adicionar
`dr_flac`/`dr_mp3`/`stb_vorbis` sob demanda. Vendorizar em
`third_party/dr_libs/` com um `PROVENANCE.md` (origem, versão, commit,
licença) — como manda a convenção pra código de terceiros.

Novo arquivo: `src/io/AudioFile.hpp` — `struct AudioFile { std::vector<float>
samples; unsigned channels; unsigned sampleRate; }` + `loadAudioFile(path)`
e (já temos) `writeWav16`. Camada `io/`, fora do core.

## 4. Os três módulos pedidos

Todos partem do **conceito** de uma máquina real e reescrevem no
`SignalGraph` — nada de emular um produto. Onde o **Navalha 2** já
resolveu, portar de lá (com o crédito de §2.1) em vez de reinventar.

### 4.1 `SAMPLER` — matéria gravada (SOURCE / MEMORY)

O `MEMORY` com uma boca pra disco. Buffer carregado de arquivo **ou**
gravado ao vivo (porta `in` + `rec` — o loop de §2.7). Reprodução:

- **one-shot / loop / gated** (modo);
- `start`/`end`/`loop` (pontos, 0–1 do buffer) — o modelo `Slice` de §2.3;
- **banco de slices** com divisão igual + cortes manuais (BLADE) —
  `SliceBank` de §2.3; `trigger` dispara o slice focado, CV seleciona o
  índice;
- `pitch` (1 V/oct — varispeed via `SlicePlayer::increment`, muda
  duração) **ou** `repitch` preservando duração via `LegacyPitchChannel`
  de §2.5 (não mais "o mesmo grão do `MEMORY`" — há algoritmo pronto);
- `scan` (CV varre a posição de leitura — *tape scrubbing*);
- **gestos de fragmentação** de §2.4: `stutter`, `burst`, `micro`,
  `reverse` como entradas de trigger / toggles;
- **desvio RASGO:** `wear` — cada disparo desgasta levemente o trecho
  lido (herança da cicatriz do `Cable` / `MEMORY`); `spread` na escolha
  do start quando há jitter.

Base de porte: **`SlicePlayer` + `HeritagePitch` + `SliceBank`** do
`NAVALHA2_JUCE`. Fontes de estudo complementares (conceito, não código):
Akai S-series / E-mu (varispeed), MPC (chop/slice), Mutable **Clouds**
(base do `MEMORY`), Roads *Microsound*.

### 4.2 `TURNTABLE` — toca-discos de DJ (SOURCE / GESTURE)

Um `SAMPLER` cuja posição de leitura é um **volante com inércia**:

- `platter` — velocidade do prato como estado com massa (aceleração
  finita); `brake`/`start` = rampas de tempo ajustável (o "power off"
  arrastando o pitch pra baixo);
- `scratch` (CV bipolar) — empurra o prato pra frente/trás; a fricção
  traz de volta à `platter`;
- `crossfader` + `cue` — dois "decks" (dois buffers) e um fader de
  potência constante (reusa o pan do `MIXER` / o source mixer de §2.5);
- desvio: sem quantização de BPM "mágica" — o beatmatch é gesto, como
  no vinil de verdade.

Nota: o Navalha 2 **rejeita explicitamente** a metáfora de deck/DJ
(`DUAL_MATERIAL_v0.11.md`). O `TURNTABLE` é onde o RASGO Modular pode
divergir do Navalha — mas então tem que ser o modelo físico do prato
(inércia + torque + atrito) bem feito, senão é gimmick. Estudo: EDO de
1ª ordem (já sabemos — ver `platter` ≈ o `drift` integrado do `CLOCK`);
técnica de scratch (baby, chirp, transformer = o crossfader cortando).

### 4.3 `TAPE` — mecanismo de fita cassete (TRANSFORM / MEMORY)

Delay/looper a fita + as **imperfeições como timbre**:

- `speed` (varispeed) + **wow & flutter** — modulação lenta (wow, ~0,5–6
  Hz) e rápida (flutter, ~6–30 Hz) da velocidade de leitura, com
  profundidade e uma componente aleatória (o `NOISE.smooth` serve);
- `saturation` — compressão magnética suave (o `tanh` do `OutputStage`
  já tem a curva) + leve compressão dinâmica;
- `hiss` — piso de ruído rosa correlacionado ao nível (o `NOISE.pink`);
- `age` — um macro que abre tudo isso junto (0 = fita nova, 1 = fita
  estragada: mais flutter, menos agudo, dropouts);
- modo **echo** (cabeça de repro atrasada, feedback — vira Space Echo)
  e modo **loop** (Frippertronics / dois gravadores).

**DECIDIDO (2026-09-06): `TAPE` NÃO vira módulo.** O `LOOPER` (#41) já
tem `age` (passa-baixa no laço + wow&flutter + `tanh` + chiado), `hold`
e `reverse` — um `TAPE` seria ~80% duplicata. O que faltava — **eco de
fita multi-cabeça** (Space Echo) e **Frippertronics** — entrou no
`LOOPER` como o knob **`heads`** (1–4 cabeças lendo `time·{0,75; 0,5;
0,25}×`, somadas, realimentação regenera todas) + `feedback` perto de 1.
Dropouts / *saturation* separada de `age` ficam como pendências do
`LOOPER`, não de um módulo novo. O varispeed de fita está no `SAMPLER`
(#48, `speed` bipolar + `wear`). Estudo que embasou: Roland RE-201
(topologia cabeça/motor/feedback), wow & flutter (DIN/IEC), *gap loss*
da cabeça.

## 5. Ordem sugerida

1. **`src/io/AudioFile.hpp`** + vendor `dr_wav` + `PROVENANCE.md` +
   teste de round-trip (`writeWav16` → `loadAudioFile` → compara).
2. **Portar `SlicePlayer` + `HeritagePitch` do `NAVALHA2_JUCE`** pro
   idioma `src/dsp/` (header-only, `noexcept`, sem exceções em RT — o
   Navalha lança em `prepare`/`trigger`, trocar por retorno/clamp) +
   `CREDITS`/`THIRD_PARTY_NOTICES` (§2.1).
3. **`SAMPLER`** — menor salto (é o `MEMORY` + `SlicePlayer` + carga de
   disco); valida a camada `io/` e o porte.
4. **`TAPE`** — só se passar no teste de §4.3 (ir além do `LOOPER`).
   Reusa `OutputStage` sat, `NOISE` wow/hiss, `SPACE` eco.
5. **`TURNTABLE`** — o mais "instrumento novo"; precisa do modelo de
   inércia do prato bem resolvido pra não ser gimmick.

Tudo **depois** do rack de partida e sempre como **nó opcional** — o
painel continua abrindo e soando sem nenhum arquivo carregado.

## 6. Riscos / perguntas em aberto

- **Determinismo:** um `SAMPLER` que lê arquivo do usuário não é
  reproduzível entre máquinas — os renders de exemplo e o "seed de
  patch" não podem depender dele. Decisão: módulos de áudio-gravado
  ficam **fora** do `seedPatch()` e dos `examples/` determinísticos, ou
  entram só com um buffer sintético embutido.
- **RT-safety:** carregar/decodificar arquivo é I/O + alloc → tem que
  acontecer numa thread de UI e trocar o buffer por ponteiro atômico,
  nunca em `process()`. O painel já tem o padrão (`gmx`, swap de buffer).
  O `SlicePlayer` do Navalha **lança exceção** em `prepare`/`trigger` —
  no porte, trocar por `tryTrigger`-style (já existe lá) e clamp.
- **Peso do buffer:** um WAV estéreo de 3 min a 48k = ~66 MB em float.
  Cap configurável + streaming de disco fica como 2ª camada.
- **Escopo:** isto abre a porta pra "DAW-num-módulo". Manter cada módulo
  **completo no que promete e nada além** (§ do padrão de
  desenvolvimento) — `TAPE` é fita, não um multiefeito.
- **`TAPE` vs `LOOPER`:** decidir se `TAPE` é módulo próprio (§4.3).
- **Crédito:** porte do Navalha 2 exige atribuição a Glerm Soares +
  Lúcio Araújo + nota do `G09.pitchshift.pd` (§2.1) — não dissolver em
  lista genérica.
