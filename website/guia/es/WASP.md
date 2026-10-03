## O que é
WASP es un filtro que distorsiona. Inspirado en un sintetizador de finales de los años setenta conocido por su sonido áspero, corta frecuencias como FILTER, pero ensucia el sonido cuando sube la resonancia. Es el filtro de los bajos agresivos, los solos cortantes y los drones que rechinan.

## Como pensar nele
FILTER es limpio y preciso; WASP es su contrapunto sucio. Tres perillas dosifican la suciedad: DRIVE empuja el sonido hacia la distorsión antes de filtrar, GRIT decide cuánto rechina el filtro al resonar y BIAS desequilibra la distorsión, que se vuelve más zumbante. Con resonancia alta suena solo, y ese sonido también es áspero.

## Controles
- CUT: la frecuencia de corte, de 20 a 24000 Hz.
- RESO: la resonancia. Cerca del máximo, el filtro suena solo.
- MODE: el tipo de filtro, pasando de pasa-bajos a pasa-banda y pasa-altos.
- DRIVE: la ganancia de entrada, que empuja el sonido hacia la distorsión antes de filtrar.
- GRIT: cuánto distorsiona el filtro al resonar. Bajo es casi limpio; alto rechina.
- BIAS: desequilibra la distorsión, añadiendo armónicos pares y un zumbido.
- DRIFT: una oscilación lenta en el corte y la resonancia.
Entradas: IN, el sonido a filtrar; FC, para mover el corte; Q, para la resonancia.

## Experimente
1. Conecte la salida SAW de un OSC a la entrada IN, y OUT a una entrada del MIXER.
2. Suba RESO hasta unos dos tercios y gire CUT: escuche cómo muerde el filtro.
3. Suba GRIT y DRIVE. El sonido se vuelve áspero y agresivo.
4. Conecte la salida ENV de un ENVELOPE a FC, con el CLOCK en la entrada GATE de la envolvente. Tiene un bajo ácido.
