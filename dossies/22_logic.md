# Dossiê — Módulo 22: Lógica e utilidades de clock (`LOGIC`)

**Família:** TIME / UTILITY
**Estado:** **implementado — marco 3** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Logic.hpp`, `tests/test_logic.cpp`

## Estado da implementação (marco 3)

Quinto e último dos essenciais (`PESQUISA_MODULOS.md §2.1`) — fecha o
**rack de partida**. O `CLOCK` faz euclidiano + divisão interna; o
`SEQUENCE`/`TURING` leem padrões. Faltava a peça que **recombina** dois
streams de gate num ritmo novo (AND/OR/XOR), **divide/multiplica** um
clock avulso, e dá um **flip-flop** e um **atraso de gate**.

- **`rate`** (0,1–40 Hz) — relógio interno, usado quando `clock` **não**
  está conectado (LOGIC sozinho = gerador de ritmo — modo autônomo);
- **`divide`** (1–32) — `div` dispara a cada N bordas de entrada;
- **`multiply`** (1–8) — subdivide o período medido em M pulsos iguais
  (Pamela's: mede o intervalo, extrapola **um** período à frente);
- **`gate_len`** (0,02–0,98) — largura do pulso `div` como fração do seu
  período;
- **`delay`** (0–1 → 0–200 ms) — atrasa o `div` por um anel de amostras;
- **lógica booleana**: `a`, `b` (limiar 0,5) → saídas simultâneas
  `and` = a·b, `or` = a+b, `xor` = a⊕b (contínuas, seguem as entradas);
- **`flip`** — flip-flop tipo T: alterna 0↔1 a cada borda ↑ de `a`;
  `reset` zera o contador do divisor **e** o flip-flop.

Sem RNG (é timing de precisão — nada de `drift`; jitter fica como
candidato). Anel de atraso alocado em `prepare()` (0,2 s). Sem
alocação / lock / IO em `process()`. Determinístico.

**Testes (13 funções, Debug + Release — `tests/test_logic.cpp`):**
`divide=2` → metade dos pulsos; `divide=1` → passa; `multiply=2` dobra a
contagem de pulsos no mesmo tempo; `gate_len` controla o duty medido;
`delay` desloca o `div` pelo nº de amostras esperado; `and`/`or`/`xor`
batem a tabela-verdade pra as 4 combinações; `flip` alterna e só na borda
↑ (nº de transições = nº de bordas / 2); `reset` zera flip e contador;
relógio interno (`rate`, sem `clock`) produz `div` sozinho; tudo finito e
em [0,1]; dois renders byte-idênticos; grafo
`CLOCK → LOGIC.clock` · `LOGIC.div → ENVELOPE.gate`.

**Pendências (candidatos):** `jitter`/humanize no `div`; probabilidade
por pulso (Bernoulli — mas o `DECISION` já faz); lógica com `logic_len`
(saída como trigger na mudança, não gate contínuo); latch S-R além do
T flip-flop; multiplicação com PLL (suaviza tempo variável) em vez de
extrapolar um período.

---

## 1. Problema musical e papel no fluxo

Ritmo generativo interessante quase nunca é um clock reto: é **dois**
clocks em relação (3 contra 4), um gate que só passa quando outro
também está alto, uma linha na metade do andamento, um acento que
inverte a cada compasso. Sem `LOGIC` isso não dá pra patchear — o
`CLOCK` gera **um** fluxo, o `SEQUENCE` lê **um** padrão.

Papel: utilidade de tempo, entre as fontes de clock e os destinos de
gate. `CLOCK → LOGIC` (÷/×, atraso); `SEQUENCE.gate` + `TURING` nos `a`/`b`
→ `and`/`xor` → `ENVELOPE.gate` (ritmo composto); `LOGIC.flip` →
alterna dois destinos a cada batida. Sozinho (`rate`), é um mini-clock
divisor.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **ALM Pamela's (New) Workout** (ficha pública) | dividir/multiplicar um clock medindo o intervalo e extrapolando | conceito (hardware) |
| **Mutable Kinks / Doepfer A-166 / "Boolean"** | AND/OR/XOR de dois gates como saídas simultâneas | prática |
| **Flip-flop T (qualquer texto de lógica digital)** | alterna o estado a cada borda de subida | teoria pública |
| **4ms / Doepfer A-160 clock divider** | contador módulo-N sobre as bordas | prática |
| **`spread`/`shape` do `DECISION`** | precisão é o padrão; humanizar (`jitter`) fica opt-in e candidato | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
# fonte de bordas: externa se `clock` conectado, senão relógio interno
if clock conectado:
  edge = (clock[frame] ≥ 0.5) && !prevClock          # borda ↑
else:
  intPhase += rate/sr ; edge = wrap(intPhase)        # relógio interno

if edge:
  clockPeriod = samplesSinceEdge ; samplesSinceEdge = 0
  # multiplicação: agenda M-1 sub-bordas em clockPeriod/M
  subInterval = clockPeriod / multiply ; subLeft = multiply
tick = edge
if subLeft > 1 && samplesSinceEdge ≥ subInterval*(multiply-subLeft+1):
  tick = true ; --subLeft

if tick:
  if ++divCount ≥ divide: divCount = 0 ; divPulse = true
     divPeriod = samplesSinceDivPulse ; samplesSinceDivPulse = 0

# duty: alto por gate_len·divPeriod amostras a partir do pulso
divRaw = (samplesSinceDivPulse < gate_len·divPeriod) ? 1 : 0
div    = delayRing.read(delaySamples)   ; delayRing.write(divRaw)

# lógica
av = a≥0.5 ; bv = b≥0.5
and = av&&bv ; or = av||bv ; xor = av≠bv
if (av && !prevA): flip = !flip                      # flip-flop T
if reset≥0.5: divCount = 0 ; flip = 0 ; subLeft = 0
```

