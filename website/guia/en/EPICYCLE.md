## O que é
EPICYCLE draws with circles: up to eight, each one turning on the rim of the one before. The tip of the last one traces a figure, and its coordinates come out as signals. It is a drawn Fourier series, the same idea as Ptolemy's epicycles.

## Como pensar nele
The same machine works at two scales. With few circles and a slow RATE, it is gesture and rhythm: a figure that comes back. With many circles and the pitch of a note at PTCH, it turns at the speed of sound and becomes timbre. MODO chooses the family of figures, and DESV detunes the speeds so the figure never closes.

## Controles
- RATE: the speed of the first circle, from 0.01 Hz to 2 kHz.
- CIRC: how many circles, from 1 to 8.
- MODO: square (odd speeds), saw (all speeds), Fibonacci radii or Fibonacci speeds.
- DESV: the deviation of the speeds; above 0, the figure never closes.
- FASE: the starting phase of the circles.
Inputs: PTCH takes the pitch at one volt per octave, for timbre mode; RST restarts the figure. Outputs: X and Y are the coordinates of the tip, R the distance from the centre, ANG the angle, GT a pulse when Y crosses zero going up.

## Experimente
1. With RATE at 0.2 Hz and CIRC at 3, patch X into the cutoff of a FILTER and Y into the pan of a MIXER channel: the sound draws a figure in space.
2. Patch GT into an ENVELOPE: the figure becomes rhythm.
3. Patch X into the MIXER, set CIRC to 8 and patch the pitch of a QUANTIZER into PTCH: now the figure is a timbre.
4. Change MODO and raise DESV slowly: the timbre opens out and never returns to the same place.
