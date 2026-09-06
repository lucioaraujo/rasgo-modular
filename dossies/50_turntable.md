# Dossiê — Módulo 50: Toca-discos com prato de inércia (`TURNTABLE`)

**Família:** SPACE (matéria gravada) — junto de `SAMPLER`/`LOOPER`/`MEMORY`
**Estado:** **implementado** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Turntable.hpp`, `tests/test_turntable.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda D — `TURNTABLE`) +
`dossies/ESTUDO_audio_sampling.md §4.2`

## Estado da implementação

O `SAMPLER` (#48) toca fatias por disparo; o `TURNTABLE` é o mesmo buffer
lido por um **prato com massa**: a posição de leitura é a integral de uma
velocidade angular que tem **inércia** — o motor puxa devagar, a mão
(`scratch`) empurra forte, o freio (`brake`) faz descer coasting. Nada
disso no rack.

**Desvio Rasgo (decisão registrada em `ESTUDO §4.2`):** o Navalha 2
**rejeita** a metáfora de deck/DJ de propósito. Aqui o RASGO diverge —
mas com o **modelo físico do prato** bem feito (não é gimmick de
"scratch button"): torque finito, atrito, acoplamento de mão.

- **`speed`** (−1..1) = velocidade alvo do prato: `sinal·2^|speed|` →
  ±0,5× a ±2× (33⅓ ↔ 45 ↔ lento; negativo = disco girando pra trás).
- **`torque`** (0–1) = força do motor — quão rápido o prato ATINGE a
  velocidade alvo. Torque baixo = "wow" longo ao largar o freio ou pôr a
  agulha (o prato leva ~1 s pra subir).
- **`friction`** (0–1) = atrito: quão rápido o prato para no `brake`
  (coasting curto ou longo) e quanto o prato "volta" sozinho depois de um
  scratch.
- **`grab`** (0–1) = quanto a CV `scratch` joga o prato — a firmeza da
  mão. `scratch` pequeno = pitch-bend (empurrãozinho pra beatmatch);
  `scratch` grande e oscilando = scratch de verdade.
- **`start`** (0–1) = onde a agulha cai (posição no disco no `trig`).
- **`wear`** (0–1, desvio Rasgo) = desgaste do vinil: estalos/clicks que
  crescem + leve instabilidade de rotação. **Determinístico** (xorshift
  semeado; `wear=0` → sem termo).
- **`loop`** (0/1) = groove travado (o prato volta ao `start` no fim).

**Gravação:** gate `rec` grava `in` no buffer (~8 s), igual ao `SAMPLER`.
Ou o painel injeta um arquivo via `setBuffer()`. Sem buffer → silêncio.

**Entradas:** `trig` (põe a agulha), `in` (áudio a gravar), `rec` (gate),
`scratch` (CV bipolar — a mão), `brake` (gate — para o motor).
**Saída:** `out`.

**Física (por amostra):**
```
motorTgt = (motorRunning ∧ ¬brakeOn) ? speedRatio : 0   (motor só liga no 1º trig)
platterVel += (motorTgt − platterVel) · motorCoef       motorCoef ~ torque²
platterVel −= platterVel · dragCoef                     dragCoef  ~ friction
                                                        (× ~22 com brake)
handTgt = scratch · 4                                   (±4× a velocidade)
grip    = clamp01(|scratch| · 5) · grab
platterVel += (handTgt − platterVel) · grip · 0,3       (a mão age com motor parado)
readPos += platterVel · (arquivo ? srcSR/outSR : 1)     (pode ficar negativo)
readPos wrap [0, recLen)                                (ou trava, se !loop)
y = interp(buf, readPos) · dropEnv · wearGain + wearCrackle
out = dcBlock(y)                                        (o vinil não tem DC)
```

**Segurança:** `dropEnv` (rampa de ~5 ms no `trig` — sem clique de
agulha); acoplamento AC de 1 polo na saída (quando o prato para, a
amostra congelada some em ~40 ms em vez de virar um degrau DC); a saída é
o material gravado (limitado por construção) + crackle de `wear`
(pequeno). `prepare()` aloca ~1,5 MB; `process()` não aloca.
Determinístico.

**Testes (Debug + Release):** grava 200 Hz, `trig` → toca a 200 Hz;
`torque` baixo → a altura DEMORA a subir no `trig` (a nota começa grave e
sobe — o wow do motor); `torque` alto → sobe rápido; `brake` gate → a
altura DESCE a zero e o som para, `friction` alto acelera a parada;
`scratch` = um seno lento com `grab` alto → a saída "scratcheia"
(cruzamentos de zero acompanham o seno, ida e volta); `scratch` pequeno
com `grab` = pitch-bend (altura desvia proporcional, volta); `speed`
negativo → disco pra trás; `wear = 1` → estalos audíveis, saída limitada;
`loop` → o groove trava e repete; dois renders byte-idênticos com
`wear`/`scratch`; sem buffer → silêncio.

**Pendências:** `crossfader` + `cue` (dois buffers, fader de potência
constante); `start`/`brake` como rampas de tempo ajustável separadas do
`torque`/`friction`; *slipmat* (o disco desliza sob o prato — segurar o
label sem parar a rotação); RIAA / caráter de cápsula; `pitch` fino
(±8 % como o slider de um SL-1200); marca de fase visual no painel.

---

## 1. Problema musical e papel no fluxo

Tudo que vive do gesto de mão num prato — o scratch, o *backspin*, o
*power-off* que arrasta a afinação pra baixo, o beatmatch por
empurrãozinho, o *tape stop* — não sai de `MEMORY`/`LOOPER`/`SAMPLER`,
que leem a posição de forma "digital" (salto, grão, incremento fixo). O
`TURNTABLE` põe a INÉRCIA no patch: `LFO → scratch` faz o disco
scratchear no compasso; `ENVELOPE → brake` faz um *tape stop* na virada;
`SEQUENCE → scratch` toca uma frase de scratch rítmica.

Distinção: o `SAMPLER` dispara fatias (posição de leitura salta); o
`LOOPER` é delay de linha; o `TURNTABLE` é uma posição de leitura
CONTÍNUA governada por um sistema de 1ª ordem com massa.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Física de motor de prato** — inércia + torque + atrito = EDO de 1ª
  ordem (`platterVel' = k_motor·(alvo − v) − k_atrito·v`). Já sabemos
  fazer (é o `platter` ≈ o `drift` integrado do `CLOCK`, o poço do
  `CHAOS`). Mecânica clássica, domínio público.
- **Técnica de scratch** (baby, chirp, transformer, tear) — o "transformer"
  é o crossfader cortando; os outros são padrões de velocidade do prato.
  Documentação de DJ farta e pública.
- **Technics SL-1200** (*referência funcional*) — motor direct-drive de
  alto torque, slider de pitch ±8 %, botão start/stop com rampa. Só o
  **comportamento**; nenhum circuito.
- **`SAMPLER` (#48)** — o buffer, a gravação ao vivo, a leitura
  interpolada, `setBuffer()`; mesma camada `io/`.
- **Turntablism digital** (ms-pinky, serato) — o vinil de controle mapeia
  posição/velocidade da agulha; aqui a "agulha" é a CV `scratch`.

**Desvio Rasgo (Atlas §49):** SEM quantização de BPM "mágica" — o
beatmatch é gesto (empurrãozinho na `scratch`), como no vinil de
verdade; o `wear` de desgaste determinístico; a mão é CV do grafo (um
`LFO` e uma mão de DJ são intercambiáveis).

## 3. Modelo

Ver o bloco de física acima. `motorCoef = 0,00002 + torque²·0,0022`
(constante de tempo de ~1,1 s a ~10 ms). `dragIdle = friction·0,00003`
no ocioso (o servo do motor segura a rotação — atrito ocioso mínimo);
com `brake` o gate troca por `friction·0,00003·22 + 0,0004` (parada de
~1 s a ~20 ms). `handTgt = scratch·4`, `grip =
clamp01(|scratch|·5)·grab`, acoplamento `·0,3`.

**No `trig`** (borda ↑): `readPos = start·recLen`, `dropEnv = 0` (rampa
pra 1 em ~5 ms) e `motorRunning = true` — pôr a agulha é o gesto que
"liga" o prato (com `torque` baixo dá o *wow* de partida: a nota começa
grave e sobe até a rotação). Depois do 1º `trig` o motor fica ligado; o
`brake` o segura em zero enquanto o gate estiver alto.

**`wear`:** `wearGain = 1 − 0,04·wear` (leve queda de nível); crackle =
ruído impulsivo esparso (xorshift; densidade e amplitude ∝ `wear²`);
instabilidade = `platterVel += wear·0,0003·(rnd−0,5)` a cada ~256
amostras.

**Extremos:** `torque = 0` → o motor mal puxa; o prato só anda se a mão
(`scratch`) empurrar — modo "vinil sem motor". `brake` + `friction = 1`
→ parada quase instantânea (tape-stop seco). `scratch` no talo oscilando
rápido → `platterVel` chega a ±4×; `readPos` vai e volta — o interp
linear alia um pouco no pico (aceito, é scratch). `speed` negativo +
`brake` → o prato desacelera de −2× até 0 (o disco "sobe" de afinação
enquanto para de girar pra trás). Buffer vazio → `trig` não faz nada,
saída 0. `loop = 0` e `readPos` sai do fim → trava na última amostra
(`dropEnv` não força release — o disco "acabou").

## 4. Três modos obrigatórios

- **autônoma:** `loop = 1` + um buffer + `scratch` de um `LFO` lento →
  um groove que scratcheia sozinho; sem cabo de controle já toca.
- **performance:** `speed`/`start` nos knobs; `scratch` (dedo/joystick
  via adaptador) e `brake` (botão) são o gesto.
- **híbrida:** `SEQUENCE → scratch` (frase de scratch rítmica),
  `ENVELOPE → brake` (tape-stop na virada), `AUDIO-IN → in` +
  `CLOCK → rec` (grava e scratcheia ao vivo).

## 5. Portas, parâmetros, limites

**Entradas:** `trig` (Control trig), `in` (Audio), `rec` (Control gate),
`scratch` (Control), `brake` (Control gate).
**Saídas:** `out` (Audio).
**Parâmetros:** `speed` (−1..1, def 0,0 = 1×), `torque` (0–1, def 0,6),
`friction` (0–1, def 0,4), `grab` (0–1, def 0,7), `start` (0–1, def 0),
`wear` (0–1, def 0), `loop` (0/1, def 0).
**Método:** `setBuffer(std::vector<float> mono, float srcRate)` (só do
painel).
**Limites:** `out` em ~[−1,1] (material gravado × envelope; crackle
pequeno). CPU: por amostra ~1 leitura interpolada + ~6 mul-add da física.
`prepare` aloca ~1,5 MB.

## 6. Alternativas descartadas

- **modo do `SAMPLER`** — o `SAMPLER` é disparo de fatia (posição salta);
  a leitura contínua com inércia é outro objeto. Complementares.
- **crossfader/dois decks já na v1** — dobra o estado (2 buffers) e é
  mais "instrumento novo"; fica como pendência. O `MIXER` + dois
  `TURNTABLE` cobrem o essencial por enquanto.
- **quantização de BPM** — decisão de identidade: o beatmatch é gesto,
  não mágica.
- **RNG livre no `wear`** — semear mantém a reprodutibilidade (regra do
  `SAMPLER`/`TRIGSEQ`/`DRUM`).
- **vinil de controle real** (timecode) — o `scratch` como CV é o
  equivalente modular e casa com o grafo.

## 7. Integração e painel

14 HP, família **SPACE**. Display da forma de onda + a "agulha" (posição/
velocidade). Knobs `SPEED`/`TORQUE`/`FRIC`/`GRAB` (linha 1), `START`/
`WEAR` + toggle `LOOP` (linha 2); jacks `TRIG`/`IN`/`REC`/`SCR`/`BRK` +
`OUT`.

O painel: mesma via de arquivo do `SAMPLER` (`loadAudioFile` → `toMono`
→ `setBuffer`). Cadeias: `LFO → TURNTABLE.scratch` (scratch rítmico);
`ENVELOPE.env → TURNTABLE.brake` (tape-stop); `2× TURNTABLE → MIXER`
(dois decks).
