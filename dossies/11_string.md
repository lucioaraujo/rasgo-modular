# Dossiê — Módulo 11: Corda por guia-de-onda (`STRING`)

**Família:** MATTER
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/StringVoice.hpp`, `tests/test_string.cpp`

## Estado da implementação (marco 2)

Feito: a outra face do `MATTER`. Em vez de somar 24 modos, um **laço de
atraso com perda** — o modelo de Karplus-Strong estendido.

- **altura:** o atraso total do laço = `sr/f0` = `D` inteiro + `frac`;
  `frac` afinado por um **all-pass de 1ª ordem** (Jaffe & Smith) →
  afinação contínua sem "escadinha";
- **brilho:** filtro de perda 1-polo no laço, mais fechado com `damping`;
- **sustain:** `decay` → ganho de realimentação (0,86–0,999);
- **posição de pinça:** `position` → a rajada de excitação passa por um
  pente (um trecho da corda cujo nó cai em `position` recebe menos
  energia — corda pinçada no meio não tem 2º harmônico);
- **arco:** entrada `in` contínua injetada no laço (×0,35);
- **estabilidade:** `tanh` no laço → a excitação contínua leva a um
  **ciclo-limite**, não à divergência (mesmo princípio do `FILTER`
  auto-oscilante); `drive` empurra mais forte no `tanh`.

Determinístico (rajada = ruído xorshift semeado). Buffer alocado em
`prepare()`; `process()` não aloca.

**Testes (7/7 alvos, Debug + Release):** a corda soa na altura de `freq`
(autocorrelação, 110/220/330 Hz, ±6%); `decay` alto sustenta > 4× mais
que `decay` baixo; `damping` baixo → mais energia de alta frequência
(corda brilhante) que `damping` alto; arco contínuo + `decay = 1` +
`freq_mod` senoidal por 6000 blocos sem NaN nem `|y| > 1,6`; dois
renders byte-idênticos; integração no grafo (`CLOCK.euclid →
STRING.pluck`); painel fecha (12 HP).

**Pendências (candidatos, não controles fictícios):** decaimento
dependente de frequência com dois coeficientes (Jaffe & Smith — agudos
morrem antes, mais realista); modelo de arco não-linear de verdade
(fricção de Helmholtz, `in` como pressão/velocidade); acoplamento
simpático entre várias cordas (uma corda excita a outra — cruza com
`Cable` relação); corpo ressonante na saída (um `MATTER` curto em
série); `position` como CV de áudio; dispersão (rigidez → inarmonicidade
como no `MATTER.structure`).

---

## 1. Problema musical e papel no fluxo

`MATTER` (Módulo 9) dá o corpo por soma de modos — ótimo para sinos,
placas, tigelas. Mas corda pinçada e corda arcada são o território do
**guia-de-onda**: mais barato (um laço, não 24), com um ataque e um
"corpo" que a síntese modal não captura naturalmente, e com o gesto de
`position` que é físico. Ter os dois modelos (`MATTER` modal + `STRING`
guia-de-onda) cobre quase todo o mundo dos objetos que soam.

Papel: `pluck` do `CLOCK`/`DECISION`/`TURING` toca notas; `freq_mod` de
um `QUANTIZER` (Módulo 12) dá alturas de escala; `in` de um LFO ou ruído
"arca" a corda; a saída vai ao `SPACE` ou ao `FILTER`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Karplus & Strong** (1983), "Digital Synthesis of Plucked-String and Drum Timbres" (Computer Music Journal) | o laço de atraso com filtro de média = corda pinçada; o comprimento é a altura | artigo público |
| **Jaffe & Smith** (1983), "Extensions of the Karplus-Strong Plucked-String Algorithm" | afinação fina por all-pass fracionário; decaimento dependente de frequência; posição de pinça e de captação | artigo público (as extensões são conhecidas; reescrito) |
| **J. O. Smith**, *Physical Audio Signal Processing* | guia-de-onda digital, filtro de perda, estabilidade do laço perto de ganho 1 | livro online público |
| **Mutable Elements / Rings** (modo corda) | `position`/`brightness`/`damping` como eixos tocáveis; excitação externa ou interna | MIT — estudo do comportamento |
| **FILTER auto-oscilante** (Módulo 2, Rasgo) | não-linearidade NO laço → ciclo-limite estável em vez de NaN | conceito próprio |

## 3. Modelo — matemática, estados, extremos

**Laço** (por amostra):
```
loopDelay = sr / f0
D    = clamp(floor(loopDelay − 0,5), 2, bufLen−3)
frac = loopDelay − D
apCoeff = (1 − frac) / (1 + frac)

