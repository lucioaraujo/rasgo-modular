# Dossiê — Módulo 26: VCO parafônico (`CHORD`)

**Família:** SOURCE
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Chord.hpp`, `tests/test_chord.cpp`
**Candidato registrado:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `OSC` é monofônico. `HARMONY` gera CV de acordes mas nada **renderiza
um acorde como áudio**. `CHORD` empilha 2–4 vozes de uma base 1 V/oct,
com o formato do acorde por parâmetro **ou** por CV (casável com o
`HARMONY` no controle → progressões que tocam sozinhas).

- **`pitch`** (1 V/oct) → altura da fundamental; **`chord_cv`** seleciona
  o formato do acorde (0–1 varre a tabela); **`fm`** (áudio) modula todas
  as vozes;
- **`chord`** (0–1) — 10 formatos curados (uníssono, oitavas, quinta,
  maior, menor, sus4, maj7, min7, dim, add9) — semitons a partir da raiz,
  como as escalas curadas do `QUANTIZER`;
- **`voices`** (2–4) — quantas vozes soam;
- **`inversion`** (0–1 → 0–3) — sobe as *n* notas mais graves uma oitava
  (inversões do acorde);
- **`voicing`** (0/1, def 0, 2026-09-04) — `0` = paralelo (cada voz salta
  pro tom do índice); `1` = **condução de vozes**: na troca de acorde
  cada voz vai pro tom do novo acorde MAIS PRÓXIMO do que ela toca
  (ajuste de oitava, atribuição gulosa) e DESLIZA até lá (~40 ms) —
  progressões suaves, como um teclista;
- **`detune`** (0–1) — espalha as vozes ±~0,25 semitom (super-saw / coro);
- **`wave`** (0–1) — serra → pulso → triângulo (morph contínuo), com
  PolyBLEP na descontinuidade da serra e nas bordas do pulso;
- desvio **`drift`** — cada voz ganha um passeio de afinação independente
  (±~4 cents), xorshift semeado. `drift = 0` → determinístico.

Somatório das vozes normalizado por `1/√voices` (potência constante).
Sem alocação em `process()`.

**Testes (12 funções, Debug + Release — `tests/test_chord.cpp`):**
`voicing = 1` numa troca de acorde → as saídas divergem da versão
paralela na janela pós-troca (as vozes deslizam), determinístico e
finito; `chord` maior → energia medida em `f`, `f·2^(4/12)`, `f·2^(7/12)`;
`chord` menor → o 3º harmônico da raiz vira menor (`2^(3/12)`);
`voices = 2` → só 2 componentes de altura; `inversion` sobe a nota grave
uma oitava (energia migra de `f` pra `2f`); `detune > 0` → batimento
(a envoltória de amplitude modula devagar); `wave` varre serra→tri
(conteúdo harmônico ímpar/par muda); `pitch` +1 oitava → todas as
frequências dobram; `chord_cv` troca o acorde ao vivo; `drift = 0`
determinístico; tudo finito e ≤ ~1,05; grafo `SEQUENCE → CHORD.pitch`
· `CHORD → FILTER`.

**Pendências (candidatos):** glide por voz separado do `voicing`; saída
estéreo (espalhar as vozes no campo); `strum` (as vozes entram
escalonadas).

---

## 1. Problema musical e papel no fluxo

Um modular clássico é monofônico por natureza — uma nota por oscilador.
Fazer um acorde exige N osciladores + N quantizadores + roteamento, e
mudar o acorde ao vivo é impraticável. O `CHORD` empacota isso: uma
entrada de altura, um knob de formato, e sai um acorde. Com `HARMONY` no
`chord_cv`, a progressão toca sozinha (o modo autônomo do instrumento).

Papel: fonte. `SEQUENCE/TURING → QUANTIZER → CHORD.pitch` (baixo que
vira acorde); `HARMONY.root` → um `CONTROL` → `CHORD.chord_cv`
(progressão); `CHORD → FILTER → ENVELOPE` (pad).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Plaits — modelo "chord"** | um oscilador que soa 4 vozes de um acorde selecionável | conceito |
| **Harmonaig / Chord Machine** | acorde a 4 vozes definido por CV; inversões | conceito (hardware) |
| **Super-saw (Roland JP-8000)** | várias serras levemente destoadas = coro/largura | conceito |
| **PolyBLEP** (Välimäki/Finke) | antialias na descontinuidade da serra/pulso | teoria pública |
| **Tabelas de acorde / vozeamento** (qualquer texto de harmonia) | semitons de cada grau; inversão = subir a base uma oitava | fato musical |

## 3. Modelo — matemática, estados, extremos

Setup por bloco:
```
base = freq · 2^(pitch)                                (Hz)
ci = round(chord_param·9 + chord_cv·9) clamp [0,9]     índice na tabela
tab[ci] = { s0, s1, s2, s3 }                           semitons
nv = round(voices) clamp [2,4]
inv = round(inversion·3) clamp [0,3]
```
Por voz *k* < nv, por amostra:
```
semi = tab[ci][k] + (k < inv ? 12 : 0)
       + detune·0.25·(k − (nv−1)/2)                    espalhamento
       + driftCur_k                                    (cents/100)
