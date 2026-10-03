## O que é
WAVETABLE is an oscillator that holds a sequence of 16 different waveforms, like the frames of an animation, from the brightest (a saw) to the smoothest (a sine). The POS knob picks where you are in that sequence. As you move POS, the tone changes smoothly, with no filter needed.

## Como pensar nele
Where OSC gives you a fixed sound to sculpt afterwards, WAVETABLE is born in motion: all it takes is something moving POS. A slow LFO makes a pad that breathes; an envelope makes each note start bright and end dark. It can also listen: connected to SIGNAL-IN, it captures one cycle of the incoming sound and starts using it as its waveform.

## Controles
- FREQ: the pitch of the note, from 8 to 8000 Hz.
- FINE: fine tuning, up to a semitone either way.
- POS: the position in the table. At the start the sound is full of harmonics; at the end it is almost a sine. The POS input adds to this knob, and that is how the tone starts moving on its own.
- WARP: bends the reading of each cycle and adds harmonics with a digital accent, without changing frames.
- FM: how much the signal on the FM input moves the pitch.
- DRIFT: a small, slow wobble in the tuning.
Inputs: 1V/O for pitch, POS to move the position, FM, CAP for the sound to capture, and GRAB, a pulse that tells it to capture a new cycle.

## Experimente
1. Connect OUT to a MIXER input and turn POS from end to end. Hear the tone go from rough to smooth.
2. Connect the output of a slow FUNCTION to the POS input. Now the sound changes by itself, in cycles.
3. To capture: connect SIGNAL-IN to the CAP input and a CLOCK pulse to GRAB. On each pulse, the oscillator starts playing a slice of the incoming sound.
