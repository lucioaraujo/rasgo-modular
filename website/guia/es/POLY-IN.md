## O que é
POLY-IN reparte las notas de un teclado MIDI o de una pista de la DAW en hasta cuatro voces. Cada voz tiene su altura, su gate y su velocidad, y cada una puede tocar su propio oscilador.

## Como pensar nele
SIGNAL-IN da una sola voz; POLY-IN da un acorde. El rack acepta hasta cuatro copias de cada módulo: con cuatro OSC y cuatro ENVELOPE, cada voz de POLY-IN toca un par, y el teclado ahora toca acordes en el patch. El módulo empieza apagado y nunca pide el micrófono: solo abre el MIDI.

## Controles
- VOZES: cuántas voces reciben notas, de una a cuatro.
- MODE: cómo se reparten las notas. *Rota* pasa cada nota a la siguiente voz libre; *Mesma* devuelve la nota repetida a la voz donde sonó; *Altura* ordena las voces, y la voz 1 siempre tiene la nota más grave. Sin voz libre, la nota nueva toma la voz más antigua.
- BEND: cuántos semitonos mueve el pitch bend todas las voces.
- ON: enciende la entrada MIDI.
Salidas: P1 a P4, la altura de cada voz; G1 a G4, el gate de cada voz, abierto mientras la tecla o el pedal sostiene la nota; V1 a V4, la fuerza de cada nota.

## Experimente
1. Encienda ON. Conecte P1 a la entrada 1V/O de un OSC y G1 a la entrada GATE de un ENVELOPE que reciba el sonido de ese OSC.
2. Añada otro OSC y otro ENVELOPE y conecte en ellos P2 y G2. Toque dos notas juntas: cada una suena en su par.
3. Con cuatro pares y MODE en *Altura*, la voz 1 es siempre el bajo del acorde: dele un filtro más cerrado o más nivel en el MIXER.
