## O que é
REFRACT is a light ray that starts inside a glass layer, crosses three others and exits into air. In each layer it bends, and its angle in each one comes out as CV at A1 to A4.

## Como pensar nele
It is Snell's law: going from one medium to another, the ray changes direction according to their indices. Going from a dense medium to a less dense one at too shallow an angle, it does not pass through: it reflects entirely. That is total reflection, the sparkle of a diamond and the path of light in an optical fibre. Each time the ray starts to reflect, a pulse comes out at TIR.

## Controles
- N1, N2, N3, N4: the indices of the four layers.
- RATE: the speed of the angle sweep.
- RANGE: how far the angle sweeps.
- SHIM: the mirage, the shimmer of the indices.
Inputs: ANG gives the angle by CV, instead of the sweep, IDX adds to all the indices. Outputs: A1 to A4 are the angles in the layers, TIR the total-reflection pulse, EXIT the ray's sideways shift.

## Experimente
1. Patch TIR into a DRUM: the sweep passes the critical angle twice per cycle, and the DRUM marks each pass.
2. Patch A2 and A4 into two FILTER cutoffs: the same ray gives two similar but different movements.
3. Raise SHIM with RANGE near the critical angle: TIR becomes a mirage rhythm.
4. Patch an LFO into IDX: the glass heats and cools, and the point of total reflection moves.
