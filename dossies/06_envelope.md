# Dossiê — Módulo 6: Envelope + VCA (`ENVELOPE`)

**Família:** UTILITY / TIME
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Envelope.hpp`, `tests/test_envelope.cpp`

## Estado da implementação (marco 1)

Feito: um **contorno de amplitude** disparado por gate, com **VCA
embutido** — o módulo gera a forma E já a aplica a um sinal de áudio.

- Máquina de estados **A / D / (S) / R**. `mode = 0` (gated / ASR):
  segura no `sustain` enquanto o gate está alto, solta no `release`.
  `mode = 1` (trigger / AD): one-shot, ignora o comprimento do gate.
- Cada segmento é um fasor linear [0,1] moldado por `curve`:
  `curve = 0` convexa (ataque "mole", começa devagar), `0,5` linear,
  `1` côncava (ataque "estalado", quase exponencial). `shape(x) =
  x^expoente`, expoente 4 → 1 → 0,25.
- **VCA:** `out = in · ((1 − vca_depth) + vca_depth · envUnit)` —
  `depth = 1` é VCA clássico, `depth = 0` deixa o áudio passar intacto
  (o `env` continua saindo).
- `level` escala só a saída de CV `env` (0..level); `time_mod` (1 V/oct)
  multiplica os três tempos por `2^time_mod` (0,03×..32×).
- Determinístico (sem RNG). Ataque retomado do nível atual (retrigger
  sem clique grosseiro).

**Testes (7/7 alvos, Debug + Release):** AD dispara, sobe no ataque
(480 amostras), pico ≈ 1 = `level`, decai, volta a ~0; ASR segura no
`sustain = 0,5` com gate alto e solta a ~0 no release; VCA `depth = 1` →
saída segue o envelope e fecha quando ele fecha, `depth = 0` → áudio
passa em ganho unitário exato; curva côncava sobe mais rápido no início
do ataque que a convexa (Δ > 0,05 a ¼ do ataque); `time_mod = 1` estica
os tempos de forma medível; `level = 0,5` → pico do `env` em 0,5;
dois renders byte-idênticos, `env ∈ [0,1]`, finito; 4000 blocos estéreo
com disparos rápidos sem NaN nem alocação; integração no grafo
(`CLOCK.euclid → ENVELOPE.gate`, `FUNCTION → ENVELOPE.in` → voz
articulada: alto nos ataques, fecha entre pulsos); painel fecha (12 HP).

**Pendências (candidatos, não controles fictícios):** saída de
"fim-de-ciclo" (EOC) pra encadear envelopes / disparar o próximo evento;
loop (AD que se auto-dispara → vira LFO, princípio Maths/Just Friends);
`rise`/`fall` como CV de áudio (hoje só control-rate no `time_mod`);
lei de VCA selecionável (linear vs exponencial na aplicação, não só na
forma); dois canais (Just Friends tem 6); anti-click no gate por
suavização de 1–2 ms no ganho do VCA (hoje o próprio ataque mínimo
protege).

---

## 1. Problema musical e papel no fluxo

Os módulos 1–5 dão fonte, timbre, relação, decisão e tempo — mas o som
está sempre ligado. Falta o gesto que **articula**: transforma um drone
em frase, um clique em nota, um ruído em percussão. O envelope é esse
gesto; o VCA é onde ele age. Juntos num módulo porque, numa lógica
generativa, "disparar uma nota" é uma coisa só: o `CLOCK`/`DECISION`
manda o gate, o `ENVELOPE` dá forma e volume.

Papel: recebe `gate` do `CLOCK` (euclid/accent) ou do `DECISION`;
recebe a voz (`FUNCTION`) em `in`; entrega a voz articulada em `out` e a
forma em `env` (que modula corte de filtro, spread, o que for). É o
último elo antes da saída na primeira peça longa.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Make Noise Maths** (hardware; manual público) | envelope como **função** com `rise`/`fall` independentes e **curva contínua** côncava↔convexa; o canal é gerador E processador (VCA/mixer no mesmo bloco) | hardware, estudo de comportamento |
| **Mannequins Just Friends** | "transient" (one-shot) vs "sustain" (segue o gate) como um contínuo; envelope que faz loop vira oscilador/LFO | hardware |
| **Joranalogue Contour 1** | AD/ASR compacto com curva e saída de fim-de-ciclo | hardware |
| **Lei do VCA** (linear vs exponencial; literatura de síntese) | a curva de aplicação do ganho é caráter, não detalhe | teoria pública |
| **ADSR de livro-texto** (Chamberlin; a máquina de estados) | A→D→S→R com retrigger | domínio público |

## 3. Modelo — matemática, estados, extremos

**Máquina de estados.** `Idle → Attack → Decay → (Sustain) → Release →
Idle`.
- borda de subida do gate → `Attack`, `segPhase = 0`,
  `segStart = env` (retrigger do nível atual);
- borda de descida (só `mode = 0`) e estado ∈ {Attack, Decay, Sustain} →
  `Release`, `segStart = env`.

**Segmentos.** `segPhase += 1 / max(1, tempo · timeScale · sr)` por
amostra, com `tempo` = `attack`/`decay`/`release`.
```
Attack:  env = segStart + (1 − segStart) · shape(segPhase)   ; ao chegar a 1 → Decay
Decay:   env = sustain  + (segStart − sustain) · (1 − shape(segPhase))
         ; ao fim: se mode=0 e gate alto e sustain>0 → Sustain, senão → Release
