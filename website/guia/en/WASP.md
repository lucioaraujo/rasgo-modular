## O que é
WASP is a filter that distorts. Inspired by a late-1970s synthesizer known for its rough sound, it cuts frequencies like FILTER does, but dirties the sound as the resonance goes up. It is the filter for aggressive basses, cutting leads and grinding drones.

## Como pensar nele
FILTER is clean and precise; WASP is its dirty counterpart. Three knobs set how dirty: DRIVE pushes the sound into distortion before filtering, GRIT decides how much the filter grinds as it resonates, and BIAS makes the distortion lopsided, buzzier. With high resonance it sounds by itself, and that sound is rough too.

## Controles
- CUT: the cutoff frequency, from 20 to 24000 Hz.
- RESO: the resonance. Near maximum, the filter sounds on its own.
- MODE: the filter type, moving from low-pass to band-pass and high-pass.
- DRIVE: the input gain, which pushes the sound into distortion before filtering.
- GRIT: how much the filter distorts as it resonates. Low is nearly clean; high grinds.
- BIAS: unbalances the distortion, adding even harmonics and a buzz.
- DRIFT: a slow wobble in cutoff and resonance.
Inputs: IN, the sound to filter; FC, to move the cutoff; Q, for the resonance.

## Experimente
1. Connect the SAW output of an OSC to the IN input, and OUT to a MIXER input.
2. Raise RESO to about two thirds and turn CUT: hear the filter bite.
3. Raise GRIT and DRIVE. The sound gets rough and aggressive.
4. Connect the ENV output of an ENVELOPE to FC, with CLOCK on the envelope's GATE input. You have an acid bass.
