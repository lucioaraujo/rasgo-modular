# Dossiê — Módulo 1: Gerador de Função (`FUNCTION`)

**Família:** SOURCE / TIME
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/FunctionGenerator.hpp`, `tests/test_function_generator.cpp`,
`examples/primeiro_fragmento.cpp`

## Estado da implementação (marco 1)

Feito: acumulador de fase (double), forma triângulo↔serra por `slope`,
saídas `uni`/`bi`, `drift` (random-walk xorshift com seed via `prepare()`),
`sync` (borda de subida), suavização de parâmetro ~5 ms, PolyBLEP no wrap
da parte serra.

**Testes (3/3 configs):** frequência bate com `rate` ±2% (0,5 / 20 / 220 /
2000 Hz); `uni∈[0,1]`, `bi` limitado, sem NaN em toda a varredura de
`slope`; `drift=0` byte-idêntico entre renders; `drift>0` reprodutível com
a mesma seed e diferente de `drift=0`; **alias @23 kHz = -47 dB, @22 kHz =
-41 dB** (abaixo do -40 dB exigido, mas @22 kHz está no limite - polyBLAMP
nos cantos do triângulo é a 2ª camada, como previsto no §3); 5000 blocos
sem NaN com `sync` e `drift` ativos; integração no `SignalGraph` (voz →
Cable → saída, ruptura → cicatriz decai ao silêncio).

**Render:** `examples/primeiro_fragmento.cpp` →
`validation-output/primeiro_fragmento.wav` (8 s): LFO (0,7 Hz, drift 0,6)
modula o `rate_mod` da voz (~90 Hz, slope 0,72) em ±0,9 oitava; aos 4 s o
Cable rompe e a cicatriz decai (~2,5 s até silêncio). Primeira música do
Rasgo Modular.

**Pendências / candidatos (não controles fictícios):** polyBLAMP nos
cantos (alias @22 kHz no limite); vários modelos de síntese (Plaits);
função que emerge do que está conectado (Stages); through-zero; entrada de
`slope` como CV de áudio (hoje só control-rate).

---

## 1. Problema musical e papel no fluxo

O Rasgo Modular não produz um único som ainda. Precisa de uma **fonte** —
mas numa lógica generativa a fonte não deveria ter função fixa. O
gerador de função é **uma rampa que é envelope, LFO ou oscilador
conforme a taxa**: a mesma matemática, escalas de tempo diferentes.
É o princípio de "função emergente" (Stages) e "temporalidade orgânica"
(Tides) — a função do módulo depende de onde ele está no fluxo, não de
um seletor de modo.

Papel: fonte de áudio E fonte de modulação, num módulo só. Alimenta o
filtro (módulo 2), é modulado por outros geradores, e pode se
auto-modular por feedback (`SignalGraph` já suporta).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Tides** (`pichenettes/eurorack`) | um gerador de inclinação (slope) unificado: unipolar/bipolar, taxa de sub-áudio a áudio, forma (`slope`) deformável de rampa-sobe → triângulo → rampa-desce; saída "uni" e "bi" | STM32F = MIT (estudo do algoritmo; reescrito do zero) |
| **Mutable Stages** | função emergente — o segmento não tem tipo fixo | idem |
| **PolyBLEP** (Välimäki & Huovilainen; "Perceptually informed synthesis of bandlimited classical waveforms", e o post de Martin Finke) | correção de banda limitada na descontinuidade da rampa quando a taxa entra na faixa de áudio: subtrai um polinômio residual de 1-2 amostras ao redor do salto | técnica pública, sem código copiado |
| **EMW VC WAVETABLE LFO** (hardware que o autor usa) | LFO com forma controlada por tensão e faixa que cruza pra áudio — precedente prático da mesma ideia | — |
| **Lorenz / random walk** (`biome.odt`, seção 5 da pesquisa) | `drift` — passo aleatório lento e correlacionado na taxa efetiva; tempo que "respira" | técnica pública |

## 3. Modelo — matemática, estados, extremos

**Acumulador de fase.** `phase ∈ [0,1)`, incremento `dp = rate / sampleRate`
por amostra. `rate` em Hz, faixa **0,01 – 12000 Hz** (de envelope de
100 s a áudio agudo). Wrap em 1.0.

**Forma (`slope ∈ [0,1]`).** Ponto de quebra `k = clamp(slope, 0.001, 0.999)`.
```
subida:  s(phase) = phase / k                    para phase < k
descida: s(phase) = (1 - phase) / (1 - k)         para phase >= k
```
`slope = 0` → só descida (rampa que cai); `0.5` → triângulo; `1` → só
subida (dente-de-serra que sobe). `s ∈ [0,1]`, unipolar.

**Saídas.**
- `uni` = `s` (0..1) — envelope / rampa;
- `bi` = `2·s - 1` (-1..1) — LFO / oscilador.

**Anti-aliasing (só relevante quando `rate` é áudio).** A descontinuidade
da derivada em `phase = k` e o wrap em `phase = 1` geram aliasing. Para o
**wrap** (salto de valor quando `slope != 0.5`, porque `s(1⁻) != s(0⁺)`
exceto no triângulo): aplicar **PolyBLEP** — subtrair
`blep(t) = t²/2 + t + 0.5` (t = phase/dp, para `phase < dp`) e
`blep(t) = -(t²/2 - t + 0.5)` (t = (phase-1)/dp, para `phase > 1-dp`),
escalado pela altura do salto. A quebra em `k` (descontinuidade só de 1ª
derivada) alia bem menos — polyBLAMP seria o certo, mas fica como
candidato de 2ª camada; o marco 1 aceita o triângulo/rampa com polyBLEP
no wrap.

**`drift ∈ [0,1]`.** Passo aleatório: a cada `N` amostras (N ~ sampleRate/20),
`driftState += (rng()·2-1) · driftStep`, com `driftStep ∝ drift²` e
`driftState` limitado a ±0,5. `rate_efetivo = rate · 2^driftState`
(±meia oitava no máximo). Seed explícita. `drift = 0` → determinístico e
estável.

**Extremos.** `rate` no piso → `dp` minúsculo, sem problema numérico
(double na fase). `rate` no teto (12 kHz @ 48 kHz sr) → `dp = 0,25`,
polyBLEP com janela de 1 amostra ainda válido. `slope` nos limites →
clamp em 0.001/0.999 evita divisão por zero. Reset → `phase = 0`,
`driftState = 0`.

## 4. Três modos obrigatórios

- **Autônoma:** o módulo roda com `rate`/`slope`/`drift` dos seus
  parâmetros; `drift > 0` dá percurso vivo sem nenhuma entrada. Um
  gerador em taxa baixa + `drift` já é uma modulação orgânica que não se
  repete.
- **Performance:** `rate` e `slope` são macros contínuos com efeito
  audível imediato e suavização (rampa de parâmetro de ~5 ms pra não
  estalar). Varrer `slope` de 0 a 1 é um gesto musical (rampa→tri→dente).
- **Híbrida:** entradas de modulação (`rate_mod`, `slope_mod`) somam aos
  parâmetros; `drift` continua agindo por cima. Uma entrada de **sync**
  (trigger) força `phase = 0` — o gerador se re-ancora sem virar
  sequenciador.

## 5. Portas, parâmetros, limites

**Entradas:** `rate_mod` (áudio/CV, soma exponencial ao rate), `slope_mod`
(CV, soma linear), `sync` (trigger, reinicia a fase).
**Saídas:** `uni` (0..1), `bi` (-1..1).
**Parâmetros:** `rate` (0,01–12000 Hz, log, default 2 Hz), `slope`
(0–1, default 0,5), `drift` (0–1, default 0), `sync_enable` (0/1).
**Limites:** saída sempre em [-1,1] por construção. CPU: ~10 mult/add
por amostra + polyBLEP condicional. Sem alocação. Estado fixo (fase,
driftState, contador de drift, rng).

## 6. Alternativas descartadas

- **Wavetable** (EMW WAVE-6, tutorial JUCE): rica, mas o marco 1 quer
  função *emergente* (env/LFO/osc do mesmo objeto), não timbre. Wavetable
  entra como módulo próprio depois.
- **BLIT / minBLEP**: mais exato que polyBLEP, mais caro e mais código.
  polyBLEP é o padrão pra dente-de-serra/rampa e chega perto.
- **Vários modelos de síntese num módulo** (Plaits): é o oposto da regra
  de profundidade pro marco 1. Candidato, não agora.
- **Through-zero** (`biome.odt`): pertence a um oscilador de FM dedicado,
  não a este gerador.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** frequência medida da saída bate com `rate` (±1%); espectro
da rampa a 1 kHz mostra aliasing abaixo de -40 dB relativo à fundamental
com polyBLEP ligado; `drift = 0` byte-idêntico entre dois renders;
`drift > 0` com a mesma seed é reproduzível; reset zera; nenhum valor
não-finito; sem alocação (teste dedicado).

**Escuta:** varrer `slope` soa como uma transformação contínua e musical,
não como três sons costurados? `drift` em taxa de LFO soa "vivo" ou só
"instável"? o gerador em áudio + o filtro (módulo 2) já produz um som que
dá vontade de continuar mexendo?

## 8. Integração no `SignalGraph`

Nasce como uma classe `Signal` (`type()` = `"FUNCTION"`), 3 entradas,
2 saídas, 4 parâmetros. Testado isolado (render de impulso/rampa,
espectro, A/B com/sem drift) antes de entrar num patch. Entra no
`test_signal_graph.cpp` como primeiro módulo real e ganha um render curto
que já é música (gerador → filtro → saída, com uma ruptura no meio).
