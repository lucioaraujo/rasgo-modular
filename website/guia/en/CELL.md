## O que é
CELL is a cellular automaton: a row of cells, lit or unlit, that rewrites itself on every pulse. Each cell looks at itself and its two neighbours, and a rule (a number from 0 to 255) decides whether it stays lit. The module reads this pattern with a head: the cell under it gives the gate, and the next three choose the note.

## Como pensar nele
It is a score that writes itself by a very simple law. Some rules give patterns that never repeat (30, 110), others give triangles (90) or things that travel (184). You do not compose the notes, you compose the law. MUT adds a little chance, so the line neither dies nor gets stuck.

## Controles
- RATE: the internal clock, when nothing is patched into CLK.
- RULE: the rule, from 0 to 255. Start with 30, 90, 110 and 184.
- CELLS: how many cells the row has, from 8 to 24.
- HEAD: where the reading head sits.
- RANGE: the range of the notes, up to 2 octaves.
- MUT: the chance of a cell flipping at random.
- GATE: the length of each note.
Inputs: CLK advances one generation, RST restarts the row, RULE changes the rule by CV. Outputs: PTCH is the note, GT the gate, DENS the share of lit cells, EOS a pulse when the row is reborn.

## Experimente
1. Patch GT into a DRUM and the DRUM into the MIXER. With RULE at 30, hear a rhythm that never repeats the same way.
2. Patch PTCH into an OSC and GT into an ENVELOPE controlling a VCA after the OSC. Turn RULE slowly.
3. Patch DENS into the CUT of a FILTER: the more life in the row, the brighter the sound.
4. Patch EOS into the ADV of HARMONY: the chord changes when the row is reborn.
