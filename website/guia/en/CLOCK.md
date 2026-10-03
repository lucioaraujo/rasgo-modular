## O que é
CLOCK is the patch's timekeeper. It sends regular pulses at the tempo you choose and builds rhythms and accents from them. Almost every patch starts with it: its pulses advance the sequence, fire the envelopes and play the drums.

## Como pensar nele
Instead of programming beat by beat, you describe the rhythm with two numbers. LEN says how many steps the cycle has, FILL says how many of them play, and CLOCK spreads those hits as evenly as possible. This method, called Euclidean rhythm, reproduces many traditional patterns from around the world: 3 in 8 gives the Cuban tresillo, 5 in 8 the cinquillo. The CLK output pulses on every step; EUC only on the chosen steps; ACC marks the accents.

## Controles
- BPM: the tempo, from 20 to 300 beats per minute.
- MULT: how many steps fit in one beat.
- LEN: the length of the cycle, from 1 to 32 steps.
- FILL: how many steps of the cycle play on the EUC output. Few make the rhythm sparse; close to LEN, almost continuous.
- ROT: rotates the pattern so it starts from another point. The density is the same, but the feel changes.
- SWING: delays every other step, giving it a lilt.
- DRIFT: lets the tempo vary slightly, like a drummer who pushes and pulls.
- GATE: how long each pulse stays on, within its step.
- ACC-A, ACC-B: accents fall on steps that are multiples of these numbers. With 4 and 3, for instance, the accents form a three-against-four pattern.
- AND: decides whether an accent needs both numbers at once or just one of them.
- FEEL: how many parts each beat is divided into: two, three (triplets), five, seven, nine, eleven, or a division drawn at random on each step.
Inputs: EXT, to follow outside pulses; RST, to go back to the start; BPM, to modulate the tempo. Outputs: CLK, EUC and ACC.

## Experimente
1. Connect EUC to the GATE input of a DRUM, and the DRUM's OUT to a MIXER input.
2. Set LEN to 8 and move FILL from 1 to 8. At 3 and at 5 you will hear familiar patterns.
3. Turn ROT and hear the same pattern start from somewhere else.
4. Connect ACC to the DRUM's ACC input and change ACC-A and ACC-B to hear the accents move around.
