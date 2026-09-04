# Dossiê — Módulo 23: Sample & Hold duplo (`SH`)

**Família:** UTILITY
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/SampleHold.hpp`, `tests/test_sample_hold.cpp`
**Candidato registrado:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `NOISE` tem **uma** saída `sh`. Um S&H **duplo com canais
correlacionáveis** é instrumento por si — a dupla que gera uma altura e
um timbre relacionados, ou duas vozes de uma constelação (Marbles `X`).

- **2 canais** independentes: cada um segura o valor de `inN` (se
  conectado) ou da própria fonte de acaso interna, no pulso de `trigN`
  (borda ↑) ou do relógio interno em `rate`;
- **`trackN`** (0/1) — se 1 **e** `trigN` conectado: *track & hold* — a
  saída segue `inN` enquanto o gate está alto, congela quando cai
  (chopper de áudio, sample de um LFO no ponto certo);
- **`slewN`** (0–1) — desliza até o valor segurado em vez de saltar
  (Buchla 266 "smooth random"); `slew = 0` → degrau duro;
- **`slope`** (−1..1, def 0) — troca o tempo de SUBIDA pelo de DESCIDA
  (`up = base·(1+slope)`, `dn = base·(1−slope)`); `>0` = desce devagar /
  sobe rápido (portamento de pluck), `<0` = o oposto; `0` = simétrico,
  idêntico ao antigo. Vale pros dois canais;
- **`spread`** (0–1) — a fonte de acaso interna vai de **uniforme** (0) a
  **sino** (1, média de 4 uniformes) — acaso *estruturado*, padrão
  `shape` do `DECISION`;
- **`correlation`** (−1…+1) — como o acaso interno do canal 2 se
  relaciona com o do canal 1: `+1` idêntico, `0` independente, `−1`
  espelhado (Marbles `X`-spread). **Não** afeta o `in` externo.

Dois streams xorshift semeados em `prepare()` → determinístico (dois
renders byte-idênticos). Sem alocação / lock / IO em `process()`.

**Testes (12 funções, Debug + Release — `tests/test_sample_hold.cpp`):** S&H só muda no pulso (nº de
degraus = nº de pulsos) e segura entre pulsos; amostra `in` quando
conectado, o acaso interno quando não; `slew` alto → derivada por
amostra pequena; `slew = 0` → degrau; `slope > 0` + onda quadrada →
descida bem mais rápida que a subida (assimétrico); `track = 1` + gate alto → a saída
acompanha `in`, congela no gate baixo; `spread = 1` concentra o acaso
perto de 0 (variância menor que uniforme); `correlation = 1` → canais
idênticos, `−1` → espelhados, `0` → descorrelacionados; relógio interno
(`rate`, sem `trig`) gera degraus sozinho; tudo finito; dois renders
byte-idênticos; grafo `CLOCK → SH.trig1` · `SH.out1 → OSC.pitch`.

**Pendências (candidatos):** modo
"cascade" (o canal 2 amostra a saída do canal 1 — shift register de 2);
gate aleatório por pulso (Bernoulli — o `DECISION` já faz); N canais
(shift register longo, Doepfer A-152).

---

## 1. Problema musical e papel no fluxo

Todo patch generativo quer duas tensões que se movem em relação: uma
altura e um cutoff que "combinam", dois osciladores que derivam juntos,
uma pergunta e uma resposta. O `NOISE.sh` dá uma. Duas fontes
**independentes** dão ruído descorrelacionado; o que falta é o **botão
de correlação** — de "gêmeos" a "espelho" a "cada um pra seu lado".

Papel: utilidade, entre um gatilho e um par de destinos. `CLOCK → SH` ·
`SH.out1 → QUANTIZER` · `SH.out2 → FILTER.cutoff_mod` (melodia + timbre
relacionados). `SH` sozinho (`rate`) = duas fontes de CV aleatória
correlacionáveis.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Sample-and-hold clássico** (Buchla 265/266, Doepfer A-148) | segura o valor de uma fonte até um gatilho | prática |
| **Buchla 266 "smooth random"** | glide até o alvo em vez de degrau (`slew`) | conceito |
| **Mutable Marbles — `X` / spread** | dois canais de acaso com correlação ajustável (gêmeos ↔ espelho) | conceito |
| **Track & hold** (qualquer texto de amostragem) | seguir a entrada enquanto o gate está alto, congelar quando cai | teoria pública |
| **`shape` do `DECISION` do Rasgo** | uniforme→sino pela média de N uniformes = acaso estruturado | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra, canal *k* ∈ {1, 2}:
```
# fonte de pulso: externa se `trigN` conectada, senão relógio interno
if trig_k conectada: edge = (trig_k ≥ 0.5) && !prevTrig_k
else:                intPhase_k += rate/sr ; edge = wrap(intPhase_k)

# acaso interno (só quando `in_k` livre):
u1 = shapedDraw(rng1, spread)                       # canal 1
u2i = shapedDraw(rng2, spread)                      # canal 2, independente
u2 = correlation ≥ 0 ? lerp(u2i, u1,  correlation)
                     : lerp(u2i, −u1, −correlation) # espelhado

if track_k && trig_k conectada && (trig_k ≥ 0.5):
    held_k = in_k[frame]                            # track & hold