**Estados:** `prevClock_`, `prevA_`, `prevReset_`, `intPhase_`,
`samplesSinceEdge_`, `samplesSinceDivPulse_`, `clockPeriod_`,
`divPeriod_`, `subInterval_`, `subLeft_`, `divCount_`, `flip_`,
`delayRing_` (vector, alocado em prepare), `delayWrite_`.

**Extremos.** `divide=1, multiply=1` → `div` = clock com o duty do
`gate_len`. `clock` para → `div` para depois de ≤1 período (só
extrapolamos um). `multiply=8` a `rate` alto → `div` vira quase contínuo
(limite aceitável). `delay` no teto com clock rápido → pulsos atrasados
podem encavalar o próximo; o anel só atrasa o sinal 0/1, resolve
sozinho. `reset` preso alto → contador e flip travados em 0. `a`/`b`
como áudio (não gate) → lógica compara com 0,5, vira square distorcido
(uso não-idiomático mas definido). Reset → tudo zerado, fase interna 0.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado; `rate` + `divide`/`multiply`/`gate_len`
  → `div` é um clock derivado tocando sozinho (dispara envelopes,
  percussão). `flip` sem `a` fica parado (correto — não há borda).
- **Performance:** `divide`/`multiply` ao vivo = mudar a subdivisão do
  ritmo sem repatch; `gate_len` de stac­cato a sustenido; `delay`
  empurra a levada pra trás do tempo (groove).
- **Híbrida:** `CLOCK → clock`; dois streams de gate (`SEQUENCE`,
  `TURING`) em `a`/`b`; `xor` → `ENVELOPE.gate` (o ritmo é a diferença
  entre os dois); `flip` alterna o `root` de um `QUANTIZER` a cada
  batida (pergunta/resposta).

## 5. Portas, parâmetros, limites

**Entradas:** `clock` (Control, trig), `a`, `b` (Control), `reset`
(Control, trig).
**Saídas:** `div`, `and`, `or`, `xor`, `flip` (todas Control, 0/1).
**Parâmetros:** `rate` (0,1–40 Hz, def 2), `divide` (1–32, def 2),
`multiply` (1–8, def 1), `gate_len` (0,02–0,98, def 0,5), `delay`
(0–1, def 0 → 0–200 ms).
**Limites:** saídas em {0, 1} (bordas suavizadas? não — gate é degrau,
é o padrão modular). CPU: comparações + 1 leitura/escrita no anel por
amostra. Memória: anel de `0,2·sr` floats (~38 kB a 48 k). Sem alocação
em `process()`.

## 6. Alternativas descartadas

- **Uma saída `logic` com seletor de modo:** 3 saídas simultâneas
  (AND/OR/XOR) é o idioma de hardware (Kinks) e deixa patchear as três
  ao mesmo tempo.
- **Multiplicação por PLL:** extrapolar um período é "bom o suficiente"
  e estável; PLL (suavizar tempo variável) é 2ª camada.
- **Clock interno sempre ligado (somado ao externo):** o interno é
  **fallback** — se há `clock`, ele manda. Somar os dois faria ritmo
  imprevisível sem querer.
- **`drift`/jitter no marco 1:** LOGIC é a régua rítmica. Precisão
  primeiro; humanizar é candidato.
- **Gate delay por buffer de eventos** em vez de anel de sinal: o anel
  de 0/1 é trivial e exato à amostra; buffer de eventos só ganharia
  memória, não precisão.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `divide=N` → nº de pulsos `div` = nº de bordas / N (±1);
`divide=1` → `div` segue o clock; `multiply=M` → M× mais pulsos no
mesmo intervalo; `gate_len` → duty medido bate (±5 %); `delay=d` →
`div` deslocado por ≈ `d·0,2·sr` amostras (correlação cruzada);
`and`/`or`/`xor` = tabela-verdade pras 4 combinações de `a`,`b`;
`flip` faz N/2 transições pra N bordas de `a` e só na borda ↑; `reset`
zera `flip` e o contador; relógio interno (`rate`, sem `clock`) gera
`div`; tudo finito e em [0,1]; dois renders byte-idênticos.

**Escuta:** `xor` de dois euclidianos soa como um terceiro ritmo
coerente (não papa)? `multiply` alto dá rufo/roll convincente ou vira
zumbido? `delay` empurra o groove "pra trás" de um jeito musical? o
`flip` alternando um parâmetro a cada batida cria forma (A/B) ou só
confunde? `divide` grande (÷16, ÷32) ainda dá um pulso "no tempo" pra
mudança de seção?

## 8. Integração e painel

Classe `Logic` (`type()` = `"LOGIC"`), 4 entradas, 5 saídas, 5
parâmetros. `panel()` próprio (~10 HP): knobs RATE/DIV/MULT/GATE/DELAY;
jacks CLK/A/B/RST in, DIV/AND/OR/XOR/FLIP out; Display (estado do
divisor / flip). Testado isolado (divisão, multiplicação, duty, atraso,
tabela-verdade, flip-flop, reset, autônomo) antes do patch. Cadeias
canônicas: `CLOCK → LOGIC(÷2) → ENVELOPE.gate`; `SEQUENCE.gate` +
`TURING` → `LOGIC.a`/`b` → `xor` → gate; `LOGIC.flip → QUANTIZER.root`
via cabo. Adicionado ao catálogo do painel (`apps/panel/ModuleCatalog.hpp`,
família TIME — junto do `CLOCK`).
