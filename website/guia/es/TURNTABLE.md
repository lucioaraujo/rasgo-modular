## O que é
TURNTABLE toca una grabación como si estuviera en un disco de vinilo. Graba un tramo, como SAMPLER, pero la lectura imita un plato de verdad: el disco tiene peso, tarda en tomar velocidad, frena poco a poco y se puede empujar hacia adelante y hacia atrás, como en el scratch de un DJ.

## Como pensar nele
SAMPLER salta de un punto a otro al instante. TURNTABLE tiene inercia, y eso es lo que da el sonido del vinilo: la afinación se desliza cuando el disco acelera o frena. La entrada SCR es la mano sobre el disco: una onda lenta ahí hace que el disco vaya y vuelva a ritmo. La entrada BRK apaga el motor, y el sonido baja hasta detenerse, el efecto conocido como tape stop. No hay sincronía automática de tempo: acertar el ritmo es parte del gesto.

## Controles
- SPEED: la velocidad que el motor intenta alcanzar, de la mitad al doble de la original. Negativo hace girar el disco hacia atrás.
- TORQ: la fuerza del motor, es decir, qué tan rápido el disco llega a su velocidad. Bajo, el arranque hace vacilar la afinación largo rato; al mínimo, el disco solo se mueve con la mano, por la entrada SCR.
- FRIC: el rozamiento. Decide qué tan rápido se detiene el disco al frenar y cuánto vuelve solo al ritmo después de un scratch.
- GRAB: la firmeza de la mano, cuánto mueve el disco la señal en SCR.
- START: dónde cae la aguja con cada pulso en TRIG.
- WEAR: el desgaste del disco: chasquidos y pequeñas irregularidades de giro.
- LOOP: apagado, el disco se acaba y se detiene; encendido, la lectura da la vuelta y vuelve a empezar.
Entradas: TRIG, para volver a poner la aguja; IN, el sonido a grabar; REC, la señal que graba mientras está encendida; SCR, la mano sobre el disco; BRK, el freno. Salida: OUT.

## Experimente
1. Grabe una frase como en SAMPLER: la voz en IN y la salida DIV de un LOGIC, con DIV en 16, en REC. Conecte OUT al MIXER y encienda LOOP.
2. Conecte la salida BI de un FUNCTION a unos 2 Hz a SCR y suba GRAB hasta el centro. El disco va y viene, en un scratch.
3. Quite el cable de SCR. Conecte la salida GATE de un DECISION con BIAS bajo a BRK. De vez en cuando, el disco frena y vuelve a girar.
