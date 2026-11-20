## O que é
ELASTIC is a chain of masses tied by springs, fixed at both ends. The incoming sound pushes a mass near one end, and the whole chain vibrates. L and R listen at two points of the chain.

## Como pensar nele
It is a string made of beads. The vibration modes are not harmonic: they get closer and closer at the top, and that gives the timbre of an elastic mesh, between a gong and rubber. STRCH stiffens the spring the more it stretches, like a real rubber band: with a strong signal, the modes rise and the sound bends.

## Controles
- N: how many masses the chain has.
- TUNE: the spring's tuning.
- DAMP: the loss; how long the sound lasts.
- STRCH: the stiffening of the spring.
- PICK: the listening point.
- MIX: the mix with the incoming sound.
Inputs: IN is the sound that makes it vibrate, PLUK plucks the chain, TUNE moves the tuning in volts per octave. Outputs: L and R are the chain heard at two mirrored points.

## Experimente
1. Patch a CLOCK into PLUK, with nothing in IN: a plucked rubber-band bass.
2. Patch a DRUM into IN with DAMP at 0.1: each hit wakes a rubber gong.
3. Raise STRCH with a loud sound: the modes rise and the timbre bends.
4. Move PICK slowly: the timbre goes from the bright end to the dark middle.
