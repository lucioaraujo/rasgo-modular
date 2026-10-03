## O que é
LPG, short for low pass gate, is a gate that opens and closes the sound on every strike, lowering the volume and darkening the tone at the same time. It imitates an old component, the vactrol, which reacts fast but lets go slowly, so every note sounds struck: a marimba, a kalimba, a drop of water.

## Como pensar nele
You could build something similar with FILTER, VCA and ENVELOPE, but you would miss the vactrol's way of moving: an instant rise and a fall that slows down near the end. LPG has it built in. All it needs is a continuous sound on IN and pulses on STRK: each pulse becomes a played note. MODE chooses whether it acts more like a filter, more like a volume control, or both.

## Controles
- MODE: to the left, filter only; to the right, volume only; in the middle, both together, which is the characteristic LPG sound.
- RESP: how long the note takes to die after the strike, from a very short tap to about two and a half seconds. The rise is always fast.
- OFST: how open the gate stays at rest. At zero it closes completely between strikes.
- RESO: the filter's resonance, which gives a more pronounced tone.
- BNCE: adds a small rebound right after each strike.
- DRIFT: varies the length of each note slightly and slowly.
Inputs: IN, the sound; STRK, the pulse that strikes; CV, to open the gate with a continuous signal.

## Experimente
1. Connect the TRI output of an OSC to the IN input, and OUT to a MIXER input. Silence for now: the gate is closed.
2. Connect the EUC output of CLOCK to the STRK input. Each pulse becomes a note with a struck tone.
3. Turn RESP to hear the notes get short or long.
4. Connect the PTCH output of a SEQUENCE, driven by the same CLOCK, to the OSC's 1V/O input. Now you have a marimba line.
