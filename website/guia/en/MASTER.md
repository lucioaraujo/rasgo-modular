## O que é
MASTER is the last stage before the sound card. It sets the final volume, adjusts the width of the stereo image and protects the output from peaks, so nothing reaches the speakers distorted or too loud. Its output is already connected to the computer's sound: there is nowhere to cable it.

## Como pensar nele
MIXER adds things up, but the sum can have peaks, an exaggerated stereo image, or a constant offset that wastes the speakers' power without making sound. MASTER takes care of that. The meter shows the level; a small square lights up when the limiter has to hold back a peak. In practice, set the volume with GAIN and leave the protections on. The silence switch in the app's header is this module's MUTE.

## Controles
- GAIN: the final volume. Raise it until the meter gets close to the top without the limiter square staying lit all the time.
- WIDTH: the width of the stereo image. At zero, mono; at 1, normal; above, wider.
- MONO: folds both sides into one, to check how the patch sounds on a single speaker.
- DC: removes the constant offset from the signal. Leave it on.
- LIMIT: the limiter, which keeps peaks from going past the ceiling. Leave it on.
- MUTE: silences the output, without a click.
- BODY: softens strong, sustained highs that would tire the ear. It only acts when they appear.
Input: IN. Outputs: OUT, already connected to the sound card; VU, the level as a control signal.

## Experimente
1. Play any seed and watch MASTER's meter.
2. Move WIDTH from zero to maximum and hear the sound open up.
3. Connect VU to the IN1 input of a CONTROL with SCALE at −1, and O1 to the CV1 of a VCA that controls a background voice. When the rest of the patch gets loud, that voice steps back.
