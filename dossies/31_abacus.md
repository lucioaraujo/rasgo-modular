# Dossiê — Módulo 31: Aritmética binária de CV (`ABACUS`)

**Família:** LOGIC / UTILITY
**Estado:** **implementado — marco 3** (2026-09-04)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Abacus.hpp`, `tests/test_abacus.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `LOGIC` recombina o TEMPO (divide/multiplica clock, AND/OR/XOR de
gates); o `CONTROL` faz utilidades **contínuas** de CV (atenuversor,
offset, `rectify` como `lerp`, slew). Faltava o processador que trata a
CV como **número inteiro** — resto, quantização a degraus, e um
**contador binário** cujos bits viram ritmo (a ideia do Numeric
Repetitor: padrões que emergem de contar, não de sequenciar). Aqui
também mora o **retificador dedicado** (meia-onda +/−, onda completa,
sinal) que o autor pediu.

- **`math`** (CV) = `a` ⊕ `b`, ⊕ por `op`: `0` soma · `1` subtração ·
  `2` multiplicação · `3` **resto** (`a` dobrado na janela `range`) ·
  `4-7` **bit a bit** (Lunetta — 2026-09-04): `a`/`b` viram inteiros de
  5 bits (janela ±`range` → 0..31), a lógica roda nos bits e o resultado
  volta a ±`range`: `4` AND · `5` OR · `6` XOR · `7` NAND;
- **`quant`** (CV) = a fonte encaixada em `steps` degraus iguais
  (espaçamento `range/steps`), com `slew` opcional;
- **`rect`** (CV) = `a` retificado por `rect_mode`: meia-onda + ·
  meia-onda − · onda completa (`|a|`) · **sinal** (`±range` ou 0);
- **contador**: cada `clock` soma `count_step` (pode ser negativo); o
  valor `c = count mod modulus` alimenta —
  - **`p1`** (gate) = o bit `bitA` de `c` (divisor de clock limpo, `bitA`
    escolhido por `pattern`);
  - **`p2`** (gate) = `bitA` XOR `bitA+1` de `c` (padrão sincopado,
    Gray-code);
  - **`carry`** (gate) = pulso quando o contador cruza um múltiplo de
    `modulus` (overflow → ritmo);
- **fonte autônoma:** sem `a` conectado, a fonte de `math`/`quant` é a
  própria rampa do contador (`c/modulus` bipolar em `range`) — o
  `ABACUS` sozinho já toca uma sequência de degraus **e** um ritmo
  (`p1`/`p2`/`carry`);
- **`rate`** = relógio interno se `clock` livre.

Desvio Rasgo: `count_step` fracionário/negativo (contar pra trás, pular);
a fonte autônoma (contador → melodia); `carry` como acento rítmico do
overflow.

Sem alocação / lock / IO em `process()`. Determinístico (sem RNG — é
aritmética pura; o contador é exato).

**Testes (14 funções, Debug + Release):** `op` 4–7 (bit a bit) →
`math` = a lógica AND/OR/XOR/NAND dos inteiros de 5 bits, exata, e
diferente da aritmética no mesmo sinal; `op=soma` → `math == a+b`
amostra a amostra; `op=resto` → `math ∈ [0, range)` e `= a − k·range`;
`quant` com `steps=4` → só 4 (±) níveis na saída, e patamares
constantes entre trocas; `rect_mode` 0/1/2/3 → `max(a,0)` / `min(a,0)` /
`|a|` / sinal, exatos; contador: `p1` divide o clock por `2^(bitA+1)`;
`carry` dispara 1×/`modulus` tiques com `count_step=1`; `count_step=−1`
→ contador anda pra trás (carry ainda 1×/modulus); `count_step=2` →
carry 2×/modulus; `reset` → contador a 0; sem `a` → `quant` sai da
rampa do contador (varia); `slew` alto → derivada por amostra pequena
nas trocas de `quant`; sem `clock` → contador anda em `rate`; tudo
finito; dois renders byte-idênticos; grafo `CLOCK → ABACUS.clock` ·
`quant → OSC.pitch` · `carry → ENVELOPE.gate`.

**Pendências (candidatos):** `shift`/rotação como `op` 8+; `modulus` e
`steps` como entradas de CV; euclidiano a partir do contador
(`c·k mod modulus`);
saída de **valor do contador** cru (CV rampa) como porta própria;
`carry` com largura proporcional; overflow "estouro" (satura vs enrola).

---

## 1. Problema musical e papel no fluxo

Contar é a operação mais simples que gera padrão. Um contador binário
com máscara dá ritmos sincopados sem sequenciador (Numeric Repetitor);
o resto (`mod`) dobra uma CV que sobe numa janela — melodia que "gira";
a quantização a inteiros transforma qualquer passeio numa escada. O
`ABACUS` junta essas três operações numéricas num módulo — e serve de
**retificador** de verdade quando é só disso que se precisa.

