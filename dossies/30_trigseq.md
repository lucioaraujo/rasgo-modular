# Dossiê — Módulo 30: Grade de trigs / sequenciador de percussão (`TRIGSEQ`)

**Família:** SEQUENCE / TIME
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/TrigSeq.hpp`, `tests/test_trigseq.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `CLOCK` faz euclidiano de UMA linha; o `SEQUENCE` faz altura+gate de
UMA voz. Faltava a **grade multipista** — 4 linhas de gate on/off tocando
juntas, a base rítmica de percussão. Seguindo a identidade RASGO ("soa
sozinho ao carregar", *alignment not presets*), o `TRIGSEQ` **não é um
editor de passos**: é um **gerador rítmico** — uma matriz de padrões
arquetípicos que o `map` percorre, com densidade por linha, e o acaso
(`chaos`) só empurra probabilidades. Modelo Grids, tabelas próprias.

- **4 linhas** `t1`/`t2`/`t3`/`t4` (bumbo · caixa · chimbal · perc) →
  4 saídas de gate; + `accent` (passos onde ≥2 linhas batem) + `any`
  (OR de todas — pulso mestre patchável);
- **`map`** (0–1) morfa entre **4 caracteres** — *straight* (rock/house)
  · *broken* (breakbeat) · *shuffle* (hip-hop/suingado) · *sparse*
  (minimal/dub) — interpolando os pesos de cada passo;
- **`density1..4`** (0–1 por linha) = quanto do padrão passa: 0 só o
  passo mais forte (peso 255), 1 qualquer passo com peso > 0 (limiar
  `(1−density)·255`, à la Grids);
- **`swing`** (0–1) atrasa os passos ímpares (a "e") até ~2/3 do passo;
- **`chaos`** (0–1) = notas-fantasma nos quase-acertos + quedas
  ocasionais (nunca no downbeat forte); xorshift **semeado**;
- **`ratchet`** (0–1) = probabilidade de um acerto virar rajada de 3
  sub-trigs no passo;
- **`fill`** (entrada, gate) + **`fill_amt`** (0–1) = enquanto alto,
  aproxima cada `density` de 1 (viradas);
- **`drift`** (0–1) = passeio lento e limitado do `map` (escala de
  minutos — assinatura RASGO);
- **`length`** (2–16) recorta o padrão; **`rate`** = relógio interno se
  `clock` livre.

Desvio Rasgo: as medições ficam pra depois; aqui o desvio é a grade ser
**generativa e contínua** (não 64 toggles) + `drift` + `chaos` como
probabilidade, não como random cru.

Sem alocação / lock / IO em `process()`. Determinístico (xorshift
semeado em `prepare()`).

**Testes (13 funções, Debug + Release):** `density=0` numa linha →
saída só nos passos de peso máximo; `density=1` → todos os passos com
peso; `density` monotônico (mais denso = mais trigs); `map` varrendo
0→1 muda o padrão (contagem de trigs por janela difere entre caracteres);
`swing` alto → os trigs dos passos ímpares chegam mais tarde (Δt > 0
medido); `chaos=0` → dois renders idênticos e nenhuma nota fora da
tabela; `chaos` alto → mais trigs que o padrão seco; `fill` alto →
densidade sobe em todas as linhas; `ratchet` alto → aparecem rajadas
(intervalos curtos entre trigs da mesma linha); `accent` só quando ≥2
linhas coincidem; `any` = OR; relógio interno gera passos sem `clock`;
`reset` volta ao passo 0; tudo finito; grafo `CLOCK → TRIGSEQ.clock` ·
`t1 → LPG.strike` · `accent → VCA.cv`.

**Pendências (candidatos):** padrão editável (overlay de toggles no
painel, por cima do gerador); 6–8 linhas; `prob` por passo explícito
(hoje é `density` por linha + `chaos` global); saída de **velocity/CV
de acento** (não só gate); euclidiano por linha como 5º caractere;
`ratchet` com nº de sub-hits variável e por-passo.

---

## 1. Problema musical e papel no fluxo

Percussão precisa de várias linhas em relação rítmica — bumbo firme,
caixa no contratempo, chimbal correndo por baixo. O rack fazia isso só
com um `CLOCK` euclidiano por voz (sem relação entre elas) ou um
`TURING` (padrão que emerge, difícil de fixar num groove). O `TRIGSEQ`
dá o **groove como campo**: um ponto no `map` é um estilo, `density`
esculpe, `chaos` humaniza, `drift` faz evoluir. É a peça que transforma
um patch de seed num *beat*, não numa textura.

Papel: fonte de tempo estruturado. `CLOCK → clock` (sincroniza);
`t1..t4` → `LPG.strike` / `MATTER.strike` / `ENVELOPE.gate` de vozes
percussivas; `accent` → `VCA.cv` ou `*_mod` (dá peso a certos passos);
`any` como relógio de outra coisa. No `seedPatch` v2: destino de `gate`
(clock/fill) e fonte de `gate` (as 6 saídas) — ritmo emergente.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Instruments Grids** (AVR, GPL-3.0 — só o *conceito* de "mapa rítmico + limiar de densidade", tabelas reescritas) | ponto num mapa 2D interpola padrões-nó; knob de *fill* é um limiar sobre o peso do passo | GPL-3.0 — conceito público, tabelas próprias |
| **Roland TR-808/909** (teoria pública) | grade de 16 passos × instrumentos; acento como linha derivada | prática de domínio público |
| **ALM Pamela's PRO Workout / Vermona randomRHYTHM** | probabilidade por passo; euclidiano por linha; *fill* momentâneo | ficha/conceito |
| **`mode` browniano do `SEQUENCE` / `spread` do `DECISION`** (código do autor) | acaso estruturado (probabilidade nudge, não flip cru) | código do autor |

## 3. Modelo — matemática, estados, extremos

Tabelas `kNode[4 caracteres][4 linhas][16 passos]` de `uint8` (0 · 64
fantasma · 160 normal · 255 downbeat).

Por tique de clock (avança pra `step`; `s = step % length`):
```
pos = clamp(map + mapDrift, 0, 1) · 3
c0 = floor(pos) ; c1 = min(3, c0+1) ; g = pos − c0
para cada linha l:
  w  = lerp(kNode[c0][l][s], kNode[c1][l][s], g)           # 0..255
  d  = clamp(density[l] + fillBoost[l] + densDrift[l], 0, 1)
  thr = (1 − d) · 255
  fire = (w > 0) && (w ≥ thr)
  if chaos > 0:
     u = rng01()
     if !fire && w>0 && u < chaos·0.35·(w/255):  fire = true       # fantasma
     elif fire && w < 250 && u < chaos·0.12:      fire = false      # queda
  laneFire[l] = fire
nHits = Σ laneFire
accent = nHits ≥ 2 ; any = nHits ≥ 1

# swing: passo ímpar espera; par dispara já
delay = (s ímpar) ? swing·0.66·period : 0
agenda os gates de laneFire/accent/any depois de `delay` amostras

# ratchet: por linha que disparou
if laneFire[l] && rng01() < ratchet·0.5:
   ratchetLeft[l] = 2 ; ratchetIvl[l] = period/3
```
Comprimento do gate: `clamp(period·0.5, 3 ms, 60 ms)`.
`fillBoost[l] = (fill ≥ 0.5) ? fill_amt·(1 − density[l]) : 0`.
`mapDrift`/`densDrift` = passeio aleatório limitado por passo, escala
`drift` (±0.3 no map, ±0.2 nas densidades), semeado.
`period`: medido entre tiques externos, ou `sr/rate` no relógio interno.

**Estados:** `step`, `phase`, `period`, `periodCount`, `prevClock`,
`prevReset`, `prevFill`, `gateCd[4]`, `accentCd`, `anyCd`,
`pendingMask` + `pendingCd` (swing), `ratchetLeft[4]` + `ratchetCd[4]` +
`ratchetIvl[4]`, `rng`, `mapDrift`, `densDrift[4]`. Sem alocação.

**Extremos.** Todas as `density`=0 → só os downbeats (peso 255) tocam;
se um caractere não tem 255 numa linha, ela fica muda (correto — é o
padrão "sem nada"). `density`=1 + `chaos`=1 → quase todo passo dispara
em todas as linhas (parede de trig — uso não-idiomático, honesto).
`swing`=1 + `rate` alto → o atraso pode passar do passo seguinte; o
agendamento sobrescreve (último tique manda) — não trava. `length`=2 →
groove de 2 passos. `clock` e relógio interno: `clock` conectado manda.
`ratchet`=1 → metade dos acertos vira rajada; com `period` pequeno as
sub-hits colam (aceitável). `fill` sem `fill_amt` → sem efeito.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → relógio interno em `rate`, `map`/
  `density` default tocam um groove *straight* meio cheio nas 4 saídas.
  Soa ao carregar.
- **Performance:** `map` ao vivo = trocar de estilo sem parar; `density`
  por linha = tirar/pôr instrumento; `swing` = feel; `fill` (botão) =
  virada; `chaos` = humanizar.
- **Híbrida:** `CLOCK → clock` (sincronia); `DRIFT.a → map_cv` (o
  groove deriva devagar); `LOGIC.and → fill` (virada a cada N compassos);
  `accent → VCA.cv` de uma voz; `any → CLOCK.ext_clock` de outra parte.

## 5. Portas, parâmetros, limites

**Entradas:** `clock` (Control trig), `reset` (Control trig), `fill`
(Control gate), `map_cv` (Control).
**Saídas:** `t1`, `t2`, `t3`, `t4` (Control gate), `accent` (Control
gate), `any` (Control gate).
**Parâmetros:** `length` (2–16, def 16), `rate` (0.1–20 Hz, def 2),
`map` (0–1, def 0,3), `density1` (0–1, def 0,7), `density2` (def 0,5),
`density3` (def 0,6), `density4` (def 0,25), `swing` (0–1, def 0),
`chaos` (0–1, def 0,1), `ratchet` (0–1, def 0), `fill_amt` (0–1, def
0,5), `drift` (0–1, def 0).
**Limites:** saídas 0/1. CPU: 4 lerps + 4 `rng` por tique, contadores
por amostra. Sem alocação. 1 `sin` nenhum (drift é walk, não LFO).

## 6. Alternativas descartadas

- **64 toggles (grade editável) como parâmetros:** não cabe no modelo
  de parâmetro `{id,min,max,default}` e contradiz "soa ao carregar" /
  *alignment not presets*. O gerador vem primeiro; o overlay editável
  fica candidato (painel).
- **`prob` por passo explícito:** 16 params a mais por linha. `density`
  por linha + `chaos` global cobre 90 % do uso; `prob` por passo entra
  quando houver o overlay editável.
- **Acento como linha própria na tabela:** derivar de coincidência (≥2
  linhas) dá um acento que *segue* o groove de graça; uma 5ª linha seria
  mais um padrão pra manter alinhado.
- **`ratchet` por-passo / nº variável:** rajada fixa de 3 é o suficiente
  pra marcar; variação fica candidata.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `density1=0` → `t1` só nos passos de peso 255 do caractere
atual; `density1=1` → `t1` em todo passo com peso > 0; varrer `density1`
0→1 → nº de trigs de `t1` não-decrescente; `map` 0 vs 1 → padrões
mensuravelmente diferentes (contagem/posição dos trigs); `swing=0.6` →
o 1º trig de um passo ímpar chega ~0.4·período depois vs `swing=0`;
`chaos=0` → dois renders byte-idênticos; `chaos=0.8` → nº de trigs >
padrão seco; `fill` alto por 1 compasso → densidade sobe e volta;
`ratchet=1` → há pares de trigs da mesma linha separados por ~período/3;
`accent` ⊆ (passos com ≥2 linhas); `any` = OR exato; sem `clock` →
trigs no ritmo de `rate`; `reset` → volta ao passo 0.

**Escuta:** um ponto do `map` soa como um *estilo* coerente (não como
padrão aleatório)? `density` esculpe o groove de forma musical (tira o
chimbal, engrossa o bumbo)? `swing` dá *feel* ou só atrasa? `chaos`
baixo humaniza sem descaracterizar? `fill` no botão faz uma virada que
resolve no "1"? `drift` faz o beat "respirar" ao longo de minutos?

## 8. Integração e painel

Classe `TrigSeq` (`type()` = `"TRIGSEQ"`), 4 entradas, 6 saídas, 12
parâmetros. `panel()` próprio (~16 HP): `Display` grande (a grade
atual), 3 fileiras de knobs (LEN/RATE/MAP/SWING · DENS1–4 ·
CHAOS/RATCH/FILL/DRIFT), jacks CLK/RST/FILL/MAP in, T1–T4/ACC/ANY out.
Testado isolado (densidade, map, swing, chaos, ratchet, fill, accent,
determinismo, relógio interno, reset) antes do patch. Cadeias canônicas:
`CLOCK → TRIGSEQ.clock` · `t1 → LPG.strike` · `t2 → MATTER.strike` ·
`accent → VCA.cv`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família SEQUENCE, junto de
`TURING`/`SEQUENCE`/`SWITCH`).
