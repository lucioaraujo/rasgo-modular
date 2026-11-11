## O que é
APOCRIFO brings the distant past into the present, as a rupture. LOOPER repeats the now; APOCRIFO quotes what sounded at least IDADE seconds ago. It records the input, never its own output: the apocryphal never quotes itself.

## Como pensar nele
There are three ways to quote. Corroded: the passage comes back reversed and eaten away, with the present ducked underneath, like a corrupted memory. Cut: an editing cut, the passage comes back clean once and the splices fall on zero crossings, like the tape editor's blade. Parenthesis: the now stutters for an instant while the patch carries on underneath, and returns exactly where the patch already is. A quote comes from a trigger at CIT or by itself, through CHANCE.

## Controles
- MODO: corroded (0), cut (1) or parenthesis (2).
- IDADE: how far back the quote comes from, from 5 to 55 s.
- DUR: how long the quoted passage lasts.
- CHANCE: how often spontaneous quotes happen; at 0, only by trigger.
- PROF: the depth of the quote over the present.
- SEED: the seed that chooses the passages.
Inputs: IN takes the sound, CIT fires a quote. Outputs: OUT is the sound with the quotes, ATIVO a gate while a quote sounds, IDADE how far back it came from.

## Experimente
1. Patch the MIXER output into IN and OUT into the MASTER.
2. Set CHANCE to 0.3, MODO to corroded and IDADE to 30: now and then, the piece from half a minute ago tears through the middle.
3. Switch to cut mode and patch the COL of COHERENCE into CIT: when the line gets too predictable, the piece cuts to the past.
4. Patch the IDADE output into the cutoff of a FILTER: the older the quote, the darker it is.
