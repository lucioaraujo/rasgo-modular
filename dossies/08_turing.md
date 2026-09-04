# Dossiê — Módulo 8: Laço de registrador (`TURING`)

**Família:** SEQUENCE
**Estado:** **implementado — marco 1** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/TuringLoop.hpp`, `tests/test_turing.cpp`

## Estado da implementação (marco 1)

Feito: registrador de deslocamento de `length` estágios (2–16), cada um
com um valor em [0,1). A cada clock:

1. o valor da frente (`reg[0]`) sai;
2. os estágios deslocam (`reg[i] = reg[i+1]`);
3. o que **reentra na cauda** é:
   - o valor que saiu (**laço preservado**) — com probabilidade `lock`;
   - senão: um valor **novo** (`mutate ≈ 1`) ou uma **mutação parcial**
     `clamp(front + ruído·mutate)` (`mutate < 1`).

`lock = 1` → laço travado (rotação pura, período = `length`);
`lock = 0` → puro acaso; `lock = 0,5` → 50% de mudar por passo (o "12
horas" do Turing Machine).

**Saídas:** `cv` = frente do registrador, bipolar, escalada por `range`,
quantizada por `steps` (1 = contínuo), com `offset`; `cv2` = soma
ponderada de alguns estágios (expansor "Volts"); `pulse` = `reg[0] ≥
0,5` como gate (expansor "Pulses"). Avança por `clock` externo OU, sem
ele, por relógio interno em `rate`. Determinístico (xorshift semeado;
registrador inicial reprodutível).

**Testes (8/8 alvos, Debug + Release):** `lock = 1` → `cv` periódico com
período `length` e não constante; `lock = 0` → não trava nesse período,
variância alta; `range = 0` → `cv` fixo no `offset`; `steps = 3` → `cv`
só em {−1, 0, 1}; relógio interno avança o registrador; dois renders
byte-idênticos; `pulse ∈ {0,1}`; integração no grafo
(`CLOCK.clock → TURING.clock`, `cv → cutoff_mod`, `cv2 → in`); painel
fecha (12 HP).

**Pendências (candidatos, não controles fictícios):** quantização a uma
escala musical (Turing Machine + Volts viram melodia — cruza com o
futuro módulo de harmonia / escalas do `RASGO_SYNTH`); entrada de
`write` (forçar um valor num estágio — o botão "write" do hardware);
`cv2` com pesos configuráveis (o expansor Volts tem trimmers); segundo
comprimento independente (padrões que se cruzam); saída de bits
individuais como gates (Pulses tem 6); modo "8 bits" vs "16 bits"
(resolução do registrador).

---

## 1. Problema musical e papel no fluxo

`DECISION` (Módulo 4) já tem déjà-vu, mas ali a memória é uma tabela
relida por probabilidade. O Turing Machine é outra coisa: uma **sequência
que emerge do acaso e depois se solidifica**. Você não escreve a melodia
— você abre o `lock`, deixa o acaso preencher, e quando algo soa bem
você fecha o `lock` e aquilo vira um loop. É composição por escuta e
seleção, não por programação. É o oposto do piano-roll.

Papel: `cv` → `rate_mod` da voz (melodia em degraus) ou `cutoff_mod` do
filtro; `pulse` → gate de envelope; `cv2` → uma segunda modulação
correlacionada. Clock vem do `CLOCK` (Módulo 5). Com `lock` num
`DECISION.x` lento, o instrumento alterna sozinho entre "improvisar" e
"repetir".

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Music Thing Turing Machine Mk II** (Tom Whitwell; hardware + doc pública) | registrador de deslocamento circular; knob "Lock" = probabilidade de mutação de bit por ciclo (CCW acaso → CW travado); expansores **Volts** (soma ponderada de bits → CV) e **Pulses** (bits → gates) | hardware, documentação aberta; sem código |
| **Mutable Marbles** `déjà-vu` | a mesma ideia de memória circular relida com probabilidade | MIT — conceito |
| **LFSR / shift-register music** (Berlin School; Gristleizer; literatura de PRNG) | um registrador realimentado gera sequências longas quase-aleatórias mas repetíveis | domínio público |
| **Quantização a escala** (Sinfonion, Marbles `t`) | 2ª camada — transformar os degraus em alturas musicais | conceito, adiado |

## 3. Modelo — matemática, estados, extremos

**Registrador.** `reg_[16]` de floats em [0,1); só os primeiros `length`
participam da rotação. Inicializado em `prepare()` com `uniform01()`
(reprodutível).

**Avanço (`advance`).**
```
front = reg[0]
para i em [0, length−2]:  reg[i] = reg[i+1]
tail = front
se uniform01() ≥ lock:
    se mutate ≥ 0,999:  tail = uniform01()
    senão:              tail = clamp(front + (uniform01()·2−1)·mutate, 0, 1)
