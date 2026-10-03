## O que é
OSC is an oscillator: it produces a steady, unchanging tone, the raw material almost every synthesizer sound is built from. It gives you five waveforms at once, each on its own output jack: sine, triangle, sawtooth, pulse, and a voice one or two octaves below (the sub). Pick one, or use several.

## Como pensar nele
Think of OSC as raw material. On its own it sounds plain, and that is on purpose: the character comes from what you connect after it, such as a filter that takes away brightness or an envelope that shapes each note. If you want a voice with a personality of its own, STRING and MATTER already have one; if you want a tone that changes by itself, try WAVETABLE. OSC is the predictable foundation, which makes it the best place to learn.

## Controles
- FREQ: the pitch of the note, from 8 to 8000 Hz. Low values become a slow vibration; the musical range sits roughly between 50 and 1000 Hz.
- FINE: fine tuning, up to a semitone up or down. Use it to tune against another voice, or to detune slightly and thicken the sound.
- PW: the pulse width, which only affects the PLS output. In the middle the wave is square and sounds hollow; near the ends it gets thin and nasal.
- FM: how much the signal on the FM input moves the pitch. With another oscillator there and this knob turned up, the tone turns metallic, like a bell.
- DRIFT: a small, slow wobble in the tuning, like an analog oscillator. At zero the note stays perfectly still.
- SUB2: sets whether the SUB output sits one octave (off) or two octaves (on) below the note.
- SYNC: enables the SYNC input. With it on, another, slower oscillator forces this one to restart its cycle, and turning FREQ changes the tone instead of the pitch.
- PROX: blends each output with a darker version of itself. A way to soften the sound without using up a filter.
Inputs: 1V/O takes pitch from a sequencer or quantizer, FM and PWM accept modulation, and SYNC takes the oscillator that drives the sync.

## Experimente
1. Connect the SAW output of OSC to a MIXER input and listen to a steady buzz.
2. Turn FREQ slowly: the note goes up and down.
3. Move the cable to the PLS output and sweep PW from one side to the other. The tone goes from hollow to thin while the pitch stays put.
4. Go back to SAW and run the sound through a FILTER before the MIXER. You are now doing subtractive synthesis: starting from a rich sound and taking away what you don't need.
