# Como cabear — o essencial

Antes das páginas de módulo: o que os cabos fazem, quais jacks aceitam o
quê, e como o som chega até você.

---

## 1. Cabo = de uma saída para uma entrada

No painel, cada jack é **saída** (embaixo do módulo, o sinal *sai*) ou
**entrada** (o sinal *entra*). Você puxa o cabo de uma saída até uma
entrada — nunca saída→saída nem entrada→entrada. Quando você começa a
puxar, os jacks que aceitam aquele cabo acendem um **halo**.

**Uma saída pode ir para vários lugares.** Não existe "múltiplo" a
puxar: puxe da mesma saída quantas vezes quiser — para 2, 5, 10
destinos, sem perda de nível. (O módulo `MULT` serve pra quando você
quer que cada cópia saia com um nível ou uma polaridade **diferente** —
o LFO indo *cheio* pro filtro e *pela metade e invertido* pro volume.)

**Por que uma saída pode ir a vários lugares, de graça:** o motor lê o
valor daquela saída uma vez por bloco de áudio e só **copia** esse
número pra cada cabo que sai dela — copiar um número não custa nada e
não "gasta" o sinal, então 10 cópias soam tão fortes quanto 1.

**Por que uma entrada só aceita um cabo (e não soma sozinha):** uma
entrada, por dentro, é só uma variável que guarda **um** valor por vez —
"qual é o sinal que chega aqui agora". Se dois cabos pudessem chegar
na mesma entrada, o motor teria que decidir sozinho **como** misturá-los
(somar? o segundo substitui o primeiro? a média?) — e cada escolha serve
pra um caso diferente. Em vez de adivinhar, o Rasgo obriga você a
**decidir explicitamente**: você escolhe o módulo que soma (`MIXER` pra
áudio, `CONTROL`/`ABACUS` pra CV) e cabeia o resultado dele, já somado,
pra entrada final. É mais um cabo no meio, mas nunca uma surpresa.
Puxar um segundo cabo para uma entrada já ocupada **substitui** o
primeiro (não soma, não da erro — só troca). Para *somar* dois sinais
num mesmo ponto, some antes:

| Somar… | Use… |
|---|---|
| áudio (várias vozes num destino) | um canal livre do **`MIXER`** · **`VCA`** (saída `sum`) · **`VCA4`** (saída `mix`) |
| CV (dois envelopes, LFO + offset…) | **`CONTROL`** (saída `sum`) · **`ABACUS`** (`a` + `b`) · **`MULT`** com os offsets |
| áudio numa grade (tudo com tudo) | **`MATRIX`** (soma por coluna) |

Botão direito num jack tira o cabo. `[espaço]` rompe e reata todos os
cabos de uma vez.

### Retroalimentar um módulo (feedback)

Você **pode** puxar a saída de um módulo de volta para uma entrada
dele mesmo, ou de algo mais atrás na cadeia. Isso cria um **laço**: A
alimenta B, B alimenta A de volta. Num circuito de verdade, um laço
desses pode ser um problema (quem calcula primeiro, se cada um depende
do outro?). No Rasgo o painel **percebe** que o cabo fecharia um laço e
marca ele como **feedback** automaticamente — não precisa avisar nada,
não existe um "modo feedback" pra ligar.

**Por que o laço não trava:** o motor calcula o patch inteiro **um
bloco de áudio por vez** (a cada vez, um punhado de amostras — o
suficiente pra durar ~1,3 ms a 48 kHz). Um cabo comum lê o valor que o
módulo de origem *acabou de calcular*, no mesmo bloco. Um cabo de
**feedback** faz diferente: ele lê o valor que o módulo de origem
calculou no **bloco anterior** — um instante atrasado, não o valor
"ainda por vir" deste bloco. Isso quebra a dependência circular (B
nunca precisa esperar o A "deste bloco" que ainda nem existe) sem
precisar resolver nenhuma equação — e o atraso de ~1,3 ms é curto
o bastante pra não soar como um eco perceptível, só como **ressonância**:
o sinal reforça a si mesmo a cada volta, como empurrar um balanço no
ritmo certo.

