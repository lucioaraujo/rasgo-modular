## O que é
TRIGSEQ genera ritmos de batería en cuatro líneas a la vez, pensadas para bombo, caja, hi-hat y una percusión extra. Usted no marca los golpes uno por uno: elige un estilo, decide la densidad de cada línea, y él crea el patrón. Al cargarse, ya está tocando.

## Como pensar nele
Cuatro CLOCK, uno por instrumento, generarían líneas que no conversan entre sí. En TRIGSEQ las cuatro nacen del mismo estilo, y por eso encajan como una batería tocada por una persona. El control MAP recorre cuatro caracteres: recto, como en el rock y el house; quebrado, como en el breakbeat; con swing, como en el hip-hop; y escaso, como en el dub. Girado despacio, el groove cambia de personalidad sin perder el pulso.

## Controles
- LEN: cuántos de los 16 pasos entran en el ciclo.
- RATE: el reloj interno, usado cuando no llega nada a CLK.
- MAP: el estilo, del recto al escaso.
- DNS1, DNS2, DNS3, DNS4: la densidad de cada línea. Más alto, más golpes.
- SWING: retrasa un paso de cada dos, dando balanceo.
- CHAOS: la probabilidad de que aparezcan golpes fantasma o falten golpes previstos, lo que hace el ritmo menos mecánico.
- RATCH: la probabilidad de que un golpe se vuelva una ráfaga rápida de repeticiones, un redoble.
- FILL: cuánto aumenta las densidades la entrada FILL, para los remates.
- DRIFT: deja que el estilo y las densidades varíen poco a poco, y el groove evoluciona solo.
Entradas: CLK, el pulso; RST, para volver al principio; FILL, una señal que dispara el remate; MAP, para modular el estilo. Salidas: T1 a T4, las cuatro líneas; ACC, los acentos; ANY, un pulso cada vez que suena cualquier línea.

## Experimente
1. Conecte la salida CLK del CLOCK a la entrada CLK de TRIGSEQ.
2. Ponga tres DRUM en el rack. Conecte T1, T2 y T3 a la entrada GATE de cada uno, y sus salidas OUT al MIXER.
3. Ajuste TONE y DECAY de cada DRUM para que suenen como bombo, caja y hi-hat.
4. Gire MAP despacio y escuche el groove cambiar de estilo. Después juegue con las densidades.
