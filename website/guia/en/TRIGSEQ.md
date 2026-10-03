## O que é
TRIGSEQ generates drum rhythms on four lanes at once, meant for kick, snare, hi-hat and an extra percussion. You don't mark the hits one by one: you choose a style, set the density of each lane, and it creates the pattern. When it loads, it is already playing.

## Como pensar nele
Four CLOCK modules, one per instrument, would produce lanes that don't talk to each other. In TRIGSEQ all four come from the same style, so they fit together like a kit played by a person. The MAP control moves through four characters: straight, as in rock and house; broken, as in breakbeat; swung, as in hip-hop; and sparse, as in dub. Turned slowly, the groove changes personality without losing the pulse.

## Controles
- LEN: how many of the 16 steps are in the cycle.
- RATE: the internal clock, used when nothing reaches CLK.
- MAP: the style, from straight to sparse.
- DNS1, DNS2, DNS3, DNS4: the density of each lane. Higher, more hits.
- SWING: delays every other step, giving it a lilt.
- CHAOS: the chance of ghost hits appearing or expected hits going missing, which makes the rhythm less mechanical.
- RATCH: the chance of a hit becoming a quick burst of repeats, a roll.
- FILL: how much the FILL input raises the densities, for fills.
- DRIFT: lets the style and densities vary gradually, and the groove evolves on its own.
Inputs: CLK, the pulse; RST, to go back to the start; FILL, a signal that triggers the fill; MAP, to modulate the style. Outputs: T1 to T4, the four lanes; ACC, the accents; ANY, a pulse whenever any lane plays.

## Experimente
1. Connect the CLK output of CLOCK to TRIGSEQ's CLK input.
2. Put three DRUM modules in the rack. Connect T1, T2 and T3 to the GATE input of each one, and their OUT outputs to the MIXER.
3. Adjust TONE and DECAY on each DRUM so they sound like kick, snare and hi-hat.
4. Turn MAP slowly and hear the groove change style. Then play with the densities.