- `SPACE.wet → SPACE.feedback_mod` — a cauda se realimenta e cresce.
- `FILTER.out → MIXER.ch2`, `MIXER.L+R → FILTER.in` — o filtro escuta a
  si mesmo pelo barramento; perto da auto-oscilação vira uma voz.
- `SHAPE.out → SHAPE.mod` — o modelador dobra sobre o próprio resultado.

**Por que o laço não "explode" (cresce pra sempre até estourar):** um
laço de feedback puro, sem limite nenhum, dobraria de volume a cada
volta e o número cresceria sem parar. O Rasgo não deixa: o `MASTER` tem
um limitador que nunca deixa a saída final passar de um teto, e muitos
módulos que aceitam feedback internamente já passam o sinal por uma
curva `tanh` (uma função que sobe quase reta perto de zero mas **achata
suavemente** perto dos extremos — dobrar a entrada não dobra mais a
saída depois de certo ponto). O resultado é um **ciclo-limite**: o laço
cresce até esbarrar nesse teto suave e se estabiliza ali, ressoando
sem crescer mais — não uma explosão nem um corte seco. Comece com o
parâmetro de retorno baixo e suba de ouvido; perto do topo (retorno ≈1)
o teto pode virar o próprio som (um zumbido/nota sustentada que **se
mantém sozinha**, sem nenhuma voz entrando mais — é a auto-oscilação).

**Feedback embutido (sem cabo):** vários módulos já têm um laço interno
num knob — não precisa cabear nada: `SPACE.feedback`, `SWIRL.feedback`,
`LOOPER.feedback`, `MEMORY.feedback`, `PLL.feedback_amount`,
`OPERATOR.feedback`, `SHIFTER.feedback` (barber-pole), e a
`resonance` do `FILTER`/`WASP`/`LPG` (perto de 1 o filtro auto-oscila).

---

## 2. Dois tipos de sinal: áudio e controle

Por dentro, **tudo no Rasgo é o mesmo tipo de número** — uma sequência
de valores entre −1 e 1 (ou perto disso), um por amostra, correndo a
48000 por segundo. Não existe um "tipo CV" separado de um "tipo áudio"
no motor. A diferença entre os dois é **o que você faz com o número**:

- **Áudio** é quando essa sequência de números, tocada direto num
  alto-falante, **é o som** — as oscilações são rápidas o bastante
  (20 a 20 000 vezes por segundo) pra o ouvido ouvir como um tom, não
  como um movimento.
- **Controle (CV, de "control voltage")** é quando essa mesma sequência
  de números não vai pra caixa de som — ela é lida por **outro módulo**,
  que usa o valor pra decidir o que fazer: "que altura tocar", "quão
  aberto deixar o filtro". É a versão elétrica de girar um knob, só que
  automática — em vez da sua mão girando devagar, é um sinal girando
  sozinho, e podendo girar rápido ou devagar, subir e descer, repetir.

O painel mostra **halo duplo** quando você conecta dois jacks do mesmo
tipo (o casamento "certo" — a maioria dos patches usa isso), mas nada
te impede tecnicamente de ligar os dois tipos.

| | **Áudio** | **Controle (CV)** |
|---|---|---|
| O que é | o som em si — algo que você ouviria | uma tensão que comanda outra coisa |
| Velocidade | rápido (20 Hz–20 kHz) | geralmente lento (um envelope, um LFO, um passeio) — mas pode ser rápido |
| Exemplos de saída | `OSC.saw`, `FILTER.out`, `DRUM.out`, `SPACE.wet` | `ENVELOPE.env`, `FUNCTION.out`, `CLOCK.euclid`, `QUANTIZER.pitch`, `NOISE.smooth`, `DRIFT.a` |
| Exemplos de entrada | `FILTER.in`, `SHAPE.in`, `MIXER.ch1` | `OSC.pitch`, `FILTER.cutoff`, `ENVELOPE.gate`, `SPACE.time_mod` |

**Cruzar os tipos é permitido, e às vezes é o objetivo** — porque, de
novo, é o mesmo tipo de número por baixo:

