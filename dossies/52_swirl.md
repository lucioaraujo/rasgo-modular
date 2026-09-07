# Dossiê — Módulo 52: Efeitos de modulação — chorus / flanger / ensemble / phaser (`SWIRL`)

**Família:** SPACE (a gaveta de efeitos do RASGO — junto de `SPACE`/`HALL`/
`LOOPER`/`MEMORY`)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Swirl.hpp`, `tests/test_swirl.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.5` (Tier 1 — a família de MODULAÇÃO
que ainda não existe) + `§6` (BBD delay/flanger)

## Estado da implementação

O RASGO tem reverb (`SPACE`, `HALL`), delay de linha (`LOOPER`), eco
granular (`MEMORY`), fold/distorção (`SHAPE`, `WASP`) — mas **nenhum**
efeito de modulação. O `SWIRL` é a família inteira num módulo: atrasos
CURTOS modulados (chorus/flanger/ensemble) + a cascata all-pass (phaser),
com o caráter **BBD** (bucket-brigade) num knob.

- **`type`** (0–3) — **chorus** (2 vozes, atraso ~12 ms, sem
  realimentação, engorda e cintila) · **flanger** (1 atraso ~0,4–7 ms,
  realimentação ±, jato/pente varrendo) · **ensemble** (3 vozes com LFOs
  incomensuráveis — Juno/Solina, lush) · **phaser** (6 all-pass de 1ª
  ordem, sem linha de atraso — notch móvel, timbre distinto do pente).
- **`rate`** (0,02–8 Hz, + CV) — velocidade do LFO. Triangular suavizado.
- **`depth`** (0–1) — profundidade da modulação (varredura do atraso / do
  corte dos all-pass).
- **`feedback`** (−1..1) — realimentação. No flanger/phaser cria a
  ressonância (positivo = pente/notch agudo, negativo = o "inverso");
  no chorus/ensemble um fio de realimentação dá vibrato. `tanh` no laço
  → nunca estoura; perto de ±1 auto-oscila (o modo autônomo).
- **`spread`** (0–1) — largura estéreo: o LFO do canal R defasa de
  `spread·0,25` de ciclo do L; nas vozes múltiplas também espalha o
  desafino.
- **`tone`** (−1..1) — filtro de 1 polo NO molhado: `<0` passa-baixa
  (escurece — o "aveludado" do BBD), `>0` passa-alta (afina, tira o
  grave), `0` neutro.
- **`age`** (0–1, desvio Rasgo) — caráter BBD: leve companding
  (comprime antes da linha, expande depois), um fio de ruído **semeado**
  (xorshift) e uma pontinha de aliasing (S&H no atraso). `age=0` → limpo.
- **`mix`** (0–1, + CV) — seco ↔ molhado. `mix=0` → passa-direto.

`age=0`, `feedback=0`, `mix` baixo → um chorus/flanger transparente.
Determinístico exceto pelo ruído de `age` (semeado — dois renders com os
mesmos parâmetros e `age` iguais são byte-idênticos).

Buffer de ~50 ms/canal pré-alocado no `prepare()` (o phaser não usa —
só estado por estágio); `process()` não aloca.

**Testes (Debug + Release):** chorus/ensemble — a saída ganha bandas
laterais em torno da entrada (o pico de 1 kHz se alarga; energia fora do
pico sobe) e L≠R com `spread`; flanger — varredura de pente audível
(um ruído branco filtrado tem o centróide oscilando no ritmo de `rate`)
e `feedback` alto → ressonância mais aguda; phaser — notch móvel (a
resposta a um impulso tem zeros que andam), timbre ≠ do flanger no mesmo
`rate`; `mix=0` → `out == in` amostra a amostra; `feedback` perto de ±1
sem entrada + `age>0` → auto-oscila limitado (não estoura, não NaN);
`tone<0` escurece o molhado (energia acima de 4 kHz cai); determinismo
com `age` fixo; tudo finito e |out| < ~1,2.

**Pendências (candidatos):** modo *vibrato* (100% molhado, sem seco);
número de all-pass do phaser variável (4–12); *through-zero flanger*
(o atraso passa por 0 → cancelamento total); BBD com modelo de célula de
verdade (companding + clock ruído); LFO com forma selecionável
(seno/tri/rampa/random); estéreo cruzado (o molhado de L realimenta em R).

---

## 1. Problema musical e papel no fluxo

Chorus, flanger, phaser e ensemble são a metade "movimento" da caixa de
efeitos — o que dá largura, cintilância e aquele varrer de pente/notch
que nem o reverb nem o delay de linha fazem. São todos a MESMA ideia
(um atraso curto modulado, com ou sem realimentação; o phaser troca a
linha de atraso por all-pass) — por isso cabem num módulo só. `voz →
SWIRL → MASTER` engorda; `LFO → SWIRL.rate` deixa a varredura respirar
fora do compasso; `ENVELOPE → SWIRL.mix` traz o efeito só no fim da
frase.

Distinção: o `LOOPER` é delay de LINHA (ecos audíveis, hold, reverse); o
`SWIRL` é atraso SUB-perceptivo modulado (você ouve o movimento, não o
eco). O `HALL`/`SPACE` são reverb (cauda difusa). O `SHAPE`/`WASP` são
não-linearidade. Nenhum se sobrepõe.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Chorus/flanger clássico** (Roland Dimension D / CE-1, MXR) | atraso curto modulado por LFO, seco+molhado; flanger = + realimentação | teoria de domínio público (linha de atraso + LFO) |
| **BBD / bucket-brigade** (MN3007 et al.) | companding + banda limitada + clock; o "aveludado" e a degradação — o `age` aproxima sem o modelo de célula | teoria pública (o `age` é fórmula própria) |
| **Ensemble** (Juno-60, ARP/Eminent Solina) | 3 linhas com LFOs a taxas incomensuráveis → cardume de vozes | teoria pública |
| **Phaser** (Bode / EHX Small Stone / MXR Phase 90) | cascata de all-pass de 1ª ordem varridos por LFO + realimentação → notches móveis | teoria de all-pass (Zölzer, DAFX — domínio público) |
| **All-pass de 1ª ordem TPT** | `g = (tan(πfc/sr) − 1)/(tan(πfc/sr) + 1)`; `y = g·x + s`, `s = x − g·y` | Zavalishin/Simper — teoria pública, mesmo núcleo do `FILTER` |
| **`LOOPER` do RASGO** (código do autor) | leitura interpolada de linha de atraso, `age` de fita, `tanh` no laço | código do autor |
| **4ms Ensemble Oscillator / Roland Dimension** (*referência funcional*) | o conjunto de gestos "chorus lush"; nenhum circuito | ficha/demos |

**Desvio Rasgo (Atlas §49):** os quatro efeitos num módulo com `type`
contínuo-ish; `age` (degradação BBD determinística, semeada); a
auto-oscilação do flanger/phaser como modo autônomo (a partir do piso de
ruído do `age`, quando `in` está livre); `spread` derivando os LFOs por
fase, não por circuito.

## 3. Modelo — matemática, estados, extremos

Parâmetros: `type` (0–3), `rate` (0,02–8 Hz, def 0,4), `depth` (0–1,
def 0,5), `feedback` (−1..1, def 0), `spread` (0–1, def 0,5), `tone`
(−1..1, def 0), `age` (0–1, def 0,15), `mix` (0–1, def 0,4).

Entradas: `in` (Audio), `rate_mod` (Control), `mix_mod` (Control).
Saídas: `l`, `r` (Audio).

Constantes por `type` (atraso base / faixa de modulação, em ms):
```
chorus   : base 12,0   mod 4,0    vozes 2   fb·0,3
flanger  : base 1,2    mod 5,5    vozes 1   fb·0,95
ensemble : base 11,0   mod 3,0    vozes 3   fb·0,2
phaser   : (all-pass — base/mod = faixa de fc 180 Hz .. 2,2 kHz, log)
```

**LFO** por canal *c* ∈ {L,R}: `phL += rate/sr`; `phR = phL +
spread·0,25`; forma triangular `tri(ph) = 2·|2·frac(ph) − 1| − 1`,
suavizada por 1 polo (~3 ms) pra não ter canto.

**Chorus / flanger / ensemble** (linha de atraso `buf_[c]`, ~50 ms):
```
inC = in + fbState_[c]·(feedback · fbScale[type])
buf_[c][w_[c]] = companding_in(inC, age)            // BBD: compress
para cada voz v em 1..vozes:
    lfoV = tri(phL + v·0,37)                          // vozes defasadas
    d = (base + depth·mod·(0,5 + 0,5·lfoV)) · sr/1000
    d += age_wobble()                                 // S&H raro, ∝ age
    acc += readBuf(c, w_[c] − d)
