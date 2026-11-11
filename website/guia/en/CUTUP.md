## O que é
CUTUP cuts up the patch's own sound in tempo. It keeps the last four seconds and, when called, makes an editing gesture over the last beats of the clock: a stutter, a shuffled burst, a microscopic repeat or a silence.

## Como pensar nele
It is the cut-up technique of William Burroughs and tape collage, put on the bus. Gestures come by gate, each at its own input, or by themselves through CHANCE. Each fragment has soft 3 ms edges, and so does the whole gesture, so the cut does not click.

## Controles
- CHANCE: how often spontaneous gestures happen; at 0, only by gate.
- MICRO: how many repeats the microscopic gesture has, from 2 to 8.
- MIX: the balance between the original and the cut-up sound.
- SEED: the seed that chooses gestures and slices.
Inputs: IN takes the sound, CLK the patch's clock, STUT fires the stutter, BURST the burst of shuffled slices, MICRO the microscopic repeat, GAP a beat of silence. Outputs: OUT is the cut-up sound, ATIVO a gate while a gesture sounds.

## Experimente
1. Patch the MIXER output into IN, OUT into the MASTER and the CLOCK into CLK.
2. Set CHANCE to 0.15: now and then, the patch cuts itself up right in time.
3. Patch the Euclidean output of CLOCK into STUT: the stutter falls on the beats of the rhythm.
4. Patch the EOS of a SEQUENCE into GAP: each phrase ends with a cut silence.
