## O que é
INTERPRET plays the incoming notes the way a performer would. It does not invent notes: it takes the pitch and gate of any sequencer and returns the played note, with a glide across the interval, a small overshoot on leaps, vibrato that grows on long notes, articulation by context, breath and brightness.

## Como pensar nele
The score says the note; the performer says how. The module looks at the recent notes: it marks the leap, sustains the peak of the phrase, shortens two quick notes, ties the rest. Breath is spent over the phrase and comes back in a rest. Three axes of character (ENERG, SUAV, BRILHO) give the personality, and DERIVA makes it change slowly. With EXPR at 0, the note comes out raw.

## Controles
- EXPR: how much interpretation; at 0, the raw note.
- GLIDE: the time to slide between notes, up to 0.3 s.
- VIB: the depth of vibrato on long notes.
- ENERG: the energetic character, which marks the leaps.
- SUAV: the smooth character, which ties the notes.
- BRILHO: the bright character, which opens the tone at the peak.
- DERIVA: how much the character changes over time.
Inputs: PTCH takes the pitch, GATE the gate of each note, CAR moves the character by CV. Outputs: PTCH is the played pitch, AMP the dynamics, BRT the brightness, FRASE the position in the phrase, GT the articulated gate.

## Experimente
1. Patch the pitch and gate of a QUANTIZER into PTCH and GATE. Patch the PTCH output into the 1V/O of an OSC and GT into the voice's envelope.
2. Set EXPR to 0 and then to 0.8: compare the raw note with the played one.
3. Patch AMP into the CV1 of the voice's VCA and BRT into the filter cutoff: the line gains dynamics and colour.
4. Raise VIB and GLIDE on a slow line: it starts to sing.
