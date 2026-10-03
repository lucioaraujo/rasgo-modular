## O que é
NOTE-OUT writes down the notes of a voice. Placed in the path of the pitch and trigger signals, it logs each note that passes, with pitch, length, intensity and accent, in the score the app records alongside the audio. It has no controls and doesn't change the sound.

## Como pensar nele
In a generative patch, notes come and go. NOTE-OUT keeps a record of them for anyone who wants to study, transcribe or rework the piece later. It sits in the middle of the path: the pitch comes in on PITCH and goes out unchanged on PTHR; the pulse comes in on GATE and goes out unchanged on GTHR. When you record with REC, the text file that accompanies the audio includes those notes.

## Controles
NOTE-OUT has no controls.
Inputs: GATE, the pulse for each note; PITCH, the pitch; VEL, the intensity; ACC, the accent. Outputs: GTHR and PTHR, the same signals, to continue on to the voice.

## Experimente
1. In a melody made with QUANTIZER, OSC and ENVELOPE, connect QUANTIZER's PTCH to NOTE-OUT's PITCH, and PTHR to the OSC's 1V/O.
2. Connect the pulse that fires the envelope to NOTE-OUT's GATE, and GTHR to the envelope's GATE. The sound stays the same.
3. Record a stretch with REC. Next to the audio file, the score's text file lists every note played.
