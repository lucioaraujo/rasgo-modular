# Dossiê — Módulo 46: Reverberação FDN (`HALL`)

**Família:** SPACE
**Estado:** **implementado — Onda C** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Hall.hpp`, `tests/test_hall.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda C, #46 — "reverb FDN")

## Estado da implementação

O `SPACE` (#10) é multitap + all-pass em série (Schroeder/Dattorro) — ótimo
de eco a cauda média. `HALL` é a **rede de atraso realimentada** (FDN) que
o `§2` anotou como pendência: **8 linhas de atraso** realimentadas por uma
**matriz de Householder** (reflexão ortogonal — sem perda, difusão máxima
por operação barata), decaimento dependente da frequência, e modulação das
linhas pra a cauda não "apitar" metálica.

**Decisão (2026-09-06):** módulo NOVO, não modo do `SPACE`. A topologia é
diferente o bastante (FDN vs comb+allpass) e o `SPACE` já está entregue e
testado — não reescrever.

- **`size`** (0–1, + CV) = escala os 8 comprimentos de linha
  (`0,3× a 1,7×`, ~8 ms a ~88 ms) — de sala pequena a hall grande.
- **`decay`** (0–1, + CV) = RT60. `RT60 = 0,2 · 75^decay` (0,2 s a 15 s).
  O ganho por linha sai de `g_i = 10^(−3·d_i/RT60)` (`d_i` = tempo da
  linha). Perto de `decay = 1`, `g_i → 1` (cauda quase infinita).
- **`damp`** (0–1) = passa-baixa de 1 polo NO laço de cada linha
  (`fc = 18 kHz − damp·16 kHz`) — o agudo decai antes do grave, como
  numa sala real.
- **`mod`** (0–1) = profundidade da modulação do ponto de leitura de cada
  linha (LFOs lentos, ~0,5–1,4 Hz, fases distintas) — *chorus* na cauda,
  quebra o ringing. **Determinístico** (senóides, sem RNG).
- **`pre`** (0–1) = pré-atraso (0 a ~120 ms) antes da rede — separa o som
  direto da reverberação (define o tamanho percebido).
- **`mix`** (0–1) = seco ↔ molhado.

**`freeze`** (gate) = `g_i → 1` (cauda infinita, Householder preserva
energia) + rampa da entrada pra 0 — congela o que está na rede, como o
`hold` do `LOOPER` mas pra o espaço.

**Estéreo:** entrada mono distribuída às 8 linhas por um vetor de sinais
alternados; saídas `l`/`r` = combinações diferentes das tomadas
(descorrelacionadas → imagem larga).

**Segurança:** a matriz de Householder é ortogonal → a rede é estável
pra `g_i ≤ 1` (nunca cresce). `freeze` (`g=1`) é *lossless* — limitado,
não decai. `softLimit` na saída + *flush* de denormais. `prepare()` aloca
~8×5 k floats por linha + o pré-atraso (~0,4 MB); `process()` não aloca.
Determinístico sempre.

**Testes (Debug + Release):** impulso na entrada → cauda que **decai**
(RMS de janela cai monotônico) com `decay` baixo, e **sustenta** com
`decay` alto; `decay` maior → cauda mais longa (T60 medido cresce);
`damp` alto → a cauda perde agudo mais rápido que o grave (energia HF cai
mais); `size` maior → densidade de ecos menor no início (mais esparso) e
pré-eco mais tardio; `freeze` → a cauda para de decair (RMS estável por
segundos) e a entrada nova não entra; `pre` atrasa o início do molhado;
`mix=0` → passa-direto bit-exato; estéreo `l`≠`r` (correlação < 0,9);
dois renders byte-idênticos com `mod>0`; entrada de ruído a −6 dBFS,
todos os extremos → saída finita e limitada.

**Pendências (candidatos):** all-pass de entrada (difusores Dattorro) pra
o *early reflections* mais denso; matriz de Hadamard como alternativa
(comparar cor); *shimmer* (pitch-shift no laço); `size` com *crossfade*
suave (hoje re-lê, pode clicar em varredura rápida — mitigado pela
interpolação); tomada de *early reflections* separada; entrada estéreo
real (hoje soma mono).

---

## 1. Problema musical e papel no fluxo

O hall de concerto, a placa (*plate*), o *ambience* que cola uma mixagem,
a nuvem infinita de pad congelado — o `SPACE` faz a versão "eco que vira
cauda", mas não a densidade lisa de um FDN bem afinado. `HALL` põe isso
no patch: entra áudio, saem `l`/`r`, e `size`/`decay`/`damp` são o espaço.
Como aceitam CV, `LFO → size` faz a sala "respirar" (e desafina a cauda,
efeito de fita), `ENVELOPE → mix` faz a reverberação nascer com a nota, e
o gate `freeze` sustenta um acorde pra sempre.

Distinção: o `SPACE` é eco→cauda por comb+allpass; o `MEMORY` é granular;
o `LOOPER` é delay de linha com hold/reverse. `HALL` é a **rede**
realimentada — a reverberação densa.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Jot & Chaigne (1991), "Digital delay networks for designing
  artificial reverberators"** — o FDN: N linhas + matriz unitária de
  realimentação + ganhos de decaimento por linha derivados de um RT60
  alvo. Teoria pública, a base de todo reverb FDN.
- **Householder / Hadamard feedback matrix** — matrizes ortogonais
  baratas pra a realimentação (Householder: `y = x − (2/N)Σx`). DSP
  clássico, domínio público.
- **Dattorro (1997), "Effect Design Part 1: Reverberator..."** — damping
  no laço, modulação das linhas pra descolorir; já citado pelo `SPACE`.
- **Frequency-dependent decay** (Moorer; Jot) — passa-baixa de 1 polo no
  laço = agudo decai mais rápido; RT60(f).
- **NE Desmodus Versio ★ (`§7 #72`), Strymon StarLab ★ (`§7 #95`)** —
  *referência funcional*: reverbs de Eurorack com *freeze*, modulação,
  `size`/`decay` como performance. Fechados; só o conjunto de gestos.

**Desvio Rasgo (Atlas §49):** a modulação determinística das linhas (a
cauda "vive" sem RNG, reprodutível), `freeze` como gate do grafo, e a
relação — `size`/`decay` endereçáveis por CV, então o espaço é um
**processo** dirigido pelo patch.

## 3. Modelo

**Preparação:** 8 comprimentos base (amostras a 48 k, primos entre si):
`{1237, 1381, 1607, 1777, 1949, 2137, 2273, 2477}`. Buffer por linha =
`⌈maxLen·1,7⌉ + modMax + 4`. Pré-atraso: buffer de `⌈0,13·sr⌉`.

**Por amostra:**
```
xin = pré-atraso(in, pre·0,12·sr)            (interp linear)
xin ·= inRamp                                 (rampa → 0 em freeze)
para i em 0..7:
  d_i  = baseLen_i · (0,3 + size·1,4)
  rd_i = d_i + mod·18·sin(2π·modPh_i)         (modPh_i a ~0,5–1,4 Hz)
  s_i  = readLine(i, rd_i)                    (interp linear no buffer)
  lp_i += (s_i − lp_i)·dampCoef ; s_i = lp_i  (passa-baixa no laço)
  s_i ·= g_i                                  (g_i de RT60; 1 em freeze)
h   = (2/8)·Σ s_i                             (Householder)
para i em 0..7:
  y_i = s_i − h
  writeLine(i, xin·inVec_i + y_i)
wetL = Σ cL_i · lineOut_i ; wetR = Σ cR_i · lineOut_i
outL = softLimit(in·(1−mix) + wetL·mix·0,6)
outR = softLimit(in·(1−mix) + wetR·mix·0,6)
```
`inVec` = `{+,+,+,+,−,−,−,−}·0,5`. `cL` toma linhas pares com sinais
alternados, `cR` as ímpares — as duas saídas descorrelacionam.

**`freeze`:** borda ↑ → alvo de `inRamp` = 0, `g_i` = 1; borda ↓ →
`inRamp` volta a 1, `g_i` volta ao valor de `decay`. Rampas de ~10 ms.

**Extremos:** `decay = 1` sem `freeze` → RT60 15 s, `g_i ≈ 0,99` (decai,
lento). `freeze` → `g_i = 1` exato, *lossless*, cauda constante (não
cresce — Householder é ortogonal). `size` no mínimo → linhas de ~8 ms →
quase um *comb* denso agudo (fica anotado: FDN curto = cor metálica, é o
"plate pequeno"). `mod` no máximo + `size` pequeno → desafinação audível
(efeito, não bug). Entrada em silêncio → cauda decai a silêncio (com
`freeze`, fica no que tinha). `mix = 0` → `wet` não soma, saída = `in`
(bit-exato).

## 4. Três modos obrigatórios

- **autônoma:** qualquer áudio do patch entra e a rede o transforma numa
  cauda que se move (`mod` leve); sem cabo de controle já é um espaço
  vivo.
- **performance:** `size`/`decay`/`mix` são os macros; `freeze` (gate)
  sustenta.
- **híbrida:** `ENVELOPE → mix` (a reverb nasce com a nota),
  `LFO → size` (a sala respira / desafina), `CLOCK → freeze` não faz
  sentido (gate longo) — o `freeze` é gesto de mão ou de um `FUNCTION`
  em *one-shot* longo.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `size` (Control), `decay` (Control),
`freeze` (Control gate).
**Saídas:** `l` (Audio), `r` (Audio).
**Parâmetros:** `size` (0–1, def 0,5), `decay` (0–1, def 0,5), `damp`
(0–1, def 0,4), `mod` (0–1, def 0,2), `pre` (0–1, def 0), `mix`
(0–1, def 0,3).
**Limites:** `l`/`r` limitados por `softLimit` (linear até 0,8, `tanh`
depois — o mesmo do `FILTER`). CPU: por amostra 8× (leitura interpolada +
1 polo + mul) + 8 `sin` (mod) + a soma da matriz (8 add). `prepare` aloca
~0,4 MB.

## 6. Alternativas descartadas

- **modo do `SPACE`** — a topologia FDN não encaixa no objeto multitap+
  allpass do `SPACE` sem reescrevê-lo; módulo à parte é mais limpo e não
  mexe no que está entregue.
- **matriz de Hadamard** — igualmente barata e ortogonal; Householder foi
  escolhida por ser 1 subtração por linha (Hadamard precisa da
  borboleta). Hadamard fica como pendência pra comparar a cor.
- **all-pass de entrada (Dattorro)** — melhora as *early reflections* mas
  é mais estado; a rede FDN já difunde. Pendência.
- **RNG na modulação** — senóides incomensuráveis já dão movimento
  não-repetitivo e determinismo.
- **oversampling** — a rede é linear (sem não-linearidade além do
  `softLimit` de saída, que raramente atua); não alia.

## 7. Integração e painel

14 HP, família **SPACE** (junto de `SPACE`/`LOOPER`/`MEMORY`). Display da
cauda (o `SCOPE` mostra). Knobs `SIZE`/`DECAY`/`DAMP`/`MOD` (linha 1),
`PRE`/`MIX` (linha 2); jacks `IN`/`SIZE`/`DEC`/`FRZ` + `L`/`R`.

Cadeias canônicas: `qualquer voz → HALL → MIXER` (`mix` ~0,3);
`ENVELOPE.env → HALL.mix`; `LFO → HALL.size`; gate de performance →
`HALL.freeze` (pad infinito).
