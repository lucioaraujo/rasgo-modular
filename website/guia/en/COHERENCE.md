## O que é
COHERENCE measures how predictable a melody is and breaks when it becomes too ordered. It keeps the latest notes and computes their Shannon entropy: a repeated note gives order 1, all twelve notes equally give 0. When order stays above the threshold long enough, it fires a rupture pulse.

## Como pensar nele
We tend to think rupture is born of chaos. Here it is the opposite: extreme order also breaks. Pressure rises with each predictable note; after PAC notes in a row, it breaks. The rupture leaves a scar: the window forgets its older half and, for CIC notes, pressure does not rise again. The module makes no sound: it decides when, and the cable decides what.

## Controles
- JAN: the window, how many notes it remembers, from 4 to 32.
- LIM: the order threshold above which pressure rises.
- PAC: patience, how many notes in a row above the threshold before it breaks.
- CIC: the scar, how many notes after a rupture without new pressure.
Inputs: PTCH takes the pitch, GATE marks each note (without it, each change of pitch counts), RST forgets everything. Outputs: ORD is the measured order, PRES the accumulated pressure, COL the rupture pulse, SURP the surprise of each note.

## Experimente
1. Patch the pitch and gate of a QUANTIZER playing a short SEQUENCE into PTCH and GATE.
2. Patch COL into the RST of a TURING feeding another voice: when the first line repeats too much, the other one remakes itself.
3. Patch PRES into the cutoff of a FILTER: the tone opens while tension grows and closes at the rupture.
4. Patch SURP into the level of a VCA: the rare note sounds louder.
