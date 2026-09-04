# Dossiê — Módulo 14: Movimento harmônico (`HARMONY`)

**Família:** DECISION / INFERENCE
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Harmony.hpp`, `tests/test_harmony.cpp`

## Estado da implementação (marco 2)

Feito: o que faz a peça **mudar de tom**. A cada fronteira de seção
(trigger `advance` externo, ou relógio interno lento em `rate`), avança o
centro tonal — `root` (0–11 st) e `scale` (índice na tabela do
`QUANTIZER`) — por **uma de seis técnicas reais** de movimento harmônico,
escolhida pelo parâmetro `movement`:

| # | `movement` | Intervalo de raiz | Fonte musicológica |
|---|---|---|---|
| 0 | Coltrane / Giant Steps | +4 st, ciclo de 3 centros | Coltrane, "Giant Steps" (1959) — divide a oitava em 3 |
| 1 | Substituição tritônica + ii-V-I | −7 st (quintas descendentes) ou, ~40%, ±1 st (a sub torna a condução cromática) | prática de jazz documentada |
| 2 | Mediante cromática não-funcional | ±3 ou ±4 st, sem direção implícita | bossa nova / Jobim ("Wave", "Corcovado") |
| 3 | Intercâmbio modal | **raiz não se move**, só o modo | acorde emprestado da família paralela |
| 4 | Jazz modal | quase sempre estático; ~30% um passo de ½ ou 1 tom | Miles Davis, "Kind of Blue" / o bridge de "So What" |
| 5 | Backdoor ii-V | +2 st | cadência bVII7→I |

`hold` (0–1) = probabilidade do centro tonal **não** avançar num pulso
(acrescenta estase tipo jazz-modal a qualquer `movement`). `scale_lo`/
`scale_hi` limitam quais escalas do `QUANTIZER` são sorteáveis (paleta).
`reset` volta a `root_start`. `change` pulsa ~20 ms quando o centro
tonal de fato muda. Determinístico (xorshift semeado).

**Saídas normalizadas para casar com o `QUANTIZER`:** `root` = semitom/12
(consumidor usa `connectToParameter(..., "root", depth=12)`); `scale` =
índice/11 (`depth=11`).

**Testes (10/10 alvos, Debug + Release):** Coltrane → raiz +4 st mod 12
(sequência exata 4,8,0,4,8,0…); Backdoor → +2 st exato; intercâmbio modal
→ **raiz fixa** em 40 passos, ≥ 3 modos distintos; jazz modal → raiz muda
em < 40% dos passos (mas > 0); `change` pulsa a cada avanço real; `reset`
volta a `root_start`; relógio interno avança (~10 avanços em 5 s a 2 Hz);
dois renders byte-idênticos; integração no grafo
(`HARMONY → QUANTIZER.root/scale` via `connectToParameter`, `TURING` →
`QUANTIZER.cv` → pitch quantizado sempre semitom inteiro válido); painel
fecha (12 HP).

**Pendências (candidatos, não controles fictícios):** as outras técnicas
de `HarmonicWanderer` (o `pickBorrowedMode` curado de verdade para o
intercâmbio modal — hoje sorteia da faixa `scale_lo..scale_hi`);
`MelodyVoicingBank` (voicing vertical — quartal/planing/bitonal) como
saída de acorde; um "arco harmônico" (a técnica muda ao longo da peça —
`GenerativeArc`); saída de `tension` para o barramento semântico
(dissonância do centro atual); alinhar `advance` a `CLOCK.reset` (seções
no compasso).

---

## 1. Problema musical e papel no fluxo

O `QUANTIZER` (Módulo 12) faz o acaso soar numa tonalidade — mas uma
tonalidade **fixa**. Música original raramente fica num tom só; ela
**modula**, e não de qualquer jeito: por movimentos harmônicos que a
tradição nomeou e que têm assinatura de intervalo própria. O `HARMONY` é
esse eixo: decide a sequência de centros tonais por onde a melodia
generativa passeia. É o elo final entre "gerar variedade" e "gerar
música com forma harmônica" — a finalidade nº 1 da v1.

Papel: `advance` de um `CLOCK` muito lento (ou `DECISION`) marca as
seções; `root`/`scale` vão pros parâmetros do `QUANTIZER`; `change`
dispara um `ENVELOPE` de "novo tom" ou reinicia o `TURING` (frase nova a
cada modulação).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **`RASGO_SYNTH/rasgo-synth-core/src/sequencer/HarmonicWanderer.hpp`** (projeto irmão) | as seis técnicas com a lógica de intervalos e a fonte musicológica de cada uma, num único cabeçalho comentado. **Só a lógica de intervalos é portada** (fato musical de domínio público); o código não é incluído entre projetos | projeto próprio (AGPLv3/GPLv3); os intervalos em si são teoria pública |
| **J. Coltrane**, "Giant Steps" (1959); análises padrão | ciclo de terças maiores dividindo a oitava em 3 | histórico |
| **A. C. Jobim** (bossa nova); análise de mediante cromática | giro de terça sem função tonal | histórico |
| **Miles Davis**, *Kind of Blue* (1959) | idioma modal: centro tonal sustentado, deslocamento raro e por passo | histórico |
| teoria de jazz padrão (Levine, *The Jazz Theory Book*) | ii-V-I, substituição tritônica, backdoor, intercâmbio modal | teoria pública |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
reset (borda) → root = root_start mod 12 ; scale = scale_lo ; coltraneStep = 0
advance = (advance conectado ? borda de subida : fasor phase += rate/sr wrap)
se advance e não (hold>0 e uniform01 < hold):
    (root, scale) ← advanceKeyCenter(movement)   // ver tabela §estado
    se mudou: changeCountdown = 20 ms
root_out  = root / 12
scale_out = scale / 11
change    = changeCountdown>0 ? 1 : 0  (decrementa)
```
`advanceKeyCenter` aplica a técnica de `movement`; `moveRoot(n)` faz
`root = (root + n) mod 12` (a oitava não importa pra um quantizador de
escala); `pickScale(lo,hi)` sorteia um índice em `[lo,hi]`.

