## O que é
ARP is an arpeggiator: it plays a chord note by note, and so a chord becomes a melody. It chooses the notes (one of the ten chord shapes of CHORD, with inversion and octaves) and the path through them. With nothing patched, it plays on its own clock.

## Como pensar nele
Separate two questions: which notes, and in what order. CHORD, INV and OCT answer the first; MODE answers the second (up, down, up and down, random, shuffled, from the centre outwards, from the outside in). SKIP opens gaps in the line, and the gaps are always the same for the same patch, so the line sounds played and repeats the same way.

## Controles
- CHORD: the chord shape, from 0 to 9.
- INV: the inversion, from 0 to 3.
- OCT: across how many octaves the arpeggio spreads, from 1 to 4.
- MODE: the path through the notes, from 0 to 7.
- SKIP: the chance that a step stays silent.
- RATE: the internal clock, used when nothing is patched into CLK.
- GATE: the length of each note, from 5 to 95 % of the step.
Inputs: CLK advances one step per pulse, RST goes back to the start, ROOT takes the tonic (one volt per octave), CHD changes the chord shape. Outputs: PTCH is the note, GT the gate of each note, EOS a pulse on each full round.

## Experimente
1. Patch PTCH into the 1V/O of an OSC, the OSC output into a VCA and the VCA into the MIXER. Patch GT into an ENVELOPE and the ENVELOPE into the VCA.
2. Turn MODE and hear the same chord take other shapes.
3. Set MODE to 5, SKIP to 0.3 and OCT to 3: it sounds like an improvised line.
4. Patch the root output of HARMONY into ROOT: the arpeggio now moves with the progression.
