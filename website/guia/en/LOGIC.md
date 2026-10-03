## O que é
LOGIC combines and transforms rhythms. It divides a pulse to create slower rhythms, compares two rhythms to make a third, and flips a state on each pulse. These are simple operations, the kind used in digital circuits, applied to musical time.

## Como pensar nele
An interesting rhythm is rarely a straight pulse. It is usually the relationship between two: one playing at half speed, one that only plays when another plays too, one that plays when either plays but never both together. LOGIC sits between pulse sources, such as CLOCK, TURING and SEQUENCE, and the modules they trigger. With nothing on the CLK input, it uses a clock of its own.

## Controles
- RATE: the internal clock, used when nothing reaches CLK.
- DIV: divides the pulse. At 2, the DIV output pulses once every two pulses; at 3, once every three.
- MULT: adds in-between pulses within each period.
- GATE: how long the DIV output's pulse stays on.
- DELAY: delays the DIV output's pulse by up to 200 milliseconds.
Inputs: CLK, the pulse to divide; A and B, the two rhythms to compare; RST, to reset. Outputs: DIV, the divided pulse; AND, when A and B are on together; OR, when either is; XOR, when only one of them is; FLIP, which changes state on each pulse at A.

## Experimente
1. Connect the CLK output of CLOCK to LOGIC's CLK input and DIV to the GATE input of a DRUM connected to the MIXER. Change DIV to hear the pulse slow down.
2. Connect CLOCK's EUC to A, and the PLS output of a TURING to B.
3. Move the drum to the XOR output and then to AND. Hear how each combination makes a different rhythm out of the same two.