**Estados:** `phase`, `root` (int 0-11), `scale` (int), `coltraneStep`,
`changeCountdown`, `prevAdvance`/`prevReset`, rng. Sem alocação.

**Extremos.** `rate` no piso (0,005 Hz) → uma seção a cada ~200 s.
`movement = 3` (intercâmbio modal) → `root` nunca muda, `change` só pulsa
quando o modo muda. `hold = 1` → o centro tonal congela (nenhum avanço
tem efeito) — uma peça de um tom só. `scale_lo == scale_hi` → escala
fixa, só a raiz anda. `advance` que fica alto → só a borda conta.

## 4. Três modos obrigatórios

- **Autônoma:** `movement` + `rate` + `scale_lo/hi` já produzem um plano
  harmônico completo sem nenhuma entrada — a peça modula sozinha.
- **Performance:** `movement` é o macro central — trocar de técnica ao
  vivo muda o *caráter* da modulação (Coltrane = vertiginoso, jazz modal
  = quase parado, mediante cromática = onírico). `hold` "segura" o tom
  atual como um gesto.
- **Híbrida:** `advance` do `CLOCK` (uma divisão bem longa, ou o
  `reset` do clock) alinha as modulações à estrutura rítmica; `change` →
  `ENVELOPE`/`TURING.reset` articula cada tom novo.

## 5. Portas, parâmetros, limites

**Entradas:** `advance` (Control, borda → avança o centro tonal),
`reset` (Control, borda → volta a `root_start`).
**Saídas:** `root` (Audio, semitom/12 — para `QUANTIZER.root`, depth 12),
`scale` (Audio, índice/11 — para `QUANTIZER.scale`, depth 11),
`change` (Control, pulso na mudança de centro).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `movement` | 0–5 | 4 (jazz modal) | a técnica de movimento harmônico |
| `rate` | 0,005–2 Hz | 0,06 | fronteira de seção quando `advance` não patchado |
| `root_start` | 0–11 st | 2 (ré) | tônica inicial / de reset |
| `scale_lo` | 1–10 | 1 | menor índice de escala sorteável |
| `scale_hi` | 1–10 | 6 | maior índice sorteável |
| `hold` | 0–1 | 0 | P(não avançar num pulso) |

**Limites:** saídas em [0,1]. CPU: por amostra só detecção de borda +
cópia; `advanceKeyCenter` por seção (raríssimo). Sem alocação, estado
fixo.

## 6. Alternativas descartadas

- **Portar `HarmonicWanderer.hpp` inteiro** (com `MelodyVoicingBank`,
  `GenerativeArc`): é a "máquina inteira" que o autor prefere não reusar
  por atacado ([[feedback_generative_design_light_touch]]). As seis
  técnicas de eixo-A cobrem o essencial; voicing vertical e arco são
  módulos/2ª camada.
- **Emitir MIDI de acordes:** o Rasgo Modular é CV; `root`/`scale` como
  parâmetros do `QUANTIZER` é o caminho nativo.
- **`std::mt19937`** (como o original): pesado e não-portável de estado;
  xorshift semeado é o padrão dos outros módulos.
- **Uma escala de 45 opções** (como `Scales.hpp`): o `QUANTIZER` já
  curou 12; `HARMONY` sorteia dentro dessas.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** Coltrane → raiz +4 st mod 12 exato; Backdoor → +2 st;
intercâmbio modal → raiz constante; jazz modal → raiz muda em ~30% dos
avanços; `change` pulsa só na mudança real; `reset` restaura;
determinismo byte-idêntico; sem alocação.

**Escuta:** cada `movement` soa como a técnica que nomeia (Coltrane
"anda muito", jazz modal "fica")? a melodia do `QUANTIZER` seguindo o
`HARMONY` soa como uma peça que **modula** ou como escalas trocando ao
acaso? `change` disparando uma frase nova reforça a sensação de seção?
`hold` alto dá um drone tonal convincente?

## 8. Integração e painel

Classe `Harmony` (`type()` = `"HARMONY"`), 2 entradas, 3 saídas, 6
parâmetros. `panel()` próprio (12 HP: display do centro tonal +
MOVE/RATE/ROOT, SC LO/SC HI/HOLD, jacks). Testado isolado (as seis
técnicas, reset, clock, determinismo) antes do patch. Numa peça:
`CLOCK` (divisão longa) → `HARMONY.advance`; `HARMONY.root/scale` →
`QUANTIZER`; `HARMONY.change` → `TURING.reset` — a peça modula e cada
tom novo traz uma frase nova.
