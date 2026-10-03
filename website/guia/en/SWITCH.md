## O que é
SWITCH is a selector switch operated by the patch. It can pick one of four sources and send it to one output, or take a single source and send it to one of four outputs. The switch happens on each pulse, in order or at random, or follows a control signal.

## Como pensar nele
In a patch that transforms itself, changing material matters as much as changing notes: the same melody played now by an oscillator, now by a string; the filter fed now by noise, now by a chord. Without SWITCH, that means swapping cables by hand. With DEMUX off, it chooses among A, B, C and D and delivers to OA. On, it takes what arrives at A and sends it to one of the four outputs, OA, OB, OC or OD.

## Controles
- STEP: how many positions the switch moves through, from 2 to 4.
- MODE: how the position changes: forwards, back and forth, at random, or only from the signal on the ADR input.
- DEMUX: off, four inputs to one output; on, one input to four outputs.
- GLID: a gradual transition between positions, in which the two sources blend for a moment. At zero the cut is clean.
- SLEW: smooths the switch to avoid clicks. Best to always leave a little.
Inputs: A, B, C and D, the sources; CLK, the pulse that advances; RST, to go back to the first position; ADR, to choose the position with a signal. Outputs: OA, OB, OC and OD; STP, the current position as a control signal.

## Experimente
1. Connect the SAW output of an OSC to A, and the OUT of a MATTER, played by CLOCK, to B. Connect OA to a MIXER input.
2. Set STEP to 2 and connect the DIV output of a LOGIC to CLK, with CLOCK's CLK output on LOGIC's CLK input and DIV at 4.
3. Every four pulses the sound switches source. Raise GLID so the switches blend.
