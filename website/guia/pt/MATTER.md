## O que é
O MATTER simula um objeto que vibra quando é tocado: uma corda, um sino, uma placa de metal, um tubo. Por dentro, ele tem 24 ressonâncias que soam juntas, como as de um objeto real. Você dá um golpe e ouve o objeto responder, com a afinação e o material que escolher.

## Como pensar nele
Um oscilador filtrado soa como eletrônica; o MATTER soa como algo sendo tocado. Você não monta o timbre harmônico por harmônico: escolhe o material em STRC e onde o objeto é golpeado em POS, e depois o excita. O golpe pode vir de dentro, pela entrada HIT, ou de fora, de qualquer som ligado em IN. Um CLOCK batendo em HIT já é percussão afinada.

## Controles
- FREQ: a afinação do objeto, de 20 a 8000 Hz.
- STRC: o material. No começo, uma corda, de som afinado e doce; no fim, sino ou metal, com ressonâncias desencontradas.
- BRITE: quantas ressonâncias agudas soam. Baixo é abafado; alto é brilhante.
- DAMP: quanto tempo o objeto continua soando depois do golpe. Baixo é um toque curto; alto é um anel longo.
- POS: o ponto onde o objeto é golpeado. Mudando o ponto, algumas ressonâncias somem e outras aparecem, como acontece num instrumento de verdade.
- EXCIT: quanto ruído entra no golpe. Baixo é um toque limpo; alto tem mais ar no ataque.
- MIX: a mistura entre o golpe cru e a ressonância do objeto. No máximo, só o corpo do objeto.
Entradas: IN, para fazer ressoar um som de fora; HIT, um pulso que golpeia; 1V/O, para a afinação; STR, para mudar o material.

## Experimente
1. Ligue OUT numa entrada do MIXER, e a saída CLK do CLOCK na entrada HIT. Você ouve batidas afinadas.
2. Gire STRC de ponta a ponta. A batida passa de corda a sino.
3. Suba DAMP para os golpes se emendarem num anel contínuo.
4. Mova POS devagar e ouça o timbre mudar conforme o ponto do golpe.
