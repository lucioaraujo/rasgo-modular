## O que é
SPACE puts the sound in a place. It repeats the sound with a delay, once or several times, and can spread those repeats until they merge into a continuous tail. With a few adjustments it goes from a crisp echo to a wide room.

## Como pensar nele
Echo and reverb are two ends of the same road. A single delay, with nothing else, is an echo. Add repeats, feed them back to the input and scramble them, and the ear stops telling them apart: it hears a room. SPACE travels that whole road. Its place is near the end of the patch, before MIXER or MASTER, and several voices going through the same SPACE sound as if they were in the same room.

## Controles
- TIME: the delay time, from 2 milliseconds to 2 seconds. Turning it while sound is passing makes the space seem to change size.
- TAPS: how many repeats come out on each pass, from 1 to 8.
- SPRD: spreads those repeats in time. At zero they come out together, as a single echo; at maximum they form a rhythmic pattern.
- FBK: how much of the repeated sound goes back to be repeated again. More means more repeats and a longer tail.
- DIFF: scrambles the repeats until they can no longer be heard one by one. Raise FBK and DIFF together and the echo becomes a room.
- TONE: the brightness of the repeats. Low, each pass gets darker, as in a real room; high, the brightness holds.
- MOD: a slight wobble in the repeat timing, which makes the tail wider and shimmering.
- MIX: the blend between the original and the processed sound, on the OUT output.
Inputs: IN, the sound; TIME and FBK, to modulate those controls. Outputs: OUT, the blend; WET, the processed sound only.

## Experimente
1. Connect a voice that plays short notes, such as a MATTER played by CLOCK, to the IN input, and OUT to a MIXER input.
2. Set TAPS to 1, SPRD and DIFF to zero, and FBK to a third. You hear a clean echo.
3. Raise DIFF and FBK together, slowly. The echo turns into a room without any jump.
4. Move TIME while the tail is ringing and hear the space change size.
