# Dossiê — Módulo 42: Oscilador aditivo / espectral (`ADDITIVE`)

**Família:** SOURCE
**Estado:** **implementado — Onda B** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Additive.hpp`, `tests/test_additive.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda B, #42)

## Estado da implementação

O `OSC` é subtrativo (tira harmônicos de uma forma rica). O `WAVETABLE`
varre uma **forma**. `ADDITIVE` é o terceiro eixo: **construir o timbre
somando parciais**, cada um com frequência e amplitude sob controle
direto. O oposto do subtrativo.

**64 parciais** somados por acumuladores de fase + LUT de seno (sem
`std::sin` no laço). As amplitudes e razões dos parciais são recalculadas
uma vez por bloco (mudam devagar); o laço por amostra só acumula. Corte
de Nyquist por parcial: o `k`-ésimo entra só enquanto `f0·razão_k <
0,98·Nyquist`, com um *fade* nos últimos ~15 % pra o parcial não sumir
com clique quando cruza o teto (afinação varrida).

Envelope espectral por quatro knobs:

- **`tilt`** (0–1) = brilho. Inclinação do espectro: `a_k = k^(−e)` com
  `e` de 2,6 (escuro, `tilt=0`) a 0,15 (quase plano, `tilt=1`); `≈1/k`
  no meio.
- **`odd`** (−1..1) = balanço ímpar/par. `+1` = só ímpares (quadrada,
  clarinete); `−1` = só pares (oitava acima, "oco"); `0` = série cheia.
- **`stretch`** (−1..1) = inarmonicidade. `razão_k = k + stretch·c·k·
  (k−1)` (`c ≈ 0,004`): `>0` estica (sino, metal), `<0` comprime. Razão
  sempre crescente em `k` (o corte de Nyquist por `break` continua
  válido).
- **`comb`** (0–1) = pente espectral. `g_k = (1−comb) + comb·(0,5 +
  0,5·cos(2π·dentes·k/64))`, `dentes = 1 + comb·11`. `comb=0` → plano;
  `comb=1` → 12 dentes, vales fundos. Formante grosseiro / efeito de
  filtro em pente sem filtrar nada.

**`drift`** (0–1, desvio Rasgo) = cintilância. Cada parcial ganha um
micro-desafino (`±drift·0,6 %`) e uma respiração de amplitude
(`±drift·12 %`) de duas senóides lentas (~0,05 e ~0,035 Hz) defasadas por
`k` no ângulo áureo. **Determinístico** — sem RNG; é a soma de senóides
incomensuráveis, quase-período longuíssimo (gate 5: percurso coerente sem
repetição curta; gate 7: render reproduzível). `drift=0` remove o termo.

**Afinação** padrão do `OSC`/`WAVETABLE`: `freq`/`fine`/`pitch` (1 V/oct)
/`fm` (linear, `fm_amount`).

**Segurança de saída** (gate 4): a soma aditiva pode ser pontuda. Um
seguidor de ganho de 1 polo (rápido pra baixo, lento pra cima) mira
`0,9/pico_do_bloco`; `tanh` suave no fim. Saída limitada em ~[−0,95;
0,95] em todo o alcance dos knobs.

Sem alocação/lock/IO em `process()` (só `prepare()` aloca a LUT de 2048 +
os vetores de 64). Determinístico sempre.

**Testes (Debug + Release):** `odd=0`, `tilt` médio, sem `stretch`/`comb`
→ espectro com harmônicas 1,2,3,… decrescentes (razão 2/1 < 1); `tilt=1`
achata (harmônica 8 mais forte, relativa, que com `tilt=0`); `odd=1` →
harmônicas pares somem (mag par < 5 % da ímpar vizinha); `odd=-1` → o
inverso; `stretch>0` → a 2ª parcial fica **acima** de 2·f0 (mede o pico);
`comb=1` → vale profundo num índice previsto pela contagem de dentes;
afinação segue 1 V/oct (uma oitava = ×2 nos cruzamentos de zero); nenhum
componente acima de Nyquist audível em `freq` alto; `fm` linear desvia a
taxa; dois renders byte-idênticos com os mesmos parâmetros (inclui
`drift>0`); saída sempre finita e < 0,98.

