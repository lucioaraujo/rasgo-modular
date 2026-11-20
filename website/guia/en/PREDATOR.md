## O que é
PREDATOR is a small ecosystem: prey that grow on their own and predators that grow by eating. The two populations rise and fall in a cycle, the predator always a little behind, and come out as CV at PREY and PRED.

## Como pensar nele
These are the Lotka–Volterra equations, from the 1920s, the same physics as the Aquorbium's ecosystem. When there is plenty of prey, the predator grows; when there are many predators, the prey falls; then the predator goes hungry, and it all starts again. The captures give pulses at CTCH, and the prey's peak, the flight, gives a pulse at FLEE. CANON repeats the prey line with a delay that shrinks as the predator gets closer: a canon that tightens into stretto.

## Controles
- PERIOD: the length of one cycle.
- SIZE: the size of the orbit.
- HUNT: how much the predator hunts.
- WILD: the noise of the environment.
- DELAY: the canon's maximum delay.
Inputs: CTCH is a capture from outside, RATE moves the speed in volts per octave. Outputs: PREY and PRED are the populations, CTCH and FLEE the pulses of the hunt and the flight, CANON the prey's canon.

## Experimente
1. Patch PREY into the cutoff of a FILTER and PRED into the level of a VCA: the timbre and the dynamics hunt each other.
2. Patch CTCH into a DRUM with HUNT high: the captures become hits in times of plenty.
3. Patch PREY and CANON into the pitches of two voices: the second imitates the first closer and closer.
4. Raise WILD: the cycles stop being alike, as in a real ecosystem.
