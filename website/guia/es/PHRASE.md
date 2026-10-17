## O que é
PHRASE propone un motivo y lo toca cuando recibe pulsos en CLK. La idea vuelve con cada frase, con cambios de dirección, pausas y acentos que usted regula. Sus salidas controlan otras voces; conecte un oscilador y una envolvente para escucharlo.

## Como pensar nele
Use CLOCK para un pulso regular o los ataques de LISTEN para responder a otro sonido. SEED identifica el motivo. VAR transforma sus pasadas; en cero es más fácil reconocer el dibujo que vuelve. La entrada PITCH ofrece un apoyo, capturado al principio de cada frase. Para elegir una escala, pase la salida PITCH por QUANTIZER con RANGE=1. PHRASE no incluye por sí solo un repertorio de estilos o tradiciones musicales.

## Controles
- LEN: longitud de la frase en eventos, de 2 a 16 pulsos. No es una duración fija en segundos.
- SPAN: ámbito del dibujo, de 1 a 36 semitonos, antes de la cuantización.
- DIR: inclina el dibujo hacia abajo o arriba. En el centro conserva la dirección del motivo de base.
- VAR: cuánto cambia cada pasada. Empiece en cero y súbalo escuchando qué sigue siendo reconocible.
- SPACE: probabilidad de pausas en pasos intermedios. El inicio y el cierre se conservan.
- CAD: fuerza del regreso al apoyo al final de la frase.
- GATE: duración de la articulación, de 5 a 400 ms. Conecte GATE a la envolvente de la voz para oír notas articuladas.
- SEED: identidad numérica del motivo, de 1 a 65535. Cambiarla recrea el dibujo y reinicia; VARIA no mueve este control.
Entradas: PITCH, apoyo a 1 V por octava; CLK, un paso por flanco ascendente; RST, reinicio; STIM, intensidad de 0 a 1; RET, pedido de cierre en el próximo paso. Salidas: PITCH, altura mantenida; GATE, articulación; ACC, acento de 0 a 1; END, pulso de 5 ms al cierre; PHASE, posición de 0 a 1 en la frase. Sin cable en STIM, la intensidad es 1. Al reabrir el patch, la frase empieza de nuevo, sin retomar la posición exacta de la ejecución anterior.

## Experimente
1. Desactive VARIA. Conecte CLOCK.EUC a PHRASE.CLK, PHRASE.PITCH a QUANTIZER.CV (RANGE=1), y QUANTIZER.PITCH a OSC.PITCH. Elija una escala en QUANTIZER.
2. Conecte OSC.SAW a ENVELOPE.IN, PHRASE.GATE a ENVELOPE.GATE y ENVELOPE.OUT al MIXER. Empiece con VAR=0 y SPACE=0. Escuche la idea repetirse; cambie DIR y SPAN por separado.
3. Coloque un VCA entre la voz y el MIXER: ENVELOPE.OUT a VCA.IN1, VCA.O1 al MIXER, LVL1=0 y CV1=1. Conecte PHRASE.ACC a VCA.CV1 para oír los acentos. Suba VAR y luego SPACE.
4. Sustituya CLOCK por LISTEN.L-HIT en CLK y conecte LISTEN.LOW a STIM. El sonido escuchado llama ahora a las notas y cambia su intensidad. Un pulso en RET pide un cierre en el próximo CLK; END puede llamar a una segunda voz.
