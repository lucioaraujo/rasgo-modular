## O que é
SIGNAL-IN brings the outside world into the patch. It takes sound from the computer's audio input, such as a microphone or another instrument, and notes from a MIDI keyboard or controller, and turns them into signals the other modules understand.

## Como pensar nele
With it, someone outside can play Rasgo Modular: the key's pitch goes to an oscillator, holding the key opens an envelope, and the keyboard's mod wheel can open a filter. It also lets the patch process outside sound, running a voice or a guitar through filters and spaces. The app only opens the audio input when there is a SIGNAL-IN in the patch.

## Controles
- GAIN: the level of the incoming audio, from zero to double.
- BEND: how many semitones the keyboard's pitch bend moves the pitch, from zero to two octaves.
- CC#: which keyboard controller the CC output follows. Number 1 is usually the mod wheel.
Outputs: L and R, the incoming audio; 1V/O, the pitch of the key played; GATE, open while the key is held; VEL, how hard it was played; CC, the value of the chosen controller.

## Experimente
1. With a microphone or instrument connected to the computer, connect L to a MIXER input and adjust GAIN until you hear the sound.
2. Run that sound through a FILTER or SPACE before the MIXER to transform it.
3. With a MIDI keyboard, connect 1V/O to the 1V/O input of an OSC, and GATE to the GATE input of an ENVELOPE that has the OSC's sound on its IN input. The keyboard now plays the patch.
