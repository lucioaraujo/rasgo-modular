## O que é
LISTEN follows the energy of incoming sound in three regions: low, mid and high. Each region provides a continuous level and a pulse when it detects an attack. You can use these signals to make another voice respond to what is playing.

## Como pensar nele
A low drum hit can call a note; cymbals can change a voice's brightness. The regions overlap, so one instrument can appear at several outputs. LISTEN measures frequencies and changes in energy; it does not separate instruments or discover the music's tempo. THRU preserves the incoming sound so you can hear it too. The meters show levels and highlight recent attacks.

## Controles
- LOW: low/mid split, from 60 to 800 Hz. Moving either split inhibits attacks for 50 ms to avoid triggers caused by the adjustment. Continuous modulation can keep them inhibited.
- HIGH: mid/high split, from 1 to 8 kHz, limited by the sample rate.
- RESP: smooths the level outputs. Raise it to let energy linger after a hit. It does not change attack detection.
- SENS: attack sensitivity. Raise it to detect smaller contrasts; lower it if one hit causes too many events.
- FLOOR: minimum amplitude for triggering, from −72 to −18 dBFS. Raising it helps reject quiet noise.
- REARM: minimum wait between attacks in the same region, from 20 to 250 ms. A high setting can miss fast hits.
Input: IN, audio. Outputs: THRU, unchanged audio on each channel; LOW, MID and HIGH, levels from 0 to 1; L-HIT, M-HIT and H-HIT, 5 ms pulses. Automatic VARIA movement leaves the calibration controls alone.

## Experimente
1. Turn VARIA off. Connect CLOCK.EUC to DRUM.GATE, DRUM.OUT to LISTEN.IN and LISTEN.THRU to a MIXER input. Start at low volume and watch the meters.
2. Connect L-HIT to PHRASE.CLK. To hear the response, connect PHRASE.PITCH to QUANTIZER.CV, with RANGE=1; QUANTIZER.PITCH to OSC.PITCH; OSC.SAW to ENVELOPE.IN; PHRASE.GATE to ENVELOPE.GATE; ENVELOPE.OUT to MIXER.
3. Change TONE and SNAP on DRUM. Adjust FLOOR and REARM until the voice responds to the hits you want. Swap L-HIT for H-HIT and hear how the conversation changes.
4. For changing intensity, place a VCA between ENVELOPE and MIXER, with LVL1=0 and CV1=1; connect PHRASE.ACC to VCA.CV1 and LISTEN.LOW to PHRASE.STIM. For external audio, send SIGNAL-IN.OUT to LISTEN.IN and enable SIGNAL-IN's ON control yourself.
