## O que é
MAGNET is a grid of 8 by 8 small magnets, each pointing up or down. Each magnet wants to align with its neighbours, and temperature shakes them. A head moves across the grid, one cell per pulse, and the magnet under it gives the gate.

## Como pensar nele
It is the Ising model, from the physics of materials. Below the critical temperature (near 2.27), large domains form and the rhythm is stable. Above it, noise wins. Near it, domains of every size appear and vanish: this is the edge of chaos, and it is where the module sounds most alive. A negative COUPL makes the magnets oppose their neighbours, like a chessboard.

## Controles
- TEMP: the temperature; the transition is near 2.27.
- COUPL: the coupling. Positive, the magnets align; negative, they oppose each other.
- FIELD: an external magnet that pushes the whole grid one way.
- SWEEP: how many attempts to flip magnets there are on each pulse.
- RATE: the internal clock, when nothing is patched into CLK.
- GATE: the length of each gate.
Inputs: CLK advances one pulse, QNCH heats everything at once and the grid reorganises itself, TEMP and FLD move temperature and field by CV. Outputs: GT is the head's gate, MAG the magnetisation, ENER the energy of the grid, FLIP how much of the grid flipped on this pulse.

## Experimente
1. Patch GT into a DRUM. With TEMP at 1, hear an almost frozen rhythm; at 4, a rhythm of noise.
2. Set TEMP near 2.3: domains appear and vanish, and the rhythm breathes.
3. Patch a rare trigger into QNCH: the grid falls apart and reorganises.
4. Patch MAG into the cutoff of a FILTER: the tone follows which way the grid leans.