elif edge:
    held_k = in_k conectada ? in_k[frame] : (k==1 ? u1 : u2)

# saída: degrau ou glide, com subida ≠ descida por `slope`
base = slew_k² · 2 s
up   = base·(1+slope) ; dn = base·(1−slope)
diff = held_k − y_k
y_k += diff · (1 − exp(−dt / (diff ≥ 0 ? up : dn)))   # (=1 se o tempo < dt)
out_k = y_k
```
`shapedDraw(rng, s)` = `lerp(uniform[−1,1), meanOf4Uniforms, s)`.

**Estados:** `prevTrig_[2]`, `intPhase_[2]`, `held_[2]`, `y_[2]`,
`rng1_`, `rng2_`. Sem alocação.

**Extremos.** Sem `trig` e `rate → 0` → S&H congela no último valor
(correto). `slew = 1` + pulsos rápidos → `y` mal sai do lugar (vira
quase DC). `track = 1` + `in` de áudio → a saída é o áudio enquanto o
gate está alto (chopper) e congela na última amostra quando cai.
`correlation = 1` → os dois canais são o mesmo S&H (redundante de
propósito, útil pra somar depois). `in` com DC → segura o DC. Reset →
`held_`, `y_`, fase interna zerados, RNGs re-semeados.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado; `rate` + `spread` + `correlation` → duas
  CVs aleatórias que se movem em relação (de gêmeas a opostas). Sem
  `trig` e `rate` baixo = deriva lenta de fundo.
- **Performance:** `rate`/`spread` ao vivo mudam o caráter (metralhadora
  ↔ passos deliberados); `slew` abre/fecha a "cola"; `correlation` gira
  a relação entre os dois destinos sem repatch.
- **Híbrida:** `CLOCK`/`SEQUENCE`/`DECISION` no `trig`; `out1` →
  `QUANTIZER` (melodia travada), `out2` → `FILTER.cutoff_mod` (timbre
  que acompanha); `in` de um `TURING` → re-amostra o laço no pulso.

## 5. Portas, parâmetros, limites

**Entradas:** `in1`, `trig1`, `in2`, `trig2` (todas Control — CV e áudio
compartilham o tipo no Rasgo).
**Saídas:** `out1`, `out2` (Control).
**Parâmetros:** `rate` (0,02–40 Hz, def 4), `slew1`/`slew2` (0–1, def 0),
`slope` (−1..1, def 0),
`track1`/`track2` (0–1, def 0), `spread` (0–1, def 0), `correlation`
(−1…+1, def 0).
**Limites:** saídas seguem a fonte (não clampadas — CV pode passar de
±1 no Rasgo; o acaso interno fica em [−1,1)). CPU: 2 xorshift + aritmética
por amostra. Sem alocação.

## 6. Alternativas descartadas

- **Canal único:** o `NOISE.sh` já é isso; o valor do módulo É a dupla
  correlacionável.
- **`correlation` afetando o `in` externo:** correlacionar dois sinais
  externos é outra operação (matriz / soma); aqui `correlation` é só a
  relação entre os dois *acasos internos*, que é o caso do Marbles.
- **Modo shift-register já no marco 1:** "o canal 2 amostra a saída do
  canal 1" é elegante mas é outra topologia; fica candidato.
- **`slew` como filtro passa-baixa da saída:** dá degrau amaciado, não
  um glide até o alvo. O modelo alvo+coef é o do Buchla 266.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** nº de degraus na saída = nº de pulsos, e segura exatamente
entre pulsos; amostra `in` quando conectado e o acaso interno quando
não; `slew` alto → |Δ por amostra| pequeno; `slew = 0` → salto imediato
ao alvo; `track = 1` + gate alto → `out ≈ in`; gate baixo → `out`
congela; `spread = 1` → variância da saída interna < uniforme;
`correlation = 1` → `out1 == out2` (mesma fonte), `−1` → `out2 ≈ −out1`,
`0` → correlação ≈ 0; relógio interno gera degraus sem `trig`; tudo
finito; dois renders byte-idênticos.

**Escuta:** o S&H a `rate` médio dá o padrão "computador dos anos 70"?
`slew` num filtro faz o timbre "respirar" ou ficar nervoso? girar
`correlation` de +1 a −1 com `out1`→altura e `out2`→cutoff: dá pra ouvir
a relação passar de "andam juntos" a "vão pra lados opostos"? `spread`
alto faz o S&H "escolher com intenção"?

## 8. Integração e painel

Classe `SampleHold` (`type()` = `"SH"`), 4 entradas, 2 saídas, 9
parâmetros. `panel()` próprio (12 HP): knobs RATE/SPRD/SLOPE/SLW1/SLW2/
CORR, toggles TRK1/TRK2, jacks IN1/TRG1/IN2/TRG2 · OUT1/OUT2, Display
(os dois valores segurados como barras). Testado isolado (degrau,
hold, track, slew, spread, correlação, relógio interno, determinismo)
antes do patch. Cadeias canônicas: `CLOCK → SH` · `SH.out1 →
QUANTIZER` · `SH.out2 → FILTER.cutoff_mod`; `SH` sozinho = par de CVs
aleatórias correlacionáveis. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família UTILITY — junto de `VCA`/`CONTROL`).
