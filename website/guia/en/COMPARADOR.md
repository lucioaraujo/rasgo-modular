## O que é
COMPARADOR mixes two signals with polarity and decides when the mix crosses a threshold. The A and B knobs have zero in the centre: to the right the input adds, to the left it comes in inverted. The COMP output is a gate that stays high while the mix is above the threshold.

## Como pensar nele
It is a balance. With A to the right and B to the left, the mix is the difference between the two signals; with the same signal in both, it cancels out. The decision comes after: the gate opens when the balance tips one way. Hysteresis keeps it from flickering when the signal is noisy near the threshold.

## Controles
- A, B: how much each input comes in, from −1 (inverted) to 1.
- OFS: shifts the whole mix up or down.
- LIM: the decision threshold.
- HIST: hysteresis, a margin around the threshold.
Inputs: A and B take the signals, LIM moves the threshold by CV. Outputs: MIX is the mix, INV the mix upside down, COMP the decision gate, CRUZ a pulse on each change of side.

## Experimente
1. Patch two slow FUNCTION modules, at different speeds, into A and B. Set A to the right and B to the left.
2. Patch COMP into the gate of an ENVELOPE and CRUZ into a DRUM: two slow movements crossing become rhythm.
3. Patch the same signal into A and B with A at 1 and B at −1: the mix disappears.
4. Patch the LVL of a SCOPE into A and set LIM to the middle: when the player passes a certain volume, another voice comes in.
