## O que é
VOLTA es un bucle de pasos que varía sin perderse. Guarda una secuencia base y la transforma por encima; cuando usted vuelve el control, la secuencia original vuelve exactamente como era.

## Como pensar nele
El TURING reescribe su propio bucle, y lo que cambia se pierde. VOLTA nunca reescribe la base: cada paso tiene su turno de cambiar, siempre en el mismo orden, y por eso se puede ir y volver. Solo un pulso en NEW cambia la base por una nueva.

## Controles
- RATE: el tempo del reloj interno, cuando no hay CLK.
- LEN: cuántos pasos tiene el bucle.
- MUT: cuántos pasos cambian a su valor alternativo.
- ERODE: cuántos pasos callan.
- MEM: los pasos protegidos, que no cambian ni callan.
- RANGE: la amplitud de la CV.
- STEPS: en cuántos escalones se cuantiza la CV.
- OFST: un desplazamiento sumado a la CV.
- SEED: la semilla del bucle y del orden de los cambios.
Entradas: CLK, el reloj; RST, vuelve al primer paso; NEW, una base nueva; MUT y ERO, señales sumadas a los controles. Salidas: CV y CV2, las alturas; PULSE, los gates; POS, la posición en el bucle.

## Experimente
1. Conecte el CLK del CLOCK al CLK de VOLTA, la CV a un QUANTIZER y de ahí a un OSC, y PULSE a un ENVELOPE.
2. Suba MUT despacio: la melodía varía. Vuelva MUT a cero: vuelve entera.
3. Conecte un FUNCTION lento a MUT: la melodía se aleja y vuelve sola.
