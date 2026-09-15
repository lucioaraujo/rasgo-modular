# SHIFTER — deslocador de frequência

**Família:** TRANSFORM · **Módulo 58**
**Essência:** move o espectro **inteiro** por um Δf fixo em Hz (não em
razão) — os parciais deixam de ser harmônicos → metálico, sineiro. Saídas
`up` e `down` simultâneas.
**Dossiê técnico:** [`../dossies/58_shifter.md`](../dossies/58_shifter.md)
· **Fonte:** `src/dsp/Shifter.hpp`

---

## A ideia

Um ring-mod (o `RING` do `SHAPE`) dá as **duas** bandas — soma e
diferença — simétricas. O `SHIFTER` faz *single sideband*: move todo o
espectro por Δf **em Hz**, e entrega `up` (espectro + Δf) e `down`
(espectro − Δf) **cada um em sua saída**. Como o deslocamento é em Hz e
não em proporção, um som harmônico vira inarmônico — o efeito Bode/Moog.

## Por dentro

**Por que deslocar em Hz (não em razão) quebra a afinação:** transpor
uma oitava é **multiplicar** todas as frequências pelo mesmo fator (ver
`CABEAMENTO.md` §4) — as razões entre os parciais ficam intactas, então
o som continua afinado, só mais agudo/grave. O `SHIFTER` faz outra
coisa: **soma** o mesmo número de Hz a cada parcial. Um som harmônico
tem parciais em múltiplos exatos da fundamental (100, 200, 300, 400
Hz…); somar 30 Hz a cada um dá 130, 230, 330, 430 Hz — as razões entre
eles **não são mais** múltiplos exatos de nada. O som perde a
identidade de "nota afinada" e ganha um caráter metálico/sineiro —
porque agora ele tem a mesma estrutura inarmônica de um sino de
verdade.

**O que é *single sideband*, e por que precisa de uma transformada de
Hilbert:** um ring-mod comum (`RING` do `SHAPE`) produz **duas** bandas
por parcial — soma e diferença — misturadas na mesma saída. *Single
sideband* consegue produzir **só uma** das duas (só a soma, ou só a
diferença) numa saída própria — pra isso, o circuito/algoritmo precisa
conhecer não só a onda em si, mas uma versão dela **defasada 90°** em
cada frequência simultaneamente (a transformada de Hilbert faz
exatamente isso, calculada aqui por um filtro FIR longo). Combinando o
sinal original com essa versão defasada de jeitos diferentes, as duas
bandas se **separam** — uma cancela e sobra só a soma (`UP`), a outra
cancela e sobra só a diferença (`DN`).

**Por que a separação piora no grave profundo:** o método depende da
defasagem de 90° ser precisa em **toda** frequência — e um filtro FIR
de tamanho finito tem sua precisão limitada nas frequências mais baixas
(quanto mais grave, mais ciclos o filtro precisaria pra "ver" a fase
direito). Abaixo de certo ponto a separação degrada e a saída tende a
parecer mais com um ring-mod comum (as duas bandas vazando juntas) — é
uma limitação real do método, presente também no hardware analógico
histórico (Bode/Moog), não um defeito desta implementação.

**O *barber-pole*/efeito Shepard-Risset, explicado:** realimentar a
saída `UP` de volta na entrada faz o espectro subir **de novo**, a cada
volta pelo laço — um som que já subiu `SHIFT` Hz sobe outros `SHIFT` Hz,
e outros, indefinidamente. Isolado, isso faria o som sair da faixa
audível rapidinho — mas como o processo continuamente introduz
conteúdo **novo** nas frequências baixas (o próprio `IN` continua
entrando) enquanto o conteúdo antigo sai pelo topo, o ouvido percebe
uma ilusão: um espectro que parece **subir eternamente**, sem nunca
"chegar" a lugar nenhum — o mesmo truque perceptivo por trás da escada
de Shepard-Risset (o "tom que só sobe"), aqui obtido por deslocamento em
vez de por camadas de senoides desenhadas.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o som a deslocar. **Plugue aqui:** uma voz, um
  acorde, um pad — algo com harmônicos pra "desafinar".
- **`SFT`** (controle) — soma ao knob `SHIFT`, em Hz (1 unidade =
  1000 Hz). **Plugue aqui:** um LFO (um vibrato de espectro), um
  `ENVELOPE`, `DRIFT`.

### Saídas (ambas áudio)

- **`UP`** — espectro + `SHIFT` (a banda lateral superior).
- **`DN`** — espectro − `SHIFT` (a inferior). Cabeie as duas em destinos
  **diferentes** — a relação entre elas é o processo.

## Os controles, um a um

**SHIFT** (−2000..2000 Hz) — o deslocamento. 0 = passa-direto. Δf pequeno
(5–20 Hz) = um "*phasing*" que nunca fecha (as duas cópias batem
devagar). Δf grande = clangor inarmônico.

**FEEDBACK** (−0,95..0,95) — parte da saída `up` volta pra entrada →
glissando infinito (o *barber pole*). Negativo puxa de `down` (desce
para sempre). `tanh` no laço.

**TONE** (−1..1) — inclina o molhado (1 polo): <0 abafa o agudo do sinal
deslocado; >0 realça. 0 = neutro.

**DRIFT** (0–1) — *wobble* lento e semeado no Δf (±50%) — o deslocamento
respira. Determinístico.

**MIX** (0–1) — seco (`IN`) ↔ deslocado, nas duas saídas. 0 = bypass.

## Como cabear

**Metalizar uma voz:**
```
CHORD (OUT) → SHIFTER (IN)
SHIFTER (UP) → MIXER (ch1)     SHIFT ~120 Hz
```

**Barber-pole (a corda de Risset que sobe eternamente):**
```
PAD → SHIFTER (IN)
SHIFTER (UP) → SPACE (in) → MIXER (ch1)
```
`SHIFT` ~8 Hz, `FEEDBACK` ~0,8.

## Potencializar

- **Phasing sem LFO:** `SHIFT` em ±5–15 Hz, `MIX` ~0,5 — as cópias
  batem devagar; um som parado ganha movimento.
- **UP e DOWN cruzando:** as duas saídas em canais do `MIXER` com pans
  opostos, um `ENVELOPE` no `SHIFT` — o espectro se abre no palco a cada
  nota.
- **Descida infinita:** `FEEDBACK` negativo — o espectro afunda para
  sempre (bom pra transições / quedas).
- **Cruze com o `SPECTRA`:** `SHIFTER → SPECTRA.in` — o `SPECTRA`
  re-oscila o espectro já inarmônico e você transpõe / congela.

## Se você conhece o Eurorack

Faz o papel de um *frequency shifter* (Bode/Moog, anos 1960; o
Doepfer A-126, o modo shift do Warps). A base é *single sideband* por
transformada de Hilbert (DSP clássico) e o *barber-pole* de
Shepard-Risset. Distinto do ring-mod do `SHAPE` (bandas simétricas) e do
`SPECTRA` (re-síntese que segue a altura).
