## O que é
BUDGET gives the voices a budget of events. Four gates go in and out, but per cycle only TETO events get through in total. When the budget runs out, further gates are left out until the cycle starts again.

## Como pensar nele
Density is not quality. BUDGET makes the patch choose: with a small budget, each note is worth more. There are two economies. Compete: whoever arrives first takes it. Cooperate: each voice that plays has its share, and only goes beyond it if the others have not yet spent theirs.

## Controles
- TETO: how many events fit in each cycle, from 1 to 32.
- COOP: off, the voices compete; on, they cooperate.
- CICLO: the length of the cycle, from 0.25 to 16 s, when nothing is patched into CLK.
Inputs: G1 to G4 take the voices' gates, CLK restarts the cycle on each pulse. Outputs: O1 to O4 are the gates that got through, SALDO what is left of the budget, FIM a pulse when it runs out.

## Experimente
1. Patch the gates of two voices into G1 and G2, and O1 and O2 into their envelopes.
2. With TETO at 6, hear the voices compete for the cycle.
3. Turn on COOP: the two start sharing the budget.
4. Patch SALDO into the cutoff of a FILTER: the sound darkens as the budget is spent.
