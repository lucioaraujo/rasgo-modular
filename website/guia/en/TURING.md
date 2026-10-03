## O que é
TURING invents melodies by chance and lets you keep the ones you like. It holds a loop of a few values that turns on each pulse; on each turn, every value may stay or be swapped for another. With the LOCK control you decide how much the loop changes, from total chance to a fixed repetition.

## Como pensar nele
Here you compose by choosing with your ears, not by writing note by note. Leave LOCK in the middle, listen to the melody transform and, when something good comes up, turn LOCK all the way: the loop locks and repeats. To vary it again, loosen it a little. The name honors the Turing Machine module by Music Thing Modular, which popularized the idea. SEQUENCE plays a written phrase; TURING lets phrases emerge and settle.

## Controles
- RATE: the internal clock, used when nothing reaches CLK.
- LEN: the length of the loop, from 2 to 16 steps.
- LOCK: the chance of the loop staying the same. At zero everything is drawn again; at maximum the phrase locks and repeats.
- MUT: when a value changes, how much it changes. Low, small variations of what was there; high, a fresh draw.
- RANGE: the span of the output values, that is, the size of the melody's leaps.
- STEPS: divides the output into steps. At 1 the values are continuous.
- OFST: the center of the values, which moves the melody's register up or down.
Inputs: CLK, the pulse that turns the loop; LOCK, to modulate the locking. Outputs: CV, the melody; CV2, another reading of the same loop, related but different; PLS, a rhythm taken from the same loop.

## Experimente
1. Connect the CLK output of CLOCK to TURING's CLK input.
2. Connect CV to the CV input of a QUANTIZER, and the QUANTIZER's PTCH to the 1V/O of an OSC running through an ENVELOPE to the MIXER. Connect PLS to the envelope's GATE input.
3. With LOCK in the middle, hear the melody change little by little.
4. When you like what you hear, turn LOCK all the way up. The phrase stays.
