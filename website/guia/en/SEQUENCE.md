## O que é
SEQUENCE plays a short phrase you write: up to eight notes, each with its own pitch, each on or silent. On every pulse at the CLK input it moves one step, sending the pitch out of the PTCH output and the note's pulse out of the GATE output. It is for a bass line, a riff, a repeating figure.

## Como pensar nele
TURING lets a phrase emerge from chance; SEQUENCE starts from a phrase you decide. The variation comes from the way it reads: forwards, backwards, back and forth, in random order, or wandering from one step to its neighbor. The same eight-note phrase yields a lot just by changing MODE and LEN. To make the pitches land on the notes of a scale, run PTCH through a QUANTIZER before the oscillator.

## Controles
- LEN: how many of the eight steps are in the phrase.
- MODE: the reading order: forwards, backwards, back and forth, random, or wandering step by step.
- RATE: the internal clock, used when nothing reaches CLK.
- GATE: how long each note stays on within its step. Short sounds detached; long, connected.
- GLIDE: makes the pitch slide from one note to the next.
- RANGE: how many octaves the eight values cover, up to two.
- P1 to P8: the pitch of each step.
- G1 to G8: turns each step on or silences it.
Inputs: CLK, the pulse that advances; RST, to go back to the first step. Outputs: PTCH, the pitch; GATE, the note's pulse; EOS, a pulse at the end of each phrase.

## Experimente
1. Connect the CLK output of CLOCK to SEQUENCE's CLK.
2. Connect PTCH to the CV input of a QUANTIZER, and the QUANTIZER's PTCH to the 1V/O of an OSC. Run the OSC through an ENVELOPE to the MIXER and connect SEQUENCE's GATE to the envelope's GATE.
3. Adjust P1 to P8 until you like the phrase, and turn some steps off with G1 to G8.
4. Change MODE and hear the same phrase read in other ways.
