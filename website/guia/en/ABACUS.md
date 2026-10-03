## O que é
ABACUS treats signals as numbers. It does arithmetic between two inputs, counts pulses and turns the count into rhythms and staircases, and cuts or mirrors the negative part of a signal. Even with nothing on inputs A and B, its internal counter generates patterns you can use as rhythm and melody.

## Como pensar nele
Counting is the simplest way to create a pattern. A counter that goes from 0 to 7 and starts over, looked at bit by bit, produces syncopated rhythms without any sequencer: each bit turns on and off at a different speed. The QNT output turns the count into a staircase of values, good to send to a QUANTIZER. And the RCT output, the rectifier, is useful on its own whenever you need the positive half of a signal or its absolute value.

## Controles
- OP: the operation between A and B that comes out of MTH: addition, subtraction, multiplication, remainder, or four bitwise operations.
- MOD: how high the counter counts before starting over, from 2 to 32.
- STEP: how many steps the QNT output is divided into.
- RNG: the size of the value window used by the MTH, QNT and RCT outputs.
- RECT: the rectifier mode: positive part only, negative part only, both mirrored upward, or just the sign, plus or minus.
- CNT: how far the counter moves on each pulse. Negative counts backwards.
- PAT: which bit of the counter becomes the P1 output. Low bits switch fast; high bits, slowly.
- SLEW: smooths the MTH and QNT outputs.
- RATE: the internal clock, used when nothing reaches CLK.
Inputs: A and B, the numbers for the operation; CLK, the pulse that counts; RST, to reset. Outputs: MTH, the result; QNT, the staircase; RCT, the rectifier; P1 and P2, two rhythms taken from the counter; CRY, a pulse each time the count starts over.

## Experimente
1. Connect the CLK output of CLOCK to ABACUS's CLK input, with MOD at 8.
2. Connect P1, P2 and CRY to the GATE inputs of three DRUM modules connected to the MIXER. You hear a syncopated rhythm that comes round every eight pulses.
3. Turn PAT and hear P1 get faster or slower.
4. Connect QNT to the CV input of a QUANTIZER, and PTCH to the 1V/O of an OSC connected to the MIXER. The count becomes a stepwise melody.
