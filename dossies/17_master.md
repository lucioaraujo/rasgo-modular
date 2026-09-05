# Dossiê — Módulo 17: Barramento de saída (`MASTER`)

**Família:** MIX / METER
**Estado:** **implementado — marco 2** (2026-09-02); default de `gain`
fixo em todo seed — **−24 dB (50% do slider)** desde 2026-09-05 (era
−38,4 / 30% em 2026-09-04) — pedido exato do autor, ver `TAREFAS.md`
registros "MASTER — volume padrão mais baixo", "MASTER — gain fixo em
30% do slider" e "painel — NOISE... / MASTER 50%"; limitador
passou a considerar PICO VERDADEIRO (entre amostras), não só pico de
amostra, em 2026-09-04 (ver `TAREFAS.md`, registro "excelência de saída
— true peak + dither TPDF"; `src/dsp/TruePeak.hpp`); **guarda
ultrassônica + governador de corpo** (`body_guard`) adicionados em
2026-09-05 (ver `TAREFAS.md`, registro "body guard")
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Master.hpp`, `src/dsp/OutputStage.hpp`,
`tests/test_mix.cpp`, `tests/test_output_stage.cpp`

## Estado da implementação (marco 2)

Feito: o **último nó antes das caixas**. Entrada estéreo, saída estéreo,
mais uma saída de controle `level` (VU).

- **largura estéreo (mid/side):** `mid = (L+R)/2`, `side = (L−R)/2`;
  `width` escala o `side` — `0` = mono, `1` = normal, `2` = largo;
- **soma mono:** `mono` força `L = R = mid` (compatibilidade / checagem);
- **ganho de master** (`gain`, −60..+12 dB);
- **proteção de saída de EXCELÊNCIA** (`src/dsp/OutputStage.hpp`,
  atualização 2026-09-02 — padrão da família RASGO, estudado de
  `NAVALHA2_JUCE/OutputStage`+`LookaheadLimiter` e
  `ANTITOTEM/OutputStage`, código do autor):
  - **guarda de finitude** — NaN/Inf → 0 (contados);
  - **bloqueio de DC** (`dc_block`) — passa-alta de 1 polo ~5 Hz;
  - **guarda ultrassônica** (sempre ligada) — LP Butterworth 2 polos a
    ~21 kHz. Transparente até ~15 kHz (−0,1 dB a 8 kHz), tira só a
    energia perto de Nyquist (ruído violeta/`bit` sustentados, hash de
    aliasing). Como o bloqueio de DC: nunca é escolha musical, não mexe
    no timbre agressivo que você OUVE;
  - **governador de corpo** (`body_guard`, 0..1, default 1,0; 0 = bypass
    exato) — 2026-09-05, pedido do autor ("gosto de barulhos, mas há
    alguns que passam do limite, incomodam o corpo"). TRÊS detectores
    estreitos (Q 3) em ~2,8 / 4,8 / 7,6 kHz + seguidores LENTOS (~240 ms)
    + um "gate de concentração" (razão banda/total: tom concentrado
    dispara, ruído de banda larga não). Quando dispara: high-shelf de 1
    polo (corte acima de ~1,4 kHz) até ~−9 dB, subindo/descendo devagar.
    Transiente, ritmo e ruído passam intocados; só o agudo **alto +
    sustentado + concentrado** em ~2,5–8 kHz é contido. Limiar ~−15 dBFS
    de banda. Telemetria: `bodyGuardDb()`;
  - **limitador com LOOK-AHEAD** (`limit`) — atraso de ~3 ms; seguidor de
    envelope (ataque ~0,5 ms / release ~120 ms) reduz o ganho **antes** do
    pico chegar → **não distorce o transiente** (o `tanh` instantâneo de
    antes distorcia);
  - **teto suave** (joelho exponencial) só bem perto do teto — pega o
    evento de 1 amostra que escapa do look-ahead;
  - teto ≈ **−1 dBFS** (0,891) com margem de 0,3 dB;
  - telemetria: `gainReductionDb()` (pra um medidor de GR / um módulo
    seguir a própria redução — desvio Rasgo);
- **`level`:** pico de `max(|L|,|R|)` com decaimento ~300 ms, limitado a
  [0,1] — pra um VU no painel.

Grafo mono → a saída é (L+R)/2. Determinístico, sem alocação em
`process()` (o buffer de look-ahead é alocado em `prepare()`). Latência:
~3 ms (o look-ahead).

**Testes (parte dos 9/9 alvos MIXER+MASTER, Debug + Release):** `width`
1 passa; `width` 0 → mono (mid nos dois); `width` 2 → side dobrado
(L 0,8 / R 0,0 de 0,6/0,2); `mono` → L = R = mid; `gain` −6 dB → metade;
`dc_block` remove offset de DC (0,3 → ~0 depois de assentar) e sem ele o
offset passa; `limit` segura ±3 perto de ±1; `level` acompanha o pico;
**`testMasterMute`:** `mute` usa rampa (1º sample ainda soa), silencia
os dois canais depois de ~80 ms, e volta ao sinal ao desligar;
dois renders byte-idênticos; integração no grafo (duas vozes → `MIXER` →
`MASTER`, L ≠ R); painel fecha. **`testMasterExcellenceGuard`:** seno a
+8 dB com transiente → **pico de saída ≤ 0,9, zero amostra estoura o
teto, GR > 3 dB** (o look-ahead segura sem distorcer); NaN/Inf na entrada
→ saída finita.

**Pendências (candidatos, não controles fictícios):** medidor de RMS +
LUFS além do pico; medidor de GR no painel; correlação de fase
(goniômetro) no display; true-peak 4× (o `TruePeakDetector` polifásico da
`NAVALHA`) pra conformidade inter-amostra BS.1770; dim/mute de
monitoração com rampa sem clique; trim pós-limitador.

---

## 1. Problema musical e papel no fluxo

O `MIXER` (Módulo 16) soma e panora, mas entrega um sinal cru — pode ter
DC do patch, largura estéreo exagerada, picos acima de ±1. Faltava o
**estágio final**: colar a imagem, proteger a saída, dar o nível de
master, e mostrar quanto está chegando. Sem o `MASTER`, o instrumento
grava/toca sinal sujo e sem controle de nível.

Papel: recebe `MIXER.out` (ou `SPACE.out`), processa o barramento
estéreo, entrega o sinal final pra saída de áudio; `level` → um VU no
painel; `gain`/`width` são os macros de master.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Matriz mid/side** (Blumlein; qualquer texto de mixagem) | `mid = (L+R)/2`, `side = (L−R)/2`; escalar o side controla a largura sem mexer no centro | teoria pública |
| **Bloqueio de DC** (Julius Smith, *Introduction to Digital Filters*) | passa-alta de 1 polo `y = x − x₁ + R·y₁`, `R` perto de 1 → corte em poucos Hz | fórmula pública |
| **Limitador suave** (`tanh`; prática de estúdio) | saturação suave com joelho, transparente abaixo do teto — o mesmo princípio do `FILTER`/`STRING` do Rasgo | conceito |
| **VU / peak meter** (padrão de mesa) | pico com decaimento controlado, normalizado | prática |

## 3. Modelo — matemática, estados, extremos

Por amostra (L, R da entrada; `R` mono → `R = L`):
```
mid  = (L+R)/2 ;  side = (L−R)/2 · width
L = mid + side ;  R = mid − side
se mono: L = R = mid
L,R ·= 10^(gain/20)
se dc_block:  y = x − x₁ + R_dc·y₁   (por canal; R_dc = e^(−2π·5/sr))
se limit:  softLimit(L), softLimit(R)
peak = max(|L|,|R|) > peak ? max(|L|,|R|) : peak·decay   (decay ~300 ms)
saída: canais ≥ 2 → [L, R] ; senão → (L+R)/2
level: clamp(peak, 0, 1) em todos os canais
```

**Estados:** `dcX_[2]`, `dcY_[2]` (bloqueio de DC), `peak_`. Sem
alocação, sem RNG.

**Extremos.** `gain = −60 dB` → silêncio. `width = 2` com sinal muito
lateral → `L` ou `R` pode passar de ±1; o limitador segura. `mono` +
`width` qualquer → o `width` é ignorado (mid puro). Entrada mono num
grafo estéreo → `R = L`, `side = 0`, passa. `dc_block` com sinal de
áudio normal (sem DC) → praticamente transparente acima de ~20 Hz.
Reset → estados do DC e `peak_` zerados.

## 4. Três modos obrigatórios

- **Autônoma:** com `gain`/`width` fixos, é o master do patch — não
  precisa de controle externo. `limit` e `dc_block` ligados por padrão
  (segurança).
- **Performance:** `gain` e `width` são os macros de master ao vivo
  (nível geral, abrir/fechar o estéreo). `mono` é um gesto de checagem.
- **Híbrida:** `gain` modulado por um `ENVELOPE`/`DECISION` via
  `connectToParameter` = fade generativo do master; `level` → um
  `DECISION.bias_mod` = o instrumento reage ao próprio volume.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio, estéreo — lê canais 0 e 1).
**Saídas:** `out` (Audio, estéreo), `level` (Control, 0..1 — VU).
**Parâmetros:** `gain` (−60..+12 dB), `width` (0..2), `mono` (0/1),
`mute` (0/1, default 0 — silencia a saída com rampa de ~8 ms, sem
estalo; pedido do autor 2026-09-05), `dc_block` (0/1, default ligado),
`limit` (0/1, default ligado), `body_guard` (0..1, default 1,0 —
governador de corpo; 0 desliga).
**Limites:** saída em ~[−1,1] (limitador). CPU: por amostra o mid/side +
2 passa-altas de 1 polo + 2 `tanh` (só se `limit`). Sem alocação.

## 6. Alternativas descartadas

- **Limitador com look-ahead / brickwall já no marco 1:** buffer de
  atraso + envelope follower; o `tanh` suave protege sem latência e sem
  código. Look-ahead é 2ª camada.
- **LUFS/RMS no marco 1:** o pico é o que importa pra não estourar; LUFS
  é medição de entrega, candidato.
- **Master mono only:** o `MIXER` já cria estéreo; o `MASTER` tem que
  processar L/R independentes.
- **Sem bloqueio de DC:** feedbacks e não-linearidades no patch podem
  acumular DC; ligado por padrão é o certo.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `width = 0` → `L = R` exato; `width = 2` → `side` dobrado
(medido); `mono` → `L = R = mid`; `gain` em dB bate com o linear;
`dc_block` remove um offset de DC injetado (< 2% depois de assentar);
`limit` segura ±3 abaixo de ±1,01; `level` acompanha o pico e decai;
dois renders byte-idênticos; sem alocação.

**Escuta:** `width` a 1,5 abre o estéreo sem soar "fora de fase"?
`mono` revela problemas de cancelamento? o limitador em transientes
fortes soa transparente ou "abafa"? o `dc_block` é inaudível em
material normal? o VU (`level`) responde como uma mesa de verdade?

## 8. Integração e painel

Classe `Master` (`type()` = `"MASTER"`), 1 entrada, 2 saídas, 5
parâmetros. `panel()` próprio (10 HP: VU + slider de GAIN + knob de
WIDTH + toggles MONO/DC/LIMIT + jacks IN/OUT/VU). Testado isolado
(largura, mono, DC, limitador, VU, determinismo) antes do patch. É o
último nó de todo patch: `…→ MIXER → MASTER → saída de áudio`. No painel
de teste, `MASTER.level` alimentaria o VU do topo.
