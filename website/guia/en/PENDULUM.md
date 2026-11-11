## O que é
PENDULUM is a double pendulum: a pendulum with another hanging from its tip. With little energy, it swings almost like a clock; with a lot, the lower arm loops over and the motion never repeats. The positions of both tips come out as CV.

## Como pensar nele
It is the classic example of chaos: a simple law, an unpredictable result. ENRGY is the chaos knob. Low, order; high, loops. CROSS marks the rhythm of the swing, and FLIP marks each full loop, the moment chaos appears.

## Controles
- ENRGY: the energy of the swing; above halfway, chaos appears.
- RATIO: the ratio between the lengths of the two arms.
- MASS: the ratio between the masses.
- SPEED: the speed of the pendulum's time.
- DAMP: friction; with it, the pendulum gradually loses strength.
- AUTO: gives a push when the pendulum nearly stops.
Inputs: KICK gives a push, SPD moves the speed by CV. Outputs: X1 and Y1 are the tip of the first arm, X2 and Y2 that of the second, CROSS a gate on each pass through the centre, FLIP a gate on each full loop.

## Experimente
1. Patch X2 into a QUANTIZER and the QUANTIZER into an OSC, and CROSS into an ENVELOPE controlling the voice's VCA: a swinging melody.
2. Raise ENRGY slowly and hear the melody go from regular to unpredictable.
3. Patch FLIP into the ADV of HARMONY: the chord changes when chaos appears.
4. Patch X1 into the pan of a MIXER channel: the sound circles in space.
