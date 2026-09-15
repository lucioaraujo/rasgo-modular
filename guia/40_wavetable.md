# WAVETABLE — oscilador de tabela

**Família:** SOURCE · **Módulo 40**
**Essência:** um oscilador cujo timbre é uma **forma varrida** — um knob
(`POS`) passeia por 16 quadros, de suave a espectral. E captura um ciclo
ao vivo do que o instrumento ouve.
**Dossiê técnico:** [`../dossies/40_wavetable.md`](../dossies/40_wavetable.md)
· **Fonte:** `src/dsp/Wavetable.hpp`

---

## A ideia

`OSC → FILTER` é síntese subtrativa — você tira harmônicos de uma serra.
Uma paleta inteira (pads que evoluem, leads digitais, o "digital que
soa analógico") vive de **varrer uma forma de onda no tempo**, não de
filtrar. O `WAVETABLE` põe esse eixo no patch: entra 1 V/oct, sai áudio,
e `POS` — knob, LFO, envelope, `DRIFT` — move a forma.

A captura fecha o círculo com o `SIGNAL-IN`: o instrumento pode
"amostrar um ciclo" do que ouve e usá-lo como forma de onda.

## Por dentro

**O que é um "quadro" (frame) de wavetable:** é **um único ciclo** de
uma forma de onda, gravado como uma tabela de números — o oscilador só
percorre essa tabela em loop, repetindo o mesmo ciclo exatamente
(exatamente o que um `OSC` faz com uma fórmula matemática de serra ou
triângulo, só que aqui a forma vem de uma tabela em vez de uma
fórmula). Ter **16** quadros diferentes guardados, e poder **misturar
suavemente** entre os vizinhos, é o que abre o eixo de `POS`: em vez de
um timbre fixo, você tem uma **paisagem** de 16 timbres que você
atravessa continuamente, do quadro 0 (serra, todos os harmônicos) ao 15
(seno, só a fundamental), passando por formas intermediárias
(quadrada, formante).

**Por que existem 10 versões de cada quadro (mip-maps), e não só
uma:** uma tabela com muitos harmônicos, tocada numa nota **grave**, é
segura (todos os harmônicos cabem abaixo do limite que a taxa de
amostragem consegue representar). A mesma tabela, tocada numa nota
**aguda**, empurra harmônicos altos pra **além** desse limite — e eles
voltam como *aliasing* (a mesma explicação do "Por dentro" do `OSC`,
#18). A solução: guardar **várias versões** do mesmo quadro, cada uma
com menos harmônicos (uma versão "mais limpa", pré-filtrada), e trocar
automaticamente pra versão mais segura conforme a nota fica mais aguda
— por isso "banda limitada por quadro" e "o antialiasing acompanha a
afinação": você nunca escolhe isso na mão, o módulo troca de mip-map
sozinho pela nota que está tocando.

`WARP` não troca de quadro — ele distorce **onde dentro do ciclo** a
tabela é lida (a mesma família de ideia da distorção de fase do `PLL`,
#37, e do Casio CZ histórico): em vez de percorrer o quadro em
velocidade constante do início ao fim, a leitura acelera em certos
trechos e desacelera em outros, o que adiciona harmônicos novos e um
"sotaque" digital característico, sem trocar a forma-base escolhida por
`POS`.

## Os jacks, um a um

### Entradas

- **`1V/O`** (controle, altura) — a nota. **Plugue aqui:**
  `QUANTIZER.pitch`, `SEQUENCE.pitch`, `TURING.cv`.
- **`POS`** (controle) — soma ao knob `POS`. **É a entrada principal do
  módulo.** **Plugue aqui:** um `FUNCTION` lento (o timbre "respira"
  entre as ondas), um `ENVELOPE` (o timbre abre com a nota), `DRIFT`,
  `NOISE.smooth`.
- **`FM`** (áudio) — modulação de frequência linear. **Plugue aqui:**
  outra voz ou um LFO. Intensidade no knob `FM`.
- **`CAP`** (áudio) — o sinal cujo ciclo você quer **capturar**.
  **Plugue aqui:** `SIGNAL-IN`, outra voz, um `SAMPLER`.
- **`GRAB`** (controle, disparo) — dispara a captura: 1024 amostras de
  `CAP` viram o quadro do topo de `POS`. **Plugue aqui:** um
  `CLOCK`/`TRIGSEQ` (captura ritmada), ou acione na mão.

### Saída

- **`OUT`** (áudio) — a voz. Vai ao `MIXER`, quase sempre via
  `FILTER`/`ENVELOPE`.

## Os controles, um a um

**FREQ** (8–8000 Hz) — a frequência base / ponto de partida da altura.

**FINE** (±100 cents) — afinação fina.

**POS** (0–1) — a posição na tabela. 0 = primeiro quadro (serra); 1 =
último (seno). A entrada `POS` soma aqui — e é onde a vida do módulo
acontece.

**WARP** (0–1) — distorção de fase da leitura do ciclo. Adiciona
harmônicos e um "sotaque" digital sem mudar de quadro.

**FM** (fm_amount, 0–1) — quantidade de FM linear.

**DRIFT** (0–1) — passeio lento na afinação. Em 0 (e sem captura), a
saída é determinística.

## Como cabear

**Autônomo** — `FREQ` fixo, um `FUNCTION` bem lento no `POS`,
`OUT → MIXER`: um drone que muda de timbre continuamente.

**Pad que evolui:**
```
SEQUENCE → QUANTIZER → WAVETABLE (1V/O)
FUNCTION (lento) → WAVETABLE (POS)
WAVETABLE (OUT) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)
```

**Captura:**
```
SIGNAL-IN (L) → WAVETABLE (CAP)
CLOCK (euclid) → WAVETABLE (GRAB)     (a cada pulso, um ciclo novo do que entra)
```

## Potencializar

- **Timbre com dinâmica:** `ENVELOPE.env → POS` — cada nota "abre"
  espectralmente no ataque e recolhe na cauda, como um filtro mas mais
  estranho.
- **Tabela viva:** `DRIFT` no `POS` — o timbre passeia por conta própria
  ao longo de minutos.
- **Wavetable do próprio patch:** `GRAB` disparado por `SCOPE.onset` de
  outra voz — o `WAVETABLE` "rouba" o timbre do que estiver tocando.
- **`WARP` + `POS`:** os dois em LFOs de velocidades diferentes — o
  timbre fica em movimento constante e nunca repete exatamente.

## Se você conhece o Eurorack

Faz o papel de um oscilador de wavetable (Piston Honda, Intellijel
Shapeshifter, Braids na faixa wavetable). A diferença: os 16 quadros são
**gerados por receita** (não um arquivo de dados), o `WARP` é distorção
de fase estilo Casio CZ, e a **captura ao vivo** (`CAP`/`GRAB`) amarra
o oscilador ao que o instrumento ouve.
