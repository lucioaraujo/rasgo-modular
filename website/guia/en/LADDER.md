## O que é
LADDER is the ladder filter, the bass filter: four 6 dB stages in cascade, with the output of the last one fed back to the input as resonance. It is the design R. Moog patented in 1969, here with a knob that goes from the transistor ladder, round, to the diode ladder, acid.

## Como pensar nele
FILTER is clean and stable; LADDER is the filter that compresses and thickens when pushed. Three things set it apart. DIODO couples the stages, and halfway there are ladders that do not exist as circuits. The peak stays tuned to CUT at any DIODO setting, and at RESO 1 the ladder sings a tuned sine. And COMP gives back the bass that, in the original ladder, disappears as resonance rises.

## Controles
- CUT: the cutoff frequency, from 20 Hz to 20 kHz.
- RESO: the resonance. At 1, the ladder self-oscillates; 0.9 means the same closeness to the threshold at any DIODO.
- DRIVE: pushes the input into the saturation inside the loop; the output level is compensated.
- COMP: gives back the bass. At 1, the bass stays whole at any resonance; at 0, it sinks as in the original.
- DIODO: from 0 (transistors, round) to 1 (diodes, acid), continuously.
Inputs: IN takes the sound, FC moves the cutoff at one volt per octave, Q moves the resonance. Outputs: LP4 is the 24 dB per octave output, LP2 the tap after the second stage, 12 dB per octave, brighter.

## Experimente
1. Patch the SAW output of an OSC into IN, LP4 into a VCA and the VCA into the MIXER. Set CUT near 200 Hz.
2. Raise RESO with COMP at 1: the peak sings and the bass stays. Lower COMP and hear the bass sink.
3. Set DIODO to 1, RESO to 0.85 and DRIVE to 0.6, with a short envelope on FC: the acid bass.
4. Unpatch IN, set RESO to 1 and patch the pitch of a QUANTIZER into FC: the ladder plays the melody like an oscillator.
