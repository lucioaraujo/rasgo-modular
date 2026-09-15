# ADDITIVE — oscilador aditivo

**Família:** SOURCE · **Módulo 42**
**Essência:** o timbre **construído parcial a parcial** — 64 senoides,
com quatro knobs que moldam o espectro direto. O oposto do subtrativo.
**Dossiê técnico:** [`../dossies/42_additive.md`](../dossies/42_additive.md)
· **Fonte:** `src/dsp/Additive.hpp`

---

## A ideia

Órgão, sino, vibrafone, pad tipo Kawai K5, drone com inarmonicidade
controlada — toda a paleta que vive de **espectro somado, não filtrado**
— não sai de `OSC → FILTER`. O `ADDITIVE` põe esse eixo no patch: em vez
de cortar agudos de uma serra, você **escolhe quanta energia cada
parcial recebe** na construção. Os quatro knobs (`TILT`/`ODD`/`STRCH`/
`COMB`) aceitam CV, então um `ENVELOPE` no `TILT` abre o brilho com a
nota, um LFO no `STRCH` respira a inarmonicidade.

## Por dentro

**A ideia por trás de síntese aditiva, em uma frase:** qualquer som
periódico pode ser descrito como uma **soma de senoides puras** em
frequências múltiplas de uma fundamental (isso é um fato matemático —
a série de Fourier —, não uma técnica inventada; todo som "tem" essa
decomposição). Um oscilador de serra ou uma voz de instrumento **já
são** essa soma, só que empacotada. O `ADDITIVE` faz o caminho oposto,
literalmente: 64 acumuladores de fase (a mesma engrenagem do `OSC`, um
por parcial) somam senoides puras, e você controla a **amplitude de
cada uma** — está construindo o timbre parcial por parcial, em vez de
receber um timbre pronto e talhá-lo por fora (como um filtro faz).

Quatro macros decidem, de uma vez, o **perfil de amplitude** das 64
parciais (ninguém mexe em 64 knobs):

- **`TILT`** — cada parcial `k` (a 2ª, a 3ª, a 10ª…) recebe uma
  amplitude proporcional a `k` elevado a um expoente negativo — quanto
  mais alto o expoente, mais rápido as parciais agudas perdem força.
  `TILT` varia esse expoente entre bem íngreme (só a fundamental e
  pouca coisa acima sobrevivem — escuro) e quase plano (todas as 64
  com força parecida — muito brilhante, quase ruidoso de tão denso).
- **`ODD`** — zera seletivamente as parciais **pares** ou **ímpares**.
  Isso não é arbitrário: uma onda quadrada de verdade **só tem**
  harmônicos ímpares — é uma consequência da sua simetria (metade do
  ciclo é o espelho invertido da outra metade). `ODD`=+1 recria
  exatamente essa condição (som oco, tipo clarinete); `ODD`=−1 mantém
  só as pares, que sozinhas soam uma oitava acima e mais nasais (falta
  a fundamental "ancorando" o grave).
