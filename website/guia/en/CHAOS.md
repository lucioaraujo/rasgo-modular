## O que é
CHAOS generates unpredictable movement from a simulated physical system: something like a ball rolling between two valleys, sometimes settling in one, sometimes jumping to the other. You can't predict when it will jump, but with the same seed everything repeats the same way.

## Como pensar nele
Other modules draw random values, like TURING and DECISION, or wander slowly, like DRIFT. CHAOS behaves in its own way: it spends a while circling one region and then suddenly switches to another. On a filter, that gives a tone that hesitates and then leaps; on a melody, phrases that stay in one register and then move abruptly. At high speeds it becomes a sound texture itself.

## Controles
- RATE: the speed of the movement, from 0.02 to 400 cycles per second. Slow to move controls; fast to listen to directly.
- DRIVE: the force pushing the ball between the valleys. Higher means bigger, more frequent jumps.
- DAMP: the friction. High, the ball settles in a valley and nearly stops; low, it swings widely and jumps from side to side.
- FRZ: freezes the movement at its current value.
Inputs: RSD, a pulse that gives a fresh push; RTM, to modulate the speed. Output: OUT.

## Experimente
1. Build a voice: the SAW of an OSC into the IN input of a FILTER, the FILTER's LO into the MIXER.
2. Connect CHAOS's OUT to the FILTER's FC input. Set RATE around 0.3, DRIVE high and DAMP low.
3. Hear the tone stay in one region for a while and then leap to another.
4. Also connect CHAOS's OUT to the CV input of a QUANTIZER, and its PTCH to the OSC's 1V/O. Now the melody changes register along with the tone.
