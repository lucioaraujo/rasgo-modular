## O que é
PRISM separa el sonido en colores, como un prisma separa la luz. Son seis bandas, del grave al agudo, cada una retrasada por su propio índice y colocada en un lugar del estéreo. Un golpe se vuelve un arcoíris que se escurre en el tiempo.

## Como pensar nele
En el vidrio, cada color avanza a su propia velocidad. Aquí DISP es el grosor del vidrio y CURVE el sentido: positiva, los agudos llegan después, como en la luz; negativa, los graves llegan después, cosa que el vidrio no hace. FDBK devuelve cada color al prisma, y los ecos se separan más en cada vuelta.

## Controles
- DISP: el retardo máximo entre los colores, hasta 400 ms.
- CURVE: el sentido y la forma de la dispersión, de −1 a 1.
- SPRD: cuánto se reparten los colores en el estéreo.
- FDBK: cuánto de cada color vuelve al prisma.
- MIX: la proporción entre el sonido original y el disperso.
Entradas: IN recibe el sonido, DISP mueve la dispersión por CV. Salidas: OUT es la mezcla, WET solo el sonido disperso.

## Experimente
1. Pase un DRUM por el PRISM con CURVE en 1, DISP en 200 ms y SPRD al máximo: cada golpe se escurre del grave al agudo, de un lado al otro.
2. Ponga CURVE en −1 y compare.
3. Suba FDBK a 0,7, con DISP en 80 ms y MIX en 0,6: ecos de colores.
4. Conecte un LFO lento a DISP: el arcoíris respira.
