## O que é
PLL is an oscillator that knows how to follow another. On its own it works like any oscillator, with its own pitch and a waveform you choose. When it receives a second signal on the REF input, it gradually adjusts its speed until it moves together with that signal, like a musician falling into step with another.

## Como pensar nele
OSC can also lock to another oscillator, but all at once (that is sync). PLL does it gradually, and that path towards locking is audible: a gliding pitch. With RATIO it locks an octave above, an octave below, or at deeper divisions of the reference. The LOCK output tells you how locked it is, and you can use it to make other things react.

## Controles
- FREQ: the pitch when there is no reference, and the starting point when there is.
- FINE: fine tuning, up to a semitone either way.
- SHAPE: the waveform, moving from sine to triangle, sawtooth and square.
- RATIO: the relationship to the reference it locks to. At 1, the same note; at 2, an octave above; near the minimum, far below.
- LOCK: how hard it chases the reference. Low, it glides slowly until it arrives; high, it locks quickly but gets more jittery.
- FM: how much the signal on the FM input moves the pitch.
- FBK: how much of its own signal goes back into the circuit, adding texture without detuning.
- FTYP: picks one of six feedback types, each with a different color.
Inputs: 1V/O for pitch, FM, and REF, the reference to follow. Outputs: OUT, RING (the PLL multiplied by the reference) and LOCK, a control signal showing how locked it is.

## Experimente
1. Connect OUT to a MIXER input. With nothing on REF it is an ordinary oscillator; turn SHAPE to hear the waveforms.
2. Connect the SAW output of an OSC to the REF input. PLL searches for the OSC's pitch and locks onto it.
3. Lower LOCK. Now it takes its time getting there, and you hear the pitch glide.
4. Set RATIO to 2. It locks an octave above the OSC.
