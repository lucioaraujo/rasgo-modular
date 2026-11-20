## O que é
PREDATOR es un pequeño ecosistema: presas que crecen solas y depredadores que crecen comiendo. Las dos poblaciones suben y bajan en ciclo, el depredador siempre un poco detrás, y salen como CV en PREY y PRED.

## Como pensar nele
Son las ecuaciones de Lotka y Volterra, de los años 1920, la misma física del ecosistema del Aquorbium. Cuando hay mucha presa, el depredador crece; cuando hay muchos depredadores, la presa cae; entonces el depredador pasa hambre, y todo vuelve a empezar. Las capturas dan pulsos en CTCH, y el pico de la presa, la huida, da un pulso en FLEE. CANON repite la línea de la presa con un retraso que se encoge cuando el depredador se acerca: un canon que se aprieta hasta el stretto.

## Controles
- PERIOD: la duración de un ciclo.
- SIZE: el tamaño de la órbita.
- HUNT: cuánto caza el depredador.
- WILD: el ruido del ambiente.
- DELAY: el retraso máximo del canon.
Entradas: CTCH es una captura venida de fuera, RATE mueve la velocidad en voltios por octava. Salidas: PREY y PRED son las poblaciones, CTCH y FLEE los pulsos de la caza y la huida, CANON el canon de la presa.

## Experimente
1. Conecte PREY al corte de un FILTER y PRED al nivel de un VCA: el timbre y la dinámica se cazan.
2. Conecte CTCH a un DRUM con HUNT alto: las capturas se vuelven golpes en los momentos de abundancia.
3. Conecte PREY y CANON a las alturas de dos voces: la segunda imita a la primera cada vez más cerca.
4. Suba WILD: los ciclos dejan de ser iguales, como en un ecosistema real.
