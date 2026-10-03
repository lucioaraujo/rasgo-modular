## O que é
HARMONY decides which keys the music passes through. From time to time, or on each pulse at the ADV input, it picks a new tonal center, following one of six moves that jazz and film musicians have used for decades. The ROOT output gives the new tonic on the same pitch scale the oscillators understand.

## Como pensar nele
QUANTIZER keeps the melody in tune, but always in the same key. Music tends to travel, and each style travels its own way. The MOVE control picks the route: Coltrane, which leaps a major third on each change and closes a cycle of three keys, as in Giant Steps; tritone substitution, which reaches the next key by an unexpected path; chromatic mediant, third-related shifts with a film-score feel; modal interchange, which keeps the tonic and changes the mode; modal jazz, which hardly ever moves; and backdoor, which rises a whole step.

In this version, the way to hear the changes is to add the ROOT output to the melody before the oscillator. Each input accepts only one cable, so the sum goes through a MATRIX, and the whole melody transposes to the new center. Scale changes, from the SCALE output, can't yet reach QUANTIZER by cable.

## Controles
- MOVE: the kind of route, among the six described above.
- RATE: the pace of the changes, when nothing reaches ADV. It ranges from one change every few minutes to two per second.
- ROOT: the starting tonic, which the module returns to when it gets a pulse on RST.
- S-LO, S-HI: the range of scales that can be drawn on each change.
- HOLD: the chance of a change being skipped, which lengthens some sections.
Inputs: ADV, the pulse that asks for a change; RST, to go back to the start. Outputs: ROOT, the tonic as a pitch; SCALE, the scale number; CHG, a pulse on each change.

## Experimente
1. Build the QUANTIZER melody: the CV output of a TURING, driven by CLOCK, into QUANTIZER's CV input, and an OSC running through an ENVELOPE to the MIXER.
2. Connect QUANTIZER's PTCH to IN1 of a MATRIX, HARMONY's ROOT to IN2, and the MATRIX's OUT1 to the OSC's 1V/O. On the MATRIX, set cell 21 to 1: OUT1 is now the melody plus the tonic.
3. Set MOVE to the first position, Coltrane, and raise RATE. On each change the melody jumps to another center, in cycles of three.
4. Connect CHG to the ADV input of a DRIFT so the rest of the patch also changes section on each change.
