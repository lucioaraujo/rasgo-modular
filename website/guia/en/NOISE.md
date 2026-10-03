## O que é
NOISE produces noise, the sound of all frequencies mixed at random, in several colors at once, each on its own output: from white, hissy and bright, to brown, deep like the sea. It also produces slow random values, which you can use to make other modules change on their own.

## Como pensar nele
NOISE has two very different uses. As sound, it is the stuff of wind, rain, cymbals and the crack of a snare. As chance, it is a source of variation for generative patches: the S&H output picks a new value on each pulse and holds it until the next, and SMTH glides from one value to another. Connect one of them to an oscillator's pitch, through a quantizer, and you have a melody that never repeats the same way.

## Controles
- RATE: how often the S&H and SMTH outputs pick a value, from very slow to 2000 times per second. If the TRIG input is connected, it takes over from this knob.
- SLEW: how long the SMTH output takes to reach each new value. At zero it jumps; at maximum it glides slowly.
- SPRD: the kind of draw. At zero every value is equally likely; at maximum, values near the middle come up more often and the changes get gentler.
- POIS: replaces the regular rhythm of the draws with irregular, random timing, keeping the same average.
Sound outputs: WHT (white), PNK (pink), BRN (brown), BLU (blue), VLT (violet) and BIT (digital noise). Chance outputs: S&H and SMTH. Inputs: TRIG, to trigger the draws, and IN.

## Experimente
1. Connect PNK to a MIXER input. You hear a soft hiss, like rain.
2. Switch to BRN, deeper, and then to WHT, brighter.
3. Connect S&H to the CV input of a QUANTIZER, and its PTCH output to the 1V/O input of an OSC that is sounding. The note starts jumping at random, always within the scale.
4. Connect the CLK output of CLOCK to the TRIG input: the jumps now happen in time with the clock.
