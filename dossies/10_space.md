# Dossiê — Módulo 10: Espaço multitap (`SPACE`)

**Família:** SPACE
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Space.hpp`, `tests/test_space.cpp`

## Estado da implementação (marco 2)

Feito: linha de atraso de 2,2 s com **N tomadas** (1–8) em tempos
diferentes; **realimentação** com filtro de tom no laço; **difusão** por
cadeia de 4 all-pass só na saída molhada; **modulação** de leitura por
LFO interno lento (chorus/cintilação). De eco rítmico a cauda de
reverberação, pelos mesmos controles.

- **tomadas:** tomada `k` lê em `baseDelay·(1 − spread + spread·(k+1)/taps)`
  — `spread` 0 → todas no mesmo tempo (eco simples), `spread` 1 → tomadas
  espalhadas do início ao fim do atraso; ganho cai −12% por tomada;
  leitura com interpolação linear;
- **realimentação:** `buffer[w] = tanh(in + feedback·LP(wet))`,
  `LP` 1-polo com coeficiente por `tone` (1 = brilhante / passa tudo,
  0 = escuro); `feedback` travado em ≤ 0,97;
- **difusão:** `wet_saída = (1−diffusion)·wet + diffusion·allpass⁴(wet)`
  — all-pass de comprimentos primos curtos (223/353/523/739 amostras),
  `g = 0,6`; fora do laço de realimentação (mantém o andamento dos ecos);
- **modulação:** LFO senoidal ~0,13 Hz desloca a leitura das tomadas em
  ±(`mod`·4 ms), com sinal alternado por tomada;
- `mix` seco/molhado; saídas `out` (misturada) e `wet` (só molhado).

**Anti-zíper (2026-09-08):** `time`, `spread`, `feedback`, `tone` e `mix`
são suavizados **por amostra** (one-pole ~10 ms) rumo ao valor do bloco.
Sem isto, quando o VARIA ou um arrasto de knob varria o `time` a tomada
de atraso saltava de posição a cada bloco = um clique na cauda. Um flag
`ctlPrimed_` assenta no valor exato no 1º bloco → **patch estático fica
byte-idêntico** (o `test_space` passa sem mudança). Mesma lição de
`Parametric`/`Shape`/`Hall`/`Resonator`/`Crush`.

Determinístico (LFO senoidal, sem RNG). Buffer + buffers de all-pass
alocados em `prepare()`; `process()` não aloca.

**Testes (8/8 alvos, Debug + Release):** com `taps = 1`, `spread = 0`,
`diffusion = 0`, `mod = 0` → a tomada chega em `~time·sr` amostras (±40);
`feedback = 0,7` → ecos em múltiplos do atraso, decaindo; `mix = 0` →
porta `out` byte-a-byte igual à entrada; `feedback = 0,95` + difusão +
mod por 6,25 s → sem NaN nem `|y| > 1,5`, e a **cauda não cresce**
(realimentação estável); `tone = 1` mantém mais agudo na cauda que
`tone = 0` (energia de alta frequência 1,3× maior); dois renders
byte-idênticos; integração no grafo (`FUNCTION → SPACE`); painel fecha
(14 HP).

**Pendências (candidatos, não controles fictícios):** reverb verdadeiro
por rede de atraso realimentada (FDN / figura-8 de Dattorro) como modo
`diffusion` alto; estéreo real (tomadas com pan, all-pass decorrelados
L/R); `time` acompanhando um clock (eco sincronizado ao `CLOCK`);
pitch-shift na realimentação (shimmer); `freeze` do buffer (cruza com o
`MEMORY`); ducking/side-chain pela entrada; interpolação cúbica na
leitura (alia um pouco com `mod` alto).

---

## 1. Problema musical e papel no fluxo

Todos os módulos até aqui produzem som "no ponto" — sem lugar. `SPACE` dá
o **onde**: distância, sala, eco, ambiente. E não como dois efeitos
separados ("delay" e "reverb") mas como um contínuo — poucas tomadas
espaçadas = eco rítmico; muitas tomadas + difusão + realimentação =
cauda. É o último bloco de quase todo patch e o que "cola" as vozes num
mesmo ar.

Papel: recebe a soma das vozes (ou `MATTER.out`, `MEMORY.out`); `time`
modulado por um LFO/`DECISION` faz o espaço respirar; `feedback` e
`diffusion` são gestos de "abrir a sala". A saída vai direto à saída
final.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **M. Schroeder** (1962) / **J. A. Moorer** (1979) | reverberação artificial por comb + all-pass; difusão = all-pass em série; damping na realimentação | artigos públicos |
| **J. Dattorro**, "Effect Design Part 1: Reverberator and Other Filters" (JAES 1997) | topologia de reverb por laço de all-pass com filtro passa-baixa (damping) no caminho de realimentação; comprimentos primos | artigo público (a topologia é conhecida; reescrito) |
| **Mutable Rainmaker** / multitap clássico (Lexicon PCM, EHX 16-second) | N tomadas com tempo e ganho próprios como material rítmico e melódico | hardware, estudo de comportamento |
| **chorus/ensemble** (Dattorro; Roland Juno) | LFO no tempo de leitura → desafinação lenta que engrossa a cauda | técnica pública |

## 3. Modelo — matemática, estados, extremos

**Escrita:** buffer circular de `2,2·sr`. Por amostra:
```
wet   = (1/taps)·Σ_{k<taps} readInterp( d_k )·(1 − 0,12k)
d_k   = baseDelay·(1 − spread + spread·(k+1)/taps) + lfo·(±1)
LP   += lpCoeff·(wet − LP)          ; lpCoeff = 0,08 + tone·0,9
buffer[w] = tanh( in + feedback·LP )
```
`readInterp(d)` = interpolação linear em `w − d` (com wrap), `d`
travado em [1, bufLen−2].

**Difusão (saída):** `y = wet`; 4×
`y ← allpass_i(y, 0,6)` com
`v = y − 0,6·buf_i[p]; buf_i[p] = v; y = buf_i[p_antigo] + 0,6·v`;
`wet_saída = (1−diffusion)·wet + diffusion·y`.

**Saída:** `out = mix·wet_saída + (1−mix)·in`.

**Estados:** buffer principal + 4 buffers de all-pass (223+353+523+739
floats), `writePos_`, `lpState_`, `lfoPhase_`, 4 `apPos_`. Sem alocação
em `process()`.

**Extremos.** `feedback = 0,97` (teto) + `tone` alto: o `LP` tira pouca
energia, mas o `tanh` na escrita limita a amplitude → cauda longa que
não diverge (testado 6,25 s). `time` mínimo (2 ms) → `d ≈ 96` amostras,
eco quase-comb (metálico) — comportamento válido. `mod` alto + `time`
pequeno → `d` pode chegar perto de 1; travado em ≥ 1. `spread = 0` +
`taps > 1` → todas as tomadas no mesmo tempo (somam → eco mais forte).
Reset → buffers zerados.

## 4. Três modos obrigatórios

- **Autônoma:** sem entrada, `SPACE` é silêncio (é um efeito). Mas com
  `feedback` alto, um único impulso vira uma textura auto-sustentada por
  muitos segundos (o "16-second delay" como instrumento).
- **Performance:** `time`, `feedback`, `diffusion`, `mix` são os gestos
  — varrer `time` faz "fita rebobinando"; abrir `feedback` e `diffusion`
  juntos leva de slap a hall continuamente; `spread` reconfigura o
  padrão rítmico das tomadas.
- **Híbrida:** `time_mod` de um LFO lento = chorus/flanger na cauda;
  `feedback_mod` de um `DECISION` = a sala "abre e fecha" por seção; a
  entrada vindo de várias vozes somadas (via `Sum`) põe tudo no mesmo
  ambiente.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `time_mod` (Control, soma linear em s),
`feedback_mod` (Control, soma).
**Saídas:** `out` (Audio, misturada), `wet` (Audio, só molhado).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `time` | 0,002–2 s | 0,28 | atraso base |
| `taps` | 1–8 | 4 | nº de tomadas |
| `spread` | 0–1 | 0,6 | espalhamento das tomadas no atraso |
| `feedback` | 0–0,95 | 0,35 | realimentação (cauda) |
| `diffusion` | 0–1 | 0,4 | eco discreto → cauda espalhada |
| `tone` | 0–1 | 0,5 | brilho da realimentação |
| `mod` | 0–1 | 0,15 | LFO no tempo de leitura (chorus) |
| `mix` | 0–1 | 0,35 | seco/molhado |

**Limites:** saída em [−1,1] (`tanh` na escrita). CPU: `taps`
interpolações + 4 all-pass + 1 LP por amostra. Buffers alocados em
`prepare()`.

## 6. Alternativas descartadas

- **FDN completo (reverb de rede) já no marco 2:** som de sala melhor,
  mais estado e ajuste; a cadeia de all-pass + multitap + damping cobre
  de eco a "reverb aproximado" com muito menos código. FDN é candidato
  para um modo.
- **All-pass dentro do laço de realimentação:** dá difusão mais densa,
  mas embaralha o andamento dos ecos (o teste `testEchoDelay` deixaria de
  fazer sentido) e complica a estabilidade. Fora do laço é a escolha do
  marco.
- **Estéreo real no marco 2:** dobra o custo e o estado; mono primeiro,
  estéreo é 2ª camada (com pan por tomada e all-pass decorrelados).
- **Interpolação cúbica / all-pass fracionário na leitura:** alia menos
  com `mod` forte; linear basta pro marco (mesma lógica dos outros
  módulos).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** com uma tomada e sem difusão/mod, o pico do impulso na
saída molhada cai em `time·sr` amostras (±0,1%); `feedback` produz ecos
em múltiplos do atraso, decaindo geometricamente; `mix = 0` byte-a-byte
igual à entrada na porta `out`; `feedback` no teto não diverge em
> 5 s; `tone` alto preserva mais energia de alta frequência na cauda;
dois renders byte-idênticos; sem alocação em `process()`.

**Escuta:** varrer `time` soa como um espaço mudando de tamanho ou como
glitch? abrir `feedback` + `diffusion` leva de eco a sala de forma
contínua e musical? `mod` engrossa a cauda ou só desafina? o `tone`
escurece a cauda como um ambiente absorvente real? somar todas as vozes
num `SPACE` "cola" o patch?

## 8. Integração e painel

Classe `Space` (`type()` = `"SPACE"`), 3 entradas, 2 saídas, 8
parâmetros. `panel()` próprio (14 HP: display das tomadas +
TIME/TAPS/SPREAD/FBK, DIFF/TONE/MOD/MIX, jacks). Testado isolado (atraso,
realimentação, mix, estabilidade, tom) antes do patch. É o bloco final
de um patch: `MATTER`/`FILTER`/`MEMORY` → `SPACE` → saída, com `time` e
`feedback` modulados — o ar em que a peça acontece.
