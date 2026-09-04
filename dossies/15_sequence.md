# Dossiê — Módulo 15: Sequenciador de passos (`SEQUENCE`)

**Família:** SEQUENCE
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/StepSequencer.hpp`, `tests/test_sequence.cpp`

## Estado da implementação (marco 2)

Feito: **a intenção, não só o acaso.** Um padrão de **8 passos
editável** — altura (`p1`..`p8`, bipolar) e gate (`g1`..`g8`) por passo,
como parâmetros; o "edit surface" é o próprio patch de texto — tocado
por uma **família de comportamentos de leitura** (`mode`):

| `mode` | Leitura |
|---|---|
| 0 forward | `idx = (idx+1) mod length` |
| 1 backward | `idx = (idx−1) mod length` |
| 2 pingpong | quica entre 0 e `length−1` |
| 3 random | `idx = uniform·length` (xorshift semeado) |
| 4 brownian | `idx += {−1, 0, +1}` (Grids/Marbles), wrap |

`length` (1–8), `glide` (portamento entre alturas), `gate_len`, `range`
(escala a altura em oitavas). Saídas: `pitch` (oitavas — pra uma voz ou
`QUANTIZER.transpose`), `gate`, `eos` (pulso quando o índice volta a 0).
Avança por `clock` externo (janela de gate estimada do intervalo entre
clocks) OU relógio interno em `rate`. `reset` volta ao passo 0.
Determinístico (rng semeado).

**Testes (9/9 alvos, Debug + Release):** forward toca `p2,p3,p4,p1,p2…`
(sequência exata); backward `p4,p3,p2,p1…`; pingpong `p2,p3,p4,p3,p2,p1,
p2…`; `gate` segue o padrão `g1..g8`; `eos` pulsa ~20× em 2 s (30 Hz /
3 passos = 10 ciclos/s); `glide = 0,5` limita o salto de altura por
amostra a < 0,02; relógio interno + modo random → dois renders
byte-idênticos e o índice se move; integração no grafo
(`CLOCK → SEQUENCE.clock`, `pitch → voz.rate_mod`); painel fecha (20 HP).

**Pendências (candidatos, não controles fictícios):** ratchet (subdividir
um passo em N disparos — Metropolix); probabilidade por passo (chance de
o gate disparar); comprimento e direção independentes por "lane"
(René/Metropolix têm X e Y); slide/tie por passo (portamento só em
passos marcados); passos com repetição (`hold` de N clocks); mais de 8
passos (encoding compacto do padrão na serialização em vez de 16
parâmetros); modo euclidiano de leitura (os passos ativos distribuídos
por E(k,n)).

---

## 1. Problema musical e papel no fluxo

`TURING` (Módulo 8) faz a sequência que **emerge do acaso** e cristaliza;
`DECISION` (Módulo 4) decide passo a passo. Mas às vezes a peça precisa
de uma frase **que você escreveu** — um riff, um baixo, um ostinato — e
o que é generativo é *como ela é relida*: pra frente, invertida,
quicando, vagando. É o princípio de Hexen §119: o sequenciador não é um
objeto, é uma **família de comportamentos de leitura sobre um padrão**.
O `SEQUENCE` traz isso, o par complementar do `TURING`.

Papel: `pitch` → `rate_mod` de uma voz (ou `QUANTIZER` pra travar na
escala do `HARMONY`); `gate` → `ENVELOPE`; `eos` → `HARMONY.advance`
(uma modulação a cada volta da frase) ou `TURING.reset`. Clock do
`CLOCK`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Hexen §119** (pesquisa RASGO) | "o sequenciador é uma família de comportamentos de leitura, não um objeto" | conceito próprio do workspace |
| **Make Noise René / Intellijel Metropolix** (hardware) | direção de leitura como parâmetro; passo com altura/gate próprios; `eos`/reset | hardware, estudo de comportamento |
| **Mutable Grids / Marbles** | modo browniano — passo a passo ±1, entre "travado" e "aleatório" | MIT — conceito |
| **ostinato / minimalismo** (Reich, Glass) | uma célula curta relida com pequenas variações de leitura = forma | prática musical |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
reset (borda) → idx = 0 ; dir = +1 ; phase = 0
clock:
  externo → borda de subida chama advance(length, mode); a janela de gate
            = gate_len · (intervalo estimado entre clocks)
  interno → phase += rate/sr ; no wrap chama advance()
se avançou e idx == 0 → eosCountdown = 3 ms

target = p[idx] · range
pitch += (target − pitch) · glideCoeff        (glideCoeff = 1 se glide=0)
gate   = g[idx] ≥ 0,5 ? (na janela de gate ? 1 : 0) : 0
eos    = eosCountdown > 0 ? 1 : 0  (decrementa)
```
`advance()` aplica o `mode` (ver tabela §estado). `dir` só é usado no
pingpong.

