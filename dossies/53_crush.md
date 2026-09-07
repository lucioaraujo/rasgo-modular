# Dossiê — Módulo 53: Destruidor lo-fi / decimador (`CRUSH`)

**Família:** TRANSFORM (é aqui que mora o verbo **DAMAGE** — `RASGO_MODULAR.md §4.2`)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Crush.hpp`, `tests/test_crush.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.5` (Tier 1 — "o verbo DAMAGE que
ainda não tem casa própria") + Schlappi 100 Grit ★ (§7 #13); Atlas §16/§17

## Estado da implementação

O `wear` do RASGO está espalhado (`SAMPLER`/`TURNTABLE`/`LOOPER`) mas não
há um módulo dedicado de degradação digital — e **DAMAGE** é o único
verbo da árvore de 18 que ficou sem módulo. O `CRUSH` é o destruidor:
redução de taxa de amostragem (sem filtro anti-alias — o aliasing É o
som), redução de bits, transbordo (clip ↔ *overflow* que enrola), glitch
(dropout / amostra travada / repique) e jitter de clock.

- **`rate`** (100 Hz–24 kHz, + CV) = a taxa do sample-and-hold interno.
  Baixo = escada grossa + aliasing forte (a "voz de robô" dos 8-bit).
- **`bits`** (1–16) = quantização da amplitude a `2^bits` níveis
  (`round(x·L)/L`). `bits=1` = quase onda quadrada.
- **`drive`** (0–4×) = ganho ANTES da quantização — empurra o sinal pro
  transbordo.
- **`wrap`** (0–1) = como o transbordo se comporta: `0` clipa (limita a
  ±1), `1` **enrola** (`mod(x+1, 2) − 1` — o estouro de inteiro, dente de
  serra brutal), entre os dois um blend.
- **`glitch`** (0–1) = probabilidade por amostra-de-hold de uma falha:
  amostra TRAVADA (fica no valor anterior), DROPOUT (silêncio) ou
  REPIQUE (o valor anterior amplificado). Semeado (xorshift). Simula uma
  conexão digital ruim / CD arranhado.
- **`jitter`** (0–1) = instabilidade da taxa do S&H (o clock treme →
  instabilidade de afinação, o "wow" digital). Semeado.
- **`tone`** (−1..1) = filtro de 1 polo na saída (o filtro de
  reconstrução, ou a falta dele): `<0` passa-baixa (dócil), `>0`
  passa-alta (só o lixo agudo), `0` neutro.
- **`mix`** (0–1, + CV) = seco ↔ destruído. `mix=0` → passa-direto.

**Modo autônomo (desvio):** `in` desconectado → o S&H interno amostra o
próprio ruído branco à taxa `rate` → uma **fonte de ruído lo-fi/glitch**
(escada grossa, dropouts, jitter). Semeada.

`bits=16`, `rate` alto, `drive=1`, `glitch=0`, `jitter=0` → quase
transparente (a escada e a quantização somem no ruído de fundo).
Determinístico exceto pelo `glitch`/`jitter`/ruído autônomo (todos
semeados — reprodutíveis).

`process()` não aloca (sem buffer — só estado de S&H).

**Testes (Debug + Release):** `mix=0` → `out == in` amostra a amostra;
`rate` baixo → a saída é uma escada (nº de degraus ≈ `rate`·duração; o
espectro ganha imagens de alias acima de `rate/2`); `bits=2` → a saída
assume só ~4 níveis distintos; `wrap=1` + `drive=3` + seno → dente de
serra (muito mais harmônicos ímpares que com `wrap=0`/clip); `glitch`
alto → há trechos de silêncio E trechos travados (a derivada zera por
janelas); `jitter` alto → a altura de um seno de entrada oscila
(cruzamentos de zero variam); `tone<0` → agudo cai; sem `in` + `mix>0`
→ ruído lo-fi finito e limitado; determinismo com `glitch`/`jitter`
fixos; tudo finito e |out| < ~1,5.

**Pendências (candidatos):** *noise shaping* selecionável (realimentar o
erro de quantização — hoje é quantização crua); *sample-rate* como
divisão de um clock EXTERNO (porta `clock`); modelo de conversor
específico (µ-law, A-law, o "SP-1200"); `bits` fracionário (dither entre
dois níveis); *aliasing* pré-crush opcional (um filtro anti-alias que
some com um knob — pra ir do limpo ao sujo).

---

## 1. Problema musical e papel no fluxo

Metade da estética digital-suja vem de quatro gestos: **baixar a taxa**
(a escada + o aliasing dos primeiros samplers), **baixar os bits** (o
degrau grosso), **estourar** (o wrap de inteiro, não o clip macio) e a
**falha** (o dropout, a amostra travada — a "conexão ruim"). O `SHAPE`
distorce de forma *analógica* (fold, tanh); o `WASP` é um filtro áspero.
Nenhum faz o lixo *digital*. `CRUSH` põe isso no patch — `voz → CRUSH →
MASTER` suja; `ENVELOPE → CRUSH.bits` faz a resolução cair na cauda da
nota; `LFO → CRUSH.rate` varre a taxa (o "engoli-fita ao contrário").

Distinção: o `wear` do `SAMPLER`/`TURNTABLE` é degradação POR DISPARO
(jitter de início + hold), ligada ao gesto de tocar uma fatia. O `CRUSH`
é um efeito de INSERÇÃO contínuo no caminho do sinal.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Redução de taxa (decimação sem anti-alias)** | sample-and-hold a `fs' < fs`; as imagens de alias acima de `fs'/2` FICAM (é o som) | teoria de amostragem, domínio público |
| **Redução de bits (quantização uniforme)** | `round(x·2^(b−1))/2^(b−1)`; erro de quantização = ruído correlacionado | teoria, domínio público |
| **Overflow de inteiro (wrap)** | quando o valor passa do fundo de escala, enrola (`mod`) em vez de saturar — a "explosão" digital | aritmética de complemento de dois (teoria) |
| **Dropout / stuck sample** (conexão S/PDIF ruim, CD arranhado) | falha estocástica: silêncio, valor travado ou repique | prática (fenômeno), reescrito |
| **Jitter de clock** | a taxa do S&H treme → modulação de altura (o "wow" digital) | teoria de conversores, domínio público |
| **Schlappi 100 Grit / OTO Biscuit / µBraids** (*referência funcional*) | o conjunto de gestos "destruidor digital"; nenhum circuito/código | ficha/demos |
| **`wear` do RASGO** (código do autor) | jitter + hold + crush semeados e determinísticos | código do autor |

**Desvio Rasgo (Atlas §49):** os cinco vetores de dano num módulo; o
`wrap` contínuo entre clipar e enrolar; o `glitch`/`jitter` **semeados**
(o mesmo patch soa igual duas vezes — dano REPRODUTÍVEL, ao contrário de
um bug); o modo autônomo (S&H do próprio ruído = fonte lo-fi).

## 3. Modelo — matemática, estados, extremos

Parâmetros: `rate` (100–24000 Hz, def 6000), `bits` (1–16, def 12),
`drive` (0–4, def 1), `wrap` (0–1, def 0), `glitch` (0–1, def 0),
`jitter` (0–1, def 0), `tone` (−1..1, def 0), `mix` (0–1, def 1).

Entradas: `in` (Audio), `rate_mod` (Control), `mix_mod` (Control).
Saída: `out` (Audio).

Por amostra:
```
r = clamp(rate + rate_mod, 20, sr/2)
step = (r / sr) · (1 + jitter · (rnd01 − 0,5) · 0,6)   // clock com jitter
holdPh += step
if holdPh >= 1:
    holdPh -= floor(holdPh)
    src = (in ? in : rnd01·2−1) · drive                // autônomo: ruído
    g = rnd01
    if g < glitch:
        sub = rnd01
        held = sub < 0,4 ? held            // TRAVADA
             : sub < 0,7 ? 0,0             // DROPOUT
             : clamp(held · 1,8, −1, 1)    // REPIQUE
    else:
        held = src