**Pendências (candidatos):** parciais por CV (contagem viva); modo
**pulsar** (`BiomaPulsar` §4 — trem de grãos formântico, pode ser modo
daqui); *unison*/detune de banco; envelope espectral desenhável no
painel; razões não-inteiras a partir de uma escala (microtonal);
ressíntese de um espectro capturado do `AUDIO-IN` (FFT — grande, fica
pra depois).

---

## 1. Problema musical e papel no fluxo

Toda a paleta que vive de **espectro construído, não filtrado** — órgão,
sino, vibrafone, pad aditivo tipo Kawai K5 / synclavier, drone com
inarmonicidade controlada — não sai de `OSC → FILTER`. `ADDITIVE` põe
esse eixo no patch: entra 1 V/oct, sai áudio, e `tilt`/`odd`/`stretch`/
`comb` moldam o espectro direto — cada um deles aceita CV, então um
`ENVELOPE` no `tilt` abre o brilho com a nota, um `LFO` no `stretch`
respira a inarmonicidade, etc.

Distinção dos vizinhos: o `OSC` é subtrativo; o `WAVETABLE` é forma
varrida (o espectro vem da tabela, não é endereçável parcial a parcial);
o `CHORD` empilha vozes afinadas, não parciais de uma voz. `ADDITIVE` é o
controle harmônico fino de **uma** voz.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Síntese aditiva clássica** (Fourier; órgão de tubos; Bell Labs;
  Kawai K5000; NED Synclavier; ANS) — timbre = soma de senóides;
  domínio público, teoria pura.
- **Xaoc Odessa** (*referência funcional* ★★, `PESQUISA §7 #85`) — banco
  de parciais de Eurorack com "tilt", "comb", "chroma", inarmonicidade
  ("stretch/squish"), *partial count* e *bank* como macros de
  performance. Só o **conceito** dos macros — nenhum código (é fechado);
  a origem pública é a síntese aditiva e o *spectral tilt* de
  processamento de sinais.
- **Inarmonicidade de cordas/barras** (`f_k = k·f0·√(1+B·k²)` do piano;
  modos de barra livre 1 : 2,76 : 5,40 do vibrafone) — teoria de
  acústica, domínio público. Aqui a lei é linearizada (`k + c·k(k−1)`)
  pra manter a razão monotônica e o corte de Nyquist barato.
- **Filtro em pente** — a modulação `cos` sobre o índice de parcial é o
  espectro de um comb; teoria de DSP básica.

**Desvio Rasgo (Atlas §49):** o `drift` de cintilância determinística
(banco que respira sem RNG, quase-período longo) e a relação — `tilt`,
`odd`, `stretch`, `comb` são todos endereçáveis por CV do próprio grafo,
então o espectro é um **processo** dirigido por outros módulos, não um
preset.

## 3. Modelo

**Preparação (`prepare`, fora do RT):**
```
lut[i] = sin(2π i / 2048),  i em [0,2048)
phase_[k] = 0,  k em [0,64)
```

**Por bloco (recalcula parciais — params mudam devagar):**
```
e     = 2,6 − 2,45·tilt                       # expoente de tilt
Σamp  = 0
para k = 1..64:
  razão_[k] = k + stretch·0,004·k·(k−1)
  a  = k^(−e)
  a *= (k ímpar) ? (1 + odd) : (1 − odd)       # balanço ímpar/par, clamp ≥0
  a *= (1 − comb) + comb·(0,5 + 0,5·cos(2π·(1+11·comb)·k/64))
  a *= 1 + 0,12·drift·sin(2π·driftA + k·2,3999)   # respiração de amp
  amp_[k−1] = a;  Σamp += |a|
det_[k−1] = 0,006·drift·sin(2π·driftB + k·1,111)   # micro-desafino
outNorm = 0,9 / max(Σamp, 1e−4)
driftA += 0,05·blocoDur ; driftB += 0,035·blocoDur
```

**Por amostra:**
```
f0 = freq · 2^(fine/1200 + pitch_in)
f0 · = 1 + fm_in · fm_amount · 4                (clamp f0 ∈ [0,01; 0,49·sr])
acc = 0
para k = 0..63:
  fk = f0 · razão_[k] · (1 + det_[k])
  se fk ≥ 0,98·nyq: break
  phase_[k] += fk · dt ; phase_[k] −= floor(...)
  g = amp_[k]
  se fk > 0,83·nyq:  g ·= (0,98·nyq − fk) / (0,15·nyq)   (clamp 0..1)
  acc += g · lut(phase_[k])
y = tanh(acc · outNorm · gainFollow_)
```