**Estados:** `phase`, `idx` (int), `dir`, `pitch`, `eosCountdown`,
`extGateSamples`/`extPeriod` (estimativa de período do clock externo),
`prevClock`/`prevReset`, rng. Sem alocação.

**Extremos.** `length = 1` → um passo repetido (drone rítmico); pingpong
com `length = 1` → fica em 0. `rate` no teto (40 Hz) → sequência quase
de áudio. `glide` alto + passos rápidos → `pitch` fica atrás (portamento
longo, correto). Clock externo com o primeiro pulso só → usa `extPeriod`
default (sr/4) até o segundo pulso calibrar. `reset` que fica alto → só
a borda conta.

## 4. Três modos obrigatórios

- **Autônoma:** padrão + `mode` + `rate` já tocam uma frase completa sem
  nenhuma entrada; trocar de `mode` re-lê a mesma frase de outro jeito.
- **Performance:** `mode` é o macro central (forward → pingpong →
  brownian → random é um contínuo de "escrito" a "vivo"); `length`
  encurta/estende a frase ao vivo; os `p`/`g` de cada passo são a
  edição.
- **Híbrida:** `clock` do `CLOCK` (com swing/drift) dá o groove;
  `eos → HARMONY.advance` amarra a forma harmônica à frase; `reset` de
  uma seção re-ancora.

## 5. Portas, parâmetros, limites

**Entradas:** `clock` (Control, borda → avança), `reset` (Control,
borda → passo 0).
**Saídas:** `pitch` (Audio, oitavas), `gate` (Control), `eos` (Control,
pulso por ciclo).
**Parâmetros:** `length` (1–8), `mode` (0–4), `rate` (0,01–40 Hz),
`gate_len` (0,05–0,95), `glide` (0–1), `range` (0–2 oct), e por passo
`p1..p8` (−1..+1) + `g1..g8` (0/1). **19 no total** — um sequenciador
editável tem os passos como controles; todos são reais.
**Limites:** `pitch` ≤ ±2 oitavas. CPU: por amostra só o glide + a
detecção de borda; `advance` por passo. Sem alocação.

## 6. Alternativas descartadas

- **Padrão gerado de um seed** (sem passos editáveis): é o `TURING`. O
  ponto do `SEQUENCE` é o padrão *escrito*.
- **Grid de 16/32 passos já no marco 2:** 16 parâmetros por 8 passos já
  é muito; 16+ passos pedem um encoding compacto do padrão na
  serialização (candidato).
- **Ratchet/probabilidade por passo no marco 1:** ótimos (Metropolix),
  mas cada um é um vetor de 8 parâmetros a mais. 2ª camada.
- **`std::mt19937`** pros modos random/brownian: xorshift semeado é o
  padrão dos outros módulos.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** cada `mode` produz a ordem de índices esperada (forward/
backward/pingpong exatos); `gate` reproduz `g1..g8`; `eos` pulsa uma
vez por ciclo; `glide` limita a derivada de `pitch`; modo random/
brownian byte-idêntico entre renders; sem alocação.

**Escuta:** uma frase curta relida em pingpong soa "musical" ou
mecânica? o modo brownian dá variação orgânica sem perder a
identidade da frase? `glide` entre passos soa como um baixista
escorregando? `SEQUENCE → QUANTIZER` (na escala do `HARMONY`) → `STRING`
já é uma linha com intenção?

## 8. Integração e painel

Classe `StepSequencer` (`type()` = `"SEQUENCE"`), 2 entradas, 3 saídas,
19 parâmetros. `panel()` próprio (20 HP: display dos passos + uma fileira
de sliders de altura + uma de toggles de gate + LEN/MODE/RATE/GATE/GLIDE/
RANGE + jacks). Testado isolado (as cinco leituras, gate, eos, glide,
determinismo) antes do patch. Numa peça: `CLOCK → SEQUENCE`,
`pitch → QUANTIZER → STRING`, `eos → HARMONY.advance` — uma frase escrita
que a leitura e a harmonia fazem evoluir.
