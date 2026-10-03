## O que é
DECISION is the patch's source of choices. On each pulse it decides whether or not to fire a signal on the GATE output, and draws two values on the X and Y outputs. You control the chance of firing, the size and shape of the draws, and how much it remembers of what it has done.

## Como pensar nele
A slow wave is too predictable, and pure noise has no shape. DECISION sits in between: chance with rules. BIAS decides whether something happens almost always or only now and then. SHAPE decides whether the values spread evenly or cluster near the center, with large leaps being rare. DEJA is the memory: instead of drawing again, it replays the last steps, and the music gains repetitions, like a theme coming back.

## Controles
- RATE: the internal clock, used when nothing reaches TRIG.
- BIAS: the chance of GATE firing on each pulse. At zero, never; at maximum, always.
- SPRD: the reach of the draws on X and Y. Low, values near the center; high, the full range.
- SHAPE: at zero all values are equally likely; at maximum, values near the center come up much more often.
- STEPS: divides the draws into steps. At 1 they are continuous.
- SLEW: makes each new value glide from the previous one.
- DEJA: the chance of repeating a value from memory instead of drawing a new one. High, the same phrase tends to repeat.
- LOOP: the size of that memory, from 1 to 16 steps.
Inputs: TRIG, the pulse for each decision; BIAS and SPRD, to modulate those controls. Outputs: X and Y, the drawn values; GATE, the decision.

## Experimente
1. Connect the EUC output of CLOCK to TRIG.
2. Connect X to the CV input of a QUANTIZER, PTCH to the 1V/O of an OSC, and the OSC through an ENVELOPE to the MIXER. Connect DECISION's GATE to the envelope's GATE.
3. Lower BIAS and hear the notes thin out, only now and then.
4. Raise DEJA gradually. The melody starts repeating itself, and with DEJA at maximum it gets caught in a loop of LOOP steps.
