# Dossiê — Módulo 16: Mixer estéreo (`MIXER`)

**Família:** MIX
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Mixer.hpp`, `tests/test_mix.cpp`

## Estado da implementação (marco 2)

Feito: onde as vozes viram uma **imagem estéreo**. 4 canais mono, cada
um com **ganho** (dB, −60..+12), **pan** (lei de potência constante) e
**mute**. Saída estéreo — L no canal 0, R no canal 1. Num grafo mono a
saída é (L+R)/2. Ganho de saída global.

- **pan de potência constante:** `t = (pan+1)·π/4`;
  `L·cos(t)`, `R·sin(t)` — energia constante ao varrer o pan, sem o
  buraco no centro da lei linear;
- **fan-in num nó, não N cabos numa porta:** o `SignalGraph` só aceita
  uma conexão por porta de entrada (fan-in explícito com orçamento de
  ganho); o `MIXER` é a forma canônica de somar várias fontes;
- canal sem cabo → tratado como silêncio.

Determinístico (sem RNG), sem alocação.

**Testes (9/9 alvos MIXER+MASTER, Debug + Release):** pan centro → L = R
= 0,707; pan totalmente à esquerda → L = 1, R = 0; à direita → L = 0,
R = 1; `gain` −6 dB → metade; `mute` → 0; `out_gain` −6 dB → metade;
grafo mono → saída = (L+R)/2; integração no grafo (duas vozes panoramadas
→ `MIXER` → `MASTER`, L ≠ R comprovado); painel fecha (16 HP).

**Pendências (candidatos, não controles fictícios):** mais canais
(`MIXER8`, ou canais estéreo de verdade — 2 portas por canal); `send`
por canal (auxiliar pra `SPACE`); suavização de ganho/pan ~5 ms (hoje
salta ao vível de bloco, ok pra control-rate); solo; VU por canal;
crossfeed (mono compatibilidade).

---

## 1. Problema musical e papel no fluxo

Os módulos de voz produzem sinal, mas o `SignalGraph` obriga fan-in explícito:
não dá pra ligar 3 vozes na mesma entrada. Faltava o **somador com
orçamento de ganho** — e, já que estamos somando, o lugar natural pra
**posicionar cada voz no campo estéreo**. Sem o `MIXER`, o instrumento é
mono e as vozes brigam pelo mesmo ponto.

Papel: recebe as vozes (`ENVELOPE.out`, `STRING.out`, `MATTER.out`,
`SPACE.out`…), dá ganho e pan a cada uma, e entrega uma saída estéreo pro
`MASTER` (Módulo 17). É o penúltimo nó de quase todo patch.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Lei de pan de potência constante** (sin/cos; qualquer texto de mixagem) | energia constante ao varrer o pan; evita o "buraco" de −6 dB no centro da lei linear | teoria pública |
| **Mixer de barramento** (mesa analógica clássica) | ganho por canal + soma + ganho de saída; mute; a soma como um estágio explícito | prática de estúdio |
| **`ORQUESTRACAO_INSTRUMENTOS.md`** (RASGO) | "fan-in com orçamento de ganho e mixer explícito" — decisão de arquitetura do próprio workspace | conceito próprio |

## 3. Modelo — matemática, estados, extremos

Por canal `c` (uma vez por bloco): `on = mute<c> < 0,5 e cabo presente`;
`gainLin = 10^(gain<c>/20)` (0 abaixo de −60 dB);
`t = clamp(pan<c>, ±1)·π/4 + π/4`; `panL = cos(t)`, `panR = sin(t)`.

Por amostra: `L = Σ_c in_c·gainLin_c·panL_c`,
`R = Σ_c in_c·gainLin_c·panR_c`; depois `·10^(out_gain/20)`.
Escrita: canais ≥ 2 → `out[0]=L, out[1]=R`; senão → `out[0]=(L+R)/2`.

**Estados:** nenhum (sem memória). Sem alocação.

**Extremos.** `gain = −60 dB` → canal mudo (gainLin 0). Todos os 4 canais
com sinal alto → a soma pode passar de ±1; o `MASTER` (a jusante) tem o
limitador. `pan` fora de [−1,1] → clampado. Grafo com >2 canais → canais
extras recebem a média (mono-fold).

## 4. Três modos obrigatórios

- **Autônoma:** com os canais roteados e ganhos/pans fixos, o `MIXER` é
  a mesa do patch — não precisa de controle externo.
- **Performance:** ganhos e pans são os gestos de mixagem ao vivo (subir
  uma voz, abrir o estéreo, mutar uma camada). Sliders de ganho, knobs
  de pan.
- **Híbrida:** um `DECISION`/LFO modulando um `gain` de canal via
  `connectToParameter` = automação de mix generativa (uma voz entra e
  sai sozinha).

## 5. Portas, parâmetros, limites

**Entradas:** `ch1`..`ch4` (Audio, mono).
**Saídas:** `out` (Audio, estéreo — L=0, R=1).
**Parâmetros:** por canal `gain<c>` (−60..+12 dB), `pan<c>` (−1..+1),
`mute<c>` (0/1); global `out_gain` (−24..+12 dB). **13 no total.**
**Limites:** saída não limitada (o `MASTER` limita). CPU: 4 mult/add por
amostra por canal. Sem alocação, sem estado.

## 6. Alternativas descartadas

- **Fan-in implícito** (N cabos numa porta): o `SignalGraph` proíbe de
  propósito (orçamento de ganho, origem rastreável). O `MIXER` é a forma
  canônica.
- **Canais estéreo de entrada** (2 portas por canal): útil pra somar
  saídas já estéreo (`SPACE`), mas dobra as portas; mono-in + pan cobre
  o caso comum. Candidato (`MIXER` estéreo).
- **Lei de pan linear:** mais simples, mas o buraco de −6 dB no centro
  é audível ao automatizar o pan.
- **Suavização de ganho já no marco 1:** salto ao vível de bloco
  (~5 ms) não estala em control-rate; ramp é 2ª camada.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** pan centro → L = R = −3 dB (0,707); pan extremo → toda a
energia num lado; `gain` em dB bate com o ganho linear medido; `mute`
zera; soma de canais idênticos com pans opostos → mono no centro; sem
alocação.

**Escuta:** varrer o pan soa como a voz atravessando o campo sem sumir
no meio? subir um canal ao vivo é imediato e sem estalo? o estéreo
"abre" o patch (as vozes deixam de brigar)? automatizar um `gain` de
canal soa como uma voz respirando na mix?

## 8. Integração e painel

Classe `Mixer` (`type()` = `"MIXER"`), 4 entradas, 1 saída, 13
parâmetros. `panel()` próprio (16 HP: 4 tiras verticais CH1..4 com
slider de ganho + knob de pan + toggle de mute + jack, e OUT + jack de
saída). Testado isolado (lei de pan, ganho, mute, mono) antes do patch.
No patch: vozes → `MIXER` → `MASTER` → saída. É o primeiro nó estéreo
do instrumento.
