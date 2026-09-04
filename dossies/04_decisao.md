# Dossiê — Módulo 4: Decisão (`DECISION`)

**Família:** DECISION
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Decision.hpp`, `tests/test_decision.cpp`

## Estado da implementação (marco 1)

Feito: gerador de **CV e gate estocásticos** que avança por eventos —
`trigger` externo (borda de subida) ou, quando `trigger` não está
conectado, um relógio interno em `rate`. A cada passo:

- **`x`** — CV bipolar (−1..1), amostrado-e-segurado, com `slew`
  opcional (0 = degrau; >0 = glide até ~500 ms — a mesma cola de
  theremin do `RASGO_SYNTH`, `reference_rasgo_synth_generative_techniques`);
- **`y`** — segunda CV, sorteio independente com a mesma distribuição
  (duas modulações decorrelacionadas de um módulo só, como os canais de
  Sapèl);
- **`gate`** — decisão de Bernoulli: `1` com probabilidade `bias`, senão
  `0`, segurada até o próximo passo (Branches).

**Distribuição configurável (`shape`, princípio Sapèl).** `shape = 0` →
uniforme; `shape = 1` → sino (média de 4 uniformes, mesma média 0,5).
Interpolação linear entre os dois. `spread` escala a excursão em torno
do centro (0 → sempre no zero; 1 → faixa cheia). `steps` quantiza `x`/`y`
em N níveis (1 = contínuo; 2..32 = escada — de S&H suave a sequência de
alturas).

**Déjà-vu (`dejavu`, `loop_length`, princípio Marbles).** Buffer circular
dos últimos 16 valores. A cada passo, com probabilidade `dejavu`, em vez
de sortear novo o módulo **relê o valor de `loop_length` passos atrás** e
o reescreve — em `dejavu = 1` com o buffer cheio isso trava um laço de
comprimento `loop_length` que repete para sempre; em `dejavu = 0` é
sempre novo; no meio, um laço que às vezes muta.

**Determinístico:** xorshift64* semeado em `prepare()` (constante fixa —
sem entrada externa de seed no marco 1). Dois renders com os mesmos
parâmetros são byte-idênticos.

**Testes (previstos 4/4, 3 configs):** `bias = 0` → gate sempre 0;
`bias = 1` → sempre 1; `bias = 0,5` → 50% ±5% em 2000 passos;
`spread = 0` → `x` ≡ 0; `spread = 1`, `shape = 0` → `x` quase-uniforme em
[−1,1] (média ~0, desvio ~0,58 ± tolerância); `shape = 1` → mesmo alcance,
desvio menor (sino); `steps = 4` → `x` só assume 4 valores;
`dejavu = 1` após encher o buffer → `x` repete com período `loop_length`;
`slew > 0` → `x` nunca salta mais que o passo permitido por amostra;
determinismo byte-idêntico; sem alocação em `process()`; integração no
grafo (`DECISION.x` → `cutoff_mod` de um `FILTER`).

**Pendências (candidatos, não controles fictícios):** entrada de seed /
`reset` explícito; distribuição verdadeiramente gaussiana (Box-Muller) em
vez de soma de uniformes; correlação ajustável entre `x` e `y` (hoje
independentes); quantizador a uma escala musical (Marbles `t`/Sinfonion —
2ª camada, cruza com o futuro módulo de harmonia); modo "evolving" com
taxa de mutação por passo separada do `dejavu`.

---

## 1. Problema musical e papel no fluxo

Música generativa precisa de uma fonte de **escolha**: o que muda, quando
muda, quanto muda. Um LFO é previsível; ruído puro é sem forma. O ponto
médio — aleatoriedade *domada*, com distribuição, correlação e memória —
é o que Branches (Bernoulli), Sapèl (distribuição uniforme→gaussiana) e
Marbles (déjà-vu, loop-lock) trouxeram pro modular. O `DECISION` é esse
ponto médio num módulo: decide (gate), sorteia (CV com forma) e lembra
(laço travável).

Papel: alimsenta `rate_mod`/`cutoff_mod`/`spread_mod` dos módulos 1 e 2,
dispara envelopes (módulo 6), e o `gate` roteia caminhos (com a condução
probabilística do `Cable`, dois níveis de acaso). É o coração generativo
do primeiro patch longo.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Branches** (`pichenettes/eurorack`) | portão de Bernoulli: um trigger de entrada sai por A ou B conforme uma moeda enviesada (`bias`); o enviesamento é o controle | STM32 = MIT — estudo, sem código |
| **Mutable Marbles** | aleatoriedade *estruturada*: `déjà-vu` (relê de uma memória circular → de novo aleatório a laço travado), `spread`, distribuição, quantização de `t` | idem |
| **Frap Tools SAPÈL** (hardware) | random *domada*: distribuição de probabilidade contínua (uniforme → gaussiana) num potenciômetro; dois canais com S&H, slew e ruído | hardware, estudo de comportamento |
| **Bastl Déjà Vu / mylar RANDOM8** (hardware) | máquina de estado da CV: aleatório → looping → evolving → travado | conceito |
| **Central limit** (soma de uniformes → normal) | jeito barato e sem `log`/`cos` de ir de uniforme a sino sem sair do RT | matemática pública |

## 3. Modelo — matemática, estados, extremos

**Avanço por evento.** `trigger` conectado → borda de subida
(`prev < 0.5 && cur >= 0.5`) dispara um passo. `trigger` NÃO conectado
(`inputs[trigger] == nullptr`) → fasor interno `phase += rate/sr`, passo
no wrap. Nunca os dois ao mesmo tempo (patch manda).

**Sorteio de um valor (`drawValue`).**
```
u  = uniform01()
se shape > 0:
    bell = (uniform01()+uniform01()+uniform01()+uniform01()) / 4
    u = (1-shape)·u + shape·bell           // média 0,5 nos dois
