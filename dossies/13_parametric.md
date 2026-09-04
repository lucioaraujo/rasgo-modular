# Dossiê — Módulo 13: Equalizador paramétrico (`PARAMETRIC`)

**Família:** TRANSFORM / UTILITY
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Parametric.hpp`, `tests/test_parametric.cpp`

## Estado da implementação (marco 2)

Feito: a ferramenta de precisão do espectro, irmã do `FILTER` (que é as
"três irmãs" expressivas). **4 estágios de biquad em SÉRIE**, cada um com
tipo, frequência, ganho (dB) e Q próprios.

- **6 tipos por estágio:** 0 Off · 1 LowCut (HPF 12 dB) · 2 LowShelf ·
  3 Peak · 4 HighShelf · 5 HighCut (LPF 12 dB). Coeficientes pelas
  **fórmulas RBJ** (Audio EQ Cookbook), normalizados por `a0`;
- alpha "Q" para Peak/cortes; alpha "slope S" para as shelves
  (`q` reinterpretado como inclinação, forma correta do Cookbook — sem
  o "dip" que a alpha de peak causaria numa shelf);
- **Direct Form II transposta** por estágio (2 estados);
- saída: `output` (dB), `drive` (0–1, `tanh` normalizado), **soft-clip
  de segurança transparente até ±1** (não colore o EQ; satura suave
  acima — o "±5V" do Parametra em escala normalizada), `mix` seco/molhado;
- **desvios Rasgo:** `sweep` (1 V/oct) desloca **todos** os estágios como
  um grupo (a relação entre as bandas como processo, Warps/Atlas §39);
  `amount` escala **todos** os ganhos.

Coeficientes recalculados **por bloco** (4 estágios, ~4 `cos`/`sin`/
`sqrt` cada — barato). Determinístico (sem RNG).

**Modelado a partir da ficha pública de recursos do VCV Parametra** (8
filtros CV, 17 tipos, VCA de saída, soft-clip). O **código do Parametra
é fechado ($30, sem fonte) e NÃO foi consultado** — só a lista de
recursos, reimplementada da teoria pública (RBJ). Governança: Parametra =
VERMELHO (referência apenas); as fórmulas RBJ são domínio público.

**Testes (10/10 alvos, Debug + Release):** todos os estágios Off → saída
**byte-a-byte igual à entrada**; Peak +12 dB Q3 @ 1 kHz → **+12,0 dB** na
frequência (±1,5) e ~0 dB uma década abaixo; Peak −18 dB → corta
> 10 dB; LowCut @ 500 Hz → −12 dB @ 80 Hz, ~0 dB @ 4 kHz; LowShelf +6 dB
@ 200 Hz → **+5,8 dB @ 50 Hz**, ~0 dB @ 6 kHz; `sweep` +1 oitava move um
peak de 500 para ~1000 Hz (medido); 5000 blocos com 4 estágios extremos
(Q10, ±24 dB, `drive` 0,8) + `sweep` senoidal ±2,5 oit sem NaN nem
`|y| > 1,05`; dois renders byte-idênticos; integração no grafo
(`FUNCTION → PARAMETRIC`); painel fecha (22 HP).

**Atualização 2026-09-02:** inclinações de **24/48 dB/oct** feitas —
parâmetro `slope<s>` (1/2/3 → 12/24/48 dB/oct) nos estágios de corte
(LowCut/HighCut), implementado como **cascata de 1/2/4 biquads idênticos**
por estágio (`Stage::z1[4]`/`z2[4]`). Teste `testCutSlopes`: slope 48
atenua > 20 dB a mais que slope 12 uma oitava abaixo do corte, e ainda
passa transparente acima. 23 parâmetros agora.

**Pendências (candidatos, não controles fictícios):** mais estágios
(`bands` configurável 1–8); suavização dos
coeficientes entre blocos (varrer `freq` rápido "escadinha" ao vível de
bloco); BP e notch explícitos (hoje um Peak com Q alto ou −∞ cobre); Q
por-estágio como CV; oversampling no `drive` (alia em nível alto);
espectro ao vivo no painel (PRE/POST); modo "match" (aprender a curva de
uma referência).

---

## 1. Problema musical e papel no fluxo

O `FILTER` (Módulo 2) é expressivo — auto-oscila, tem `spread`, é para
*tocar*. Mas às vezes um patch só precisa de **precisão**: tirar 3 dB
numa ressonância chata, dar um shelf de brilho, cortar o sub. Isso é
outro instrumento — o EQ paramétrico, a ferramenta de engenharia. Ter os
dois (`FILTER` gestual + `PARAMETRIC` cirúrgico) cobre o espectro do
"esculpir frequência" no Rasgo.

Papel: no fim de uma cadeia de voz (`MATTER`/`STRING`/`FUNCTION`) antes
do `SPACE`, ou na soma final; `sweep` de um LFO lento = um "movimento"
espectral de seção; `amount` de um envelope = o EQ "abre" na nota.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Robert Bristow-Johnson**, "Cookbook formulae for audio EQ biquad filter coefficients" | a referência canônica de todo EQ digital: LP/HP/lowshelf/highshelf/peak/notch a partir de `w0`, `Q`/`S` e ganho; normalização por `a0` | domínio público (é uma "receita", não código) |
| **VCV Parametra** (ficha pública de recursos, `vcvrack.com/Parametra`) | N biquads em série CV-controláveis; toggle de estado por banda; VCA de saída; soft-clip de segurança; visor PRE/POST | módulo **fechado**, VERMELHO — só a lista de recursos, sem código |
| **Cascata de biquads** (qualquer texto de DSP) | inclinações de 24/48 dB/oct = 2/4 seções de 12 dB em série | conhecido |
| **`FILTER` do Rasgo** (Módulo 2) | `spread`/relação entre bandas como processo; `drive` + soft-limit | conceito próprio |

## 3. Modelo — matemática, estados, extremos

**Por estágio** (coeficientes, uma vez por bloco):
```
w  = 2π·f0·freqScale / sr        ; freqScale = 2^clamp(sweep, ±4)
cw = cos w ;  sw = sin w
A  = 10^(gainDb·amount / 40)
peak/cortes:  alpha = sw / (2·Q)
shelves:      S = clamp(Q, 0.05, 1) ;  alpha = ½·sw·√((A + 1/A)(1/S − 1) + 2)
```
Depois as fórmulas RBJ para cada `type` (LowCut/HighCut = HPF/LPF 12 dB;
LowShelf/HighShelf; Peak), tudo dividido por `a0`.

**Por amostra** (Direct Form II transposta, por estágio ativo):
```
y  = b0·x + z1
z1 = b1·x − a1·y + z2
z2 = b2·x − a2·y
x  = y
```
Depois: `x ·= 10^(output/20)`; se `drive>0`, `x = tanh(x·g)/tanh(g)`;
`x = softClip(x)` (transparente até ±1); `out = mix·x + (1−mix)·dry`.

**Estados:** 2 floats por estágio (`z1`,`z2`) × 4. Sem alocação, sem RNG.

**Extremos.** `f0` travado em [15, 0,45·sr] → biquads sempre estáveis
(RBJ é estável para `Q>0`, `f0<Nyquist`). `Q` travado em [0,05, 20].
`gain` ±24 dB × `amount` — se `amount` grande, o ganho cresce, mas o
`softClip` e o `output` seguram. Todos Off → o laço de amostra não toca
o sinal → `out == dry` exato. `sweep` ±4 oit → `freqScale` de 1/16 a 16,
`f0` re-travado. Reset → `z1=z2=0` em todos.

## 4. Três modos obrigatórios

- **Autônoma:** com os 4 estágios configurados, é um EQ de "canal" fixo
  — corrige/molda um som sem nenhuma entrada de controle.
- **Performance:** `sweep` e `amount` são os dois macros — varrer
  `sweep` desliza a curva inteira (um "wah" de precisão ou um filtro
  varrendo); `amount` de 0 a >1 traz a EQ de neutra a exagerada.
- **Híbrida:** `sweep` de um LFO/`DECISION` = movimento espectral por
  seção; `amount` de um `ENVELOPE` = a cor da EQ segue a nota; a entrada
  vindo da soma de várias vozes = EQ de barramento.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `sweep` (Control, 1 V/oct sobre todas as
frequências), `amount` (Control, escala todos os ganhos).
**Saídas:** `out` (Audio).
**Parâmetros:** por estágio `s∈{1..4}`: `type<s>` (0–5), `freq<s>`
(20–20000 Hz log), `gain<s>` (−24..+24 dB), `q<s>` (0,1–10); globais
`output` (−24..+24 dB), `drive` (0–1), `mix` (0–1). **19 no total** — um
EQ paramétrico tem muitos controles por natureza; todos são reais.
**Limites:** saída em ~[−1,1] (soft-clip). CPU: 4 biquads (~5 mult/add)
por amostra + o recálculo por bloco. Sem alocação.

## 6. Alternativas descartadas

- **Copiar/portar o Parametra:** fechado (VERMELHO) e desnecessário — as
  fórmulas RBJ são públicas e é o que qualquer EQ implementa.
- **17 tipos + Q em degraus (1/2/4/8/16) como o Parametra:** 6 tipos com
  Q contínuo cobrem o mesmo espaço musical com menos enum; slopes de
  24/48 dB são 2ª camada (cascata).
- **SVF TPT (como o `FILTER`) em vez de RBJ:** o SVF é ótimo para
  modulação a taxa de áudio sem `cos`/`sin` por amostra, mas para shelf
  e peak com ganho preciso em dB o RBJ é o padrão e a modulação
  control-rate (por bloco) basta pra um EQ.
- **8 estágios já no marco 2:** 4 cobrem quase todo uso (corte grave +
  2 sinos + shelf agudo); `bands` configurável é candidato.
- **Soft-clip que colore (satura em ±0,8):** engolia o ganho do EQ nos
  testes; o soft-clip tem que ser transparente até ±1 (é segurança, não
  timbre) — o `drive` é o controle de saturação intencional.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** todos Off → `out` byte-a-byte igual a `in`; Peak `g` dB @
`f0` → ganho medido = `g` (±1,5 dB) e ~0 dB duas oitavas fora; LowCut →
inclinação de ~12 dB/oct abaixo do corte, ~0 dB acima; shelf → `g`/2 dB
na frequência de canto, `g` dB na assíntota; `sweep` desloca a curva por
2^(V/oct); estabilidade com parâmetros extremos + `sweep` (sem NaN, sem
estouro); dois renders byte-idênticos; sem alocação.

**Escuta:** um corte de −3 dB numa ressonância soa "transparente"
(cirúrgico) e não "filtrado"? varrer `sweep` soa musical ou robótico? o
`drive` engrossa ou só distorce? `PARAMETRIC` depois de `MATTER`/`STRING`
melhora o timbre da voz sem tirar o caráter? lado a lado com o `FILTER`,
os dois sentem-se instrumentos diferentes (um gesto, um cirúrgico)?

## 8. Integração e painel

Classe `Parametric` (`type()` = `"PARAMETRIC"`), 3 entradas, 1 saída, 19
parâmetros. `panel()` próprio (22 HP: espectro + uma fileira por estágio
`TYPE/FREQ/GAIN/Q` + `OUT/DRIVE/MIX` + jacks). Testado isolado (flat,
peak, cortes, shelf, sweep, estabilidade) antes do patch. Numa peça:
soma das vozes → `PARAMETRIC` (com `sweep` de um LFO lento) → `SPACE` →
saída.
