## O que é
OPERATOR is an FM synthesizer with four sine oscillators, called operators. In FM, an operator does not sound on its own: it pushes another operator's frequency back and forth very fast, and new harmonics are born from that. This is how electric pianos, bells, snappy basses and metallic tones are made, as on the digital synthesizers of the 1980s.

## Como pensar nele
Three decisions make the tone: who modulates whom (ALGO), the frequency ratio between operators (RB, RC, RD), and the strength of the modulation (INDEX). Start with INDEX low and raise it slowly: the sound goes from a clean sine to something bright and harsh. The most useful move is to connect an envelope to the IDX input, so each note starts bright and darkens, like an electric piano key.

## Controles
- FREQ: the pitch of the note, from 8 to 8000 Hz.
- FINE: fine tuning, up to a semitone either way.
- ALGO: picks one of eight arrangements of who modulates whom. At the start the operators are chained and the sound is harsher; at the end they sound side by side, closer to an organ.
- RB, RC, RD: each operator's frequency ratio to the first. Whole numbers sound in tune; fractional values such as 2.5 sound like bells; high values, like metal.
- INDEX: the strength of the modulation. At zero you hear only pure sines; at maximum, a very bright tone.
- FBK: makes the first operator modulate itself. On its own it already turns the sine into something close to a sawtooth.
- DRIFT: detunes each operator a tiny bit, slowly. At zero everything stays stable.
Inputs: 1V/O for pitch and IDX to modulate the FM strength.

## Experimente
1. Connect OUT to a MIXER input with INDEX at zero: a plain sine.
2. Raise INDEX slowly and hear the harmonics appear.
3. Set RB to a fractional value and the sound becomes a bell.
4. Connect the ENV output of an ENVELOPE to the IDX input, and the CLK output of CLOCK to the GATE input of the ENVELOPE. Each note opens bright and fades, like an electric piano.
