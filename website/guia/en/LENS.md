## O que é
LENS is a camera lens for sound. The spectrum is split into eight bands. The band at FOCUS stays sharp; the others go out of focus: their attack spreads in time, like a blurred image.

## Como pensar nele
Think of a portrait photo: a sharp face and a blurred background. DEPTH is the depth of field, the aperture: small, only the focus band stays sharp; large, almost everything does. BLUR says how much the out-of-focus part blurs. With BLUR at 0, the lens changes nothing. What goes out of focus also opens up in the stereo field, like the bokeh in the background of a photo.

## Controles
- FOCUS: the frequency in focus.
- BLUR: how much the out-of-focus part blurs.
- DEPTH: the depth of field.
- MIX: how much of the blur comes in.
Inputs: IN is the sound, FOC moves the focus in volts per octave, BLUR adds to BLUR. Outputs: L and R, with the out-of-focus part opened in the stereo field.

## Experimente
1. Patch a DRUM into IN, FOCUS at 2000 and a small DEPTH: the snap stays sharp and the body blurs.
2. Set FOCUS low: now the body is sharp and the brightness turns to haze.
3. Patch a slow LFO into FOC: the lens hunts for focus, like a camera.
4. Put a voice into IN and raise DEPTH slowly: the blur recedes until it disappears.