- um **oscilador em taxa de áudio numa entrada de modulação** (`fm`,
  `cutoff`) faz o parâmetro vibrar rápido demais pra ouvir como um
  movimento — o ouvido interpreta isso como **timbre novo**, não como
  "o filtro abrindo e fechando". É assim que FM e modulação de filtro em
  áudio geram harmônicos que não existiam antes.
- **áudio de verdade numa entrada de modulação** (a saída de um `OSC`
  ligada no `mix` de outro módulo, por exemplo) é "modulação por áudio"
  — o segundo som empresta sua forma de onda pra mexer no primeiro,
  em vez de um LFO limpo.
- **CV numa entrada de áudio** (um `ENVELOPE` lento ligado direto num
  `FILTER.in`) normalmente só produz um clique/rampa surda — CV é
  devagar demais pra soar como tom — mas nada trava, e o resultado pode
  ser um efeito válido dependendo da velocidade da CV.

Nesses casos o halo fica simples em vez de duplo (é um lembrete visual
de "isto não é o caminho mais comum", não um bloqueio).

---

## 3. O caminho do som — como chegar até os alto-falantes

**Toda voz tem que chegar ao `MIXER`, e o `MIXER` ao `MASTER`.** O
`MASTER` já está ligado à sua placa de som — você não cabeia a saída
dele em lugar nenhum, isso já está feito.

```
  a sua voz  →  [processadores]  →  MIXER (um canal)  ─┐
  outra voz  →  [processadores]  →  MIXER (outro canal) ┤
                                                        MIXER "L+R" → MASTER "IN" → (alto-falantes)
```

### O `MIXER`

Quatro canais de **áudio**: os jacks `1` `2` `3` `4` embaixo. Puxe a
saída de áudio de qualquer módulo para um canal livre. Por canal:

- **`CH1..4`** (slider) — o nível daquele canal.
- **`PAN`** — esquerda ↔ direita.
- **`MUTE`** — corta o canal.

A saída **`L+R`** é a soma dos quatro. `OUT` (knob) é o nível geral do
mixer. Essa saída `L+R` vai para o `MASTER.IN`.

### O `MASTER`

- **`IN`** — recebe o `L+R` do mixer (ou, se você quiser uma voz só,
  direto uma saída de áudio).
- **`GAIN`** (slider) — o volume final. Nos seeds ele nasce em 75% (−6
  dB); suba/baixe a gosto.
- **`WIDTH`** — largura do estéreo: o quanto o canal esquerdo e o
  direito soam **diferentes** um do outro. Em 0, os dois canais tocam
  exatamente igual (o som fica "no centro", mono). Subindo, a diferença
  entre L e R aumenta — o som parece ocupar mais espaço ao redor de
  você, mais "largo". É útil em especial depois de um `SPACE`/`HALL`
  (que já entregam L/R levemente diferentes) pra exagerar ou reduzir
  essa sensação de espaço.
- **`STANDBY`** (botão do cabeçalho) — muta o `MASTER` com uma rampa
  limpa (o volume desce suavemente em vez de cortar — evita o estalo
  de silenciar um sinal no meio de uma onda).
- **Proteção sempre ligada, em três camadas** (é por isso que o patch
  não estoura mesmo com o gain alto):
  - **bloqueio de DC** — remove qualquer deslocamento constante do
    sinal (uma "maré" que empurra a onda toda pra cima ou pra baixo em
    vez de oscilar em volta de zero — pode vir de certos processos não
    lineares, como o `Fold`). DC não se ouve como tom, mas rouba
    margem do limitador e pode fazer alto-falantes esquentar à toa.
  - **guarda de agudo** — um filtro suave que impede que os agudos
    extremos (ultrapassando o que o ouvido usa, mas que ainda pode
    gerar distorção digital) se acumulem e endureçam o som.
  - **limitador de pico verdadeiro** ("true peak") — um limitador comum
    olha só pro valor de cada amostra; um som digital pode ter picos
    **entre** amostras (quando reconstruído em áudio analógico de
    verdade) que passam despercebidos por essa checagem simples e
    estouram no alto-falante mesmo assim. O limitador do `MASTER`
    reconstrói o sinal em mais detalhe internamente pra pegar esses
    picos escondidos também — por isso o teto é confiável de verdade,
    não só "no papel".
