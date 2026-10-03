## O que é
FILTER lets part of a sound's frequencies through and cuts the rest. It has three outputs working at the same time: LO, which passes the lows; CTR, the middle band; and HI, the highs. As you turn the cutoff, you hear the sound darken or brighten, like covering your mouth while you speak.

## Como pensar nele
It is the second module in almost every patch: after a source rich in harmonics, such as an OSC saw, the filter sculpts the tone. What sets it apart is SPRD: at zero, all three outputs share the same cutoff; as you raise it, they move apart and become three different filters, each on its own region of the sound. With resonance at maximum, the filter starts sounding by itself, like a sine oscillator.

## Controles
- CUT: the cutoff frequency, from 20 to 20000 Hz. This is the main control: turn it and hear the tone open and close.
- RESO: emphasizes frequencies near the cutoff, giving a more nasal tone. At maximum the filter starts sounding on its own.
- SPRD: spreads the three outputs apart in frequency. At zero all three follow the same cutoff; at maximum each sits in its own region of the sound.
- DRIVE: saturates the signal before filtering, fattening the sound.
Inputs: IN, the sound to filter; FC, to move the cutoff (1 V per octave); Q, to move the resonance; SPR, to move the spread. Outputs: LO, CTR, HI and ALL, which adds the three together.

## Experimente
1. Connect the SAW output of an OSC to the IN input, and the ALL output to a MIXER input.
2. Turn CUT slowly from one side to the other and hear the brightness come and go.
3. Raise RESO halfway and do it again: the cutoff becomes pronounced, almost a voice.
4. Connect the ENV output of an ENVELOPE to the FC input, with CLOCK on the envelope's GATE input. The filter opens on every pulse, a sound heard in a great deal of electronic music.
