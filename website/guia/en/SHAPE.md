## O que é
SHAPE enriches a sound by distorting it in controlled ways. It takes a simple wave, such as a sine, and folds it, saturates it, or multiplies it by another, creating new harmonics. The technique is associated with synthesizers from the west coast of the United States, which preferred adding harmonics to cutting them with filters.

## Como pensar nele
A filter removes harmonics from a rich sound; SHAPE goes the other way, starting from a plain one. Its main gesture is FOLD: when the signal goes past a limit, it is folded back, and every fold adds brightness. With an envelope on the FCV input, each note opens the tone at the attack and closes it afterwards. RING multiplies the sound by whatever comes into MOD, which gives bell-like tones.

## Controles
- RING: blends the sound with its product with the signal on the MOD input. At maximum you hear only the product, metallic.
- FOLD: the amount of folding. The more, the more high harmonics.
- SYM: shifts the center of the fold and adds even harmonics, making the sound more nasal.
- WRAP: swaps part of the fold for a hard cut that reappears on the other side, harsher.
- SAT: rounds off the peaks after the fold, softening the result.
- LVL: the output volume.
- DRIFT: a slow wobble in the amount of folding.
Inputs: IN, the sound; MOD, the second signal for RING; FCV, to modulate the fold.

## Experimente
1. Connect the SIN output of an OSC to the IN input, and OUT to a MIXER input. You hear a clean sine.
2. Raise FOLD slowly and hear the sine gain brightness, fold by fold.
3. Move SYM to one side: the sound gets more nasal.
4. Bring FOLD back to zero, connect the SAW output of another OSC to MOD and raise RING. Tune the two oscillators to distant notes and the sound turns into a bell.
