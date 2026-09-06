# Dossiê — Módulo 47: Voz de percussão (`DRUM`)

**Família:** SOURCE
**Estado:** **implementado — Onda C** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Drum.hpp`, `tests/test_drum.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda C, #47)

## Estado da implementação

O `MATTER`+`NOISE`+`ENVELOPE` já montam um bumbo/caixa/prato à mão.
`DRUM` empacota isso num gesto: **um gate → um golpe**, com o essencial
de uma voz de percussão sintética num punhado de knobs.

Três camadas por golpe:
1. **corpo** — um oscilador senoidal com **envelope de altura**: no
   disparo a frequência salta pra `tone·(1 + bend·6)` e cai de volta a
   `tone` numa exponencial rápida (~40 ms). É o "pow" do bumbo 808.
   `map` mistura a senóide com `tanh(corpo·3)` — de senóide pura (808) a
   corpo com clique/harmônicos (909).
2. **estalo** — uma rajada de ruído branco por um passa-alta de 1 polo
   (frequência sobe com `map`: 808 grave/surdo → acústico brilhante) com
   envelope próprio bem curto (~8–30 ms). `snap` é a dose.
3. **envelope de amplitude** — exponencial, ataque quase instantâneo,
   `decay` de ~20 ms a ~2 s.

- **`tone`** (20–1000 Hz, + CV 1 V/oct) = altura do corpo.
- **`bend`** (0–1) = profundidade do envelope de altura (0 = tonal,
  1 = varredura de bumbo).
- **`decay`** (0–1) = tempo de decaimento geral.
- **`snap`** (0–1) = dose do ruído de ataque.
- **`map`** (0–1) = caráter 808 → 909 → acústico (corpo mais limpo/mais
  sujo, ruído mais grave/mais agudo, um pouco de *drive* embutido).
- **`drive`** (0–1) = saturação de saída (`tanh` com *makeup*) — o
  *crunch* do 909.
