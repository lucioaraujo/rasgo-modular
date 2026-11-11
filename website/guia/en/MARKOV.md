## O que é
MARKOV is a Markov chain: it learns where a melody goes from where (after this note came that one three times, that other one once) and improvises by drawing the next note from what it learned. With nothing patched, it already plays, starting with a pentatonic walk.

## Como pensar nele
It is a student who imitates the style of what it hears, not the exact notes. TEMP is its daring: near 0 it repeats the most common path, near 2 it invents. MEM is how much it remembers. The SURP output tells how unexpected each note was, and that can move the patch too.

## Controles
- RATE: the internal clock, when nothing is patched into CLK.
- GATE: the length of each note.
- TEMP: how daring the draw is, from 0 to 2.
- MEM: how much of what it learned it keeps.
- LEARN: turns learning on or off. Off, it freezes its vocabulary.
- OCT: transposes the output by octaves.
Inputs: CLK advances one note, LRN.P and LRN.G take the melody to learn (pitch and gate), RST starts over. Outputs: PTCH is the note, GT the gate, SURP the degree of surprise.

## Experimente
1. Patch PTCH into an OSC and GT into an ENVELOPE controlling a VCA. Hear the opening walk.
2. Patch the pitch and gate of a PHRASE into LRN.P and LRN.G: MARKOV starts answering in that voice's style.
3. After teaching it, turn LEARN off and raise TEMP: it improvises with the vocabulary it kept.
4. Patch SURP into the MIX of a SPACE: unexpected notes get an echo.
