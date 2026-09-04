# Dossiê — Módulo 5: Clock euclidiano (`CLOCK` / `EuclidClock`)

**Família:** TIME
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/EuclidClock.hpp` (classe `EuclidClock`, `type()` =
`"CLOCK"` — o nome de classe evita colisão com o `Clock` legado de
`Graph.hpp`), `tests/test_clock.cpp`

## Estado da implementação (marco 1)

Feito: o **tempo** do patch. Três saídas de gate:

- **`clock`** — pulso de passo estável em `bpm · mult` (`mult` = passos
  por tempo, 0,25–8). Gate alto pela fração `gate_len` de cada passo.
- **`euclid`** — gate euclidiano: `fill` pulsos distribuídos o mais
  uniformemente possível numa grade de `length` passos, com `rotate`.
  Fórmula de Bresenham `((i+rotate)·fill) mod length < fill` — O(1), sem
  buffer, equivalente cíclico ao Bjorklund pros casos musicais.
- **`accent`** — nasce do **AND/OR de dois divisores** (`accent_a`,
  `accent_b`, `accent_mode`): acento quando o passo é múltiplo de A
  *e/ou* de B. Polirritmia "de graça" (vpme Euclidean Circles).

`swing` (0–1) atrasa os passos ímpares dentro da própria fatia.
`drift` (0–1) faz random-walk lento no andamento efetivo (±~12%, mesmo
princípio do `drift` do Módulo 1). `drift = 0` → determinístico.

Avanço por **evento**: clock interno em `bpm·mult` OU, se `ext_clock`
estiver conectado, nas bordas de subida externas — o período é estimado
do intervalo entre as duas últimas bordas, pra `gate_len`/`swing`
continuarem coerentes. `reset` (borda) zera o contador de passos e a
fase.

**Testes (6/6 alvos, Debug + Release):** andamento interno bate
(120 BPM ×2 → ~4 Hz / 16 passos em 4 s; 60 BPM ×1 → ~1 Hz);
`fill = 32` → euclid em todo passo; `E(3,8)` → ~3/8 dos passos são onset
(30 ± 2 em 80 passos); acento OR de divisores 4 e 3 em [0,24) = 12
eventos; acento AND = múltiplos de 12 = 2 eventos; `fill = 0` isola o
acento (nenhum onset euclidiano); `drift = 0` byte-idêntico entre
renders, `drift > 0` reprodutível com a mesma seed; `ext_clock` a ~10 Hz
→ euclid segue as bordas (±2 em ~50); `reset` reinicia o padrão (passo 0
é onset); 4000 blocos estéreo com ext+reset+swing+drift, todas as saídas
∈ {0,1}, sem alocação; integração no grafo
(`CLOCK.euclid → DECISION.trigger → FILTER`); painel fecha (16 HP).

**Pendências (candidatos, não controles fictícios):** Bjorklund real
(a fórmula de Bresenham difere do Bjorklund canônico numa minoria de
`(k,n)` — rotação diferente, mesma densidade); saída de "fim de ciclo";
divisão fracionária real (hoje `mult` contínuo mas o gate é por passo);
humanização por passo (micro-timing gaussiano além do `drift` global);
`euclid` com dois planos independentes (Euclidean Circles tem 3);
sincronização a um transporte global (Ensemble Bus — G4).

---

## 1. Problema musical e papel no fluxo

Módulos 1–4 dão fonte, timbre, relação e decisão — mas nada os organiza
no tempo. Numa lógica generativa o ritmo não deveria ser um piano-roll:
deveria ser um **campo** de onde padrões emergem. O euclidiano
(distribuir `k` pulsos em `n` passos o mais uniformemente possível) gera
quase todas as células rítmicas tradicionais do mundo a partir de dois
números; o AND/OR de divisores gera polirritmia sem programar nada; a
deriva faz o andamento respirar. O `CLOCK` é essa fonte de tempo.

Papel: `euclid`/`accent`/`clock` disparam o `trigger` do `DECISION`
(módulo 4) e os envelopes (módulo 6); `reset` re-ancora tudo; o próprio
`CLOCK` pode ser escravo de outro clock (ou, no futuro, do transporte do
Ensemble Bus).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **G. Toussaint**, "The Euclidean Algorithm Generates Traditional Musical Rhythms" (2005) + **Bjorklund** | E(k,n): k pulsos distribuídos o mais uniformemente possível em n passos = tresillo, cinquillo, clave, etc. | teoria pública |
| **vpme Euclidean Circles v2 / QD** (hardware) | euclidiano por canal + **AND/OR entre divisores** pra acento polirrítmico; rotação | hardware, estudo de comportamento |
| **ALM Pamela's PRO Workout** (hardware) | clock mestre com divisões/multiplicações, swing, e **"random"/drift como parâmetro** do próprio andamento | hardware |
| **Mutable Grids** (`pichenettes/eurorack`) | ritmo como mapa contínuo (densidade, "chaos") em vez de passos programados | MIT — conceito |
| **Bresenham / DDA** | `(i·k) mod n < k` gera a mesma distribuição do euclidiano em O(1) sem construir a sequência | algoritmo público |

## 3. Modelo — matemática, estados, extremos

**Passo interno.** `stepHz = clamp(bpm + bpm_mod, 1, 1000)/60 · mult ·
(1 + driftState)`. Fasor `phase += stepHz/sr`; no wrap, `advanceStep()`.
`phaseInStep = phase`.

**Passo externo** (`ext_clock` conectado). Borda de subida →
`advanceStep()`, `extPeriod = extSamples` (se já viu uma borda antes),
`extSamples = 0`. A cada amostra `extSamples += 1`;
`phaseInStep = clamp(extSamples/extPeriod, 0, ~1)`.

**Onset euclidiano.**
```
fill <= 0        -> nunca
fill >= length   -> sempre
senão: j = (i + rotate) mod length;  (j·fill) mod length < fill
```

**Acento.** `hitA = step mod accent_a == 0`,
`hitB = step mod accent_b == 0`; `accent_mode` → `hitA && hitB` (AND) ou
`hitA || hitB` (OR).

**Janela de gate.** `swOff = (i ímpar) ? swing·0,45 : 0`;
`win = swOff ≤ phaseInStep < swOff + gate_len`. `clock` usa
`phaseInStep < gate_len` (sem swing). `euclid`/`accent` = `onset && win`.

**Deriva.** A cada `sr/20` amostras: `driftState += ruído·(0,02·drift²)`,
limitado a ±0,12. rng xorshift semeado em `prepare()`.

**Estado.** `phase` (double), `stepCounter` (int, com wrap em 720720 ~
LCM(1..16) pra preservar a fase de acento e euclidiano),
`phaseInStep`, `prevExt`/`prevReset`, `extSamples`/`extPeriod`/`extSeen`,
`driftState`/contador, rng. Sem alocação.

**Extremos.** `bpm` mínimo + `mult` mínimo → passo a cada ~12 s, fasor
`double` ok. `mult` máximo + `bpm` máximo → 40 Hz de passo, ainda
control-rate. `ext_clock` com primeira borda só: `extSeen` falso →
`phaseInStep` usa `extPeriod` default (sr/4) até a 2ª borda calibrar.
`length = 1` → um passo, `fill ≥ 1` → gate contínuo pulsado. `reset` que
fica alto → só a borda conta.

## 4. Três modos obrigatórios

- **Autônoma:** `bpm` + `mult` + `fill`/`length` já produzem um ritmo;
  `drift > 0` faz o andamento respirar sem nenhuma entrada — a peça tem
  pulso vivo de graça.
- **Performance:** `fill`, `rotate`, `swing`, `mult` são macros
  gestuais — subir `fill` adensa; `rotate` desloca o acento percebido;
  `swing` humaniza. `accent_a`/`accent_b` são um gesto de "liga
  polirritmia".
- **Híbrida:** `ext_clock` escraviza a outro clock (ou ao clock de outro
  instrumento no Ensemble Bus); `reset` re-ancora na estrutura;
  `bpm_mod` deixa um LFO lento fazer accelerando/ritardando.

## 5. Portas, parâmetros, limites

**Entradas:** `ext_clock` (Control, borda → avança um passo),
`reset` (Control, borda → zera contador e fase), `bpm_mod` (Control, soma
linear ao BPM).
**Saídas:** `clock`, `euclid`, `accent` (todas Control, gate 0/1).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `bpm` | 20–300 | 120 | andamento base |
| `mult` | 0,25–8 | 2 | passos por tempo |
| `length` | 1–32 | 8 | tamanho da grade euclidiana |
| `fill` | 0–32 | 4 | nº de pulsos euclidianos |
| `rotate` | 0–31 | 0 | rotação do padrão |
| `swing` | 0–1 | 0 | atraso dos passos ímpares |
| `drift` | 0–1 | 0 | random-walk no andamento |
| `gate_len` | 0,05–0,95 | 0,5 | largura do gate (fração do passo) |
| `accent_a` | 1–16 | 4 | 1º divisor do acento |
| `accent_b` | 1–16 | 3 | 2º divisor do acento |
| `accent_mode` | 0/1 | 0 | OR (0) / AND (1) |

**Limites:** saídas ∈ {0,1}. CPU: por amostra só o fasor + 3 modulos +
o passo aleatório da deriva (raro). Sem alocação, estado fixo.

## 6. Alternativas descartadas

- **Bjorklund canônico (recursão / vetor):** aloca ou precisa de buffer
  de trabalho; a fórmula de Bresenham dá a mesma densidade e quase
  sempre a mesma sequência (a menos de rotação). Bjorklund exato fica
  como 2ª camada se o ouvido pedir.
- **Piano-roll / sequenciador de passos:** é outra família (Hexen §119,
  "sequenciador como família de comportamentos"); o `CLOCK` é campo, não
  partitura.
- **Multi-canal euclidiano no marco 1** (Euclidean Circles tem 3): mais
  portas e estado; um plano euclidiano + um de acento já dá polirritmia.
  Candidato.
- **Humanização gaussiana por passo:** útil, mas o `drift` global já dá
  o "respirar"; micro-timing por passo é 2ª camada.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** frequência de `clock` bate com `bpm·mult` (±3%);
densidade de `euclid` = `fill/length` (±3%); acento OR/AND bate com a
teoria dos divisores; `drift = 0` byte-idêntico; `ext_clock` → um passo
por borda; `reset` reinicia; saídas ∈ {0,1}; sem alocação (teste
dedicado).

**Escuta:** `E(5,8)`, `E(7,16)` soam como as células que a gente
reconhece? subir `fill` ao vivo adensa de forma musical ou mecânica? o
acento polirrítmico "amarra" o groove ou briga com ele? `drift` baixo
faz a máquina soar viva sem soar quebrada? `CLOCK → DECISION → FILTER` já
é uma música com pulso?

## 8. Integração e painel

Classe `EuclidClock` (`type()` = `"CLOCK"`), 3 entradas, 3 saídas, 11
parâmetros. `panel()` próprio (16 HP: display do padrão + BPM/MULT/LEN/
FILL/ROT numa fileira, SWING/DRIFT/GATE/ACC A/ACC B noutra, toggle AND +
jacks). Testado isolado (contagem de bordas, densidade, divisores,
deriva) antes do patch. Entra na primeira peça longa como a fonte de
tempo: `CLOCK` → `trigger` do `DECISION` e dos envelopes (módulo 6).
