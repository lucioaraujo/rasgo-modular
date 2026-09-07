# Dossiê — Módulo 55: Ressoador modal excitado externamente (`RESONATOR`)

**Família:** TRANSFORM (transforma um sinal que PASSA — a excitação
externa vira matéria ressoada; junto de `FILTER`/`FORMANT`/`WASP`)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Resonator.hpp`, `tests/test_resonator.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.5` (Tier 2) + Mutable Rings/Elements
(§3); Mannequins Three Sisters ★★ (`§7 #50` — "a relação entre as saídas
é o processo"); `BiomaModalResonator` (§4); 4ms SMR (§8)

## Estado da implementação

O `MATTER` é voz AUTO-CONTIDA (tem exciter interno, física de rigidez/
tensão/desgaste). O `RESONATOR` é o Rings/Elements no modo "ressoador":
um banco de modos afinados que um sinal EXTERNO bate/arqueia — você toca
com o que quiser (`DRUM`, `NOISE`, uma voz, o `AUDIO-IN`). E as três
saídas (`low`/`mid`/`high`) se CRUZAM quando você varre `tilt` — a
relação entre elas é o processo (Three Sisters).

- **`freq`** (20–5000 Hz, + CV 1 V/oct) = a fundamental do banco.
- **`structure`** (0–1) = as razões dos parciais: `0` harmônico exato
  (1, 2, 3, 4…), até `1` esticado/inarmônico (`k·√(1+B·k²)` — rigidez de
  barra/sino/corda grossa).
- **`partials`** (1–24) = quantos modos.
- **`decay`** (0–1) = o tempo de anel (Q dos ressoadores) — curto =
  pluck, longo = arco/drone.
- **`damp`** (0–1) = amortecimento dos AGUDOS no anel — os parciais altos
  decaem antes dos graves (corda de verdade: ataque brilhante, cauda
  escura).
- **`tilt`** (−1..1) = inclinação espectral das amplitudes dos parciais:
  `<0` grave forte (a saída `low` domina), `>0` agudo forte (`high`
  domina), `0` plano. Varrer `tilt` **cruza** `low`↔`high`.
- **`position`** (0–1) = onde a excitação "bate" — um pente sobre a
  entrada que zera alguns parciais (o ponto de pluck de uma corda: 0,5 =
  sem harmônicos pares).
- **`mix`** (0–1) = seco (a excitação crua) ↔ ressoado.

**Entradas:** `in` (Audio — a excitação; livre → um ruído interno de
−36 dB arqueia o banco = modo autônomo), `strike` (Control trig — um
impulso interno por borda, exciter embutido), `freq_mod` (Control).
**Saídas:** `low` (soma do 1/3 grave dos parciais), `mid`, `high`
(1/3 agudo) — cada uma com o `mix` aplicado; `out` não existe, `mid` +
`low` + `high` somados dão o banco cheio.

`decay` curto + `mix` alto → um pluck do que entra. `structure` alto →
inarmônico (sino). `strike` + `in` livre → uma voz percussiva autônoma.
Determinístico (o ruído do modo autônomo é xorshift semeado; `in`
conectado → determinístico puro). `process()` não aloca (os coeficientes
dos ≤ 24 biquads são recalculados só quando um parâmetro muda).

**Testes (Debug + Release):** `in` = impulso, `structure=0` → a saída
tem picos espectrais nos harmônicos de `freq` (`magAt(2·freq)` etc.
fortes) e decai; `decay` alto → o anel dura mais (RMS da cauda maior);
`damp` alto → o agudo decai mais rápido que o grave (medido);
`structure=1` → os picos NÃO caem em múltiplos inteiros (inarmônico);
`tilt=−1` → `rms(low) > rms(high)`; `tilt=+1` → `rms(high) > rms(low)`;
varrer `tilt` de −1 a +1 → as duas curvas se cruzam; `mix=0` → `low`
soma a fração grave do seco… na verdade `mix=0` → cada saída = a
excitação (bypass — ver §3); `position=0,5` → o 2º harmônico some;
`strike` → o banco toca sem `in`; sem `in` e sem `strike` → toca do
ruído interno, finito e limitado; determinismo; `structure` estável não
faz o banco explodir (poles < 1).

**Pendências (candidatos):** modo "corda" (waveguide de verdade, não só
modal); `damp` dependente da amplitude (o não-linear do Rings);
excitação por ONSET do próprio mix (arbhar — autoescuta); `even`/`odd`
como no `SHAPE`; um 4º conjunto de modos (membrana/placa 2D) no
`structure`; entrada de `position` por CV; `freq` polifônico (banco por
voz).

---

## 1. Problema musical e papel no fluxo

Bater numa coisa e ela cantar. O `MATTER` faz isso com um exciter
próprio; o `RESONATOR` deixa VOCÊ escolher o que bate — um `DRUM` seco
vira uma marimba, um `NOISE` vira vento numa taça, uma voz vira um coro
metálico. E as saídas `low`/`mid`/`high` não são um crossover fixo: são
zonas do banco que trocam de dominância conforme `tilt`/`structure` —
`RESONATOR.low → MIXER` e `RESONATOR.high → outro destino`, e varrer
`tilt` faz a energia migrar entre eles (o gesto Three Sisters).

