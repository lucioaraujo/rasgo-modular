## O que é
RUNGLER has two oscillators and an 8-bit register. On each turn of oscillator A, the register moves one place and stores the state of oscillator B at that instant. The last three bits become a voltage of 8 steps, which comes out at RUNG and goes back to modulate both oscillators.

## Como pensar nele
It is the heart of the Benjolin, by Rob Hordijk. The loop closes: with little modulation, a pattern that repeats; with a lot, the circuit stumbles between locking into a rhythm and turning into noise. LOOP is Hordijk's switch: at maximum, the bit that leaves the end comes back to the start and the 8-step pattern repeats forever; in between, a new bit comes in now and then and the pattern transforms.

## Controles
- FRQA: the frequency of oscillator A, the clock.
- FRQB: the frequency of oscillator B.
- LOOP: the loop switch.
- MODA: how much the rungler modulates A.
- MODB: how much the rungler modulates B.
- LEVEL: the level.
Inputs: PTA and PTB add pitches in volts per octave, LOOP adds to the loop switch. Outputs: TRIA and TRIB are the triangles, PWM the comparator, RUNG the 8 steps.

## Experimente
1. Set FRQA to 8 Hz and patch RUNG into a QUANTIZER and an OSC: a melody that thinks for itself.
2. Patch PWM into a FILTER and a VCA with both frequencies at audio rate: the Benjolin's raspy sound.
3. Raise LOOP all the way: the pattern locks. Lower it a little: it starts to change.
4. Raise MODA and MODB: the pattern stumbles into chaos.
