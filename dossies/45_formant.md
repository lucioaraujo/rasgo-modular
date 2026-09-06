# Dossiê — Módulo 45: Ressoador espectral multibanda (`FORMANT`)

**Família:** TRANSFORM
**Estado:** **implementado — Onda B** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Formant.hpp`, `tests/test_formant.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda B, #45)

## Estado da implementação

O `PARAMETRIC` é um EQ **estático** (biquads em série). `FORMANT` é o
oposto: **5 passa-faixas em paralelo** cujas frequências/bandas/ganhos
seguem uma **tabela de vogais** e são varridos por um único knob — o
espectro **fala**.

- **`vowel`** (0–1, + CV) = posição na sequência **A → E → I → O → U**.
  Cada vogal tem 5 formantes (frequência, ganho relativo, largura de
  banda — dados fonéticos medidos, voz de baixo, do domínio público).
  Interpola frequência em log, ganho em dB, banda linear entre as vogais
  vizinhas.
- **`shift`** (−1..1) = escala TODAS as frequências de formante por
  `2^(shift·1,5)` (≈ 0,35× a 2,8×) — comprimento do trato vocal
  (tamanho / "gênero"), o *formant shift* clássico.
- **`res`** (0–1) = estreita as bandas (`bw / (1 + res·8)`) — de coloração
  sutil (0) a bandas que **cantam/apitam** (1). `Q = f / bw`.
- **`mix`** (0–1) = seco ↔ ressoado. Em 0 passa direto; em 1 só o banco.
- **`drift`** (0–1, desvio Rasgo) = cada formante ganha um wobble lento e
  independente (±`drift`·3 %) de senóides incomensuráveis — a voz
  "respira". **Determinístico**, sem RNG.

Núcleo: 5× SVF TPT (Simper/Cytomic), mesmo do `FILTER`/`WASP` —
não-linearidade **no laço** (satura o estado, não só a saída) pra o banco
segurar `res` alto sem estourar. Saída de banda `v1` normalizada pelo
`k` (ganho de pico do SVF ≈ 1/k) × ganho da vogal, somada, `softLimit` no
fim.

Sem alocação/lock/IO em `process()` (sem tabelas alocadas — os dados de
vogal são `constexpr`). Determinístico sempre.

**Sem entrada → silêncio** (é TRANSFORM, como o `FILTER`). Modo "voz que
fala sozinha": `res` alto + qualquer excitação do patch (ruído, um
`OSC`) + `vowel` de um `LFO`/`SEQUENCE`.

**Testes (Debug + Release):** `vowel` em "A" → picos espectrais perto de
600/1040/2250 Hz (F1–F3 da vogal A); `vowel` em "I" → F1 baixo (~250) e
F2 alto (~1750) — a assinatura de /i/; varrer `vowel` move o pico de F2
monotônico de A pra I; `shift` positivo empurra todos os picos pra cima
(centroide sobe); `res` alto estreita os picos (energia fora das bandas
cai) sem estourar; `mix=0` = passa-direto bit-exato; `drift=0` → dois
renders byte-idênticos; entrada de ruído branco → saída limitada e
finita em todos os extremos.

**Pendências (candidatos):** modo **vocoder** (portadora externa + N
bandas de análise/síntese); excitador interno (buzz glotal) pra virar
voz-fonte; 6ª banda / nasal; mais vogais e ditongos; `vowel` 2D (grade
de vogais, não só a sequência); *unvoiced*/sopro por banda; tracking de
pitch do F0 pra o "canto" seguir a nota.

---

## 1. Problema musical e papel no fluxo

A voz sintética, o "talkbox", o pad que pronuncia vogais, o Serge ResEQ
tocado como instrumento — não sai de `OSC → FILTER`. O `FILTER` tem um
pico; a voz tem cinco, em posições que **definem** a vogal. `FORMANT` põe
esse banco no patch: entra áudio, sai áudio, e `vowel`/`shift` são o
gesto de "falar". Como os dois aceitam CV, `SEQUENCE → vowel` toca uma
melodia de vogais e `ENVELOPE → res` faz a voz "apertar" no ataque.

Distinção: o `PARAMETRIC` é EQ estático em série (esculpe uma vez); o
`FILTER`/`WASP` têm um polo/pico; o `MATTER`/`STRING` são ressoadores
modais **excitados** (voz-fonte). `FORMANT` é o banco de formantes
paralelo, varrido — processa o que passa.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Teoria fonte-filtro da voz** (Gunnar Fant, 1960) — a voz = fonte
  glotal × filtro do trato vocal; as vogais são posições de formante.
  Domínio público.
- **Tabelas de formante de vogais cantadas** — os valores medidos
  (F1–F5, ganho, banda por vogal e tipo de voz) são os das tabelas
  publicadas (Csound `fof`/`formant`, "Appendix on formant values" —
  fato fonético, não código). Aqui: a linha de voz de **baixo**.
- **Frap Tools Fumana** (*referência funcional* ★, `PESQUISA §7 #19`) —
  banco de passa-faixas de Eurorack tocável, com *spectral animator*.
  Fechado; só o **conceito** (banco paralelo varrido, não série).
- **Random*Source Serge Resonant EQ** (★, `PESQUISA §7 #51`) — o ResEQ
  original: bancos de passa-faixas de ganho fixo tocados como
  instrumento. Circuito documentado no DIY Serge.
