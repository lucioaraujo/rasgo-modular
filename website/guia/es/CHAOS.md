## O que é
CHAOS genera un movimiento imprevisible a partir de un sistema físico simulado: algo así como una bola que rueda entre dos valles, a veces asentándose en uno, a veces saltando al otro. No se puede prever cuándo va a saltar, pero con la misma semilla todo se repite igual.

## Como pensar nele
Otros módulos sortean valores, como TURING y DECISION, o pasean despacio, como DRIFT. CHAOS tiene un comportamiento propio: pasa un rato rondando una región y de pronto cambia a otra. En un filtro, eso da un timbre que duda y luego salta; en una melodía, frases que se quedan en un registro y de pronto se mudan. A velocidades altas, él mismo se vuelve una textura sonora.

## Controles
- RATE: la velocidad del movimiento, de 0,02 a 400 ciclos por segundo. Lento para mover controles; rápido para escucharlo directamente.
- DRIVE: la fuerza que empuja la bola entre los valles. Más alto, saltos más grandes y más frecuentes.
- DAMP: el rozamiento. Alto, la bola se asienta en un valle y casi se detiene; bajo, oscila mucho y salta de un lado al otro.
- FRZ: congela el movimiento en el valor actual.
Entradas: RSD, un pulso que da un empujón nuevo; RTM, para modular la velocidad. Salida: OUT.

## Experimente
1. Arme una voz: la SAW de un OSC a la entrada IN de un FILTER, LO del FILTER al MIXER.
2. Conecte OUT de CHAOS a la entrada FC del FILTER. Ponga RATE en torno a 0,3, DRIVE alto y DAMP bajo.
3. Escuche el timbre quedarse un rato en una región y saltar a otra.
4. Conecte también OUT de CHAOS a la entrada CV de un QUANTIZER, y su PTCH a 1V/O del OSC. Ahora la melodía cambia de región junto con el timbre.
