## O que é
SAMPLER records a stretch of whatever goes into it, up to about eight seconds, divides that stretch into slices, and plays them when it receives pulses. It is the gesture of hip-hop and electronic music samplers: take a recording, chop it up and play it back in another order, faster, slower or backwards.

## Como pensar nele
While the REC input is on, it records what arrives at IN. When REC turns off, the stretch is kept. From then on, each pulse at TRIG plays a slice, and the POS input chooses which one. With a TRIGSEQ triggering and a SEQUENCE choosing slices, the recording recombines into a new rhythm. Feeding the patch's own output into it, the instrument starts reusing what it has just played.

## Controles
- START: where, within the slice, playback begins.
- SPEED: the playback speed, from a quarter to four times the original. Negative plays backwards. Normally, faster also sounds higher.
- SLICE: how many equal slices the recording is divided into, from 1 to 16.
- REPIT: off, speed and pitch move together, as on tape; on, the pitch comes from the PIT input and the slice keeps its length.
- WEAR: wear on each trigger: an imprecise start, loss of definition, a grainy sound.
- LOOP: off, each slice plays once; on, it repeats.
Inputs: TRIG, the pulse that plays; IN, the sound to record; REC, the signal that records while it is on; POS, to choose the slice; PIT, to tune. Output: OUT.

## Experimente
1. Connect a voice that plays a phrase, for example a SEQUENCE playing an OSC, to the IN input.
2. To record, connect the DIV output of a LOGIC to REC, with CLOCK's CLK output on LOGIC's CLK and DIV at 16. Connect SAMPLER's OUT to the MIXER.
3. Set SLICE to 8, connect T1 of a TRIGSEQ to TRIG and the CV output of a TURING to POS. The recorded phrase comes back chopped and reordered.
4. Move SPEED to the left of center and hear the slices backwards.
