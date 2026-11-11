## O que é
POLYGON is polymeter drawn as a figure. There is one shared cycle and up to four layers; each layer is a polygon of N vertices inscribed in that cycle, and each vertex is a pulse. A triangle and a square in the same cycle give three against four.

## Como pensar nele
Think of a clock with several invisible hands, each dividing the cycle into different equal parts. The pulses of each layer are regular; the interest lies where they meet. The COINC output marks those meetings, and that is where music usually asks for an accent. F rotates each polygon, up to one side, to shift the layer without changing its division.

## Controles
- N1, N2, N3, N4: how many vertices each layer has, from 0 (off) to 16.
- F1, F2, F3, F4: the rotation of each layer, up to one side of the polygon.
- CICLO: the length of the cycle, from 0.25 to 32 s, when nothing is patched into CLK.
- PASSOS: with a clock at CLK, how many pulses the cycle lasts.
Inputs: CLK drives the cycle by pulses, RST restarts. Outputs: G1 to G4 are the pulses of each layer, COINC pulses when two or more vertices fall together, EOS at the end of each cycle, FASE is the position in the cycle.

## Experimente
1. With N1 at 3 and N2 at 4, patch G1 into a DRUM and G2 into another: three against four.
2. Patch COINC into a third sound: only the meetings.
3. Set N3 to 5 and turn F3 slowly: the layer of five slides over the others.
4. Patch the CLOCK into CLK with PASSOS at 16: the polymeter follows the patch's tempo.
