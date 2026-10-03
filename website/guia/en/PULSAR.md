## O que é
PULSAR produces a train of tiny sound pulses, each followed by silence. When the pulses repeat quickly you hear a note; when they slow down you hear a rhythm. The technique comes from the composer Curtis Roads and has a curious property: pitch and tone are controlled separately, so you can change the color of the sound without detuning it.

## Como pensar nele
FREQ sets how many times per second the pulse repeats, and so the note. FRMT sets what happens inside each pulse, and so the tone: low sounds hollow, high sounds nasal. Bring FREQ down to around 30 repeats per second and each pulse becomes an event you hear on its own; with MASK and JITR, those events turn irregular, like a cloud.

## Controles
- FREQ: the pulse repetition rate, from 20 to 2000 Hz. This is the pitch of the note.
- FRMT: the frequency inside each pulse, from 0.1 to 8 times FREQ. This is the tone: low is hollow, high is bright.
- SHAPE: the shape of each pulse, from a plain sine to a narrower pulse rich in harmonics.
- WIND: the contour of each pulse. To the left the edges are hard and the sound is bright; in the middle it is clean; to the right each pulse attacks fast and decays, like percussion.
- JITR: makes the timing and level of each pulse irregular. At zero the train is rigid.
- MASK: the chance of each pulse being skipped. It opens gaps and creates rhythmic patterns without changing the note.
- SPRD: spreads alternate pulses between left and right, widening the sound in stereo.
- LEVEL: the output level.
Inputs: PIT for pitch and FQM to modulate the tone. Outputs L and R, for stereo.

## Experimente
1. Connect L to a MIXER input and turn FRMT slowly. The tone changes, but the note stays where it is.
2. Bring FREQ close to its minimum. The note falls apart into pulses you can hear one by one.
3. With FREQ low, raise MASK and JITR. The pulses become sparse and irregular, like rain.
4. Connect the BI output of a slow FUNCTION to the FQM input. The tone wanders on its own without going out of tune.
