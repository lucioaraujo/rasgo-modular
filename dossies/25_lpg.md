# Dossiê — Módulo 25: Low-pass gate a vactrol (`LPG`)

**Família:** TRANSFORM / UTILITY
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Lpg.hpp`, `tests/test_lpg.cpp`
**Candidato registrado:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

`FILTER` + `VCA` + `ENVELOPE` aproximam, mas o **lag de vactrol** — a
resposta assimétrica com "cauda longa" de uma fotocélula (LDR) —
é o que faz o timbre *plucky* da costa oeste: um golpe abre o filtro
**e** a amplitude juntos, e a queda tem a curva natural de um ressoador
sendo percutido.

- **entrada** `in`, gatilho `strike`, CV direta `cv`, saída `out`;
- **vactrol** — seguidor de envelope não-linear: sobe rápido (~2 ms),
  desce devagar com curva **fast-then-slow** (mais lento quanto mais
  fechado — a "cauda") — `response` (0–1) escala o tempo de queda
  (~30 ms a ~2,5 s);
- **`mode`** (0–1) contínuo: `0` = só filtro (o env controla o corte,
  sem VCA); `1` = só VCA (filtro aberto); `0,5` = **os dois** (o LPG
  clássico) — crossfade suave (`cos`/`sin`);
- **`offset`** (0–1) — abertura de repouso: `0` = fecha total quando
  ocioso; `>0` = sempre um pouco aberto (drone);
- **`resonance`** (0–1) — ressonância do filtro (2 polos);
- **`bounce`** (0–1, def 0, 2026-09-04) — o *overshoot* do vactrol: na
  borda ↑ de `strike`, uma senoide amortecida (janela ~32 ms, ~1,3
  ciclos) soma ao env → a condutância passa ACIMA do regime por um
  instante (o "boing" plucky). `bounce = 0` → idêntico ao antigo;
- desvio **`drift`** — oscilação lenta minúscula (±~3 %) no tempo do
  vactrol, xorshift semeado. `drift = 0` → determinístico.

Filtro de 2 polos inline (dois 1-polo em cascata + realimentação leve —
sem `tan`/`exp` por amostra, estável). Sem alocação em `process()`.

**Antialias (feito — 2026-09-04):** o filtro do LPG é **linear** (sem
saturação no laço), então não gera alias como `WASP`/`SHAPE` — não
precisa de oversampling. O único risco é *zipper* na modulação rápida
do corte (`strike`/`cv` bruscos); um 1-polo de ~0,5 ms suaviza o `fc`
alvo antes de derivar `gc`.

**Testes (12 funções, Debug + Release — `tests/test_lpg.cpp`):**
golpe → o env sobe rápido e cai devagar (tempo de subida ≪ tempo de
queda); `bounce` alto → o pico do env passa do regime (overshoot),
`bounce = 0` → pico exatamente em 1; `response` alto → cauda mais longa (medida do −60 dB); `mode = 0`
→ filtragem sem gating de amplitude (corta agudo, mantém corpo); `mode =
1` → gating de amplitude sem colorir (espectro ~igual, só o envelope
muda); `mode = 0,5` → os dois (agudo E amplitude caem juntos); `offset`
> 0 → deixa passar em repouso; `cv` abre direto sem `strike`; `resonance`
alta → pico no corte, ainda estável; sem `strike` e `offset = 0` →
silêncio; `drift = 0` determinístico; tudo finito ≤ ~1,05; grafo
`CLOCK → LPG.strike` · `OSC → LPG.in`.

**Pendências (candidatos):** resposta de subida também dependente de
nível; segunda cabeça (ping do próprio filtro ressonante — LPG que soa
sozinho, tipo Rings LPG).

---

## 1. Problema musical e papel no fluxo

O Rasgo tem o filtro expressivo (`FILTER`), o VCA (`VCA`) e o envelope
(`ENVELOPE`), mas montar um LPG com eles dá um resultado *quase* certo e
sem a alma: falta a curva do vactrol, que não é um AD reto — é
rápido-devagar, com a cauda se arrastando. É o som "costa oeste"
(Buchla), o *pluck* de marimba/kalimba eletrônica.

Papel: transformação percussiva, depois de uma fonte. `OSC → LPG` com
`CLOCK → strike` (uma linha de "sinos"); `NOISE → LPG` (percussão de
ruído filtrado com decaimento natural); `MATTER → LPG` (dá o
"amortecimento do golpe" que o modal não tem).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Buchla 292 / série 200 Low-Pass Gate** | um elemento (vactrol) controla filtro **e** VCA juntos; modo filtro/both/VCA | conceito (hardware) |
| **Make Noise Optomix / MMG** | crossfade contínuo entre resposta de filtro e de VCA | conceito |
| **Mannequins Three Sisters (modo LPG)** | LPG como comportamento, não módulo separado | conceito |
| **Modelo de fotocélula (LDR)** | resposta exponencial **assimétrica**: rápido pra acender, lento e não-linear pra apagar ("memória" do vactrol) | teoria pública |
| **SVF / 1-polo em cascata** | filtro de 2 polos barato e estável | teoria pública |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
g0 = strike conectado ? (strike ≥ 0.5 ? 1 : 0) : 0
target = clamp(max(g0, offset) + (cv conectado ? cv : 0), 0, 1)

# vactrol: sobe rápido, desce devagar (mais devagar quanto mais fechado)
atkCoef = 1 − exp(−1/(0.002·sr))                      (~2 ms)
relBase = 1 − exp(−1/(response²·2.5·sr + 0.03·sr))
relCoef = relBase · (0.15 + 0.85·env) · (1 + driftCur)   # cauda: freia perto de 0
coef = target > env ? atkCoef : relCoef
env += (target − env)·coef ; env = clamp(env, 0, 1)

# mode: 0 = só filtro · 1 = só VCA · 0.5 = OS DOIS totalmente ativos
fResp = 1 − max(0, mode − 0.5)·2      # 1 até mode 0.5, →0 em mode 1
aResp = min(1, mode·2)                # 0→1 até mode 0.5, fica 1
fEnv = 1 − fResp·(1 − env)            # corte (=1 quando o filtro não responde)
aEnv = 1 − aResp·(1 − env)            # ganho de VCA (=env em mode ≥ 0.5)
fc = 20 + fEnv²·12000 [Hz]

# filtro de 2 polos com realimentação leve
gc = fc/(fc + sr·0.3183)
fb = resonance·2.4·(1 − gc·0.4)
x  = in − fb·(lp1 − lp2)
lp1 += gc·(x − lp1) ; lp2 += gc·(lp1 − lp2)
out = lp2 · aEnv
```

