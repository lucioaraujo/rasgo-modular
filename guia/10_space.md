# SPACE — atraso multitap com difusão

**Família:** SPACE · **Módulo 10**
**Essência:** uma linha de atraso com várias tomadas, realimentação
filtrada e uma cadeia de all-pass que espalha os ecos até virarem cauda.
De eco rítmico a reverberação, pelo mesmo objeto.
**Dossiê técnico:** [`../dossies/10_space.md`](../dossies/10_space.md)
· **Fonte:** `src/dsp/Space.hpp`

---

## A ideia

Eco e reverberação não são dois efeitos diferentes — são as duas pontas
de um contínuo. Um atraso com uma tomada e sem difusão é um *slapback*;
abra as tomadas, a realimentação e a difusão e o mesmo módulo chega a um
*hall*, sem trocar de nada. O lugar dele no fluxo é o fim: `MATTER` /
`FILTER` / `MEMORY` → `SPACE` → saída. Somar várias vozes num único
`SPACE` "cola" o patch.

## Por dentro

**Uma linha de atraso "multitap", o que significa:** existe **um** buffer
de 2,2 s guardando o áudio que passou — mas em vez de lê-lo num único
ponto (um eco só), o módulo o lê em **até 8 pontos diferentes** ao
mesmo tempo (as `TAPS`), cada um "olhando" um tempo passado diferente,
e soma tudo na saída — várias repetições do mesmo som, cada uma vinda
de um instante distinto do passado, sem precisar de 8 buffers
separados. `SPREAD` decide **como** esses 8 pontos se distribuem no
tempo: 0 os concentra todos juntos (na prática, quase o mesmo tempo —
um eco só); 1 os espalha ao longo de todo o intervalo — um padrão
rítmico de repetições em tempos diferentes.

`FEEDBACK` pega a **soma** das tomadas e a manda de volta pro início da
linha — cada volta pelo laço soma mais uma geração de repetições
(a cauda cresce), passando por um filtro de tom (`TONE`) que escurece
um pouco a cada volta (a mesma física de "o agudo se perde primeiro"
descrita no `HALL`, #46, e no `DAMP` do `MATTER`/`RESONATOR`).

**`DIFFUSION`, o que faz um eco discreto virar uma cauda contínua:** um
**filtro all-pass** é um tipo especial de filtro que não muda **o
volume** de nenhuma frequência — só muda **a fase** (o "alinhamento no
tempo") de cada uma, de um jeito diferente pra cada frequência.
Isoladamente isso é quase inaudível; mas encadear **vários** all-pass
em série faz picos e ecos discretos se **espalharem** no tempo — cada
repetição individual deixa de soar como um "eco" nitidamente separado e
passa a se misturar com as vizinhas, formando uma textura contínua e
lisa (é literalmente a técnica clássica — Schroeder/Moorer/Dattorro —
usada em quase todo reverb digital pra transformar ecos discretos numa
"nuvem" sem repetições identificáveis). `DIFFUSION` mistura a saída
crua (ecos discretos, ainda reconhecíveis) com essa versão passada
pelos all-pass (espalhada, tipo sala) — subir `DIFFUSION` **junto** com
`FEEDBACK` é exatamente a transição de "eco que repete" pra "espaço que
reverbera", sem trocar de módulo nem de algoritmo, só de proporção.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — `SPACE` é um efeito: sem entrada, silêncio.
  **Plugue aqui:** uma voz, um processador, ou a soma de várias vozes
  (por um `MIXER`) pra colar o patch inteiro.
- **`TIME`** (time_mod) (controle) — CV que soma ao knob `TIME` (em
  segundos). **Plugue aqui:** um LFO lento ("fita rebobinando" / chorus
  na cauda), `DRIFT.a`.
- **`FBK`** (feedback_mod) (controle) — CV que soma ao knob `FBK`.
  **Plugue aqui:** um `DECISION` (a sala "abre e fecha" por seção),
  `TRIGSEQ.any` invertido (a cauda "bombeia" com o groove).

### Saídas (ambas áudio)

- **`OUT`** — seco e molhado misturados por `MIX`. A saída normal — vai
  ao `MIXER`.
- **`WET`** — só a parte molhada (tomadas + difusão), sem o seco. Útil
  pra rotear o espaço por um caminho separado do sinal direto.

## Os controles, um a um

**TIME** (2 ms – 2 s) — o tempo base do atraso. Varrer soa como o espaço
mudando de tamanho (rápido, como fita acelerando).

**TAPS** (1–8) — quantas tomadas somam na saída molhada. Uma = eco
simples; mais = textura mais densa.

**SPREAD** (0–1) — espalha os tempos das tomadas. 0 = eco único; 1 =
padrão rítmico.

**FEEDBACK** (0–0,95) — quanto da saída volta pra entrada da linha. Mais
repetições, cauda mais longa.

**DIFFUSION** (0–1) — mistura os ecos crus com os ecos passados pela
cadeia de all-pass. Abrir `FBK` + `DIFF` juntos leva de *slap* a *hall*.

**TONE** (0–1) — o brilho da realimentação. Baixo, a cauda escurece a
cada volta (mais natural); alto, mantém o brilho.

**MOD** (0–1) — a profundidade de um LFO lento no ponto de leitura
(chorus / ensemble na cauda).

**MIX** (0–1) — seco ↔ molhado na saída `OUT`. 0 = a `OUT` é byte-a-byte
igual à entrada.

## Como cabear

**Colar o patch:**
```
MATTER  ┐
FILTER  ┼→ MIXER → SPACE (IN) → MASTER (IN)
STRING  ┘
```

**Eco → cauda num gesto:**
```
voz → SPACE (IN) → MIXER (ch1)
```
Comece com `TAPS` 1, `SPREAD` 0, `DIFF` 0, `FBK` 0,3 — um eco limpo.
Suba `DIFF` e `FBK` juntos, devagar — o eco vira sala sem nenhum salto.

## Potencializar

- **Ritmo pelas tomadas:** `TAPS` 5 e gire `SPREAD` de 0 a 1 — o eco
  único se abre num padrão.
- **Fita rebobinando:** varrer o `TIME` na mão com `FBK` alto.
- **Cauda que escurece:** `FBK` alto, `TONE` baixo — a cauda "abafa" a
  cada repetição como um ambiente real.
- **`WET` separado:** `WET → SWIRL → MIXER (ch2)` — o espaço vai por um
  caminho de modulação enquanto o seco vai direto.

## Se você conhece o Eurorack

Faz o papel de um reverb/delay multitap — a linhagem
Schroeder/Moorer/Dattorro pra reverb por comb + all-pass, e o Mutable
Rainmaker / multitap pro lado das N tomadas rítmicas. O companheiro é o
**`HALL`** (#46), uma rede de atraso realimentada (FDN) de 8 linhas —
mais denso, estéreo. E o **`MEMORY`** (#7) faz o lado granular.