- **`LEVEL`** (saída) — o VU como CV, caso você queira que algo siga o
  volume da saída (por exemplo, um `SCOPE`/visual, ou um parâmetro que
  reage ao quão alto o patch está tocando naquele instante).

**Sem `MIXER` no caminho:** dá pra cabear uma voz direto no `MASTER.IN`.
Funciona, mas você perde os 4 canais, o pan e o mute — use o mixer.

---

## 4. Os tipos de entrada que você mais vai encontrar

### Entrada de áudio — `IN`

O sinal a ser processado. Sai de uma voz (`OSC`, `MATTER`, `NOISE`…) ou
de outro processador. `FILTER.in`, `SHAPE.in`, `SPACE.in`, `LPG.in`.

### Altura — `1V/O` / `PITCH`

Diz a nota. Convenção **1 volt por oitava**: cada volt dobra a
frequência. **Por que essa convenção existe:** o ouvido percebe altura
de forma **exponencial**, não linear — subir uma oitava é sempre
"dobrar a frequência", não "somar um número fixo de Hz" (de 220 Hz pra
440 Hz é uma oitava; de 440 Hz pra 660 Hz **não** é, embora a distância
em Hz seja a mesma). 1 V/oitava resolve isso: **somar** sempre a mesma
quantidade de tensão sempre **multiplica** a frequência pelo mesmo
fator, então uma escala musical inteira vira uma régua reta de tensão
— cada semitom é 1/12 de volt, sempre, em qualquer oitava. É por isso
que essa CV **soma** ao knob de frequência do módulo em vez de
substituí-lo: o knob decide a "oitava base", e a CV desloca dali pra
cima/baixo em semitons. Fontes típicas: `QUANTIZER.pitch`,
`SEQUENCE.pitch`, `TURING.cv`, `SIGNAL-IN.pitch`, um `CHORD`/`HARMONY`
via `CONTROL`.

### Disparo — `GATE` / `TRIG` / `HIT` / `PLK` / `CLK` / `SYNC` / `GRAB`

Espera um **pulso** — "aconteceu algo agora" — mas dois sabores
diferentes, que importam pra quem recebe:

- **`TRIG`** (trigger) é um pulso **curto e de duração irrelevante**: só
  marca o **instante** em que algo acontece. Um `ENVELOPE` disparado por
  um trigger sempre faz o mesmo ataque→decaimento completo, não importa
  quão rápido o trigger passou — como bater um sino: a duração da batida
  não muda a duração do som do sino.
- **`GATE`** carrega **duração** de verdade: fica **alto enquanto durar**
  (uma nota segurada) e desce quando a nota termina. Um `ENVELOPE` com
  estágio de *sustain* precisa disso pra saber **quando soltar** — ele
  segura o nível de sustain enquanto o gate estiver alto, e só entra no
  *release* quando o gate cai. Trocar um `GATE` por um `TRIG` nesse caso
  faz a nota nunca sustentar (dispara e já solta).

Fontes típicas: `CLOCK.euclid` / `CLOCK.clock`, `TRIGSEQ.t1..t4`,
`SEQUENCE.eos`, `DECISION.gate`, `LOGIC`, `SCOPE.onset`.

### Modulação — nomes de knob, ou `FM` / `PWM` / `sweep` / `_mod` / `cutoff` / `amount`

Uma CV que **mexe um parâmetro sozinha**, sem você tocar em nada. O
valor da CV **soma ao knob** — o knob vira o **ponto de partida** e a
CV o move a partir dali, pra cima e pra baixo. Um exemplo concreto: o
`FILTER.cutoff` está em 800 Hz (o knob); um `ENVELOPE` ligado nessa
entrada, oscilando de 0 a +1, faz o corte real do filtro variar entre
800 Hz (quando o envelope está em 0) e algo bem mais agudo (quando o
envelope está no pico) — o knob nunca muda de posição visualmente, mas
o valor **de verdade** usado pelo filtro sobe e desce com a CV. Fontes
típicas: `FUNCTION` (LFO/envelope), `ENVELOPE.env`, `NOISE.smooth`
(deriva orgânica), `DRIFT.a..d` (evolução em minutos), `SH`, `MULT`
(uma fonte espalhada com pesos), `LFO` de qualquer `OSC`.

