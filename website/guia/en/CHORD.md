## O que é
CHORD turns one note into a chord. You give it a pitch and it plays two to four voices tuned above it, in shapes that range from unison to chords with a ninth. In a modular, a chord normally takes several oscillators and quantizers; here one module is enough.

## Como pensar nele
Think of it as a hand on a keyboard that already knows the chord shape. The CHORD knob picks the shape, VOX how many notes it has, and INV how the notes are stacked. A slow wave on the CHRD input changes the shape by itself; the ROOT output of HARMONY on the PITCH input makes the root follow key changes. With VLEAD, each voice moves to the nearest note of the next chord, gliding, and the change sounds as smooth as a choir.

## Controles
- FREQ: the fundamental, from 16 to 4000 Hz.
- CHORD: the chord shape, from ten tables, from unison to chords with a ninth.
- VOX: how many voices sound, from two to four.
- INV: moves the lowest notes up an octave. It changes the look of the chord without changing the notes.
- VLEAD: on a chord change, makes each voice move to the nearest note, gliding. At zero they all jump at once.
- DTUNE: detunes the voices from one another. A little thickens the sound; a lot becomes a wall.
- WAVE: the waveform of the voices, from sawtooth to pulse and triangle.
- DRIFT: lets each voice wobble slightly in tuning, slowly.
Inputs: PITCH for the fundamental, CHRD to change the shape, and FM.

## Experimente
1. Connect OUT to a MIXER input and turn CHORD slowly. Hear the shapes go by.
2. Change VOX from 2 to 4 and the chord fills out.
3. Raise DTUNE a little: the voices drift apart and the sound gets wider.
4. Connect the CLK output of CLOCK to the CLK input of a SEQUENCE, and the PTCH output of the SEQUENCE to the PITCH input of CHORD. The chord now moves with the sequence.
