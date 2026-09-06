# Dossiê — Módulo 48: Matéria gravada como voz (`SAMPLER`)

**Família:** SPACE (memória) — na taxonomia §4.1 fica com `LOOPER`/`MEMORY`
**Estado:** **implementado — Onda D** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Sampler.hpp`, `src/dsp/PitchShift.hpp`,
`tests/test_sampler.cpp`, `tests/test_pitch_shift.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda D, #48) +
`dossies/ESTUDO_audio_sampling.md`

## Estado da implementação

O `MEMORY` (#7) é o buffer granular; o `LOOPER` (#41) o delay de linha.
`SAMPLER` é o **toca-fatias**: um `trig` → um golpe de um trecho gravado,
com varispeed, reverse, repitch e desgaste — o "chop" do MPC/Akai no
patch.

**Base de porte (`ESTUDO_audio_sampling §2`):** o `SlicePlayer` do
`RASGO/NAVALHA2_JUCE` (de-click adaptativo, varispeed, reverse) —
Navalha de **Glerm Soares** / Navalha 2 de **Lúcio Araújo**,
GPL-3.0-or-later (compatível com AGPLv3-or-later). O `repitch` usa
`dsp/PitchShift.hpp` = o `HeritagePitch`/`G09.pitchshift.pd` (Miller
Puckette), portado com atribuição.

- **`start`** (0–1) = ponto de partida dentro da fatia.
- **`speed`** (−1..1) = varispeed bipolar `sinal·2^(|speed|·2)` →
  ±0,25×–±4×; negativo = reverso.
- **`slices`** (1–16) = divide o buffer em N fatias iguais; a CV `pos`
  escolhe qual disparar.
- **`repitch`** (0–1) = `0` transposição via velocidade (fita); `1`
  velocidade solta + pitch-shifter (duração da fatia preservada).
- **`wear`** (0–1, desvio Rasgo) = desgaste **por disparo**: jitter de
  início + redução de taxa (hold) + bit-crush, todos crescendo com o
  knob. **Determinístico** — xorshift semeado NO disparo (mesma
  sequência de triggers → mesmo áudio).
- **`loop`** (0/1) = one-shot ↔ loop da fatia (ponto de loop sem
  crossfade — pendência).

**Gravação:** gate `rec` alto → grava `in` no buffer (~8 s pré-alocado).
Na descida, `recLen_` congela. **Ou** o painel injeta um arquivo via
`setBuffer(mono, srcRate)` — chamado da thread de UI, **nunca de
`process()`** (`ESTUDO §5`); o arquivo tem prioridade sobre o gravado.
Sem buffer → silêncio.

**Determinismo:** total. O `wear` semeia no disparo; sem arquivo/gravação
externa, `SAMPLER` fica **fora** do `seedPatch()` determinístico só se
carregar arquivo — o modo gravação ao vivo é reprodutível.

**Segurança:** de-click adaptativo (`clamp(dur·0,24, 0,5 ms, 5 ms)` —
do `SlicePlayer`); `readBuf` clampa aos limites da fatia; `inc_` com piso.
`prepare()` aloca ~1,5 MB (buffer de 8 s); `process()` não aloca.

**Testes (Debug + Release):** grava 8000 amostras de 200 Hz → `trig` toca
a 200 Hz; `speed=0,5` → toca a 2× (400 Hz); `slices=2` + `pos` 0/1 →
toca a metade certa (150 Hz vs 500 Hz); `repitch=1` + PIT +1 oitava →
altura sobe pra 400 Hz **e a fatia dura o dobro** de `repitch=0`;
`loop=1` → soa muito depois do fim da fatia, `loop=0` para; `wear=1` →
piso espectral (bit-crush) 2×+ acima do limpo, saída limitada; dois
renders byte-idênticos com `wear`/`repitch`; `setBuffer` + `trig` toca o
arquivo; sem buffer → silêncio absoluto. `PitchShift`: ±1 oitava e +7
semitons afinam certo (razão linear e exata na faixa [0,25; 4]).

**Pendências (candidatos):** crossfade no ponto de loop; `end`/`length`
separado do `start`; modo *scrub* (`pos` varre a posição de leitura em
tempo real, não só seleciona fatia); *time-stretch* real (granular, não
o delay-shifter); vários buffers / *kits*; detecção de transientes pra
auto-fatiar; `wear` como cicatriz acumulada (hoje reseta por disparo).

---

## 1. Problema musical e papel no fluxo

Toda a prática de *sampling* — pegar um break e picar, tocar um sample
afinado por um sequenciador, o *stutter*, o vinil ao contrário — não sai
do `MEMORY` (que é granular/textura) nem do `LOOPER` (delay). `SAMPLER`
põe o toca-fitas/MPC no patch: `TRIGSEQ.t1 → SAMPLER.trig`, `pos` de um
`SEQUENCE` browniano, e o break se recombina. `AUDIO-IN → SAMPLER.in` +
um `CLOCK → rec` = *resampling* do que o instrumento toca.

Distinção: `MEMORY` = nuvem de grãos assíncronos; `LOOPER` = linha de
atraso com hold/reverse; `SAMPLER` = disparo de fatias com varispeed.

## 2. Fontes ESTUDADAS (conceito, não código — exceto o porte anotado)

- **Navalha 2** (`RASGO/NAVALHA2_JUCE`) — `SlicePlayer.cpp`: varispeed
  (`increment = bufSR/outSR·rate`), reverso, de-click adaptativo
  (`clamp(dur·0,24, 0,5 ms, 5 ms)`), stop-fade. **Portado** (não só
  estudado): GPL-3.0-or-later, crédito Glerm Soares + Lúcio Araújo. Ver
  `ESTUDO_audio_sampling §2.1`.
- **`G09.pitchshift.pd`** (Miller Puckette, Pd) — o pitch-shifter por
  linha de atraso janelada; domínio público. Via `dsp/PitchShift.hpp`
  (porte do `HeritagePitch` do Navalha 2).
- **Akai S-series / E-mu** — varispeed = ler o buffer em passo ≠ 1
  (teoria, domínio público).
- **MPC (chop/slice)** — dividir um trecho em N e disparar as fatias.
- **`dossies/ESTUDO_audio_sampling.md`** — o estudo à parte que embasou
  a decisão da dependência (`dr_wav`, camada `io/`) e o risco de
  determinismo.

**Desvio Rasgo (Atlas §49):** o `wear` de desgaste determinístico por
disparo (a fita se degrada mas o render é reprodutível — a cicatriz do
`Cable`/`MEMORY`), e a relação — `start`/`pos`/`speed`/`pitch` todos por
CV do grafo, o *sampling* como processo.

## 3. Modelo

**Gravação:** `rec` ↑ → `recWrite_ = 0`; enquanto alto `rec_[recWrite_++]
= in`; `rec` ↓ → `recLen_ = recWrite_`. Cap em `bufLen_` (8 s).

**Disparo** (`trig` ↑, `effLen ≥ 8`):
```
idx      = clamp(⌊pos·nSlices⌋, 0, nSlices−1)
sliceLen = effLen / nSlices ; sliceLo_ = idx·sliceLen ; sliceHi_ = +sliceLen
wear (semeado agora): startJit = (±)·wear·0,02·sliceLen
                      crushHold_ = 1 + ⌊wear²·12·rnd⌋
                      crushStep_ = 2^(−(16 − wear²·12 − 1))
reverse_ = speed < 0
mag      = 2^(|speed|·2)                              (0,25 .. 4)
pitchFac = repitch < 1 ? 2^(pitch·(1−repitch)) : 1
inc_     = mag · pitchFac · (arquivo ? srcSR/outSR : 1)
shifter.setRatio(repitch>0 ? 2^(pitch·repitch) : 1)
playPos_ = reverse_ ? sliceHi_−1−jit : sliceLo_ + start·sliceLen + jit
total_   = (sliceHi_ − sliceLo_) / inc_ + 1
fade     = clamp(total_/sr · 0,24 , 0,5 ms, 5 ms) ; attackS_ = releaseS_ = fade·sr
```

**Por amostra (tocando):**
```
raw = interp(buf, playPos_)                 (clampado a [sliceLo_, sliceHi_−1])
raw = holdEvery(raw, crushHold_)            (redução de taxa — wear)
raw = round(raw / crushStep_)·crushStep_    (bit-crush — wear)
voiced = repitch>0 ? lerp(raw, shifter(raw), repitch) : raw
env = min(ataque, release)                  (release só se !loop)
y = voiced · env
playPos_ += reverse_ ? −inc_ : inc_
se passou do fim: loop ? volta ao início da fatia : para
```

**Extremos:** `speed = 0` → `mag = 1` (velocidade normal). `slices = 16`
+ `pos` varrendo → dispara fatias diferentes a cada trigger. `repitch =
1` + `pitch = +2` oitavas → shifter no teto (clampa em 4×), duração da
fatia intacta. `wear = 1` → `crushStep_` de ~4 bits + hold de até 12
amostras → lo-fi extremo, ainda limitado. Buffer vazio → `effLen < 8` →
`trig` ignorado, saída 0. Fatia < de-click (2× fade) → `total_ ≤ 1` →
`declick` devolve 0 (não estala com fatia minúscula).

## 4. Três modos obrigatórios

- **autônoma:** `loop = 1` + um buffer (gravado ou de arquivo) + `pos`
  de um `LFO` → um trecho que se recombina sozinho; sem cabo de
  controle já toca.
- **performance:** `pos`/`speed`/`start` são os macros; `trig` (dedo ou
  `TRIGSEQ`) dispara.
- **híbrida:** `AUDIO-IN → in`, `CLOCK → rec` (resample ao vivo);
  `TRIGSEQ → trig`, `SEQUENCE → pos` (o break browniano);
  `ENVELOPE → wear` (a fita se degrada no clímax).

## 5. Portas, parâmetros, limites

**Entradas:** `trig` (Control trig), `in` (Audio), `rec` (Control gate),
`pos` (Control), `pitch` (Control v/oct).
**Saídas:** `out` (Audio).
**Parâmetros:** `start` (0–1), `speed` (−1..1, def 0,5), `slices` (1–16,
def 1), `repitch` (0–1, def 0), `wear` (0–1, def 0), `loop` (0/1, def 0).
**Método extra:** `setBuffer(std::vector<float> mono, float srcRate)` —
só do painel, fora do RT.
**Limites:** `out` em ~[−1,1] (o material gravado × envelope; o `wear`
não amplifica). CPU: por amostra 1 leitura interpolada + (se `repitch`)
o pitch-shifter (2 leituras janeladas de uma linha de 10 ms). `prepare`
aloca ~1,5 MB.

## 6. Alternativas descartadas

- **corpo no `MEMORY`** — o `MEMORY` é granular assíncrono; disparo de
  fatia com varispeed é outro objeto. Complementares.
- **time-stretch granular** já na v1 — o `repitch` por delay-shifter é o
  do Navalha (barato, com caráter); stretch real (nuvem de grãos
  sincronizados) é maior, fica como pendência.
- **carregar arquivo dentro do módulo** — quebra "core sem dependência";
  o `dr_wav` vive em `io/`, o painel chama `setBuffer()`.
- **`wear` com RNG livre** — semear no disparo mantém a
  reprodutibilidade (a regra do `TRIGSEQ`/`DRUM`).
- **crossfade no ponto de loop** — vale, mas é estado extra; v1 aceita o
  possível clique (anotado), o de-click de borda já cobre o começo/fim.

## 7. Integração e painel

14 HP, família **SPACE** (junto de `LOOPER`/`MEMORY`/`HALL` — memória e
espaço). Display da forma de onda + fatias (o painel pode desenhar as
marcas). Knobs `START`/`SPEED`/`SLICE`/`REPIT` (linha 1), `WEAR` +
toggle `LOOP` (linha 2); jacks `TRIG`/`IN`/`REC`/`POS`/`PIT` + `OUT`.

O painel: botão "carregar WAV" → `loadAudioFile()` (`io/AudioFile.hpp`) →
`toMono()` → `Sampler::setBuffer()`. `rasgo_modular_io` linkado só pelo
painel e pelos testes de `io/`.

Cadeias canônicas: `TRIGSEQ.t1 → SAMPLER.trig`, `SEQUENCE → SAMPLER.pos`;
`AUDIO-IN → SAMPLER.in`, `CLOCK → SAMPLER.rec`; `SAMPLER → HALL`.