**Estados:** `env_`, `lp1_`, `lp2_`, `driftCur_`, `driftTgt_`,
`driftCounter_`, `rng_`. Sem alocação.

**Extremos.** Sem `strike` e `offset = 0` → `target = 0` → `env → 0` →
silêncio (é gate: fechado quando ocioso). `response = 1` + golpe curto →
cauda de ~2 s (aceitável, é o limite "swell"). `mode = 1` + `resonance`
alta → o `fEnv = 1` mantém o corte no teto, então a ressonância mal
aparece (correto — no modo VCA não há filtragem). `cv` em DC = 1 → LPG
totalmente aberto (passa direto). `in` de áudio forte → o filtro + o
`aEnv ≤ 1` seguram; sem saturação extra (o `MASTER` cuida do teto).
Reset → `env_`, filtro, drift zerados, RNG re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** sem entrada de áudio, `out = 0` (é processador). Com
  `offset > 0` e `in` de um `OSC` no rack, deixa passar um drone
  filtrado — mas isso já é "híbrida". Sem áudio, nada. (Aceitável: LPG é
  o penúltimo elo, sempre tem fonte antes.)
- **Performance:** `response` ao vivo = de *pluck* seco a *swell* longo;
  `mode` varre de "wah" (filtro) a "gate" (VCA) passando pelo LPG
  clássico; `offset` abre uma "fresta" pra deixar o drone respirar.
- **Híbrida:** `CLOCK`/`SEQUENCE` no `strike` (linha percussiva);
  `ENVELOPE.env` ou `LFO` na `cv` (o LPG "respira" além do golpe);
  `NOISE → in` + `strike` = caixa/prato com decaimento natural.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `strike` (Control, trig), `cv` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:** `mode` (0–1, def 0,5), `response` (0–1, def 0,4),
`offset` (0–1, def 0), `resonance` (0–1, def 0,2), `bounce` (0–1, def 0),
`drift` (0–1, def 0).
**Limites:** saída ≤ ~1,05 (env pode passar de 1 transitoriamente com
`bounce` — teto 1,35). CPU: 1 `exp` por bloco + aritmética (+ 1 `sin` por
amostra só durante a janela de bounce). Sem alocação.

## 6. Alternativas descartadas

- **AD reto no lugar do vactrol:** é exatamente o que `ENVELOPE` já dá;
  o valor do `LPG` É a curva assimétrica com cauda.
- **`mode` como seletor de 3 posições:** o crossfade contínuo
  (`cos`/`sin`) dá os intermediários e casa com o Optomix.
- **Filtro de 4 polos:** 2 polos é o do Buchla 292 e é "suficiente" —
  o caráter vem do vactrol, não da inclinação.
- **`bounce`/overshoot no marco 1:** ficou pra 2ª camada (2026-09-04) —
  `bounce` (0–1): senoide amortecida na janela pós-golpe somada ao env.
  Modelo simples (não um ressoador físico), determinístico, `bounce = 0`
  = idêntico à curva base.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** golpe → tempo de subida do `env` ≪ tempo de queda;
`response` maior → tempo até −60 dB maior; `mode = 0` → a resposta em
frequência muda com o `env` (energia alta cai) mas o RMS de um seno
grave ~constante; `mode = 1` → o espectro fica ~igual e só o envelope de
amplitude segue o `env`; `mode = 0,5` → os dois; `offset > 0` → saída
não-nula em repouso; `cv = 1` sem `strike` → abre; `resonance` alta →
pico medido perto de `fc`, sem estourar; sem `strike`+`offset=0` →
silêncio; `drift = 0` → dois renders byte-idênticos; tudo finito.

**Escuta:** o *pluck* soa "físico" (marimba/kalimba) ou "sintético
seco"? a cauda tem aquele arrasto de vactrol ou corta abrupto? girar
`mode` de 0 a 1 dá uma passagem musical (wah → LPG → gate)? `response`
alto num acorde faz *swell* de harmônio ou só fica lento e sem graça?

## 8. Integração e painel

Classe `Lpg` (`type()` = `"LPG"`), 3 entradas, 1 saída, 5 parâmetros.
`panel()` próprio (~10 HP): knobs MODE/RESP/OFST/RESO/BNCE/DRIFT, jacks
IN/STRK/CV in, OUT, Display (o `env` do vactrol como barra + a forma de
onda). Testado isolado (curva do vactrol, modos, offset, cv, ressonância,
silêncio, determinismo) antes do patch. Cadeias canônicas: `OSC → LPG`
· `CLOCK → LPG.strike` (linha de sinos); `NOISE → LPG` + `strike`
(percussão); `MATTER → LPG` (golpe amortecido). Adicionado ao catálogo
do painel (`apps/panel/ModuleCatalog.hpp`, família TRANSFORM).