delayed = buffer[writePos − D]
lp     += lpA·(delayed − lp)                    ; lpA = 0,05 + (1−damping)·0,9
ap      = apCoeff·lp + apPrev − apCoeff·apOut   ; all-pass de 1ª ordem
apPrev  = lp ;  apOut = ap
v       = tanh( (fbGain·ap + 0,35·in) · driveGain )
buffer[writePos] = v ;  writePos++
saída   = mix·ap + (1−mix)·in
```
`fbGain = 0,86 + decay·0,139`. `driveGain = 1 + drive·5`.

**Excitação (`pluck`):** enche o trecho do laço que será lido a seguir
com `exciter·0,5·(n − n_ant)·combGain`, `combGain` rampa de 0 a 1 até
`combLag = position·D` (o pente da posição de pinça).

**Estados:** buffer (`sr/15` floats), `writePos`, `lp`, `apPrev`,
`apOut`, `prevPluck`, rng. Sem alocação.

**Extremos.** `freq` no piso (20 Hz) → `loopDelay = 2400` ≤ `bufLen`
(3200). `freq` no teto (4 kHz) → `D = 10`, all-pass ainda bem
condicionado. `decay = 1` → `fbGain = 0,999`; o `tanh` segura → ciclo
longo mas limitado. `damping = 0` → LP quase aberto, corda muito
brilhante, ainda estável (o `tanh` e o `fbGain < 1` limitam). Arco
(`in`) forte → limita no `tanh` (comportamento de corda "esmagada").
Reset → buffer e estados zerados.

## 4. Três modos obrigatórios

- **Autônoma:** só `pluck` (interno, de um `CLOCK`) + parâmetros já é uma
  voz de corda completa. Com `in` = um LFO lento de nível baixo, a corda
  "canta" sozinha (auto-oscila via o `tanh`).
- **Performance:** `decay`, `damping`, `position` são os três gestos —
  `decay` de pizzicato a drone, `damping` de nylon a aço, `position` de
  "sul tasto" a "sul ponticello".
- **Híbrida:** `pluck` do ritmo, `freq_mod` do `QUANTIZER`/`TURING` (cada
  nota afinada), `in` de ruído filtrado = arco, `damp_mod` de um
  envelope = a corda abafa ao longo da nota.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio, arco/excitação contínua), `pluck` (Control,
borda → rajada), `freq_mod` (Control, 1 V/oct), `damp_mod` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `freq` | 20–4000 Hz (log) | 110 | altura (comprimento do laço) |
| `decay` | 0–1 | 0,7 | sustain (ganho de realimentação) |
| `damping` | 0–1 | 0,4 | brilho (LP no laço) |
| `position` | 0,02–0,5 | 0,14 | posição de pinça (pente na excitação) |
| `exciter` | 0–1 | 0,6 | nível da rajada de pinça |
| `drive` | 0–1 | 0 | quanto empurra no `tanh` do laço |
| `mix` | 0–1 | 1 | seco/molhado |

**Limites:** saída em [−1,1] (`tanh` no laço). CPU: ~1 LP + 1 all-pass +
1 `tanh` por amostra + a rajada (D somas) no `pluck`. Sem alocação.

## 6. Alternativas descartadas

- **KS clássico (filtro de média `(x[n]+x[n−1])/2`):** afina só em
  frequências onde `sr/f0` é inteiro + ½; o all-pass fracionário afina
  em qualquer freq, custa quase nada.
- **Guia-de-onda com duas linhas de atraso (onda progressiva +
  regressiva):** mais fiel fisicamente (permite captação em ponto,
  reflexões nas duas pontes), mais estado; uma linha + all-pass basta
  pro marco. Segunda linha é candidato.
- **Modelo de arco de Helmholtz de verdade:** ótimo, não-linear e caro
  de acertar; o `tanh` no laço dá um arco "que funciona" já. Candidato.
- **`STRING` como modo do `MATTER`:** os dois são MATTER na taxonomia,
  mas o motor é tão diferente (soma de modos × laço de atraso) que
  fundir só complica. Módulos separados; `structure` cruzando entre eles
  é 2ª camada (é o que Elements faz).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** a periodicidade do som após um `pluck` (autocorrelação)
bate com `freq` (±2%, all-pass afinando); o T60 medido cresce
monotonicamente com `decay`; `damping` controla a energia de alta
frequência da corda; arco contínuo → oscilação sustentada limitada (sem
NaN, sem crescer); dois renders byte-idênticos; sem alocação em
`process()`.

**Escuta:** `position` muda o timbre do ataque como uma corda real
(meio = oco, perto da ponte = brilhante)? `decay` alto dá um drone
musical ou um zumbido? o arco (`in`) soa como arco ou como ruído? a
afinação está estável ao varrer `freq` (glissando limpo)? `TURING →
QUANTIZER → STRING` já é uma linha melódica que dá vontade de ouvir?

## 8. Integração e painel

Classe `StringVoice` (`type()` = `"STRING"`), 4 entradas, 1 saída, 7
parâmetros. `panel()` próprio (12 HP: display da corda +
FREQ/DECAY/DAMP/POS, EXCITE/DRIVE/MIX, jacks). Testado isolado (altura,
sustain, brilho, arco, determinismo) antes do patch. Numa peça:
`QUANTIZER → STRING.freq_mod`, `CLOCK → STRING.pluck`, saída → `SPACE`.