Papel: transformação de CV + fonte de ritmo/padrão. `TURING.cv →
ABACUS.a` + `quant → OSC.pitch` (o passeio do Turing vira escada);
`CLOCK → ABACUS.clock` + `p1`/`p2`/`carry` → gates de percussão;
`LFO → ABACUS.a` + `rect` → envelope de meia-onda. Sozinho: contador →
`quant` (melodia) + `carry` (acento). No `seedPatch` v2: destino de
`mod` (a/b) e de `gate` (clock), fonte de `slow` (math/quant/rect) e de
`gate` (p1/p2/carry).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Noise Engineering Numeric Repetitor / Bin Seq / Abacus** | contador binário + máscara → ritmo; "aritmética como gerador" | ficha/conceito, código não consultado |
| **Retificador clássico** (Maths / Serge DUSG têm saída retificada; ponte de diodos) | meia-onda e onda-completa como saídas separadas | prática de domínio público |
| **Aritmética modular** (teoria) | `a mod n` dobra uma reta numa janela; contadores módulo-N | teoria pública |
| **Divisor binário / Johnson counter** (teoria de circuito digital) | bit `k` de um contador = divisão por `2^(k+1)`; XOR de bits adjacentes = Gray code | teoria pública |
| **`op` do `CONTROL` / `LOGIC` do Rasgo** (código do autor) | seleção de operação por parâmetro; detecção de borda | código do autor |

## 3. Modelo — matemática, estados, extremos

`slewCoef = slew<=0 ? 1 : 1 − exp(−dt / (slew²·0.5))` (SampleHold).
`qStep = range / steps`.

Por amostra:
```
a = in.a (ou aSrc do contador se não conectado) ; b = in.b (ou 0)
op:  0 soma  m = a+b
     1 sub   m = a−b
     2 mul   m = a·b
     3 resto m = a − floor(a/range)·range            # [0, range)
     4-7 bit a bit: ia,ib = round((·+range)/(2·range)·31) ∈ 0..31
         4 AND  5 OR  6 XOR  7 NAND (& 31)
         m = ir/31·2·range − range
mathOut  += (clamp(m, −8, 8) − mathOut) · slewCoef

qv = round(a / qStep) · qStep
quantOut += (clamp(qv, −8, 8) − quantOut) · slewCoef

rect_mode: 0 max(a,0)  1 min(a,0)  2 |a|
           3 (a>ε ? range : a<−ε ? −range : 0)        # sinal — sem slew
```

Por tique de clock (interno em `rate` ou externo):
```
count += iround(count_step)
bucket = floor(count / modulus)
if bucket ≠ prevBucket: carryCd = 0.005·sr ; prevBucket = bucket
c = ((count mod modulus) + modulus) mod modulus       # 0..modulus−1
bitA = clamp(int(pattern·3.999), 0, 3)
p1 = (c >> bitA) & 1                                   # nível, mantido
p2 = ((c >> bitA) & 1) XOR ((c >> (bitA+1)) & 1)
aSrc = (float(c)/modulus)·2·range − range              # rampa bipolar do contador
```
Por amostra: `p1out/p2out` = níveis mantidos; `carryOut` = `carryCd>0`.

**Estados:** `count` (long), `prevBucket` (long), `prevClock`,
`prevReset`, `phase`, `p1_`, `p2_`, `aSrc_`, `carryCd_`, `mathOut_`,
`quantOut_`. Sem alocação.

**Extremos.** `b` não conectado → soma/sub = `a`, mul = 0, resto
ignora `b`. `a` e `b` nulos → `math`/`quant`/`rect` = 0, mas o contador
segue (fonte autônoma usa `aSrc`). `steps=2` → `quant` binário (−q/0/q).
`range` pequeno + `op=resto` → `math` "gira" rápido numa janela mínima.
`count_step=0` → contador congela, `carry` nunca dispara, `p1`/`p2`
travados. `count_step` grande vs `modulus` → `carry` a cada tique
(estouro contínuo — honesto). `modulus=2` → `c ∈ {0,1}`, `p1` = ÷2,
`p2` = ÷2 XOR ÷4 (mas bit 1 é 0 → `p2` = `p1`). Multiplicação com `a`,`b`
> 1 → `math` pode passar de ±1; o `clamp(±8)` evita explosão, o
destino que se cuide.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → relógio interno em `rate`, contador
  anda; `quant` sai a rampa do contador encaixada em `steps` (uma
  sequência de degraus), `p1`/`p2` = padrões divididos, `carry` =
  acento a cada `modulus`. Toca — melodia + ritmo — ao carregar.
