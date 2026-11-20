## O que é
GENDYN is a sound source in which the wave is a polygon of a few points. On each turn, each point shifts a little in height and in distance to the next, at random. The wave never repeats, but it never runs away either: a sound that lives.

## Como pensar nele
It is Iannis Xenakis's dynamic stochastic synthesis, the one behind Gendy3, from 1991. Each point takes a random step, like a drunkard, and the edges reflect it back, so it never sticks to them. With BAND at 0, the pitch is held and only the shape of the wave breathes: a tuned, living note. With BAND at 1, the period wanders too, and the sound goes all the way to noise. DIST chooses the kind of step; the Cauchy distribution, Xenakis's favourite, gives rare, large jumps.

## Controles
- PTS: how many points the polygon has.
- FREQ: the central pitch.
- BAND: how far the pitch may wander.
- AMP: the step of the amplitudes.
- DUR: the step of the durations.
- DIST: the kind of step.
- LEVEL: the level.
Inputs: PTCH adds a pitch in volts per octave, FRZ freezes the wave while the gate is high, AMP adds to the step of the amplitudes. Output: OUT is the sound.

## Experimente
1. Set BAND to 0.1 and patch OUT into a FILTER and a VCA: a breathing bass.
2. Raise BAND and AMP to 1: Xenakis's mass of noise.
3. Patch a slow gate into FRZ: the wave freezes and thaws.
4. Patch a sequence into PTCH with BAND low: a melody of living timbres.
