## O que é
ACCENT gives a line its phrasing. A performer does not accent with volume alone: the accented note is louder, brighter and shorter, and there are many reasons to accent it. ACCENT gathers those causes, each with its own weight, into an emphasis from 0 to 1, and sends that emphasis to several outputs at once: level, brightness, length, sweep.

## Como pensar nele
There are six causes of accent: the downbeat (METR), the size of the leap (SALTO), the highest note of the recent phrase (APICE), the accent coming from outside (EXT), the note after a long note (AGOG) and an outside accent on a weak beat (SINC). Three controls shape the whole: HER leaves a tail on the next note, SUPR keeps everything from becoming an accent, ROT makes the downbeat drift slowly. PROF is how much of this reaches the outputs.

## Controles
- METR: the weight of the downbeat of each cycle.
- SALTO: the weight of the size of the interval from the previous note.
- APICE: the weight of the highest note of the last eight.
- EXT: the weight of the accent coming in at the ACC input.
- AGOG: the weight of the note that follows a long note.
- SINC: the weight of an outside accent on a weak beat.
- CICLO: how many notes the metric cycle has, from 2 to 16.
- ROT: how fast the downbeat moves.
- HER: the tail an accent leaves on the next note.
- SUPR: after a strong accent, the next one costs more.
- PROF: how deeply the emphasis reaches the outputs.
Inputs: GATE marks each note, PTCH takes the pitch, ACC the outside accent, RST restarts the cycle. Outputs: ENF is the emphasis, VCA the level, FC brightness for a filter, DEC length for an envelope, SWP a sweep that rises with accents in a row, GT a gate on accented notes only, NIV the emphasis in four steps (0, one third, two thirds, 1).

## Experimente
1. Play a SEQUENCE line through a QUANTIZER into an OSC. Patch the pitch and gate of the QUANTIZER into PTCH and GATE.
2. Patch the VCA output into the CV1 of the voice's VCA: the line starts breathing like a phrase.
3. Run the OSC through a LADDER and patch SWP into its FC: accents in a row climb the sweep, as in an acid bass.
4. Set CICLO to 5 and ROT to 0.5: the downbeat moves and the notes stay the same.
