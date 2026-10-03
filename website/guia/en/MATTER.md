## O que é
MATTER simulates an object that vibrates when struck: a string, a bell, a metal plate, a tube. Inside it has 24 resonances that ring together, like those of a real object. You strike it and hear the object answer, with the tuning and material you choose.

## Como pensar nele
A filtered oscillator sounds electronic; MATTER sounds like something being played. You don't build the tone harmonic by harmonic: you choose the material with STRC and where the object is struck with POS, and then you excite it. The strike can come from inside, through the HIT input, or from outside, from any sound connected to IN. A CLOCK hitting HIT is already tuned percussion.

## Controles
- FREQ: the tuning of the object, from 20 to 8000 Hz.
- STRC: the material. At the start, a string, sweet and in tune; at the end, bell or metal, with scattered resonances.
- BRITE: how many high resonances ring. Low is muffled; high is bright.
- DAMP: how long the object keeps ringing after the strike. Low is a short tap; high is a long ring.
- POS: the point where the object is struck. As the point moves, some resonances disappear and others appear, as on a real instrument.
- EXCIT: how much noise goes into the strike. Low is a clean tap; high has more air in the attack.
- MIX: the blend between the raw strike and the object's resonance. At maximum, only the body of the object.
Inputs: IN, to make an outside sound resonate; HIT, a pulse that strikes; 1V/O, for tuning; STR, to change the material.

## Experimente
1. Connect OUT to a MIXER input, and the CLK output of CLOCK to the HIT input. You hear tuned hits.
2. Turn STRC from end to end. The hit goes from string to bell.
3. Raise DAMP until the hits run together into a continuous ring.
4. Move POS slowly and hear the tone change with the striking point.
