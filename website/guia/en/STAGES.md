## O que é
STAGES draws a curve made of several linked segments, from two to eight. Depending on the settings, that curve becomes a multi-stage envelope, a slow movement that never repeats exactly, a sequence of stepped values, or even an oddly shaped sound.

## Como pensar nele
FUNCTION makes a single ramp, and ENVELOPE the usual four stages. STAGES makes compound shapes, and you don't draw point by point: a few controls shape the whole drawing at once. The control that changes its character most is HOLD. At zero the segments are ramps and the result is continuous movement; at maximum each segment jumps and holds, and STAGES starts working as a sequencer. With LOOP on, the shape repeats by itself; off, it runs once for each pulse on GATE.

## Controles
- SEGS: how many segments the shape has, from 2 to 8.
- RATE: the speed of one full cycle, when LOOP is on.
- CNTR: the layout of the levels: a rising staircase, an arch, or a falling staircase.
- CURVE: how each segment goes from one level to the next: fast at first, in a straight line, or slow at first.
- HOLD: at zero, smooth ramps; at maximum, steps that jump and hold.
- TILT: makes the first segments longer than the last ones, or the other way round.
- JITR: makes levels and durations vary slightly on each cycle, always the same for the same seed.
- LOOP: on, the shape repeats; off, it runs once per pulse.
Inputs: GATE, the pulse that fires it; RST, to go back to the start; RTM, to modulate the speed. Outputs: OUT, the curve; EOC, a pulse at the end of each cycle; STEP, a pulse on each segment.

## Experimente
1. Build a voice: the SAW of an OSC into the IN input of a FILTER, the FILTER's LO into the MIXER. Lower CUT.
2. Connect STAGES's OUT to the FILTER's FC input, with SEGS at 6. The tone travels along a shape that comes and goes.
3. Move CNTR and TILT and hear the drawing change.
4. Turn HOLD all the way up and SEGS to 8. Connect OUT to the CV input of a QUANTIZER and the QUANTIZER's PTCH to the OSC's 1V/O: STAGES now plays a melody.