- **`FM`** é entrada de **áudio**, não de controle lento: um oscilador
  em **taxa de áudio** ligado aqui faz a frequência do módulo vibrar
  rápido demais pra soar como "vibrato" — o ouvido funde isso num
  **timbre novo**, com harmônicos que não existiam antes (é a mesma
  ideia da síntese FM clássica: quanto mais rápido e mais forte o
  modulador, mais harmônicos aparecem).

---

## 5. As fontes de modulação (o que gera CV)

Quando uma página diz "cabeie um LFO aqui" ou "um envelope aqui":

| Você quer… | Use… |
|---|---|
| um LFO (onda lenta que repete) | **`FUNCTION`** em taxa baixa · qualquer `OSC` bem grave · **`SWIRL`** não |
| um envelope (sobe no gate, desce) | **`ENVELOPE`** (saída `env`) · **`FUNCTION`** disparado · **`STAGES`** |
| aleatório em degraus | **`NOISE.sh`** · **`SH`** · **`TURING`** |
| aleatório que desliza | **`NOISE.smooth`** · **`SH`** com `slew` |
| deriva lenta (minutos), o patch evoluindo | **`DRIFT`** (`a`/`b`/`c`/`d`) |
| uma tensão fixa que você ajusta na mão | **`MULT`** ocioso (os `OFFSET`) · **`CONTROL`** com `scale`=0 |
| clock / pulsos no tempo | **`CLOCK`** · **`LOGIC`** · **`TRIGSEQ`** |
| altura de escala | **`QUANTIZER`** (alimentado por `SEQUENCE`/`TURING`/`NOISE.sh`) |

---

## 6. O patch mínimo, comentado

```
CLOCK.euclid   → ENVELOPE.gate     (o pulso que dispara a nota)
QUANTIZER.pitch → OSC.1V/O          (a altura — QUANTIZER alimentado por SEQUENCE ou TURING)
OSC.saw        → FILTER.in          (a voz crua entra no filtro)
FILTER.out     → ENVELOPE.in        (o filtro passa pelo VCA do envelope)
ENVELOPE.out   → MIXER.ch1          (a voz com dinâmica entra no mixer)
ENVELOPE.env   → FILTER.cutoff      (bônus: o filtro abre junto com a nota)
```

E o `MIXER.L+R → MASTER.IN` já vem feito. Sobe o `GAIN` do `MASTER` e
você ouve.

A partir daqui: mais vozes nos canais 2–4 do mixer; um `SPACE` entre o
`FILTER` e o mixer; um `LFO` (`FUNCTION`) no `cutoff` ou no `PW`; um
`DRIFT` em dois ou três parâmetros pra o patch andar sozinho.

---

## 7. Regras rápidas

- Saída → entrada. Sempre.
- Uma saída → muitos destinos (de graça). Uma entrada → **um** cabo (o
  segundo substitui o primeiro; para somar, use `MIXER`/`VCA`/`CONTROL`).
- Áudio para ser ouvido **tem** que chegar num canal do `MIXER` (ou no
  `MASTER.IN`).
- CV numa entrada de parâmetro **soma ao knob**.
- `GATE`/`TRIG` querem um pulso, não um sinal contínuo.
- `1V/O` quer altura (de um `QUANTIZER`, `SEQUENCE`, `TURING`…).
- Saída de volta para trás na cadeia = **feedback** (o painel marca
  sozinho; atraso de 1 bloco, não trava). Muitos módulos já têm um knob
  `feedback` interno.
- O `MASTER` é o fim da linha — a saída dele já vai pra placa de som.
- Nada estoura: o `MASTER` tem limitador e guarda de agudo sempre
  ligados.