// quantização
L = 2^(bits−1)                                          // bits 1..16 → 1..32768
q = round(held · L) / L

// transbordo: clip ↔ enrola
qc = clamp(q, −1, 1)
qw = q − 2·round(q · 0,5)                                // sawtooth wrap [−1,1]
o = qc + (qw − qc) · wrap

// filtro de tom (1 polo)
toneZ += (o − toneZ) · toneCoef
o = tone < 0 ? lerp(o, toneZ, −tone) : lerp(o, o − toneZ, tone)

out = in·(1−mix) + o·mix
out = clamp(out, −4, 4)
```

**Estados:** `holdPh_`, `held_`, `toneZ_`, `rng_`, `sr_`. Sem alocação.

**Extremos.**
- `mix=0` → `out = in` exato (bypass).
- `rate ≥ sr/2` + `bits=16` + `drive=1` + `glitch=jitter=0` → quase
  transparente.
- `bits=1` → `q ∈ {−1, 0, +1}` (com `drive` alto, ±1) — quase quadrada.
- `wrap=1` + `drive` alto → o sinal enrola várias vezes → dente de serra
  de frequência ∝ `drive` (harmônicos até Nyquist, aliasa — aceito).
- `glitch=1` → quase todo hold é uma falha (travada/dropout/repique) —
  a saída vira staccato/ruído; o clamp segura o repique.
- `jitter=1` → a taxa varia ±30% → um seno de entrada sai com vibrato
  largo e irregular.
- `rate` no mínimo (100 Hz) → escada MUITO grossa; um seno de 1 kHz sai
  como uma onda de ~100 Hz cheia de alias.
- sem `in`, `mix=0` → silêncio (nada pra destruir e o seco é 0).

## 4. Três modos obrigatórios

- **autônoma:** `in` livre, `mix=1` → o S&H do ruído interno a `rate` +
  `glitch`/`jitter` = uma fonte de ruído lo-fi/glitch que já toca ao
  carregar; `LFO → rate` varre o caráter.
- **performance:** `bits`/`rate`/`drive` na mão são o gesto de sujar;
  `wrap` vira o "modo explosão"; `mix` traz o dano por dose.
- **híbrida:** `ENVELOPE → bits` (a resolução cai na cauda), `LFO →
  rate` (a taxa varre), `SEQUENCE → glitch` (falha rítmica), `voz →
  CRUSH → SWIRL` (o lixo digital passa pelo chorus).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `rate_mod` (Control), `mix_mod` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:** ver §3.
**Limites:** `out` ~[−1,1] pra sinal são; clamp de segurança a ±4. CPU:
por amostra 1 `round` + 1 `round` (wrap) + 1 polo + (com glitch/jitter/
autônomo) ~2 draws de xorshift. Sem alocação, sem buffer, sem `sin`.

## 6. Alternativas descartadas

- **um módulo por vetor** (BITCRUSH, DECIMATE, GLITCH separados) — são a
  mesma família (dano digital); 3 módulos onde 1 resolve.
- **modo do `SHAPE`/`WASP`** — aqueles são distorção ANALÓGICA (fold,
  tanh, filtro). O lixo digital (quantização, aliasing, wrap de inteiro,
  dropout) é outro objeto.
- **modo do `SAMPLER`** — o `wear` do `SAMPLER` é por disparo, ligado ao
  gesto de fatia. O `CRUSH` é inserção contínua.
- **anti-alias no decimador** — mataria o som (o aliasing É o efeito).
  Fica como pendência: um filtro que SOME com um knob.
- **noise shaping por padrão** — suaviza o ruído de quantização; o
  `CRUSH` quer o cru. Fica como `mode` pendente.
- **`glitch` sem seed** — dano irreprodutível é bug, não instrumento
  (regra do `SAMPLER`/`TRIGSEQ`/`DRUM`).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `mix=0` → `out == in`; `rate=1000`, `in` = seno 3 kHz →
a saída tem energia forte em `|3000 − k·1000|` (imagens de alias) que o
seco não tem; `bits=2` → o histograma da saída tem ≤ ~5 valores
distintos; `wrap=1`, `drive=3`, `in` = seno 200 Hz → `magAt(600)` +
`magAt(1000)` (ímpares) sobem > 3× vs `wrap=0`; `glitch=0,8` → a fração
de amostras com `|out| < 1e-4` (dropout) é > 10% E há platôs (derivada
zero) longos; `jitter=0,8`, `in` = seno 440 Hz → o nº de cruzamentos de
zero numa janela varia > 15% entre janelas; `tone=−1` → energia acima
de 5 kHz cai > 8 dB; sem `in`, `mix=1`, `rate=2000` → RMS > 0,05, finito;
`glitch`/`jitter` fixos → dois renders byte-idênticos.

**Escuta:** o 8-bit soa "console dos anos 90" ou só abafado? o `wrap`
explode de um jeito musical (dá pra tocar) ou é só ruído? o `glitch`
soa como uma conexão ruim ou como um erro de código? `jitter` dá o "wow"
de fita digital? o modo autônomo (ruído lo-fi) tem caráter próprio?

## 8. Integração e painel

Classe `Crush` (`type()` = `"CRUSH"`), 3 entradas, 1 saída, 8
parâmetros. `panel()` próprio (10 HP): `Display` (a saída — a escada
aparece no osciloscópio), knobs `RATE`/`BITS`/`DRIVE` (linha 1),
`WRAP`/`GLTCH`/`JITR` (linha 2), `TONE`/`MIX` (linha 3); jacks
`IN`/`RTM`/`MXM` (entrada) · `OUT`. Testado isolado (bypass, escada,
níveis, wrap, glitch, jitter, tom, autônomo, determinismo) antes do
patch. Cadeias canônicas: `voz → CRUSH → MASTER`; `ENVELOPE → CRUSH.bits`;
`LFO → CRUSH.rate`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família TRANSFORM — junto de `SHAPE`/
`WASP`) e ao `LearnCatalog.hpp`.
