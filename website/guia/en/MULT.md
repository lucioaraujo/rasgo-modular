## O que é
MULT takes a control signal and distributes it to four outputs, each with its own size and offset adjustment. With nothing on its input, it becomes a bank of four fixed values you set by hand.

## Como pensar nele
In Rasgo Modular, one output can already go to several destinations with ordinary cables. MULT is for when each destination needs a different dose: the same envelope opening a filter fully, lowering another voice's volume a little, nudging an effect. One input, four adjusted versions. With DUAL on, it splits into two distributors with two outputs each.

## Controles
- DUAL: off, IN goes to all four outputs; on, IN goes to O1 and O2, and IN2 goes to O3 and O4.
- SCL1, SCL2, SCL3, SCL4: the size of the signal on each output. At 1 it passes unchanged; below 1, smaller; above, larger; negative, inverted. At zero only the offset remains.
- OFF1, OFF2, OFF3, OFF4: a value added to each output. With nothing on the input, it is the output's value.
- SLEW: smooths all four outputs together.
Inputs: IN and IN2. Outputs: O1 to O4.

## Experimente
1. Build two voices, each with its own FILTER, going through the MIXER.
2. Connect the ENV output of an ENVELOPE, played by CLOCK, to MULT's IN. Connect O1 to the first FILTER's FC and O2 to the second's FC.
3. Leave SCL1 at 1 and set SCL2 to a negative value. On each note, one filter opens and the other closes.
