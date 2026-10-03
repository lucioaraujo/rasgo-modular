## O que é
DRUM is a ready-made percussion voice: on each pulse at the GATE input, it plays a hit. With a few knobs it goes from kick to snare, tom and hi-hat, and shifts in character between the sound of old drum machines and a more acoustic sound.

## Como pensar nele
Building a kick drum from scratch takes several modules; DRUM gives you one ready to go. It needs something to tell it when to play: a CLOCK or, better, a TRIGSEQ, which generates rhythmic patterns on four lanes. Four DRUM modules, one on each TRIGSEQ lane, make a kit. And since TONE accepts a pitch through the PIT input, you can play a line of toms.

## Controles
- TONE: the pitch of the body of the hit, from 20 to 1000 Hz. Low is a kick; mid, a tom or snare; high, something like a clave.
- BEND: how far the pitch drops right after the hit. High gives a kick its weight; zero keeps the hit on a single pitch.
- DECAY: the length, from a short click to a sustained boom.
- SNAP: the amount of crack in the attack, which gives body to snare and hi-hat.
- MAP: the character, moving through drum-machine tones to an acoustic sound.
- DRIVE: saturates the output and makes the hit more aggressive.
- ROLL: makes the module trigger itself, from a slow roll to a buzz. At zero it only plays when it receives a pulse.
- DRIFT: varies each hit slightly, so repetition sounds played rather than programmed.
Inputs: GATE, the pulse that fires the hit; ACC, for accents; PIT, for pitch.

## Experimente
1. Connect OUT to a MIXER input, and the CLK output of CLOCK to the GATE input. You hear a kick keeping time.
2. Raise TONE and SNAP, lower DECAY: the kick becomes a dry snare.
3. Raise ROLL with nothing on GATE: the module plays by itself, in a roll.
4. Connect CLOCK to a TRIGSEQ and its T1 and T2 outputs to two DRUM modules set up differently. You have a small drum kit.
