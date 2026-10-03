## O que é
VOCODER makes one sound speak with another's voice. It listens to a sound, usually a voice, and measures how much energy there is in each frequency band; then it applies that pattern to a second sound, rich in harmonics. The result is the second sound carrying the speech of the first: the robot-voice effect heard in so much electronic music.

## Como pensar nele
It needs two inputs: CAR, the carrier, which is the sound that will speak (a saw, a chord), and MOD, the one doing the talking (a voice through SIGNAL-IN, a drum, a recording). With few bands the result is coarse and robotic; with many, the speech becomes clear. Nothing on CAR? It uses an internal saw, tunable through the PIT input.

## Controles
- BANDS: how many frequency bands, from 4 to 20. Few sound robotic; many make the speech intelligible.
- SHIFT: changes the size of the resulting voice, from large to small, without changing what is said.
- ATK: how quickly each band responds. Short, consonants come out crisp; long, everything softens.
- REL: how long each band takes to let go. Short is sharp; long smears the syllables into a pad.
- SIBIL: how much of the voice's high end passes straight through, so S and T stay clear.
- FRZ: freezes the current pattern. The carrier keeps saying the last syllable indefinitely.
- MIX: the blend between the original carrier and the speaking carrier.
Inputs: CAR, the carrier; MOD, the voice that speaks; PIT, the pitch of the internal saw when nothing is on CAR.

## Experimente
1. Connect the SAW output of an OSC to CAR, and the L output of SIGNAL-IN, with a microphone, to MOD. Connect OUT to a MIXER input and speak.
2. Lower BANDS to 6 and hear the voice go robotic; raise it to 20 and it becomes clear.
3. Raise REL: the words run together into a continuous sound.
4. Without a microphone, connect the OUT output of a DRUM to MOD. Each hit opens the carrier in its own tone.