wetRaw = acc / vozes
wet = companding_out(wetRaw, age)                     // BBD: expand
wet = tone1pole(wet, tone)                            // <0 LP, >0 HP
fbState_[c] = wet
w_[c] = (w_[c] + 1) mod len
```

**Phaser** (`type == 3`, sem linha de atraso):
```
inC = in + fbState_[c]·feedback·0,7
fc = 180 · (2200/180)^(0,5 + 0,5·depth·tri(phL))     // log sweep
g  = (tan(π·fc/sr) − 1) / (tan(π·fc/sr) + 1)
x = inC
para k em 1..6:  s = apS_[c][k]; y = g·x + s; apS_[c][k] = x − g·y; x = y
wet = tone1pole(x, tone)
fbState_[c] = wet
```

**Saída:** `dry = in`; `out_l = dry·(1−mix) + wet_L·mix`,
`out_r = dry·(1−mix) + wet_R·mix`. Clamp de segurança a ±4.

**`companding` (BBD, `age`):** `y = sign(x)·|x|^(1 − 0,25·age)` na
entrada, `^(1/(1 − 0,25·age))` na saída — comprime a dinâmica antes da
linha, restaura depois (o piso de ruído fica mais alto no molhado, como
num BBD real). `age_wobble()`: a cada ~512 amostras, `wob = (rnd−0,5)·
age·0,4` amostras somadas ao atraso. Ruído: `+ (rnd·2−1)·age·0,0004` no
molhado.

**Estados:** `buf_[2]` (linha), `w_[2]`, `fbState_[2]`, `phL_`, `phLpZ_`
(suavização do LFO), `apS_[2][6]` (phaser), `toneZ_[2]` (filtro de tom),
`compZ_[2]`, `wobCnt_`, `wob_`, `rng_`, `sr_`. Sem alocação em
`process()`.

**Extremos.**
- `mix=0` → `out_l = out_r = in` exato (bypass).
- `depth=0` → atraso fixo (chorus vira um coloração estática; flanger um
  comb parado).
- `feedback=±1` + `in` livre: com `age>0` o piso de ruído alimenta o laço
  → auto-oscila num tom que varre com o LFO; o `tanh` no laço segura.
  Com `age=0` e `in` livre → silêncio (nada pra realimentar).
- `rate` no talo (8 Hz) + `depth=1` → FM audível na linha de atraso
  (aceito — é vibrato extremo).
- `phaser` + `feedback` negativo → notches deslocados (timbre "oco").
- `age=1` → molhado bem mais escuro e ruidoso; a dinâmica "achata"
  (companding agressivo).
- `spread=0` → L e R idênticos (mono). `spread=1` → LFOs em quadratura.

## 4. Três modos obrigatórios

- **autônoma:** `type=flanger`, `feedback` ~0,95, `age` ~0,3, `in`
  livre → auto-oscila num tom que o LFO varre; `rate` lento = uma sirene
  que respira. Soa ao carregar (via o piso de ruído do `age`).
- **performance:** `type`/`mix`/`feedback` na mão são o gesto; `rate` e
  `depth` a textura. `mix` sobe só no fim da frase.
- **híbrida:** `LFO → rate` (varredura fora do compasso), `ENVELOPE →
  mix` (efeito no ataque/cauda), `SEQUENCE → rate_mod` (a varredura
  muda de velocidade por passo), `voz → in` + `SWIRL.l/r → MASTER`.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `rate_mod` (Control), `mix_mod` (Control).
**Saídas:** `l` (Audio), `r` (Audio).
**Parâmetros:** ver §3.
**Limites:** `out` em ~[−1,1] pra sinal são; clamp de segurança a ±4.
CPU: por amostra ~2 leituras interpoladas de linha (chorus/ens.) OU
6 all-pass (phaser) + 1 `tanh` no laço + 1 polo de tom + (com `age`)
`pow` do companding. `prepare` aloca ~2·2400 floats (~19 KB). Sem `sin`
no caminho quente (LFO é triangular). RNG só com `age>0`.

## 6. Alternativas descartadas

- **um módulo por efeito** (CHORUS, FLANGER, PHASER separados) — são a
  mesma topologia (atraso curto modulado); 3 módulos de ~10 params cada
  onde 1 de 8 resolve. O phaser é o único com núcleo diferente (all-pass)
  e mesmo assim cabe como `type`.
- **modo do `LOOPER`** — o `LOOPER` é delay de LINHA (janela de `time`,
  hold, reverse, ecos audíveis); a modulação sub-perceptiva é outro uso
  e outro range de atraso (ms, não s). Complementares.
- **modo do `SPACE`/`HALL`** — reverb é cauda difusa (FDN/multitap);
  misturar chorus lá encheria um reverb de lógica de LFO.
- **BBD com modelo de célula real** (companding + clock por estágio) —
  caro; o `age` dá o caráter que importa. Fica como evolução.
- **LFO senoidal** — o triangular suavizado custa menos (sem `sin` por
  amostra) e o canto amaciado é indistinguível na varredura lenta.
- **through-zero flanger na v1** — o atraso passando por 0 exige uma 2ª
  linha lida ao contrário; fica como pendência (o desvio, quando entrar,
  é o cancelamento TOTAL no cruzamento).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `mix=0` → `out == in` amostra a amostra; `type=chorus`,
`in` = seno 1 kHz → o espectro ganha bandas laterais (energia fora de
±20 Hz do pico sobe vs. bypass); `spread>0` → `rms(L−R) > 0`;
`type=flanger`, `in` = ruído branco → o centróide espectral (medível)
oscila no período de `rate`; `feedback` alto no flanger → o pico do
pente fica mais estreito (Q maior); `type=phaser` → a resposta a um
impulso tem zeros que se movem entre dois renders com `rate` diferente,
e o espectro médio ≠ do flanger no mesmo `rate`/`depth`; `tone=−1` →
energia acima de 4 kHz no molhado cai > 6 dB vs `tone=0`; `feedback=1` +
`age=0,3` + sem `in` → auto-oscila, `|out| < 1,2`, finito; dois renders
com `age=0,4` byte-idênticos; `age=0` → determinístico puro.

**Escuta:** o chorus engorda sem "enjoar" (LFO lento demais = estático,
rápido demais = seasick)? o flanger varrendo tem o "jato" clássico? o
phaser soa DIFERENTE do flanger (mais oco, menos metálico)? o ensemble
tem o brilho Juno? o `age` escurece de um jeito "fita" ou só abafa? a
auto-oscilação do flanger é musical ou é um apito?

## 8. Integração e painel

Classe `Swirl` (`type()` = `"SWIRL"`), 3 entradas, 2 saídas, 8
parâmetros. `panel()` próprio (12 HP): `Display` (a saída — o
osciloscópio já mostra o movimento), knobs `TYPE`/`RATE`/`DEPTH`/`FBK`
(linha 1), `TONE`/`SPRD`/`AGE` (linha 2), `MIX`; jacks `IN`/`RTM`/`MXM`
(entrada) · `L`/`R` (saída). Testado isolado (chorus/flanger/phaser/
ensemble, bypass, auto-oscilação, tom, determinismo) antes do patch.
Cadeias canônicas: `voz → SWIRL → MASTER`; `LFO → SWIRL.rate`;
`ENVELOPE → SWIRL.mix`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família SPACE — a gaveta de efeitos,
junto de `SPACE`/`HALL`/`LOOPER`/`MEMORY`) e ao `LearnCatalog.hpp`.
