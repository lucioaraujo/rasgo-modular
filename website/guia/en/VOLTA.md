## O que é
VOLTA is a loop of steps that varies without getting lost. It keeps a base sequence and transforms it on top; when you turn the knob back, the original sequence returns exactly as it was.

## Como pensar nele
TURING rewrites its own loop, and what changes is lost. VOLTA never rewrites the base: each step has its turn to change, always in the same order, so you can go and come back. Only a pulse at NEW replaces the base with a new one.

## Controles
- RATE: the internal clock rate, when there is no CLK.
- LEN: how many steps the loop has.
- MUT: how many steps switch to their alternative value.
- ERODE: how many steps go silent.
- MEM: the protected steps, which neither change nor go silent.
- RANGE: the CV's amplitude.
- STEPS: how many steps the CV is quantized to.
- OFST: an offset added to the CV.
- SEED: the seed of the loop and of the order of changes.
Inputs: CLK, the clock; RST, back to the first step; NEW, a new base; MUT and ERO, signals added to the knobs. Outputs: CV and CV2, the pitches; PULSE, the gates; POS, the position in the loop.

## Experimente
1. Connect CLOCK's CLK to VOLTA's CLK, CV to a QUANTIZER and from there to an OSC, and PULSE to an ENVELOPE.
2. Raise MUT slowly: the melody varies. Bring MUT back to zero: it returns whole.
3. Connect a slow FUNCTION to MUT: the melody drifts away and returns on its own.
