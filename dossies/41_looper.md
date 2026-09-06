# Dossiê — Módulo 41: Delay com hold / reverse / fita (`LOOPER`)

**Família:** SPACE
**Estado:** **implementado — Onda A** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Looper.hpp`, `tests/test_looper.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda A, #41); gap do §6

## Estado da implementação

O `SPACE` é reverb (Schroeder/Dattorro multitap); o `MEMORY` é granular.
Falta o **delay de linha** com os modos que o §6 pediu — **hold**
(congela e repete infinito), **reverse** (lê pra trás) e **caráter de
fita/BBD** num knob.

- **`time`** (1 ms–2 s, + CV) = tempo de atraso, suavizado (sem zíper);
- **`feedback`** (0–1,1) = realimentação; acima de 1,0 auto-oscila
  (freio suave no laço, não estoura);
- **`hold`** (toggle, ou gate `freeze` alto) = para de escrever, o
  ponteiro de leitura cicla só a **janela de `time`** → loop infinito do
  que estava ali;
- **`reverse`** (toggle, ou gate `rev` alto) = leitura pra trás, 2 grãos
  Hann sobrepostos (sem clique no wrap);
- **`age`** (0–1) = "quão gasto": um knob que combina perda de agudo no
  laço (passa-baixa 1 polo), **wow & flutter** (modulação lenta + rápida
  do ponto de leitura), saturação suave e um fio de ruído;
- **`heads`** (1–4, +2026-09-06) = **eco de fita multi-cabeça** (Roland
  Space Echo / RE-201). 1 = eco simples. 2–4 = cabeças extras leem
  frações do `time` — `{1; 0,75; 0,5; 0,25}×` — e somam (÷ nº de
  cabeças); a realimentação regenera todas → eco denso e rítmico. `FBK`
  perto de 1 + `time` longo + várias cabeças = **Frippertronics** denso.
  Só no modo forward (não com `hold`/`reverse`);
- **`mix`** (0–1) = seco/molhado; saída extra **`wet`** (100% molhado).

`age = 0`, `feedback` moderado, sem `hold`/`reverse` → delay digital
limpo. Determinístico exceto pelo ruído de `age` (semeado — dois renders
com os mesmos parâmetros e `age` iguais são byte-idênticos).

Buffer de ~2,2 s pré-alocado no `prepare()`; `process()` não aloca.

**Testes (Debug + Release):** eco simples — um impulso reaparece depois
de `time` s e de novo `feedback×` mais fraco; `feedback` alto → cauda
longa que decai; `feedback > 1` → cresce mas fica limitado (freio);
`hold` → a saída vira um loop periódico de `time` s que não decai, e a
entrada nova é ignorada; `reverse` → um sweep ascendente sai descendente
(cruzamentos de zero espelhados) sem clique; `age` alto → o eco perde
agudo a cada volta (energia acima de 4 kHz cai) e ganha um pouco de wow
(desvio do período de eco); `mix = 0` → só o seco; `wet` = 100% molhado;
`heads = 3` → ecos extras em `time·{0,5; 0,75}` além do principal, e
`heads = 1` não tem nada antes do 1º eco; tudo finito; determinismo com
`age` fixo.

**Pendências (candidatos):** `mod` dedicado (chorus) separado do wow;
crossfade de `time` grande (hoje suaviza, um salto brusco ainda desliza);
`duck` (o eco abaixa quando entra sinal); combos de cabeça selecionáveis
(o RE-201 tem 12); `heads` também no `reverse`.

**Decisão de 2026-09-06 (`ESTUDO_audio_sampling §4`):** `TAPE` NÃO vira
módulo — seria 80% duplicata do `LOOPER`. O que faltava (eco multi-cabeça
tipo Space Echo, Frippertronics) entrou aqui como `heads` + `feedback`
longo.

---

## 1. Problema musical e papel no fluxo

Metade da música com fita/tape echo vive de três gestos: **repetir uma
frase pra sempre** (hold), **soltar ela ao contrário** (reverse) e **o
eco apodrecendo** a cada volta (age). O delay digital limpo é o caso
`age = 0`; os outros três são o que dá vida. `LOOPER` põe isso no patch —
entra áudio, sai áudio + a saída `wet`, e `time`/`feedback`/`age` são os
macros, `hold`/`reverse` os gestos.

## 2. Fontes ESTUDADAS (conceito, não código)

- **tape echo (Echoplex / RE-201 Space Echo)** — cabeça de leitura móvel
  (wow&flutter), perda de agudo e saturação na realimentação, o "som que
  degrada". Teoria pública de mecânica de fita.
- **BBD (Bucket Brigade)** — companding + banda limitada + clock ruído;
  o `age` aproxima o mesmo caráter sem o modelo de célula.