Distinção: `MATTER` = voz (exciter + ressoador num objeto). `FILTER`/
`FORMANT` = filtros (bandas fixas, não afinadas a uma série harmônica).
`RESONATOR` = banco de modos AFINADOS, excitado de fora, com saídas que
se relacionam.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Rings / Elements** (Émilie Gillet) | banco de modos afinados a uma série; `structure` (harmônico→inarmônico); excitação externa; `position`; `brightness`/`damping` | MIT (só o conceito público) |
| **Mannequins Three Sisters** | a RELAÇÃO entre as saídas (LOW/CENTER/HIGH que se cruzam) É o processo | ficha/conceito |
| **`BiomaModalResonator`** (§4, código do autor) | ressonador modal — biquads em paralelo, uma série de frequências | código do autor (AGPL — só o algoritmo) |
| **Filtro ressonante de 2 polos / biquad BP** | pólo em `r·e^(jω)`, `r` do tempo de decaimento; teoria de DSP clássica | domínio público |
| **Inarmonicidade de barra/corda rígida** | `f_k = k·f0·√(1 + B·k²)` — a mesma do `ADDITIVE.stretch` | acústica, domínio público |
| **`ADDITIVE`/`FORMANT` do Rasgo** | acumulador/biquad por parcial, tilt espectral, série esticada | código do autor |

**Desvio Rasgo (Atlas §49):** as 3 saídas que se CRUZAM (`tilt` como o
gesto); o `structure` esticado como a mesma fórmula do `ADDITIVE`
(coerência interna); o modo autônomo (ruído interno arqueia o banco);
excitação sempre EXTERNA (o exciter é opcional via `strike`, não
embutido como no `MATTER`).

## 3. Modelo — matemática, estados, extremos

Parâmetros: `freq` (20–5000 Hz, def 220), `structure` (0–1, def 0),
`partials` (1–24, def 12), `decay` (0–1, def 0,5), `damp` (0–1, def 0,3),
`tilt` (−1..1, def 0), `position` (0–1, def 0,3), `mix` (0–1, def 1).

Entradas: `in` (Audio), `strike` (Control trig), `freq_mod` (Control).
Saídas: `low`, `mid`, `high` (Audio).

**Coeficientes** (recalc quando `freq`/`structure`/`partials`/`decay`/
`damp`/`tilt`/`position` mudam), para `k` ∈ 1..P:
```
B = structure² · 0,004
fk = k · f0 · √(1 + B·k²)                    # esticado (harmônico se B=0)
if fk > 0,45·sr: mode k inativo (fade)
# tempo de decaimento (s): mais longo pra grave (damp), base de decay
T_k = (0,04 + decay·decay·6,0) · (1 − damp·(1 − 1/√k))
r_k = exp(−1 / (T_k · sr))                   # raio do pólo
w_k = 2π·fk/sr
a1_k = 2·r_k·cos(w_k) ; a2_k = −r_k²
# amplitude do parcial (tilt espectral + pente de position)
amp_k = k^(−0,5 + tilt·1,2)                  # tilt<0 grave, >0 agudo
amp_k *= |sin(π·k·position)|                 # pente de pluck
b0_k = amp_k · (1 − r_k²)                    # normaliza o ganho de pico
```

Por amostra:
```
x = in conectado ? in : (rnd·2−1)·0,015      # autônomo: ruído
if strike borda ↑: x += 1,0                  # impulso do exciter interno
lo = mi = hi = 0
para k em 1..P:
    y_k = a1_k·y1_k + a2_k·y2_k + b0_k·x
    y2_k = y1_k ; y1_k = y_k
    (k ≤ P/3)          ? lo += y_k
    : (k ≤ 2P/3)       ? mi += y_k
    :                     hi += y_k
lo,mi,hi = tanh de cada (segurança) / normaliza por √(nº de modos na banda)
out_band = x·(1−mix) + band·mix              # por banda
```

**Estados:** `y1_[24]`, `y2_[24]`, `a1_[24]`, `a2_[24]`, `b0_[24]`,
`active_[24]`, `prevStrike_`, `rng_`, `sr_`, cache dos params + `dirty_`.
Sem alocação.

**Extremos.**
- `mix=0` → cada saída = a excitação (bypass; `low`=`mid`=`high`=`in`).
- `decay=1` → `T_k` até ~6 s; `r_k` perto de 1 → anel longo, mas
  `r_k < 1` sempre (não auto-oscila, não explode).
- `structure=1` → parciais bem esticados; `magAt` nos múltiplos inteiros
  fraco, picos nos `k·√(1+B·k²)`.
- `partials=1` → um ressoador só (um seno amortecido = quase um `FILTER`
  BP muito ressonante).
- `position=0` ou `1` → `sin(π·k·position)` → todos os `amp_k` ≈ 0
  (exceto arredondamento) → saída quase muda; `0,5` → só ímpares.
