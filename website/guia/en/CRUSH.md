## O que é
CRUSH dirties sound the digital way. It imitates the flaws of the first samplers and video games, and those of a bad connection: fewer samples per second, fewer volume steps, numbers that overflow and come back on the other side, stretches that stick or vanish. The result ranges from a slight roughness to a robot voice in pieces.

## Como pensar nele
SHAPE distorts like an analog circuit, rounding and folding the wave. CRUSH breaks the sound into steps. Each control is a different kind of damage, and you can use just one or stack several. The glitches are random, but drawn from the patch's seed, so the same patch damages the sound the same way every time. With an envelope on the MXM input, a note starts clean and gradually degrades.

## Controles
- RATE: how many times per second the sound is read, from 100 to 24000. Low values make the sound stair-stepped and bring in ghost notes, the metallic fizz of 8-bit machines.
- BITS: how many volume steps remain, from 1 to 16. At 16 you barely notice; at 1, everything becomes a square wave.
- DRIVE: the gain before the damage, pushing the sound past its limits.
- WRAP: what happens when the sound goes past the limit. At zero it is clipped; at maximum it reappears on the opposite side, which sounds much harsher, especially with DRIVE high.
- GLTCH: the chance of a glitch: a stretch that sticks, a hole of silence, a stutter.
- JITR: makes the reading irregular, and the pitch wavers a little, like a tired digital tape.
- TONE: a simple filter on the output. To the left it darkens; to the right it leaves only the high fizz; in the middle it changes nothing.
- MIX: the blend between the clean and the damaged sound. At zero the sound passes untouched.
Inputs: IN, the sound; RTM, to modulate RATE; MXM, to modulate MIX.

## Experimente
1. Connect the SAW output of an OSC to the IN input, and OUT to a MIXER input.
2. Lower RATE to about 4000 and BITS to 8. The sound gets grainy.
3. Raise GLTCH gradually and hear the glitches appear.
4. Lower MIX to zero and connect the ENV output of an ENVELOPE to MXM, with CLOCK on the envelope's GATE input. Each note starts clean and falls apart.
