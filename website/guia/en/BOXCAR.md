## O que é
BOXCAR measures a signal in short windows and averages many measurements in a row. The technique comes from laboratory instruments used to find a weak signal buried in noise. In a patch, it serves as a very steady volume follower, a waveform reconstructor, and an oscillator made from what it has measured.

## Como pensar nele
SH captures an instant; BOXCAR captures a slice of time and averages it. On each pulse at TRIG, it opens a window at some point in the cycle, measures, and adds that measurement to the previous ones. Whatever repeats identically on every cycle firms up, and whatever is random cancels out. MODE chooses what to do with the result: output just the average, as a control signal that follows the sound slowly; play back the reconstructed cycle; or replay that cycle on its own, like an oscillator. It also has a separate output, GEIG, with irregular pulses like those of a Geiger counter.

## Controles
- DLY: where in the cycle the window opens.
- APER: the width of the window. Narrow, it measures almost an instant; wide, it averages a stretch.
- AVG: how many measurements go into the average, from 1 to 64. More measurements, less noise and a slower response.
- SCAN: makes the window travel through the cycle by itself, in either direction. At zero it stays put.
- MODE: follower, reconstruction or oscillator.
- RATE: the internal clock, when nothing reaches TRIG, and the replay speed in oscillator mode.
- THRSH: the level the signal must cross to trigger a measurement, when nothing reaches TRIG.
- GEI: the density of the pulses on the GEIG output. At zero it is silent.
- BLEND: the blend between the original and the processed signal.
Inputs: IN, the signal to measure; TRIG, the pulse for each measurement; SWP, to move the window; THR, to modulate the threshold. Outputs: OUT and GEIG.

## Experimente
1. Connect GEIG to the GATE input of a DRUM connected to the MIXER and raise GEI. You hear hits at unpredictable moments.
2. Now connect a voice that plays notes, such as an OSC running through an ENVELOPE, to the IN input, with MODE in the first position and APER wide.
3. Connect OUT to the CV1 input of a VCA that controls another sound. The second sound now follows the voice's volume, without trembling.