- **4ms SMR (Spectral Multiband Resonator)** — banco de 6 ressoadores
  afináveis com morph e rotação.
- **SVF TPT** (Simper/Cytomic) — o núcleo já usado no `FILTER`/`WASP`
  (paper público).

**Desvio Rasgo (Atlas §49):** o `drift` de wobble determinístico por
formante (a voz respira sem RNG), e a relação — `vowel`/`shift`/`res`
todos endereçáveis por CV do grafo, então a "fala" é um **processo**
dirigido por outros módulos.

## 3. Modelo

**Dados** (`constexpr`, 5 vogais × 5 formantes): frequência (Hz), ganho
(dB rel. a F1), largura de banda (Hz).

**Por amostra:**
```
vw   = clamp01(vowel_knob + vowel_in) · 4      (posição em [A,E,I,O,U])
seg  = ⌊vw⌋ ; t = vw − seg                     (vogais seg e seg+1)
sh   = 2^( clamp(shift_knob + shift_in, −1, 1) · 1,5 )
res  = clamp01(res_knob)
wet  = 0
para k em 0..4:
  fk  = exp( lerp(ln f[seg][k], ln f[seg+1][k], t) ) · sh · (1 + drift_k)
  fk  = clamp(fk, 20, 0,45·sr)
  bwk = lerp(bw[seg][k], bw[seg+1][k], t) / (1 + res·8)
  gk  = dB→lin( lerp(g[seg][k], g[seg+1][k], t) )
  Kk  = clamp(bwk / fk, 0,02, 2)                (k do SVF ≈ 1/Q)
  bnd = svf[k].run(x, fk, Kk, sr, Band)         (tap v1)
  wet += bnd · Kk · gk                          (× Kk normaliza o pico ≈ 1/Kk)
out = softLimit( x·(1−mix) + wet·mix·1,6 )
```

**`drift`:** 5 fases lentas a `{0,017; 0,023; 0,031; 0,037; 0,043}` Hz;
`drift_k = 0,03 · drift · sin(2π · fase_k + k·1,3)`.

**Extremos:** `shift` no máximo + vogal "I" → F5 ≈ 3340·2,8 ≈ 9,3 kHz
(ok a 48 k); clamp a 0,45·sr pega o caso patológico. `res=1` → `Kk` no
piso 0,02 → bandas muito estreitas, quase apitando; a saturação no laço
do SVF segura. `mix=0` → `wet` não entra, saída = `x` (bit-exato).
Entrada em silêncio → saída em silêncio (sem excitador interno nesta v1).
Ruído branco na entrada, `res=1` → 5 tons senoidais nas frequências dos
formantes, `softLimit` no fim.

## 4. Três modos obrigatórios

- **autônoma:** com `res` alto e `drift`, as bandas ressoam qualquer
  excitação do patch (o ruído de fundo de um `OSC`, um `NOISE`) e o
  `vowel` de um `LFO` faz o timbre "falar" sozinho.
- **performance:** `vowel` e `shift` são o gesto de fala; `res` o quão
  "apertada" a voz soa.
- **híbrida:** `SEQUENCE → vowel` (melodia de vogais), `ENVELOPE → res`
  (a voz aperta no ataque), áudio de qualquer voz do grafo na entrada.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `vowel` (Control), `shift` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:** `vowel` (0–1, def 0), `shift` (−1..1, def 0), `res`
(0–1, def 0,4), `mix` (0–1, def 1), `drift` (0–1, def 0).
**Limites:** `out` limitado por `softLimit` (linear até 0,8, `tanh`
depois — o mesmo do `FILTER`). CPU: por amostra 5× SVF (5 `tan` + ~8
mul-add cada) + 5 `sin` (drift) + 5 `exp`. Sem alocação.

## 6. Alternativas descartadas

- **biquad peak (RBJ) em vez de SVF** — o SVF TPT afina limpo em toda a
  faixa (o biquad "aperta" perto de Nyquist) e o `FILTER`/`WASP` já o
  usam; reaproveitar.
- **tabela de vogais alocada / carregável** — os dados são fato
  fonético fixo; `constexpr` no header, sem I/O, determinístico.
- **excitador glotal interno** (virar SOURCE) — quebra "TRANSFORM"; fica
  como pendência (`FORMANT` + buzz = uma voz). O `MATTER`/`STRING` já
  cobrem ressoador-fonte.
- **modo vocoder já** — precisa de N bandas de análise + envelope
  follower por banda; é um módulo maior. Pendência anotada.
- **`vowel` 2D (grade)** — a sequência A→E→I→O→U cobre o gesto de fala
  clássico e cabe num knob; a grade fica como evolução.

## 7. Integração e painel

14 HP, família **TRANSFORM** (junto de `FILTER`/`WASP`/`PARAMETRIC`).
Display do espectro (o `SCOPE` mostra). Knobs `VOWEL`/`SHIFT`/`RES`/`MIX`
(linha 1), `DRIFT` (linha 2); jacks `IN`/`VOW`/`SHF` + `OUT`.

Cadeias canônicas: `OSC.saw → FORMANT`, `LFO → FORMANT.vowel` (pad que
fala); `NOISE → FORMANT`, `res` alto (5 tons por vogal — coro sintético);
`SEQUENCE → FORMANT.vowel` (melodia de vogais sobre um drone).
