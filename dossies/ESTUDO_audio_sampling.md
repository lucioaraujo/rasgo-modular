# Estudo à parte — áudio gravado como matéria: sampler, toca-discos, fita

**Estado:** estudo / decisão de arquitetura pendente — **não implementado**
**Pedido do autor (2026-09-02):** *"talvez já possamos inserir audio
library, para trabalhar os sinais desses áudio de alguma forma… samplers,
djs turntable, cassete mechanisms, etc."*

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

## 2. A biblioteca de áudio — o que puxar (e o que não)

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

## 3. Os três módulos pedidos

Todos partem do **conceito** de uma máquina real e reescrevem no
`SignalGraph` — nada de emular um produto.

### 3.1 `SAMPLER` — matéria gravada (SOURCE / MEMORY)

O `MEMORY` com uma boca pra disco. Buffer carregado de arquivo **ou**
gravado ao vivo (porta `in`). Reprodução:

- **one-shot / loop / gated** (modo);
- `start`/`end`/`loop` (pontos, 0–1 do buffer);
- `pitch` (1 V/oct — varispeed, muda duração; ou `repitch` preservando
  duração via o mesmo grão do `MEMORY`);
- `trigger` (dispara), `scan` (CV varre a posição de leitura — *tape
  scrubbing*);
- **desvio RASGO:** `wear` — cada disparo desgasta levemente o trecho
  lido (herança da cicatriz do `Cable` / `MEMORY`); `spread` na escolha
  do start quando há jitter.

Fontes de estudo (conceito/teoria, não código): Akai S-series /
E-mu (varispeed = ler o buffer em passo ≠ 1), MPC (chop/slice),
Mutable **Clouds** (já é a base do `MEMORY`), Karplus/granular clássico
(Roads, *Microsound*).

### 3.2 `TURNTABLE` — toca-discos de DJ (SOURCE / GESTURE)

Um `SAMPLER` cuja posição de leitura é um **volante com inércia**:

- `platter` — velocidade do prato como estado com massa (aceleração
  finita); `brake`/`start` = rampas de tempo ajustável (o "power off"
  arrastando o pitch pra baixo);
- `scratch` (CV bipolar) — empurra o prato pra frente/trás; a fricção
  traz de volta à `platter`;
- `crossfader` + `cue` — dois "decks" (dois buffers) e um fader de
  potência constante (reusa o pan do `MIXER`);
- desvio: sem quantização de BPM "mágica" — o beatmatch é gesto, como
  no vinil de verdade.

Estudo: física de motor de prato (inércia + torque + atrito — EDO de 1ª
ordem, já sabemos fazer, ver `platter` ≈ o `drift` integrado do `CLOCK`);
técnica de scratch (baby, chirp, transformer = o crossfader cortando).

### 3.3 `TAPE` — mecanismo de fita cassete (TRANSFORM / MEMORY)

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

Estudo: Roland Space Echo RE-201 (topologia cabeça/motor/feedback —
esquemas e análises públicas fartas), Mellotron (uma fita por tecla),
teoria de wow & flutter (norma DIN/IEC de medição — define as bandas),
perda de agudo por *gap loss* da cabeça (filtro passa-baixa dependente
da velocidade).

## 4. Ordem sugerida

1. **`src/io/AudioFile.hpp`** + vendor `dr_wav` + `PROVENANCE.md` +
   teste de round-trip (`writeWav16` → `loadAudioFile` → compara).
2. **`SAMPLER`** — menor salto (é o `MEMORY` + carga de disco); valida a
   camada `io/`.
3. **`TAPE`** — reusa muita coisa que já existe (`OutputStage` sat,
   `NOISE` para wow/hiss, `SPACE` para o eco). Alto valor tímbrico.
4. **`TURNTABLE`** — o mais "instrumento novo"; precisa do modelo de
   inércia do prato bem resolvido pra não ser gimmick.

Tudo **depois** do rack de partida (`CONTROL`, `LOGIC`) e sempre como
**nó opcional** — o painel continua abrindo e soando sem nenhum arquivo
carregado.

## 5. Riscos / perguntas em aberto

- **Determinismo:** um `SAMPLER` que lê arquivo do usuário não é
  reproduzível entre máquinas — os renders de exemplo e o "seed de
  patch" não podem depender dele. Decisão: módulos de áudio-gravado
  ficam **fora** do `seedPatch()` e dos `examples/` determinísticos, ou
  entram só com um buffer sintético embutido.
- **RT-safety:** carregar/decodificar arquivo é I/O + alloc → tem que
  acontecer numa thread de UI e trocar o buffer por ponteiro atômico,
  nunca em `process()`. O painel já tem o padrão (`gmx`, swap de buffer).
- **Peso do buffer:** um WAV estéreo de 3 min a 48k = ~66 MB em float.
  Cap configurável + streaming de disco fica como 2ª camada.
- **Escopo:** isto abre a porta pra "DAW-num-módulo". Manter cada módulo
  **completo no que promete e nada além** (§ do padrão de
  desenvolvimento) — `TAPE` é fita, não um multiefeito.