- `tilt=−1` → `amp_k = k^(−1,7)` → grave domina (`low` >> `high`).
- `damp=1` → o agudo decai em ~`T/√k` do grave — cauda escura.
- `freq` alto + `partials` alto → parciais acima de Nyquist são
  desativados (fade), não aliasa.
- `in` violento (clip) → `tanh` por banda segura; `r_k<1` não acumula.

## 4. Três modos obrigatórios

- **autônoma:** `in` e `strike` livres → o ruído interno de −36 dB
  arqueia o banco → um drone ressonante (uma taça, um sino soprado);
  `decay` alto = quase infinito. Soa ao carregar.
- **performance:** `freq`/`structure`/`decay` na mão dão o material;
  `tilt` migra a energia entre `low` e `high` (o gesto); `strike` é o
  dedo batendo.
- **híbrida:** `DRUM → in` (uma marimba tocada pelo bumbo);
  `TRIGSEQ.t1 → strike` (o banco toca um ritmo); `RESONATOR.low → MIXER`
  + `RESONATOR.high → SWIRL` (as duas zonas em destinos diferentes);
  `SEQUENCE → freq_mod` (uma melodia no ressoador).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `strike` (Control trig), `freq_mod`
(Control).
**Saídas:** `low`, `mid`, `high` (Audio).
**Parâmetros:** ver §3.
**Limites:** saídas em ~[−1,1] (`tanh` por banda + `r_k<1`). CPU: por
amostra `P` biquads (≤ 24 mul-add) — o custo domina; `prepare`/recálculo:
`P` `sqrt`+`exp`+`cos`+`pow`. Sem alocação. Recálculo só na mudança de
parâmetro (não por amostra).

## 6. Alternativas descartadas

- **modo do `MATTER`** — o `MATTER` tem exciter e física próprios; a
  identidade dele é a voz fechada. O `RESONATOR` é o banco EXCITADO DE
  FORA + as 3 saídas que se relacionam. Complementares.
- **modo do `FILTER`** — o `FILTER` é multimodo genérico (bandas não
  afinadas). Um banco de 24 modos afinados a uma série harmônica é outro
  objeto.
- **waveguide de corda** (delay + filtro no laço) — é o próximo passo
  (`RESONATOR+` ou um `STRING` externo-excitado); a v1 é modal (biquads),
  que dá sino/barra/placa bem e corda razoavelmente.
- **1 saída só** — perde o gesto Three Sisters (a relação entre bandas).
- **`brightness` separado de `tilt`** — `tilt` já faz o papel (amplitude
  dos parciais); um 2º knob de brilho seria redundante.
- **`position` por CV na v1** — o pente exige recálculo dos 24 `b0`; a
  CV entra como pendência (custo por amostra).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `in` = 1 impulso, `structure=0`, `freq=220`, `partials=8`
→ `magAt(440)`, `magAt(660)`, `magAt(880)` todos > 5× o piso, e a RMS da
cauda decai; `decay=0,9` vs `decay=0,1` → RMS[1..2 s] muito maior;
`damp=0,9` → `magAt(freq·6)` na cauda cai > 12 dB vs `damp=0` enquanto
`magAt(freq)` cai < 6 dB; `structure=1` → `magAt(440)` (2º harmônico
exato) fraco, pico deslocado; `tilt=−1` → `rms(low) > 3·rms(high)`;
`tilt=+1` → `rms(high) > 3·rms(low)`; `tilt` varrido → as curvas
`rms(low,t)` e `rms(high,t)` se cruzam uma vez; `mix=0` → `low == in`
amostra a amostra; `position=0,5` → `magAt(freq·2) < 0,2·magAt(freq)`;
`strike` sem `in` → RMS > 0,05 após o impulso; sem `in`/`strike` → RMS
do ruído interno > 0,01, `|out| < 1,2`, finito; dois renders com `in`
conectado byte-idênticos; `decay=1` 10 s sem NaN.

**Escuta:** um `DRUM` no `in` vira marimba/vibrafone de verdade? o
`structure` alto dá sino ou só desafina? varrer `tilt` de `low` pra
`high` soa como a energia "andando" pelo espectro? o modo autônomo tem
o "canto de taça"? `damp` dá o decaimento natural de uma corda?

## 8. Integração e painel

Classe `Resonator` (`type()` = `"RESONATOR"`), 3 entradas, 3 saídas, 8
parâmetros. `panel()` próprio (12 HP): `Display` (a saída), knobs
`FREQ`/`STRC`/`PRTS` (linha 1), `DECAY`/`DAMP`/`TILT` (linha 2),
`POS`/`MIX` (linha 3); jacks `IN`/`STRK`/`FQM` (entrada) · `LOW`/`MID`/
`HIGH` (saída). Testado isolado (harmônicos, decay, damp, inarmônico,
tilt cruzando, bypass, position, strike, autônomo, determinismo,
estabilidade) antes do patch. Cadeias canônicas: `DRUM → RESONATOR.in`;
`TRIGSEQ → RESONATOR.strike`; `RESONATOR.low`/`.high` → destinos
diferentes. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família TRANSFORM — junto de `FILTER`/
`FORMANT`/`WASP`) e ao `LearnCatalog.hpp`.
