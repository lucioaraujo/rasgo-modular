# OSC — oscilador

**Família:** SOURCE · **Módulo 18**
**Essência:** a voz "neutra" do subtrativo — 1 V/oct preciso, cinco formas
ao mesmo tempo, PWM, hard sync, FM linear e um sub-oscilador embutido.
**Dossiê técnico:** [`../dossies/18_oscilador.md`](../dossies/18_oscilador.md)
· **Fonte:** `src/dsp/Oscillator.hpp`

---

## A ideia

Todo sintetizador subtrativo começa igual: um oscilador entrega uma forma
"crua", rica em harmônicos, e um filtro esculpe. O `OSC` é esse tijolo.
O `FUNCTION` é gerador de função (sem sync, sub, PWM); o `MATTER` e o
`STRING` são vozes de personalidade forte. O `OSC` é o que **não** tem
caráter próprio — é matéria-prima previsível, pra você dar o caráter com
o resto do patch.

O que o torna útil como base: rastreia altura com precisão, entrega as
cinco formas em jacks separados (você mistura as que quiser), e traz de
graça um sub-oscilador que engrossa o grave.

## Por dentro

Um acumulador de fase (a mesma ideia do `FUNCTION`, #01: um número que
sobe de 0 a 1 e recomeça — ver `guia/01_gerador_de_funcao.md` §"Por
dentro") gera a onda; a frequência vem de
`freq · 2^(fine/1200) · 2^(pitch) · 2^(drift)` — cada fator é uma
potência de 2 porque, em 1 V/oitava, **somar** tensão **multiplica**
frequência (ver `CABEAMENTO.md` §4). Some `fine` (em cents, 1/100 de
semitom), `pitch` (a CV `1V/O`) e `drift`, e cada um dobra ou reduz a
frequência pela sua fatia, todos ao mesmo tempo — daí o rastreio 1 V/oct
exato mesmo com vários fatores empurrando a afinação ao mesmo tempo.

**Por que a serra e o pulso precisam de um "remendo" (PolyBLEP) e o
triângulo/seno não:** uma serra ou um pulso têm uma **quebra abrupta**
no meio do ciclo (a serra despenca de +1 pra −1 instantaneamente; o
pulso salta de um nível pro outro). Uma quebra desse tipo, gerada
digitalmente amostra a amostra, contém harmônicos **acima** do que a
taxa de amostragem consegue representar direito — e esses harmônicos
"dobram" de volta pra dentro da faixa audível como frequências erradas,
inarmônicas, que não deveriam estar ali (é o *aliasing*, o "chiado
digital" que fica mais audível quanto mais aguda a nota). O PolyBLEP
suaviza só o **instante exato** da quebra com uma curvinha polinomial
curta, sem mudar o resto da forma de onda — o suficiente pra tirar boa
parte desse chiado sem amaciar o timbre.

**O sub-oscilador** não é um segundo oscilador independente — é um
circuito simples (um "flip-flop": alterna entre dois estados) que
**conta ciclos** da fase principal e vira de +1 pra −1 a cada volta
completa (ou a cada duas, com `SUB2`) — daí ele nunca desafina em
relação ao `OSC` principal, mesmo durante um hard sync (que reinicia a
contagem junto).

**Hard sync**, na entrada `SYNC`: cada borda de subida do sinal externo
força a fase do `OSC` de volta a 0 **imediatamente**, não importa onde
ela estava. Se a fonte do `SYNC` for mais lenta que o `FREQ` do `OSC`,
o oscilador começa cada ciclo do zero mas é forçado a recomeçar antes
de terminar o próprio ciclo natural — o resultado é uma forma de onda
que muda de "cara" (mais ou menos harmônicos) conforme a razão entre as
duas frequências, o efeito clássico de sync (o `FREQ` deixa de soar
como afinação e passa a soar como **timbre**, um "varredor de
formante").

**FM linear, e por que "linear" importa:** a maioria dos sintetizadores
digitais faz FM **de fase** (desloca onde o acumulador está na volta).
Aqui a modulação é **linear em Hz**: o valor da entrada `FM` é somado
**diretamente à frequência instantânea**, em Hertz — e em intensidade
alta, essa soma pode ficar **negativa**, fazendo a fase andar **pra
trás** por um instante (through-zero, "atravessando o zero"). O efeito
sonoro é que a afinação percebida do `OSC` continua estável mesmo com
FM forte — só o timbre fica mais metálico — enquanto FM de fase costuma
"puxar" a afinação percebida junto com a intensidade.

## Os jacks, um a um

### Entradas

- **`1V/O`** (controle, altura) — a nota. Cada volt dobra a frequência;
  soma ao knob `FREQ`. **Plugue aqui:** `QUANTIZER.pitch`,
  `SEQUENCE.pitch`, `TURING.cv`, `SIGNAL-IN.pitch`. Sem cabo, a afinação
  é só o `FREQ`.
- **`FM`** (áudio) — modulação de frequência linear. **Plugue aqui:** a
  saída de **outro `OSC`** (ou qualquer voz) — em taxa de áudio vira
  timbre metálico; um LFO lento aqui é vibrato. A intensidade é o knob
  `FM`.
- **`PWM`** (controle) — soma à largura de pulso `PW`. Afeta só a saída
  `PLS`. **Plugue aqui:** um `FUNCTION` lento (o pulso "respira"), um
  `ENVELOPE`.
- **`SYNC`** (controle, disparo) — hard sync. **Plugue aqui:** a saída de
  um oscilador ou de um `CLOCK`; cada borda de subida reinicia a fase do
  `OSC`. **Só funciona com a chave `SYNC` ligada.**

### Saídas

Todas são **áudio**, tocam ao mesmo tempo, e vão para o `MIXER` (direto
ou via `FILTER`/`ENVELOPE`):

- **`SIN`** — senoide limpa. Boa como sub-grave ou pra FM.
- **`TRI`** — triângulo. Suave, poucos harmônicos.
- **`SAW`** — dente-de-serra. A forma clássica do subtrativo — todos os
  harmônicos, brilhante.
- **`PLS`** — pulso; a largura é o knob `PW`. Nasal, oco.
- **`SUB`** — quadrada uma ou duas oitavas abaixo (chave `SUB2`).
  Some com uma das outras no `MIXER` pra encorpar o grave.

## Os controles, um a um

**FREQ** (8–8000 Hz) — a frequência base / ponto de partida da altura.

**FINE** (±100 cents) — afinação fina, um semitom pra cada lado. Pra casar
com outra voz, ou desafinar de propósito.

**PW** (0,02–0,98) — largura de pulso: de cada ciclo, **que fração do
tempo** a onda `PLS` fica no nível alto antes de descer pro baixo. Em
0,5, metade do tempo em cada nível — onda quadrada, harmônicos ímpares
só. Empurrando pra qualquer extremo, o tempo "em cima" fica bem curto
(ou bem longo) — o trem vira uma sucessão de picos finos, com um
espectro de harmônicos diferente (mais denso, mais nasal/fino) porque
a proporção alto/baixo muda o "peso" relativo de cada harmônico.
Modular `PW` com um LFO lento (a entrada `PWM`) faz o timbre respirar
sem tocar na afinação — é o efeito clássico de PWM.

**FM** (0–1) — quanto da entrada `FM` modula a frequência. Alto + outro
`OSC` na entrada = timbre metálico e afinado.

**DRIFT** (0–1) — passeio lento na afinação (no máximo meio semitom). Em 0
é determinístico; um toque tira o "morto" de um seno parado.

**SUB2** (chave) — sub uma oitava abaixo (desligado) ou duas (ligado).

**SYNC** (chave) — liga a entrada `SYNC`. Um oscilador lento no `SYNC`
"trava" a afinação do `OSC` na dele e transforma o `FREQ` num varredor
de formante.

**PROX** (0–1) — mistura cada saída com uma versão abafada dela mesma —
uma "profundidade" / escurecimento sem gastar um `FILTER`. Em 0 é a
saída crua.

## Como cabear

**Autônomo** — `FREQ` fixo, nada conectado, `SAW` → `MIXER`: um drone
afinado. Um toque de `DRIFT` o deixa vivo.

**Performance** — mexa `FREQ` e `PW` ao vivo; um LFO no `SYNC` (chave
ligada) varre o formante; `FINE` afina contra outra voz.

**Híbrido** — a cadeia melódica canônica:

```
SEQUENCE → QUANTIZER → OSC (1V/O)
OSC (SAW) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)
CLOCK (euclid) → ENVELOPE (gate)
ENVELOPE (env) → FILTER (cutoff)          (o filtro abre com a nota)
```

## Potencializar

- **Coro por batimento:** dois `OSC`, `FINE` em ±5 cents, as duas `SAW`
  em dois canais do `MIXER` — o batimento engrossa.
- **FM generativa:** `OSC` A no `FM` do `OSC` B, com a altura dos dois
  vindo do mesmo `QUANTIZER` — o timbre metálico segue a melodia.
- **Trava de sync como gesto:** um `OSC` lento no `SYNC` de outro; um
  `FUNCTION` bem devagar no `FREQ` do principal — o formante varre em
  ciclos.
- **PWM viva:** `FUNCTION` no `PWM` + a `PLS` no filtro — o clássico
  "chorus" de largura de pulso de um oscilador só.

## Se você conhece o Eurorack

Faz o papel de um **VCO analógico** (Doepfer A-110, Make Noise DPO,
Intellijel Dixie). As diferenças:

- as **cinco formas em jacks separados** ao mesmo tempo — você soma no
  `MIXER`, não escolhe num knob;
- **FM linear through-zero de verdade** (Buchla 259), não FM de fase;
- **sub-oscilador embutido** (Roland Juno), sempre travado na fase;
- **`drift`** — a afinação respira sozinha, de forma reprodutível.
