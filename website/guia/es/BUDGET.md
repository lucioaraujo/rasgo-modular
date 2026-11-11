## O que é
BUDGET da a las voces un presupuesto de eventos. Cuatro gates entran y salen, pero por ciclo solo pasan TETO eventos en total. Cuando el presupuesto se acaba, los gates siguientes quedan fuera hasta que el ciclo vuelva a empezar.

## Como pensar nele
Densidad no es calidad. BUDGET obliga al patch a elegir: con poco presupuesto, cada nota vale más. Hay dos economías. Competencia: quien llega primero se lo lleva. Cooperación: cada voz que toca tiene su parte, y solo pasa de ella si las otras todavía no gastaron la suya.

## Controles
- TETO: cuántos eventos caben en cada ciclo, de 1 a 32.
- COOP: apagado, las voces compiten; encendido, cooperan.
- CICLO: la duración del ciclo, de 0,25 a 16 s, cuando no hay nada conectado en CLK.
Entradas: G1 a G4 reciben los gates de las voces, CLK reinicia el ciclo en cada pulso. Salidas: O1 a O4 son los gates que pasaron, SALDO lo que queda del presupuesto, FIM un pulso cuando se acaba.

## Experimente
1. Conecte los gates de dos voces a G1 y G2, y O1 y O2 a sus envolventes.
2. Con TETO en 6, escuche a las voces disputarse el ciclo.
3. Encienda COOP: las dos pasan a repartirse el presupuesto.
4. Conecte SALDO al corte de un FILTER: el sonido se oscurece a medida que se gasta el presupuesto.