- **`roll`** (0–1) = **auto-disparo** interno (0 = só gate externo; senão
  ~2–40 Hz) — rufo/*buzz* e o modo autônomo (toca sozinho).
- **`drift`** (0–1, desvio Rasgo) = humanização por golpe — cada disparo
  varia levemente altura/decay/nível de um **xorshift semeado avançado no
  disparo** (determinístico dada a sequência de gates; `drift=0` → golpes
  idênticos).

**Entradas:** `gate` (trigger), `accent` (CV — escala nível e brilho),
`tone` (CV, 1 V/oct sobre o knob).
**Saída:** `out`.

**Segurança:** `tanh` de saída (o `drive`) já limita; sem `drive` o
envelope mantém o corpo em ~[−1,1]. `prepare()` não aloca (sem buffers —
só osciladores e envelopes de estado); `process()` não aloca.
Determinístico dada a sequência de gates.

**Testes (Debug + Release):** gate → um golpe (silêncio antes, pico logo
depois, decai); `decay` maior → cauda mais longa; `bend` alto → a
frequência do corpo começa alta e cai (taxa de cruzamento de zero cai ao
longo do golpe); `snap` → mais energia de ruído/agudo nos primeiros
20 ms; `map` sobe o brilho/harmônicos; `drive` adiciona harmônicos e
mantém a saída limitada; `roll` > 0 sem gate → golpes periódicos (modo
autônomo); `accent` alto → golpe mais forte; dois renders byte-idênticos
com `drift`/`roll`; extremos limitados.

**Pendências (candidatos):** modo de corpo por **ressoador modal**
(reusar o `MATTER` — surdo/conga afinados); segundo oscilador de corpo
(clave/rim); *choke* (gate de corte, chimbal aberto/fechado); `map` como
seleção discreta de kit em vez de morph contínuo; saída de *envelope*
pra encadear; FM no corpo (cowbell/agogô).

---

## 1. Problema musical e papel no fluxo

O `TRIGSEQ` gera a grade rítmica, mas precisa de vozes pra tocar. Montar
bumbo com `MATTER`+`ENVELOPE`+`NOISE` toda vez é trabalhoso e ocupa
3 módulos. `DRUM` é a voz pronta: `TRIGSEQ.t1 → DRUM.gate`, ajusta
`tone`/`decay`/`map`, e tem um bumbo. Quatro `DRUM` = um kit. Como
`tone`/`snap`/etc aceitam CV, `SEQUENCE → DRUM.tone` toca uma linha de
toms, `ENVELOPE → DRUM.decay` faz o chimbal "abrir".

Distinção: o `MATTER`/`STRING` são ressoadores modais/de corda
(percussão afinada, ring longo); o `DRUM` é a voz sintética de bateria
(bumbo/caixa/prato) empacotada — o golpe seco.

## 2. Fontes ESTUDADAS (conceito, não código)

- **TR-808 / TR-909** (*referência funcional*) — a topologia da voz de
  bumbo: oscilador (ponte-T no 808, VCO no 909) + envelope de altura
  ("sweep") + envelope de amplitude + ruído de ataque. Circuitos
  documentados à exaustão no DIY (808 bridged-T, 909 hybrid). Só o
  **princípio** (corpo com pitch-sweep + estalo + envelope), reescrito.
- **Percussão sintética clássica** (Roads, *Computer Music Tutorial*;
  Dodge & Jerse) — o modelo "excitação curta + ressonância + ruído".
- **Mutable Rings / Plaits (modo bumbo/caixa)** — o "drum a partir de
  ressoador"; aqui o corpo é senoidal (pendência: modo modal via
  `MATTER`).
- **vpme QD ★ (`PESQUISA §7 #24`)** — drum de Eurorack minimalista;
  poucos knobs, mapa de caráter.

**Desvio Rasgo (Atlas §49):** a humanização por golpe determinística
(xorshift semeado no disparo — cada golpe varia mas o render é
reprodutível), `roll` como auto-disparo (a voz toca sozinha), e a
relação — `tone`/`map`/`decay` endereçáveis por CV, o golpe é um
processo.

## 3. Modelo

**No disparo** (borda ↑ de `gate`, ou `roll` interno):
```
ampEnv = 1 ; pitchEnv = 1 ; noiseEnv = 1 ; bodyPhase = 0
rng = xorshift(rng)                              (avança 1×)
hitPitch = 1 + drift·0,06·(rng01·2−1)            (± 6 % de altura)
hitDecay = 1 + drift·0,25·(rng01'·2−1)
hitLevel = accent (amostrado) · (1 + drift·0,2·(rng01''·2−1))
```

**Por amostra:**
```
f0 = tone · 2^(tone_cv) · hitPitch
f  = f0 · (1 + bend·6·pitchEnv)                  (salto de altura)
bodyPhase += f·dt ; wrap
raw = sin(2π bodyPhase)
raw = (1−map)·raw + map·tanh(raw·3)              (corpo mais duro no 909)

nz   = ruído branco → passa-alta 1 polo (fc = 800 + map·6000 Hz)
body = raw · ampEnv
snapS= nz · snap · noiseEnv · (0,6 + map·0,5)

y = body·(0,9 − map·0,2) + snapS
y = tanh(y · hitLevel · (1 + drive·4)) / (1 + drive·1,5)

ampEnv   ·= exp(−dt / decayT)   decayT   = 0,02·100^(decay·hitDecay)
pitchEnv ·= exp(−dt / 0,035)
noiseEnv ·= exp(−dt / (0,006 + decay·0,04))
```

**`roll`:** contador interno; quando `roll > 0`, dispara a cada
`sr / (2 + roll·38)` amostras. `accent` interno = 1.

**Extremos:** `decay = 1` + `bend = 0` + `tone` grave → um zumbido longo
(sub). `snap = 1` + `decay` curtíssimo → só o clique. `drive = 1` → corpo
quadrado saturado, `tanh` limita. `roll = 1` → 40 Hz de golpes (quase um
tom). `tone` no máximo (1 kHz) + `bend` alto → varredura até 7 kHz no
ataque (click agudo) — sem alias audível (senóide band-limited; a
saturação de `map`/`drive` alia um pouco no ataque agudo — caráter
aceito, anotado). `gate` sem cabo e `roll = 0` → silêncio (a voz espera).

## 4. Três modos obrigatórios

- **autônoma:** `roll > 0` → a voz toca um rufo sozinha; `drift` leve
  humaniza. Sem nenhum cabo já soa.
- **performance:** `tone`/`map`/`decay` são os macros; `gate` (dedo ou
  `TRIGSEQ`) toca.
- **híbrida:** `TRIGSEQ.t1 → gate`, `TRIGSEQ.accent → accent`;
  `SEQUENCE → tone` (toms afinados); `LFO → map` (o caráter muda no
  compasso).

## 5. Portas, parâmetros, limites

**Entradas:** `gate` (Control trig), `accent` (Control), `tone`
(Control v/oct).
**Saídas:** `out` (Audio).
**Parâmetros:** `tone` (20–1000 Hz, def 55), `bend` (0–1, def 0,6),
`decay` (0–1, def 0,4), `snap` (0–1, def 0,3), `map` (0–1, def 0),
`drive` (0–1, def 0), `roll` (0–1, def 0), `drift` (0–1, def 0).
**Limites:** `out` limitado por `tanh` (o `drive`; sem drive o envelope
segura). CPU: por amostra 1 `sin` + 1 `tanh` (map) + 1 `tanh` (drive) +
3 `exp` recalculados por golpe (não por amostra — os coefs são fixos
durante o golpe). `prepare` não aloca.

## 6. Alternativas descartadas

- **corpo por ressoador modal (`MATTER`)** já na v1 — mais rico pra
  surdo/tom afinado, mas é outro banco de estado; a senóide com
  pitch-sweep cobre bumbo/caixa/tom sintéticos. Modo modal fica como
  pendência.
- **envelope de amplitude AD editável** (attack/decay separados) — o
  ataque de percussão é sempre ~instantâneo; um knob de `decay` só é o
  suficiente. `punch` (ataque > 0) fica como pendência.
- **`map` como seletor de kit discreto** — o morph contínuo casa com a
  identidade (o `WAVETABLE.pos`, o `TRIGSEQ.map`); seleção discreta seria
  um `mode`, menos expressivo.
- **oversampling** — a saturação de `drive`/`map` alia um pouco no
  ataque agudo; é transiente e curto, o custo de 2× não compensa.
- **RNG livre na humanização** — semear no disparo mantém a
  reprodutibilidade (mesmos gates → mesmo áudio).

## 7. Integração e painel

14 HP, família **SOURCE** (junto de `OSC`/`OPERATOR`/… — é uma voz).
Display da forma do golpe (o `SCOPE`). Knobs `TONE`/`BEND`/`DECAY`/`SNAP`
(linha 1), `MAP`/`DRIVE`/`ROLL`/`DRIFT` (linha 2); jacks
`GATE`/`ACC`/`PIT` + `OUT`.

Cadeias canônicas: `TRIGSEQ.t1 → DRUM.gate` (+ `.accent → ACC`);
`SEQUENCE → DRUM.tone` (toms); `4× DRUM → MIXER` (kit);
`DRUM.out → HALL` (bateria com espaço).
