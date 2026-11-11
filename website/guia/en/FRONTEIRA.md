## O que é
FRONTEIRA listens to a melody and marks where a new idea begins. We hear a boundary where something changes: a leap after steps, a long note after short ones, a rest. The module measures those three changes note by note and pulses when their strength passes the threshold.

## Como pensar nele
It is Cambouropoulos's model, built on the grouping rules of Lerdahl and Jackendoff. To know whether a note is a boundary it needs to hear the next one, so the pulse arrives one note later. A long silence closes the phrase at once. PHRASE generates a motif; FRONTEIRA listens.

## Controles
- LIM: the strength threshold above which a change becomes a boundary.
- MEM: how many notes it uses as the reference for what is normal.
- MIN: the minimum number of notes between two boundaries.
- SIL: how much silence closes the phrase, from 0.3 to 4 s.
Inputs: PTCH takes the pitch, GATE marks each note. Outputs: FRONT pulses at the start of each cell, FORCA is the strength of the last boundary, NOTAS how many notes the cell had, FIM pulses when silence closes the phrase.

## Experimente
1. Patch SIGNAL-IN into SCOPE, the PIT output of SCOPE into PTCH and the ONS output into GATE. Turn on the ON of SIGNAL-IN.
2. Patch FRONT into the FRASE input of the MIXER, with the stage in gesture mode: each new idea from the player moves the stage.
3. Patch FIM into the RST of a SEQUENCE: when the player stops, the accompaniment restarts with the next phrase.
4. Patch NOTAS into the length of an ENVELOPE: long phrases ask for longer notes.