fk = base · 2^(semi/12) · (1 + fm·0)  ... fm soma em Hz:
fk = base · 2^(semi/12) + fm[frame]·base·0.5
phase_k += fk/sr ; wrap
saw = 2·phase_k − 1 − polyBLEP(phase_k, fk/sr)
pulse = saw − (2·(phase_k+0.5 wrap) − 1 − polyBLEP(...))    (dois saws)
tri = 2·|saw|·... (integrador leaky do pulse, ou 2·|2·phase−1|−1)
v = wave<0.5 ? lerp(saw, pulse, wave·2) : lerp(pulse, tri, (wave−0.5)·2)
acc += v
out = acc · (1/√nv) · 0.5
```
`driftCur_k` desliza pra um alvo xorshift novo a ~6 Hz (±drift·0,04 em
fração de semitom ≈ ±4 cents no teto).

**Estados:** `phase_[4]`, `driftCur_[4]`, `driftTgt_[4]`, `driftCounter_`,
`triState_[4]` (integrador do tri), `rng_`. Sem alocação.

**Extremos.** `chord = 0` (uníssono) + `detune = 0` → todas as vozes na
mesma nota (soma coerente, mais alto — o `1/√nv` compensa). `detune`
alto + `voices = 4` → batimento denso, quase coro. `pitch` muito agudo →
as vozes agudas passam de Nyquist; a PolyBLEP segura o pior do alias mas
há limite (candidato: cap de frequência por voz). `inversion = 3` com um
acorde de 4 → todas as 4 vozes uma oitava acima da posição fundamental.
`fm` forte → as vozes desafinam juntas (vibrato/growl coerente). Reset →
fases e drift zerados, RNG re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** sem entrada, `pitch = 0` → toca um acorde na `freq` base
  (fonte de verdade — soa ao carregar). `drift > 0` → o acorde "vive"
  (as vozes batem levemente).
- **Performance:** `chord`/`inversion`/`voices` ao vivo = trocar o
  acorde, virar, engrossar; `detune` de "órgão" a "coro"; `wave` do
  brilho da serra ao veludo do triângulo.
- **Híbrida:** `SEQUENCE → QUANTIZER → pitch` (a fundamental anda);
  `HARMONY` (via `CONTROL`) → `chord_cv` (o formato muda com a
  progressão); `LFO → fm` (movimento).

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control, v/oct), `chord_cv` (Control), `fm`
(Audio).
**Saídas:** `out` (Audio).
**Parâmetros:** `freq` (16–4000 Hz, def 110), `chord` (0–1, def 0,3),
`voices` (2–4, def 3), `inversion` (0–1, def 0), `voicing` (0/1, def 0),
`detune` (0–1, def 0,15), `wave` (0–1, def 0), `drift` (0–1, def 0).
**Limites:** saída ≤ ~1,05 (o `1/√nv·0,5` mais o headroom). CPU: 4×
(fase + 1–2 PolyBLEP + morph) por amostra — o mais caro dos SOURCE, mas
linear em `voices`. Sem alocação.

## 6. Alternativas descartadas

- **N osciladores `OSC` internos como sub-módulos:** dá pra reusar o
  código do `OSC`, mas o `CHORD` precisa de vozes leves (sem os 5
  outputs, sync, sub) — uma voz enxuta é mais barata e o suficiente.
- **Acorde por wavetable (uma tabela por acorde):** perde o 1 V/oct por
  voz e a afinação justa; empilhar osciladores é mais honesto e
  destoável.
- **`chord` como tabela de escala + acorde diatônico:** amarra a um
  contexto tonal; a tabela de formatos absolutos é mais modular (o
  `QUANTIZER` cuida da escala se o músico quiser).
- **Voice-leading no marco 1:** é o próximo nível (precisa saber o
  acorde anterior); fica candidato, e o `HARMONY` já tem a lógica de
  movimento pra alimentar.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** acorde maior → energia em `f·2^(0)`, `2^(4/12)`,
`2^(7/12)` acima do piso; menor → o grau médio em `2^(3/12)`;
`voices = 2` → 2 componentes; `inversion` sobe a fundamental (energia de
`f` → `2f`); `detune > 0` → modulação de amplitude lenta (batimento) que
não existe em `detune = 0`; `wave` serra→tri muda a razão de harmônicos
pares; `pitch` +1 → todas as frequências ×2; `drift = 0` → dois renders
byte-idênticos; tudo finito.

**Escuta:** o acorde soa "afinado" (uníssono limpo) ou tem batimento
sujo? `detune` num pad soa "largo e vivo" ou "desafinado"? trocar
`chord` ao vivo tem clique (deveria ter suavização de fase? — as vozes
só mudam de frequência, a fase é contínua, então não)? `inversion` muda
o "peso" do acorde de forma audível? com `HARMONY` no `chord_cv`, a
progressão faz sentido harmônico?

## 8. Integração e painel

Classe `Chord` (`type()` = `"CHORD"`), 3 entradas, 1 saída, 8
parâmetros. `panel()` próprio (~12 HP): knobs FREQ/CHORD/VOX/INV/
DTUNE/WAVE/DRIFT/VLEAD, jacks PITCH/CHRD/FM in, OUT, Display (as vozes como
barras de altura). Testado isolado (formatos de acorde, vozes, inversão,
detune, wave, 1 V/oct, drift, determinismo) antes do patch. Cadeias
canônicas: `SEQUENCE → QUANTIZER → CHORD.pitch → FILTER → ENVELOPE`
(pad); `HARMONY → CONTROL → CHORD.chord_cv` (progressão). Adicionado ao
catálogo do painel (`apps/panel/ModuleCatalog.hpp`, família SOURCE).
