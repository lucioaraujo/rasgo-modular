## O que é
ARP es un arpegiador: toca un acorde nota por nota, y así un acorde se vuelve melodía. Elige las notas (una de las diez formas de acorde del CHORD, con inversión y octavas) y el camino por ellas. Sin nada conectado, toca con su propio reloj.

## Como pensar nele
Separe dos preguntas: qué notas, y en qué orden. CHORD, INV y OCT responden la primera; MODE responde la segunda (subiendo, bajando, ida y vuelta, al azar, barajado, del centro hacia fuera, de fuera hacia el centro). SKIP abre huecos en la línea, y los huecos son siempre los mismos para el mismo patch, así que la línea suena tocada y se repite igual.

## Controles
- CHORD: la forma del acorde, de 0 a 9.
- INV: la inversión, de 0 a 3.
- OCT: por cuántas octavas se extiende el arpegio, de 1 a 4.
- MODE: el camino por las notas, de 0 a 7.
- SKIP: la probabilidad de que un paso quede en silencio.
- RATE: el reloj interno, usado cuando no hay nada conectado en CLK.
- GATE: la duración de cada nota, del 5 al 95 % del paso.
Entradas: CLK avanza un paso por pulso, RST vuelve al principio, ROOT recibe la tónica (un voltio por octava), CHD cambia la forma del acorde. Salidas: PTCH es la nota, GT el gate de cada nota, EOS un pulso en cada vuelta completa.

## Experimente
1. Conecte PTCH al 1V/O de un OSC, la salida del OSC a un VCA y el VCA al MIXER. Conecte GT a un ENVELOPE y el ENVELOPE al VCA.
2. Gire MODE y escuche el mismo acorde tomar otros dibujos.
3. Ponga MODE en 5, SKIP en 0,3 y OCT en 3: parece una línea improvisada.
4. Conecte la salida root del HARMONY a ROOT: el arpegio pasa a andar con la progresión.