Sustain: env = sustain  ; gate baixo → Release
Release: env = segStart · (1 − shape(segPhase))              ; ao fim → Idle (env=0)
```

**Curva.** `shape(x) = x^e`, `e = 1 + (0,5 − curve)·6` para
`curve < 0,5` (1..4, convexa), `e = 1/(1 + (curve − 0,5)·6)` para
`curve ≥ 0,5` (1..0,25, côncava). `curve = 0,5` → linear.

**VCA.** `envUnit = clamp(env, 0, 1)`;
`gain = (1 − vca_depth) + vca_depth · envUnit`;
`out = in · gain`; `env_out = envUnit · level`.

**time_mod.** `timeScale = clamp(2^time_mod, 1/32, 32)`.

**Estado.** `stage`, `segPhase`, `env`, `segStart`, `prevGate` — 5
escalares. Sem alocação, sem RNG.

**Extremos.** `attack` mínimo (1 ms) → `attack·sr = 48`, `segPhase`
avança 1/48 → 48 amostras de subida, sem clique grosseiro. Todos os
tempos no máximo (10 s) + `time_mod = 5` → clamp de `timeScale` a 32 →
320 s de segmento, `segPhase` minúsculo, `float` ok (perde resolução
além disso — anotado). `sustain = 0` em `mode = 0` → Decay vai a 0 e
segue pra Release (trivial). `sustain = 1` → Decay é imediato. Gate que
pisca mais rápido que o ataque → retrigger a partir do nível parcial
(comportamento correto). Entrada de áudio ausente → `out = 0`, `env`
continua.

## 4. Três modos obrigatórios

- **Autônoma:** sem entrada de áudio, o `env` sozinho é uma fonte de
  modulação com forma; com um gate lento e `mode = 1`, um pulso de
  contorno periódico. (Loop/auto-disparo pra virar LFO é 2ª camada.)
- **Performance:** `attack`/`decay`/`curve` são macros gestuais de
  articulação — de "pluck" (ataque curto, curva côncava) a "swell"
  (ataque longo, convexa). `mode` é um gesto discreto (staccato ↔
  sustentado).
- **Híbrida:** `gate` do `CLOCK`/`DECISION` articula no tempo do patch;
  `time_mod` deixa um LFO/`DECISION.x` alongar e encurtar as notas
  (frases que respiram); `env` sai pra modular outros módulos (o mesmo
  contorno abrindo o filtro).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `gate` (Control, borda dispara / nível
sustenta), `time_mod` (Control, 1 V/oct sobre os tempos).
**Saídas:** `out` (Audio, = `in` pelo VCA), `env` (Control, 0..level).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `attack` | 0,001–10 s (log) | 0,01 | tempo de subida |
| `decay` | 0,001–10 s (log) | 0,3 | queda ao sustain |
| `sustain` | 0–1 | 0 | nível sustentado (0 = AD) |
| `release` | 0,001–10 s (log) | 0,4 | queda ao silêncio |
| `curve` | 0–1 | 0,6 | convexa ↔ côncava |
| `mode` | 0/1 | 0 | gated (ASR) / trigger (AD) |
| `vca_depth` | 0–1 | 1 | quanto o env controla o VCA |
| `level` | 0–1 | 1 | pico da saída `env` |

**Limites:** `env ∈ [0,1]`, `out` limitado pelo próprio `in`. CPU: por
amostra 1 `pow` (o `shape`) + alguns mult/add. Sem alocação.

## 6. Alternativas descartadas

- **Envelope e VCA como módulos separados:** mais puro em teoria, mas
  "disparar uma nota" é um gesto só; Maths e Just Friends também fundem.
  Um VCA autônomo separado é candidato (pra somar/rotear várias vozes).
- **ADSR completo com 4 estágios plenos sempre:** o default `sustain = 0`
  já cobre AD; ASR sai de graça. DAHDSR seria excesso pro marco 1.
- **One-pole analógico (RC):** natural, mas a curva fica presa na
  exponencial; o fasor + `shape` dá `curve` contínuo e previsível.
- **Loop no marco 1** (AD → LFO): ótimo (Maths/JF), mas é um modo a
  mais e um risco de realimentação; 2ª camada com a saída EOC.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** o pico do `env` bate com `level` (±2%); o tempo pra atingir
90% do pico bate com `attack` (±10%); ASR segura exatamente em `sustain`;
`curve` muda a forma de forma monotônica e medível; `time_mod = 1` dobra
os tempos (±5%); VCA `depth = 0` passa o áudio bit-a-bit; dois renders
byte-idênticos; sem NaN; sem alocação (teste dedicado).

**Escuta:** ataque côncavo curto soa "percussivo" e o convexo longo soa
"orquestral"? o retrigger rápido soa musical ou "engasgado"? o VCA fecha
"limpo" (sem clique) nos ataques de 1 ms? `CLOCK → ENVELOPE → saída` com
a voz já soa como uma frase, não como um drone recortado?

## 8. Integração e painel

Classe `Envelope` (`type()` = `"ENVELOPE"`), 3 entradas, 2 saídas, 8
parâmetros. `panel()` próprio (12 HP: display do contorno +
ATK/DEC/SUS/REL numa fileira, CURVE/TRIG/VCA/LVL noutra, jacks embaixo).
Testado isolado (forma, sustain, VCA, curva, time_mod) antes do patch.
É o último módulo do marco 1 — com ele a **primeira peça longa** fica
possível: `CLOCK` → `DECISION` → `ENVELOPE` articulando `FUNCTION` →
`FILTER` → saída, tudo determinístico por seed e sem repetição.
