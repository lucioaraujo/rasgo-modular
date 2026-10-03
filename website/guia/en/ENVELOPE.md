## O que é
ENVELOPE shapes a note over time: how it starts, how it falls, how much it sustains and how it fades. It receives a pulse on the GATE input and draws a curve for each pulse. It has a volume control built in, so passing a sound through it is enough to hear notes instead of a continuous sound.

## Como pensar nele
Sources sound all the time; ENVELOPE turns that flow into phrases. It has two uses that can happen at once. Through the IN input and OUT output, it articulates the sound that passes through. Through the ENV output, it hands you the curve itself, which you can send to open a filter, change a tone or dose an effect. The four stages of the curve have names that appear on every synthesizer: attack, decay, sustain and release, hence ADSR.

## Controles
- ATK: the attack, the time the note takes to reach its peak after the pulse. Short sounds percussive; long, like a bow coming in slowly.
- DEC: the decay, the time it takes to fall from the peak down to the SUS level.
- SUS: the level the note holds while the pulse stays high. At zero there is no sustain and the note sounds plucked.
- REL: the time the note takes to fade after the pulse ends.
- CURVE: the shape of the curves, from softer to punchier, with the attack felt earlier.
- TRIG: in one position, the note holds as long as the pulse lasts; in the other, each pulse fires the whole curve without waiting.
- VCA: how much the built-in volume acts on the sound passing through. At zero the sound goes through unchanged and only the ENV output matters.
- LVL: the output volume.
Inputs: IN, the sound; GATE, the pulse that fires the note; TIME, to speed up or slow down all stages. Outputs: OUT, the articulated sound; ENV, the curve.

## Experimente
1. Connect the SAW output of an OSC to the IN input, OUT to a MIXER input, and the EUC output of CLOCK to GATE. You hear notes.
2. Raise ATK and hear each note come in slowly. Bring it back and play with DEC to make the notes short or long.
3. Put a FILTER between the OSC and the ENVELOPE, with CUT low, and connect ENV to the filter's FC input. Now each note also opens the tone.
