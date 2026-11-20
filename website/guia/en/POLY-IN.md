## O que é
POLY-IN shares the notes of a MIDI keyboard or a DAW track out to up to four voices. Each voice has its own pitch, gate and velocity, and each can play its own oscillator.

## Como pensar nele
SIGNAL-IN gives a single voice; POLY-IN gives a chord. The rack takes up to four copies of each module: with four OSCs and four ENVELOPEs, each POLY-IN voice plays one pair, and the keyboard now plays chords in the patch. The module starts off and never asks for the microphone: it only opens MIDI.

## Controles
- VOZES: how many voices receive notes, from one to four.
- MODE: how the notes are shared out. *Rota* moves each note to the next free voice; *Mesma* gives a repeated note back to the voice it played on; *Altura* puts the voices in order, and voice 1 always holds the lowest note. With no free voice, the new note takes the oldest voice.
- BEND: how many semitones the pitch bend moves every voice.
- ON: turns the MIDI input on.
Outputs: P1 to P4, the pitch of each voice; G1 to G4, the gate of each voice, open while the key or the pedal holds the note; V1 to V4, how hard each note was played.

## Experimente
1. Turn ON on. Connect P1 to the 1V/O input of an OSC and G1 to the GATE input of an ENVELOPE that receives that OSC's sound.
2. Add another OSC and another ENVELOPE and connect P2 and G2 to them. Play two notes together: each one sounds on its own pair.
3. With four pairs and MODE on *Altura*, voice 1 is always the bass of the chord: give it a darker filter or more level on the MIXER.
