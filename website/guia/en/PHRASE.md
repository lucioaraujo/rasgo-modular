## O que é
PHRASE proposes a motif and plays it when pulses arrive at CLK. The idea returns with each phrase, with changes of direction, rests and accents that you control. Its outputs drive other voices; connect an oscillator and an envelope to hear it.

## Como pensar nele
Use CLOCK for a regular pulse or LISTEN's attacks to respond to another sound. SEED identifies the motif. VAR transforms successive passes; at zero, the returning shape is easier to recognise. The PITCH input supplies an anchor, sampled at the start of each phrase. To choose a scale, send the PITCH output through QUANTIZER with RANGE=1. PHRASE does not supply a repertoire of musical styles or traditions on its own.

## Controles
- LEN: phrase length in events, from 2 to 16 pulses. This is not a fixed duration in seconds.
- SPAN: the shape's pitch span, from 1 to 36 semitones, before quantisation.
- DIR: leans the shape down or up. At the centre it keeps the base motif's direction.
- VAR: how much each pass changes. Start at zero and raise it while listening for what remains recognisable.
- SPACE: rest probability on intermediate steps. The beginning and ending are preserved.
- CAD: strength of the return to the anchor at the ending.
- GATE: articulation length, from 5 to 400 ms. Connect GATE to the voice's envelope to hear articulated notes.
- SEED: numeric motif identity, from 1 to 65535. Changing it rebuilds the shape and restarts; VARIA leaves this control alone.
Inputs: PITCH, anchor at 1 V per octave; CLK, one step per rising edge; RST, restart; STIM, intensity from 0 to 1; RET, request closure on the next step. Outputs: PITCH, held pitch; GATE, articulation; ACC, accent from 0 to 1; END, 5 ms pulse at closure; PHASE, position from 0 to 1 within the phrase. With STIM unpatched, intensity is 1. Reopening the patch restarts the phrase rather than resuming the exact previous performance position.

## Experimente
1. Turn VARIA off. Connect CLOCK.EUC to PHRASE.CLK, PHRASE.PITCH to QUANTIZER.CV (RANGE=1), and QUANTIZER.PITCH to OSC.PITCH. Choose a scale on QUANTIZER.
2. Connect OSC.SAW to ENVELOPE.IN, PHRASE.GATE to ENVELOPE.GATE and ENVELOPE.OUT to MIXER. Start with VAR=0 and SPACE=0. Hear the idea repeat; change DIR and SPAN one at a time.
3. Put a VCA between the voice and MIXER: ENVELOPE.OUT into VCA.IN1, VCA.O1 into MIXER, LVL1=0 and CV1=1. Connect PHRASE.ACC to VCA.CV1 to hear the accents. Raise VAR, then SPACE.
4. Replace CLOCK with LISTEN.L-HIT at CLK and connect LISTEN.LOW to STIM. The sound being heard now calls the notes and changes their intensity. A pulse at RET requests closure on the next CLK; END can call a second voice.