c = (u - 0.5) · 2                            // [-1,1], centrado
c = c · effSpread                            // effSpread = clamp(spread + spread_mod)
se steps >= 2:
    níveis igualmente espaçados em [-1,1]:
    c = round( (c*0.5+0.5) · (steps-1) ) / (steps-1) · 2 - 1
```

**Déjà-vu.** Buffers `xHist[16]`, `yHist[16]`, `gHist[16]`, cabeça `head`,
contador `filled`. No passo:
```
se filled >= loopLen  e  uniform01() < effDejavu:
    idx = (head - loopLen + 16) % 16
    x = xHist[idx];  y = yHist[idx];  g = gHist[idx]
senão:
    x = drawValue();  y = drawValue();  g = (uniform01() < effBias) ? 1 : 0
xHist[head]=x; yHist[head]=y; gHist[head]=g
head=(head+1)%16;  filled = min(filled+1, 16)
```
`loopLen = clamp(round(loop_length), 1, 16)`.

**Slew.** Alvos `xTarget`/`yTarget` do passo; por amostra
`x += (xTarget - x) · slewCoeff`, com
`slewCoeff = 1 - exp(-1 / (tau·sr))`, `tau = slew · 0.5 s`. `slew = 0` →
`x = xTarget` (degrau). `gate` nunca é suavizado (é lógico).

**Extremos.** `rate` no piso (0,01 Hz) → passo a cada 100 s, fasor em
`double` sem problema. `trigger` que fica alto → só a borda conta, sem
passos repetidos. `steps = 1` → sem quantização. `loop_length` >
`filled` → nunca relê (ainda enchendo). `spread`/`bias` clampados a [0,1]
depois da soma da modulação. `slew` altíssimo + `rate` alto → `x` fica
atrás do alvo (comportamento correto, não bug). Reset (`prepare`) → tudo
zero, rng resemeado, `filled = 0`.

## 4. Três modos obrigatórios

- **Autônoma:** sem `trigger` e sem entradas, o relógio interno em `rate`
  já produz `x`/`y`/`gate` que evoluem; `dejavu` entre 0,3 e 0,7 dá
  material que se reconhece sem se repetir.
- **Performance:** `bias`, `spread`, `shape`, `dejavu` são macros
  gestuais — subir `dejavu` "congela" a música ao vivo; abrir `spread`
  aumenta a amplitude do acaso; `shape` move de "nervoso" (uniforme) a
  "centrado" (sino). `steps` é um gesto discreto (contínuo ↔ escada de
  alturas).
- **Híbrida:** `trigger` vindo do clock (módulo 5) sincroniza as
  decisões ao ritmo; `bias_mod`/`spread_mod` deixam outro sinal do patch
  dirigir o acaso (um LFO lento abrindo e fechando o `spread` = seções).

## 5. Portas, parâmetros, limites

**Entradas:** `trigger` (Control, borda de subida — avança um passo),
`bias_mod` (Control, soma a `bias`), `spread_mod` (Control, soma a
`spread`).
**Saídas:** `x` (Audio/CV bi, −1..1), `y` (Audio/CV bi, −1..1),
`gate` (Control, 0/1).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `rate` | 0,01–50 Hz (log) | 2 | clock interno quando `trigger` não está patchado |
| `bias` | 0–1 | 0,5 | P(gate = 1) — Bernoulli |
| `spread` | 0–1 | 0,6 | excursão de `x`/`y` em torno do centro |
| `shape` | 0–1 | 0 | uniforme → sino |
| `steps` | 1–32 | 1 | quantização de `x`/`y` (1 = contínuo) |
| `slew`  | 0–1 | 0 | glide de `x`/`y` (0..~500 ms) |
| `dejavu` | 0–1 | 0 | P(reler da memória em vez de sortear) |
| `loop_length` | 1–16 | 8 | período do laço quando `dejavu` alto |

**Limites:** saídas em [−1,1] (`x`/`y`) e {0,1} (`gate`) por construção.
CPU: por amostra só o slew (2 mult/add); o sorteio é por passo (raro).
Sem alocação. Estado fixo: 3 buffers de 16 floats + fasor + rng + alvos.

## 6. Alternativas descartadas

- **Gaussiana real (Box-Muller / ziggurat):** `log`+`sqrt`+`cos` por
  sorteio, e o sino da soma de 4 uniformes é indistinguível na prática
  pra modulação. Central limit fica; Box-Muller é candidato se o ouvido
  pedir cauda mais longa.
- **Quantizar a uma escala musical já no marco 1:** `steps` iguala
  espaçamento em tensão, não em semitons. O quantizador harmônico é
  outro módulo (cruza com o futuro de harmonia / `RASGO_SYNTH` escalas).
- **`x` e `y` correlacionados por um controle:** útil (Sapèl tem), mas
  mais estado e mais um parâmetro; independentes já entregam duas
  modulações. 2ª camada.
- **Um módulo só de "gate" e outro só de "CV":** contra a regra de
  profundidade — decisão, sorteio e memória são o mesmo gesto musical.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `bias` calibrado (frequência de `gate=1` bate com `bias`
±5% em 2000 passos); `spread=0` → `x` idêntico a zero; histograma de `x`
com `shape=0` plano, com `shape=1` com pico central; `steps=N` → `x`
assume exatamente N valores; `dejavu=1` → autocorrelação de `x` com lag
`loop_length` = 1,0; `slew` limita a derivada de `x`; dois renders
byte-idênticos; sem alocação (teste dedicado).

**Escuta:** `dejavu` subindo ao vivo soa como a música "se lembrando" ou
como um glitch? a distribuição `shape` muda o *caráter* da modulação de
forma audível? `x` com `slew` e `steps` juntos soa como uma linha
melódica ou como uma escada mecânica? o módulo sozinho (autônomo) já dá
vontade de conectar em algo?

## 8. Integração e painel

Classe `Signal` (`type()` = `"DECISION"`), 3 entradas, 3 saídas, 8
parâmetros. `panel()` próprio (14 HP: display do histograma/últimos
valores + `RATE`/`BIAS`/`SPREAD`/`SHAPE` numa fileira, `STEPS`/`SLEW`/
`DEJAVU`/`LOOP` noutra, jacks embaixo). Testado isolado (histogramas,
autocorrelação, calibração de `bias`) antes do patch. Entra no primeiro
render longo como a fonte de decisão: `DECISION` (clock interno) →
`cutoff_mod` do `FILTER` e `gate` → envelope (módulo 6).
