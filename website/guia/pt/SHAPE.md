## O que é
O SHAPE enriquece um som distorcendo-o de formas controladas. Ele pega uma onda simples, como um seno, e a dobra, satura ou multiplica por outra, criando harmônicos novos. É uma técnica ligada a sintetizadores da costa oeste dos Estados Unidos, que preferiam acrescentar harmônicos a cortá-los com filtros.

## Como pensar nele
O filtro tira harmônicos de um som rico; o SHAPE faz o caminho inverso, a partir de um som pobre. O gesto principal é FOLD: quando o sinal passa de um limite, ele é dobrado de volta, e cada dobra acrescenta brilho. Com um envelope na entrada FCV, a nota abre o timbre no ataque e fecha depois. RING multiplica o som pelo que entra em MOD, o que dá timbres de sino.

## Controles
- RING: mistura o som com o produto dele pelo sinal da entrada MOD. No máximo, você ouve só o produto, metálico.
- FOLD: a quantidade de dobra. Quanto mais, mais harmônicos agudos.
- SYM: desloca o centro da dobra e acrescenta harmônicos pares, deixando o som mais anasalado.
- WRAP: troca parte da dobra por um corte seco que reaparece do outro lado, mais brutal.
- SAT: arredonda os picos depois da dobra, amaciando o resultado.
- LVL: o volume de saída.
- DRIFT: uma oscilação lenta na quantidade de dobra.
Entradas: IN, o som; MOD, o segundo sinal para o RING; FCV, para modular a dobra.

## Experimente
1. Ligue a saída SIN de um OSC na entrada IN, e OUT numa entrada do MIXER. Você ouve um seno limpo.
2. Suba FOLD devagar e ouça o seno ganhar brilho, dobra por dobra.
3. Mova SYM para um lado: o som fica mais anasalado.
4. Volte FOLD a zero, ligue a saída SAW de outro OSC em MOD e suba RING. Afine os dois osciladores em notas distantes e o som vira sino.
