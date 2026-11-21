## O que é
TEMPO makes the tempo breathe. It gives CLOCK a moving tempo: it speeds up toward the climax, broadens at the end of each phrase, borrows and returns time, holds a fermata and slows down to a stop at the end.

## Como pensar nele
A fixed clock sounds like a machine; a person playing never keeps the tempo still. TEMPO comes out at RATE, which goes to CLOCK's RATE input. On its own it already breathes in phrases; connected to FORM, it follows the same shape as the piece.

## Controles
- ACCEL: how much the tempo speeds up with tension.
- FINAL: the arrival tempo at the end, as a fraction.
- SHAPE: the curve of the final slowdown.
- RIT: how long the final slowdown lasts, in seconds.
- MODE: strict, with phrase arch, or elastic, with rubato too.
- PHR: the phrase length in beats.
- ARCH: the phrase arch — faster in the middle.
- RUBATO: time borrowed and given back within the phrase.
- MOLA: how quickly the tempo follows changes, always without jumps.
- FERMT: how many beats the fermata lasts.
- SEED: the seed of the jumps and the rubato.
Inputs: TENS, the tension; SECT, section changes; CODA, the start of the ending; FERM, a fermata; CLK, the clock that counts the beats. Outputs: RATE, for CLOCK; ARCH, the phrase arch as a signal; BEAT, the beat already shaped; END, the end.

## Experimente
1. Connect TEMPO's RATE to CLOCK's RATE and CLOCK's CLK to TEMPO's CLK. Hear the tempo breathe in phrases.
2. Connect FORM's TENS, SECT and END to TEMPO's TENS, SECT and CODA: the piece speeds up at the climax and slows to a stop at the end.
3. Connect a rare pulse (from a DECISION, for example) to FERM: now and then time stops and comes back.
