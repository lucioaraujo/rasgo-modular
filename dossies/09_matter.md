# Dossiê — Módulo 9: Ressoador modal (`MATTER`)

**Família:** MATTER
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Matter.hpp`, `tests/test_matter.cpp`

## Estado da implementação (marco 2)

Feito: banco de **24 modos** ressonantes (ressoadores de 2 polos), cada um
em `f_i = f0 · razão_i`, excitado pelo sinal de entrada e/ou por um golpe
interno.

- **razões:** `razão_i = i · (1 + structure · 0,055 · (i−1))` — `structure`
  0 → harmônicas exatas (corda); `structure` 1 → progressivamente
  esticadas (rigidez → sino/metal);
- **amplitude alvo do modo:** `pow(i, −(2 − 2·brightness)) · |sin(i·π·position)|`
  — `brightness` faz o rolloff (0 = escuro `1/i²`, 1 = plano); `position`
  é a posição de excitação (um modo cujo nó cai em `position` não recebe
  energia — corda tocada no nó do 2º harmônico não o excita);
- **`b0 = amp_alvo · sin(w)`** (compensa o ganho natural do ressoador a
  baixa freq → a resposta impulsiva fica ~proporcional a `amp_alvo`),
  normalizado pelo modo mais forte;
- **decaimento:** `T60_i = (0,05 + (1−damping)·4) / (1 + i·0,35)` — modos
  altos decaem mais rápido (físico); `r_i = exp(−ln1000 / (T60_i·sr))`,
  travado em ≤ 0,99995;
- **golpe interno:** borda de subida em `strike` → rajada de ruído
  xorshift de ~10 ms escalada por `exciter`;
- `mix` seco/molhado; `softLimit = tanh` na saída (makeup ×3 antes).

Coeficientes recalculados **por bloco** (24 modos, barato). Determinístico
(rng do golpe semeado em `prepare()`).

**Testes (8/8 alvos, Debug + Release):** há ressonância logo após o golpe
e ela **decai** (RMS a ~4,5 s < 35% do início); a **periodicidade**
(autocorrelação) acompanha `freq` em 110/220/440 Hz (±8%) com `structure`
harmônico; `damping` baixo sustenta 3× mais que `damping` alto;
`structure` 0 vs 1 dá timbres audivelmente diferentes; `damping = 0`
(pior caso, `r` perto de 1) + ruído + `freq_mod` senoidal: 6000 blocos
sem NaN nem `|y| > 1,5`; dois renders byte-idênticos; integração no grafo
(`CLOCK.euclid → MATTER.strike`); painel fecha (14 HP).

**Pendências (candidatos, não controles fictícios):** corda por
guia-de-onda (Karplus-Strong / Elements) como segundo modelo dentro do
mesmo módulo (`structure` cruzando de modal a corda); modos com
amplitude e razão vindos de um perfil de material nomeado (madeira,
vidro, pele); excitação por "arco" (ruído contínuo filtrado com pressão);
`freq_mod` por amostra (hoje uma vez por bloco); interpolação suave dos
coeficientes entre blocos (glissando de `freq` hoje "escadinha" ao vível
de bloco); saídas separadas de modos pares/ímpares.

---

## 1. Problema musical e papel no fluxo

Os Módulos 1-2 dão fonte e filtro; 7-8 dão memória e sequência. Falta o
**corpo** — o objeto que soa. Um ressoador modal transforma qualquer
excitação (um clique, um gate, ruído, a voz) no som de uma coisa: uma
corda, um sino, uma placa, um tubo. É a diferença entre "um oscilador
filtrado" e "algo sendo tocado". Numa lógica generativa, `CLOCK`/
`DECISION` batem no `MATTER` e o instrumento tem percussão afinada de
graça.

Papel: recebe `strike` do `CLOCK` (euclid/accent) ou `in` da voz;
`structure`/`position` modulados por `DECISION`/`TURING` fazem o material
mudar a cada nota; a saída vai ao `FILTER`, ao `SPACE` (Módulo 10) ou
direto.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Rings / Elements** (`pichenettes/eurorack`, STM32F4) | ressoador modal + corda num módulo; `structure`/`brightness`/`damping`/`position` como eixos contínuos do material; excitação externa ou interna | MIT — estudo do comportamento, sem código |
| **P. Cook**, *Real Sound Synthesis for Interactive Applications* (2002); **Adrien** (síntese modal) | um corpo = soma de modos amortecidos; a resposta impulsiva é a "assinatura" do objeto | teoria pública |
| **Fletcher & Rossing**, *The Physics of Musical Instruments* | inarmonicidade da corda rígida (modos não são múltiplos exatos); modos altos decaem mais rápido | teoria pública |
| **J. O. Smith**, *Physical Audio Signal Processing* | ressoador de 2 polos, normalização de ganho, estabilidade perto do círculo unitário | livro online público |

## 3. Modelo — matemática, estados, extremos

**Por modo** (recorrência):
`v[n] = a1·v[n−1] − a2·v[n−2] + b0·x[n]`, com
`a1 = 2r·cos(w)`, `a2 = r²`, `w = 2π·f_i/sr`. Polos em `r·e^{±jw}`.

**Coeficientes** (por bloco):
```
razão_i     = i·(1 + structure·0,055·(i−1))
f_i         = f0·razão_i           (modo mudo se f_i ≥ 0,49·sr)
T60_i       = (0,05 + (1−damping)·4) / (1 + i·0,35)
r_i         = min(exp(−6,9078/(T60_i·sr)), 0,99995)
amp_alvo_i  = i^{−(2−2·brightness)} · |sin(i·π·position)|
b0_i        = amp_alvo_i · sin(w_i)      → normalizado por max_i(amp_alvo_i)
```

**Saída:** `y = tanh( 3·Σ_i v_i )`. `out = mix·y + (1−mix)·excitação`.

**Excitação:** `x = in + (golpe ativo ? exciter·6·ruído·burst : 0)`,
`burst *= 0,9985` por amostra (≈ 10 ms).

**Estados:** `z1`/`z2` por modo (48 floats), `burst_`, `prevStrike_`,
rng. Sem alocação.

**Extremos.** `damping = 0` → `T60` de 4 s no modo 0, `r` travado em
0,99995 (não em 1 — sem realimentação divergente); `tanh` segura picos.
`freq` no teto (8 kHz) → só o modo 0 cabe abaixo de Nyquist, o resto é
mudo. `position` no mínimo (0,02) → quase só `sin` pequenos → poucos
modos excitados (só os agudos). `structure = 1` + `i` alto → razão ~
`i·(1 + 0,055·i)` ainda finita e abaixo de Nyquist ou mudo. Reset →
`z1 = z2 = 0` em todos os modos.

## 4. Três modos obrigatórios

- **Autônoma:** só com `strike` interno (um `CLOCK` batendo) e sem `in`,
  o `MATTER` é uma voz percussiva afinada completa; `structure`/`damping`
  fixos já dão um instrumento.
- **Performance:** `structure`, `brightness`, `damping`, `position` são
  os quatro macros do material — varrer `structure` leva de corda a sino
  continuamente; `position` muda o "ataque" (onde bate); `damping` de
  pizzicato a drone.
- **Híbrida:** `in` = a voz do `FUNCTION`/`FILTER` excita o corpo
  (voz "ressoada"); `strike` do `CLOCK` articula; `freq_mod` (1 V/oct) e
  `struct_mod` de `TURING`/`DECISION` fazem cada nota ter altura e
  material próprios.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio, excitação), `strike` (Control, borda → rajada
interna), `freq_mod` (Control, 1 V/oct), `struct_mod` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `freq` | 20–8000 Hz (log) | 110 | fundamental |
| `structure` | 0–1 | 0 | harmônico → esticado (corda → sino) |
| `brightness` | 0–1 | 0,6 | rolloff dos modos altos |
| `damping` | 0–1 | 0,35 | escala os T60 (sustentado → seco) |
| `position` | 0,02–0,5 | 0,18 | posição de excitação |
| `exciter` | 0–1 | 0,5 | nível da rajada de ruído interna |
| `mix` | 0–1 | 1 | seco/molhado |

**Limites:** saída em [−1,1] (`tanh`). CPU: 24 modos × (~4 mult/add) por
amostra + `updateModes` (24 `exp`/`sin`/`pow`) uma vez por bloco. Sem
alocação. Estado fixo.

## 6. Alternativas descartadas

- **Karplus-Strong / guia-de-onda já no marco 2:** dá corda excelente,
  mas é um segundo motor. `structure` cruzando de modal a corda é
  candidato (é o que Elements faz).
- **FFT de resposta impulsiva de amostras reais (convolução):** som
  "real", mas custo, latência e sem os eixos contínuos (`structure` etc.)
  que fazem o módulo tocável.
- **Modos com `a1`/`a2` interpolados por amostra:** mais suave em
  glissando, mais caro; recalcular por bloco alia pouco em modulação
  lenta (2ª camada, como o oversampling do `FILTER`).
- **Normalização por soma das amplitudes:** deixa o módulo fraco quando
  muitos modos estão ativos; normalizar pelo modo mais forte mantém
  nível consistente.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** a periodicidade do som após um golpe (autocorrelação) bate
com `freq` para `structure = 0` (±3%); com `structure` alto a
autocorrelação enfraquece (inarmônico); `damping` controla o T60 medido
(±20%); `damping = 0` estável (r < 1 travado, `tanh` segura); dois
renders byte-idênticos; `struct_mod`/`freq_mod` sem NaN; sem alocação
em `process()` (só `updateModes`, que roda fora do laço de amostra mas
dentro do bloco — anotado).

**Escuta:** varrer `structure` soa como uma transição contínua
corda→sino ou como dois sons? `position` muda o caráter do ataque de
forma audível? `damping` alto dá "clave", baixo dá "tigela tibetana"?
`CLOCK → MATTER.strike` já é percussão afinada que dá vontade de tocar?
a voz passada por `MATTER.in` soa "ressoada" ou só filtrada?

## 8. Integração e painel

Classe `Matter` (`type()` = `"MATTER"`), 4 entradas, 1 saída, 7
parâmetros. `panel()` próprio (14 HP: display dos modos +
FREQ/STRUCT/BRIGHT/DAMP, POS/EXCITE/MIX, jacks). Testado isolado
(ring/decay, altura, damping, timbre, estabilidade) antes do patch.
Entra numa peça como o corpo percussivo: `CLOCK → MATTER.strike`,
`structure`/`freq` de um `TURING`, saída → `SPACE`.