**Seguidor de ganho** (por bloco, depois de gerar): `pico` = máx |y| do
bloco; `alvo = min(1, 0,9/pico)`; `gainFollow_ += (alvo − gainFollow_)·
(alvo < gainFollow_ ? 0,5 : 0,02)`.

**Extremos:** `freq` a 8 kHz → só 2–3 parciais passam o corte → quase
seno, sem alias. `stretch = −1` → razões comprimidas (parciais quase
juntos, batimento denso) mas ainda crescentes. `comb = 1` + `tilt = 0` →
poucos parciais sobrevivem os vales → risco de silêncio: `outNorm` usa
piso, `tanh` não estoura, saída fica só baixa (correto). `odd = ±1` →
metade do banco zera; `Σamp` cai, `outNorm` compensa. `fm_in` grande →
`f0` clampado, sem parcial acima de Nyquist.

## 4. Três modos obrigatórios

- **autônoma:** `freq` audível, `tilt` médio, `drift` leve → um espectro
  que cintila sozinho; sem nenhum cabo já é um drone vivo.
- **performance:** `tilt`/`odd`/`stretch`/`comb` são os quatro macros
  expressivos; `freq` a afinação.
- **híbrida:** `ENVELOPE.env → tilt` (brilho abre com a nota);
  `LFO → stretch` (inarmonicidade respira); `SEQUENCE → 1V/O`.

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control v/oct), `tilt` (Control), `stretch`
(Control), `fm` (Audio).
**Saídas:** `out` (Audio).
**Parâmetros:** `freq` (8–8000 Hz, def 110), `fine` (−100..100 cent),
`tilt` (0–1, def 0,5), `odd` (−1..1, def 0), `stretch` (−1..1, def 0),
`comb` (0–1, def 0), `fm_amount` (0–1, def 0), `drift` (0–1, def 0).
**Limites:** `out` em ~[−0,95; 0,95] (`tanh` + seguidor de ganho). CPU:
por amostra até 64 (`+`, `×`, leitura de LUT) — pior caso ~64·4 flops;
por bloco 64·(1 `pow` + 2 `sin` + `cos`). `prepare` aloca ~8 KB
(LUT 2048 floats + 3 vetores de 64).

## 6. Alternativas descartadas

- **`std::sin` por parcial por amostra** — 64 transcendentais/amostra,
  caro sem ganho de qualidade sobre a LUT de 2048 + interp linear.
- **razão `k·√(1+B·k²)`** (inarmonicidade real de piano) — a raiz por
  parcial por bloco é ok, mas a lei fica não-monotônica pra `B<0`
  (compressão), quebrando o corte por `break`. A linearização
  `k + c·k(k−1)` mantém monotônica e bidirecional.
- **FFT inversa** (gerar o bloco no domínio da frequência) — mais barato
  pra 64+ parciais, mas trava a modulação por amostra de `f0`/`fm` e a
  fase fica descontínua entre blocos. Os acumuladores de fase dão
  vibrato/FM de graça.
- **RNG no `drift`** (como o `WAVETABLE`) — aqui a soma de senóides
  incomensuráveis já dá percurso não-repetitivo **e** determinismo total;
  RNG só adicionaria não-reprodutibilidade sem ganho audível.
- **contagem de parciais como knob** — fica como pendência; 64 fixos
  cobrem o alcance audível em toda `freq` útil (o corte de Nyquist já
  "desliga" os de cima).

## 7. Integração e painel

12 HP, família **SOURCE** (junto de `OSC`/`WAVETABLE`/`PLL`/`CHORD`).
Display do espectro/forma (o osciloscópio da saída já mostra a forma).
Knobs `FREQ`/`FINE`/`TILT`/`COMB` (linha 1), `ODD`/`STR`/`FM`/`DRIFT`
(linha 2); jacks `1V/O`/`TILT`/`STR` + `FM`/`OUT`.

Cadeias canônicas: `SEQUENCE → QUANTIZER → ADDITIVE → VCA`;
`ENVELOPE.env → ADDITIVE.tilt` (brilho segue a nota);
`FUNCTION/LFO → ADDITIVE.stretch` (o timbre metaliza e volta).