- **Performance:** `op`/`rect_mode` ao vivo mudam a operação; `modulus`
  e `steps` reescrevem o padrão; `count_step` inverte/pula a contagem;
  `pattern` desloca a síncope de `p1`/`p2`.
- **Híbrida:** `TURING.cv → a` + `quant → OSC.pitch`; `CLOCK → clock` +
  `carry → ENVELOPE.gate` (acento); `SCOPE.pitch → a` + `rect` (só a
  metade positiva do pitch detectado); `DRIFT.a → b` (a operação
  deriva).

## 5. Portas, parâmetros, limites

**Entradas:** `a` (Control), `b` (Control), `clock` (Control trig),
`reset` (Control trig).
**Saídas:** `math` (Control), `quant` (Control), `rect` (Control),
`p1` (Control, gate), `p2` (Control, gate), `carry` (Control, gate).
**Parâmetros:** `op` (0–7, def 0), `modulus` (2–32, def 8), `steps`
(2–16, def 8), `range` (0.1–4, def 1), `rect_mode` (0–3, def 2),
`count_step` (−4–4, def 1), `pattern` (0–1, def 0,3), `slew` (0–1,
def 0), `rate` (0.1–30 Hz, def 4).
**Limites:** CV de saída clampada a ±8. CPU: aritmética + 1 `exp` por
bloco (slew) + `round`. Sem alocação, sem `sin`, sem RNG.

## 6. Alternativas descartadas

- **Modo do `LOGIC`:** o `LOGIC` já tem 5 parâmetros e um propósito
  claro (recombinar tempo); somar aritmética de CV e contador binário
  ali sobrecarrega. `ABACUS` é o par de CV do `LOGIC`.
- **Bit a bit real (`a AND b`) já no marco 1:** exigia definir a
  quantização; ficou pra 2ª camada (2026-09-04) como `op` 4–7 —
  janela ±`range` → inteiro de 5 bits (0..31), AND/OR/XOR/NAND, volta a
  ±`range`. 5 bits: fino o bastante pra padrão, grosso o bastante pra a
  lógica ser audível.
- **`rect` como `lerp(x,|x|)` (igual ao `CONTROL`):** o `CONTROL` já faz
  o retificador *contínuo* (quanto). O `ABACUS` faz o retificador
  *discreto* (qual modo) — meia +/−, completa, sinal. Complementar, não
  duplicado.
- **Slew no `rect`:** o retificador tem que ser instantâneo (é o ponto
  dele); quem quiser suavizar põe um `CONTROL`/`LPG` depois.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `op=0`, `a` rampa, `b` DC 0.2 → `math[f] == a[f]+0.2`;
`op=3`, `a` sobe de 0 a 3, `range=1` → `math` faz dente de serra 0→1
(3 ciclos); `steps=4`, `range=2`, `a` rampa −2..2 → `quant` assume
exatamente {−2,−1,0,1,2}·(0.5) e fica constante entre as trocas;
`rect_mode=2`, `a` senoide → `math`... `rect == |a|` exato; `rect_mode=3`
→ `rect ∈ {−range,0,range}`; `count_step=1`, `modulus=8` → `carry`
dispara a cada 8 tiques; `count_step=2` → a cada 4; `count_step=−1` →
contador decresce, `carry` ainda a cada 8; `p1` com `bitA=0` → meio
período do clock; `reset` → `c` volta a 0; sem `clock` → tiques em
`rate`; `slew=0.5` → |Δ quant| por amostra pequeno; dois renders
byte-idênticos.

**Escuta:** o contador sozinho (`quant` + `carry`) soa como uma frase
que se repete com lógica (não aleatória)? `op=resto` numa CV que sobe
dá um arpejo que "gira" de forma musical? `pattern` desloca a síncope de
`p1`/`p2` de um jeito útil pra percussão? o `rect` de meia-onda num LFO
faz um bom envelope unipolar? `count_step` negativo/2 muda a sensação
rítmica do `carry`?

## 8. Integração e painel

Classe `Abacus` (`type()` = `"ABACUS"`), 4 entradas, 6 saídas, 9
parâmetros. `panel()` próprio (14 HP): `Display` (valor do contador +
bits), knobs OP/MOD/STEPS/RANGE · RECT/CNT/PAT/SLEW/RATE, jacks
A/B/CLK/RST in, MATH/QNT/RCT · P1/P2/CRY out. Testado isolado
(aritmética, quant, rect, contador, carry, reset, slew, autônomo) antes
do patch. Cadeias canônicas: `CLOCK → ABACUS.clock` · `quant →
OSC.pitch` · `carry → ENVELOPE.gate`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família DECISION, junto de
`DECISION`/`DRIFT`/`QUANTIZER`/`HARMONY`).