reg[length−1] = tail
```

**Saídas (`computeOutputs`).**
```
cv  = quantiza( (reg[0]−0,5)·2·range + offset , steps )
cv2 = clamp( (0,5·reg[0] + 0,25·reg[2] + 0,15·reg[5] + 0,1·reg[7] − 0,5)·2·range + offset )
pulse = reg[0] ≥ 0,5 ? 1 : 0
```
`quantiza(x, N)`: se `N ≥ 2`, N níveis igualmente espaçados em [−1,1].

**Clock.** `clock` conectado → borda de subida avança. Não conectado →
fasor `phase += rate/sr`, avança no wrap.

**Extremos.** `lock = 1`: `uniform01() ≥ 1` é sempre falso → nunca muta →
rotação pura → período exatamente `length`. `lock = 0`: sempre muta.
`length = 2`: laço curtíssimo (gangorra). `range = 0`: `cv = offset`
constante. `steps = 1`: sem quantização. Estágios `reg[2/5/7]` usados no
`cv2` mesmo com `length < 8` — são floats válidos (array de 16 sempre
inicializado), só não giram; `cv2` continua limitado. Reset → registrador
re-sorteado, fasor zerado.

## 4. Três modos obrigatórios

- **Autônoma:** relógio interno + `lock` intermediário → uma linha que
  evolui e às vezes se repete, sem nenhuma entrada.
- **Performance:** `lock` é o macro central — girar de CCW a CW ao vivo
  leva de "improviso" a "riff travado"; `length` muda a métrica
  percebida; `range`/`steps`/`offset` afinam o alcance melódico.
- **Híbrida:** `clock` do `CLOCK` sincroniza ao ritmo; `lock_mod` de um
  `DECISION.x`/LFO lento faz o instrumento decidir sozinho quando
  cristalizar e quando dissolver a sequência.

## 5. Portas, parâmetros, limites

**Entradas:** `clock` (Control, borda avança), `lock_mod` (Control, soma
a `lock`).
**Saídas:** `cv` (Audio/CV bi), `cv2` (Audio/CV bi), `pulse` (Control,
0/1).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `rate` | 0,01–50 Hz | 4 | clock interno quando `clock` não está patchado |
| `length` | 2–16 | 8 | estágios do registrador |
| `lock` | 0–1 | 0,5 | P(preservar o laço) por passo |
| `mutate` | 0–1 | 1 | mutação total (1) ↔ parcial por ruído (<1) |
| `range` | 0–1 | 0,6 | escala da saída |
| `steps` | 1–32 | 1 | quantização de `cv` |
| `offset` | −1..+1 | 0 | deslocamento da saída |

**Limites:** saídas em [−1,1] e {0,1}. CPU: por amostra só detecção de
borda + cópia das saídas; o `advance` é por passo (raro). Sem alocação.
Estado: 16 floats + fasor + rng.

## 6. Alternativas descartadas

- **Registrador de bits (0/1) puro** como o hardware: os expansores
  Volts/Pulses reconstroem CV a partir de bits. Usar floats [0,1) por
  estágio dá o mesmo comportamento de laço com resolução melhor e menos
  código de reconstrução. O `pulse` recupera a leitura "bit".
- **Sequenciador de passos editável:** é outra família (Hexen §119,
  "sequenciador como família de comportamentos"); o Turing é acaso
  cristalizável, não partitura.
- **Quantização a escala no marco 1:** `steps` é espaçamento em tensão,
  não em semitons. O quantizador harmônico é módulo próprio (2ª camada).
- **Realimentação polinomial (LFSR real):** gera sequências de período
  máximo, mas o `lock` probabilístico é mais musical (o hardware Turing
  também não é um LFSR clássico).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `lock = 1` → autocorrelação de `cv` com lag `length` =
1,0; `lock = 0` → autocorrelação baixa em todo lag; `range = 0` → `cv`
constante; `steps = N` → `cv` assume ≤ N valores; determinismo
byte-idêntico; `pulse ∈ {0,1}`; sem alocação (teste).

**Escuta:** girar `lock` ao vivo soa como a música "se decidindo"?
`length` 3 vs 5 vs 8 muda o caráter métrico de forma clara? o `cv`
quantizado soa como melodia ou como escada? `pulse` como gate de
envelope dá um contracanto rítmico útil? `TURING → voz` + `CLOCK` já é
uma peça?

## 8. Integração e painel

Classe `TuringLoop` (`type()` = `"TURING"`), 2 entradas, 3 saídas, 7
parâmetros. `panel()` próprio (12 HP: display do registrador +
RATE/LEN/LOCK/MUT, RANGE/STEPS/OFST, jacks). Testado isolado (laço
travado, acaso, range/steps, clock interno, determinismo) antes do
patch. Entra na peça longa como a fonte melódica cristalizável:
`CLOCK → TURING.clock`, `cv → rate_mod` da voz, `lock` modulado por um
`DECISION` lento.