- **`STRCH`** — desloca a **posição** de cada parcial pra longe (ou
  perto) da sua razão inteira exata com a fundamental — o mesmo
  princípio do `structure` do `MATTER` (#9): razões exatas soam
  "corda/afinado", razões esticadas soam "sino/metal". O efeito cresce
  **quadraticamente** com o índice da parcial — a 2ª quase não se move,
  a 60ª se move bastante — porque é assim que a rigidez física também
  se comporta em objetos reais (barras, sinos).
- **`COMB`** — depois do perfil acima decidido, este passa um **pente**
  por cima: zera parciais em intervalos regulares (a cada tantas). O
  resultado tem a mesma "cara" de um filtro em pente ou de um formante
  grosseiro (picos e vales regulares no espectro), mas chegou lá
  **compondo a soma diretamente**, sem passar sinal nenhum por um
  filtro de verdade.

Um seguidor de ganho + `tanh` na saída seguram o nível: somar 64
senoides que por acaso alinham seus picos ao mesmo tempo pode gerar
picos de amplitude bem mais altos que uma senoide só — a proteção evita
que esses picos ocasionais estourem.

## Os jacks, um a um

### Entradas

- **`1V/O`** (controle, altura) — a nota. **Plugue aqui:**
  `QUANTIZER.pitch`, `SEQUENCE.pitch`, `TURING.cv`.
- **`TILT`** (controle) — soma ao knob `TILT` (o brilho). **Plugue
  aqui:** `ENVELOPE.env` (o timbre abre com a nota), um `FUNCTION`.
- **`STRCH`** (controle) — soma ao knob `STRCH` (a inarmonicidade).
  **Plugue aqui:** um LFO lento — o som "metaliza" e volta.
- **`FM`** (áudio) — modulação de frequência linear. **Plugue aqui:**
  outra voz ou um LFO.

### Saída

- **`OUT`** (áudio) — a soma das 64 parciais. Vai ao `MIXER`
  (frequentemente direto — o espectro já é o timbre; um filtro depois é
  opcional).

## Os controles, um a um

**FREQ** (8–8000 Hz) — a fundamental / ponto de partida.

**FINE** (±100 cents) — afinação fina.

**TILT** (0–1) — o brilho / inclinação do espectro. 0 = escuro; 1 =
todas as parciais fortes. É o oposto de um filtro: você não corta, você
dosa.

**ODD** (−1..1) — o balanço ímpar/par. 0 = série cheia; +1 = só ímpares
(quadrada, oco); −1 = só pares (oitava acima, nasal).

**STRCH** (stretch, −1..1) — a inarmonicidade. 0 = harmônico; +1 =
esticado (sino, metal); −1 = comprimido (parciais juntas).

**COMB** (0–1) — o pente espectral. 0 = plano; 1 = 12 dentes, vales
fundos (formante ou filtro em pente).

**FM** (fm_amount, 0–1) — quantidade de FM linear.

**DRIFT** (0–1) — cintilância: micro-desafino e respiração de amplitude
por parcial, de senoides lentas incomensuráveis (sem RNG, reprodutível).
0 = estático.

## Como cabear

**Autônomo** — `FREQ` fixo, `DRIFT` pequeno, um LFO no `STRCH`:
um drone que oscila entre harmônico e sineiro.

**Voz com timbre dinâmico:**
```
SEQUENCE → QUANTIZER → ADDITIVE (1V/O)
ENVELOPE (env) → ADDITIVE (TILT)      (o brilho segue a nota)
ADDITIVE (OUT) → MIXER (ch1)
CLOCK (euclid) → ENVELOPE (gate)
```

## Potencializar

- **Sino de verdade:** `STRCH` alto + `TILT` médio + `DECAY` curto num
  `ENVELOPE` (VCA) — o esticamento das parciais + a cauda dá o badalar.
- **Órgão:** `STRCH` 0, `ODD` 0, `TILT` médio-alto, `COMB` num LFO lento
  — os "registros" abrem e fecham.
- **Metal que derrete:** um LFO lento no `STRCH` + um mais rápido no
  `COMB` — dois movimentos espectrais independentes.
- **Cruze com o `SPECTRA`:** `ADDITIVE → SPECTRA.in` com `voices` baixo
  e `stretch` — o `SPECTRA` faz uma caricatura inarmônica do espectro
  que você construiu.

## Se você conhece o Eurorack

Faz o papel do Xaoc Odessa (que também tem os macros tilt/comb/stretch,
mas é fechado). A base é síntese aditiva clássica (Fourier, órgão, Kawai
K5, Synclavier). Distinto do `WAVETABLE` (o espectro vem de uma tabela,
não é endereçável parcial a parcial), do `OPERATOR` (FM — o espectro
emerge da modulação) e do `CHORD` (empilha vozes, não parciais).
