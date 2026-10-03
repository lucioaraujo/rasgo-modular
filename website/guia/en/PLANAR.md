## O que é
PLANAR blends four sounds placed at the corners of a square. A point moves inside the square, and the output is a mix of the four, weighted by the point's distance from each corner. As the point moves slowly, the tone morphs from one sound into another. The technique is called vector synthesis.

## Como pensar nele
Think of a joystick: X moves the point left and right, Y up and down. You can move it by hand, with two slow waves, or with a DRIFT, and the point traces figures. The position also comes out of the X' and Y' outputs, so the same movement can drive other controls in the patch. With a long pulse on the GST input, PLANAR records the path you make with the X and Y knobs and then repeats it endlessly.

## Controles
- X, Y: the position of the point in the square. Signals on the inputs of the same name add to these values.
- CURVE: the way of blending. At one end the blend is linear, good for control signals; at the other it keeps the volume constant, so the sound doesn't dip when the point is in the middle.
- SMTH: makes the point glide to its new position instead of jumping.
- RATE: the speed at which the recorded gesture repeats and of DRIFT's wander.
- DRIFT: makes the point wander around the square on its own.
Inputs: A, B, C and D, the four sounds; X and Y, to move the point; GST, to record and repeat a gesture. Outputs: OUT, the blend; X' and Y', the position of the point.

## Experimente
1. Connect four different sources to A, B, C and D: for example the SAW of an OSC, the OUT of a WAVETABLE, the OUT of CHORD and the PNK of a NOISE. Connect PLANAR's OUT to the MIXER.
2. Move X and Y by hand and hear the sound pass from one corner to another.
3. Connect the BI outputs of two slow FUNCTION modules, at different speeds, to the X and Y inputs. The point traces a figure and the tone keeps changing.
4. Connect Y' to the FC input of a FILTER in the sound's path: the filter follows the movement.
