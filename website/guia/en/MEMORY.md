## O que é
MEMORY continuously records the last three seconds of whatever goes into it and plays that material back in tiny pieces, called grains. Each grain is a short fragment, and many grains together form a cloud of sound that can be transposed, smeared in time, or frozen. The technique is called granular synthesis.

## Como pensar nele
With MEMORY, the patch starts remembering what it played. A phrase that has just sounded comes back as texture, higher or lower, stretched, blurred. Short, sparse grains give a rough rhythm; long, numerous grains, a smooth cloud. With HOLD on, recording stops and the stored stretch becomes fixed material that the grains keep exploring. LOOPER repeats whole stretches; MEMORY breaks them into particles.

## Controles
- GRAIN: the length of each grain, from 5 milliseconds to half a second. Short, a grainy texture; long, almost the original sound.
- DENS: how many new grains are born per second. Few give isolated dots; many, a continuous cloud.
- POS: where in the recording the grains are read from, from the most recent to the oldest.
- SPRAY: scatters each grain's reading point, blurring time.
- PITCH: transposes the grains, up to two octaves up or down.
- FBK: feeds the cloud back into the recording, so the material builds up and wears down.
- BLEND: the blend between the incoming sound and the cloud.
- HOLD: stops recording and freezes the stored material.
Inputs: IN, the sound; POS, to move the reading point; PTCH, to transpose; FRZ, to freeze with a signal. Output: OUT.

## Experimente
1. Connect a voice that plays a melody to the IN input, and OUT to a MIXER input.
2. Set GRAIN to about 0.1 seconds and DENS near 30, and hear the melody turn into a cloud.
3. Raise SPRAY and set PITCH an octave up.
4. Turn HOLD on. Recording stops, and the cloud carries on from the last stored stretch.