- **delay digital com HOLD** (4ms DLD, Make Noise Mimeophon) — congelar
  a janela e repetir; a base do `hold`.
- **reverse granular** — 2 grãos Hann sobrepostos lidos pra trás,
  crossfade sem clique. Teoria de granulação (Roads).
- **freio suave no laço de feedback** — `tanh` no valor realimentado pra
  auto-oscilação estável em vez de estouro (padrão do `FILTER`/`WASP`).

## 3. Modelo — por amostra

```
d = suaviza(time + time_cv)                  s → amostras, 1 polo lento
holdA = hold ∨ (freeze_in ≥ 0,5)
revA  = reverse ∨ (rev_in ≥ 0,5)

// leitura
se revA:
  gA = 0,5 − 0,5·cos(2π·revPh);  gB = 1 − gA
  wet = interp(buf, w−1−revPh·d)·gA + interp(buf, w−1−frac(revPh+0,5)·d)·gB
  revPh += 1/d;  revPh −= ⌊revPh⌋
senão:
  wow = age·(0,003·sin(φ_wow) + 0,0015·sin(φ_flut))
  wet = interp(buf, w − d·(1 + wow))

// realimentação com caráter de fita
fb = wet
fb = lp1(fb, age)                             perde agudo com age
fb = tanh(fb·(1 + age·0,6))·(1/(1+age·0,3))   satura leve
fb += ruído_semeado · age · 0,0006

// escrita
se holdA:  buf[w] inalterado; w congelado; revPh/janela ciclam em d
senão:     buf[w] = in + fb·feedback ;  w = (w+1) mod bufLen

out = in·(1−mix) + wet·mix
wet_out = wet
```

**Extremos:** `time` no mínimo (1 ms) + `feedback` alto → quase comb/
ressonância (aceito). `feedback = 1,1` → cresce até o `tanh` segurar,
loop estável. `hold` com buffer em silêncio → loop de silêncio. `time`
por CV negativo → clampado ao mínimo. `age = 1` → agudo bem abafado +
wow audível + satura — "fita velha". Buffer todo escrito e `d` >
bufLen → clampado.

## 4. Três modos obrigatórios

- **autônoma:** `time` médio, `feedback` ~0,5, `age` leve → um eco que
  respira; `DRIFT → time` deixa o andamento do eco derivar.
- **performance:** `hold` e `reverse` são gestos de mão; `feedback` e
  `age` os macros de textura.
- **híbrida:** `freeze` de uma linha do `TRIGSEQ`, `rev` de outra — os
  gestos entram no fluxo generativo; `time` de um `QUANTIZER` pra o eco
  cair em subdivisões.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `time` (Control), `freeze` (Control/gate),
`rev` (Control/gate).
**Saídas:** `out` (Audio, seco+molhado), `wet` (Audio, 100% molhado).
**Parâmetros:** `time` (0,001–2,0 s, def 0,3), `feedback` (0–1,1,
def 0,4), `age` (0–1, def 0,2), `mix` (0–1, def 0,5), `hold` (0/1),
`reverse` (0/1).
**Limites:** `out`/`wet` em ~[−1,1] (o `tanh` no laço + o buffer não
crescem sem limite). Buffer 2,2 s × sr (≈ 380 KB a 48 k). CPU: por
amostra 2–3 interpolações + 1 `tanh` + 1 polo + 2 `sin` (wow).

## 6. Alternativas descartadas

- **modo do `SPACE`** — `SPACE` é FDN/multitap (reverb); misturar hold/
  reverse lá encheria um módulo de reverb de lógica de looper.
- **modo do `MEMORY`** — `MEMORY` é granular (grão/nuvem/spray); o delay
  de linha é outro animal.
- **reverse sem crossfade** — clica a cada `time` s (bem audível); os
  2 grãos Hann custam pouco e resolvem.
- **modelo de célula BBD real** — caro e o `age` dá o caráter que
  importa; fica como evolução.

## 7. Integração e painel

12 HP, família **SPACE**. Display da saída (o osciloscópio já mostra).
Knobs `TIME`/`FBK`/`AGE` (linha 1), `MIX`/`HEADS` (linha 2) + toggles
`HOLD`/`REV`; jacks `IN`/`TIME`/`FRZ`/`REV` + `OUT`/`WET`.

Cadeias canônicas: `voz → LOOPER → MASTER` (eco); `DRIFT → LOOPER.time`
(andamento derivando); `TRIGSEQ.t1 → LOOPER.freeze` (congela no compasso);
`LOOPER.wet → SHAPE` (só o eco distorcido); `HEADS = 3` + `AGE` alto +
`FBK` ~0,8 = Space Echo.
