## O que é
SCOPE shows a signal's waveform, like an oscilloscope, and measures what it hears: volume, brightness, pitch and the start of each note. Those measurements come out through cables, as control signals. That way the patch can react to its own sound.

## Como pensar nele
On an ordinary oscilloscope, the screen is the end of the road. Here, what SCOPE measures can go back into the patch: the brightness of the mix opens a filter, one voice's pitch tunes another, the start of each note fires an envelope. Sound passes through it unchanged, via the THRU output, so it can sit in the middle of any chain. With SIGNAL-IN, it can also listen to an outside sound and make Rasgo Modular follow it.

## Controles
- TRIG: the level the signal must cross to steady the picture on the screen.
- EDGE: whether that crossing counts on the way up or the way down.
- REJ: a tolerance margin, so noise doesn't trigger the screen several times.
- RESP: how fast the volume, brightness and pitch measurements are. Fast, they track every detail; slow, they show the trend.
- HOLD: freezes the measurements at their current values.
- SENS: the sensitivity of the note-start detector. High, any rise counts; low, only strong attacks.
Inputs: IN, the sound; EXT, an external signal to steady the screen. Outputs: THRU, the untouched sound; TRIG, the trigger pulse; LVL, the volume; BRT, the brightness; PIT, the pitch; ONS, a pulse at the start of each note.

## Experimente
1. Put SCOPE between a voice and the MIXER: the voice on IN, THRU to the MIXER. Watch the waveform.
2. Connect BRT to the FC input of another voice's FILTER. When the first voice gets bright, the second opens up too.
3. Connect ONS to the GATE input of a DRUM. Each note of the first voice now comes with a hit.
