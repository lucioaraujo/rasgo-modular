## O que é
ORBIT is a small solar system: a sun in the centre and two or three planets in gravity. The positions of the planets come out as CV, and their meetings come out as pulses.

## Como pensar nele
These are Newton's and Kepler's laws playing the patch. Wider orbits take longer, so the planets move in rhythms that nearly line up and drift apart. MASS makes the planets pull on each other: low, the orbits are regular; high, they break down into chaos. A planet that escapes returns to its orbit, so the system never gets lost. LIGHT delays the conjunction pulse by the distance, like light that takes time to arrive.

## Controles
- PLNTS: how many planets, 2 or 3.
- RATIO: how far apart the orbits are.
- ECC: how flattened the ellipses are.
- MASS: how much the planets pull on each other.
- SPEED: the speed of the system's time.
- LIGHT: the delay of the conjunction by distance, up to about 1 s.
Inputs: NUDGE pushes the first planet and changes the shape of its orbit, SPD moves the speed by CV. Outputs: X1 and Y1 are the position of the first planet, X2 and Y2 that of the second, DIST the distance between them, CONJ a pulse at conjunction, PERI a pulse when the first planet passes closest to the sun.

## Experimente
1. Patch CONJ into a DRUM and PERI into another: two slow rhythms that almost repeat.
2. Patch X1 into a QUANTIZER and the QUANTIZER into an OSC: a melody in orbit.
3. Raise MASS slowly: the orbits break down and the rhythm becomes unpredictable.
4. Patch DIST into the MIX of a SPACE: as the planets drift apart, the space opens up.
