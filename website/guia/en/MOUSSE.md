## O que é
MOUSSE is a foam. Press it, and part of it sinks at once and part flows down slowly; let go, and it comes back unhurried. The DEPTH output is how far it has sunk: an envelope made not of straight lines but of matter.

## Como pensar nele
An ordinary envelope rises and falls along lines. The foam has memory: hits in a row sink it further, and with FATG it softens with use. STIFF is its hardness, SLOW how fluid it is, RET how many times slower the return is than the descent. With nothing at HIT, it is played by itself at the pace of RATE.

## Controles
- STIFF: the hardness. Hard, it sinks little and returns fast.
- VISC: the viscosity, the time it takes to flow, from 0.05 to 8 s.
- SLOW: how much of the sinking is slow rather than immediate.
- RET: how many times slower the return is, from 1 to 8.
- FORCE: the force of each hit.
- FATG: fatigue; hits in a row soften the foam.
- RATE: the pace of internal hits, when nothing is patched into HIT.
Inputs: HIT gives a hit, PRESS leans on it with continuous force. Outputs: DEPTH is how far it has sunk, SPEED how fast, REST a gate when the foam is at rest.

## Experimente
1. Patch a DRUM into HIT and DEPTH into the CUT of a FILTER, with the cable attenuated and inverted: each hit darkens the sound and the brightness returns slowly.
2. Raise FATG and play hits in a row: the foam gives way.
3. Patch the ENV output of DYNAMICS into PRESS: the mix itself pushes the foam.
4. Patch REST into the gate of an ENVELOPE: a sound only when the foam rests.
