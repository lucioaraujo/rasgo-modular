## O que é
VCA controls the volume of one sound with another signal. It has two identical channels: the sound comes into IN, a control signal comes into CV, and the output volume rises and falls with that control. It is the most used module in any modular.

## Como pensar nele
Think of it as a hand on a volume knob, moved by another module. With an envelope on the control, each note gets a beginning, a middle and an end. With a slow wave, the volume undulates, an effect called tremolo. It also works for scaling a control signal before sending it elsewhere, and the SUM output adds both channels, like a small mixer. Note that the level starts at zero, so the channel stays silent until something reaches CV or you raise LVL.

## Controles
- LVL1, LVL2: the volume of each channel. The control signal adds to this value.
- CV1, CV2: how much the control signal acts, and in which direction. Negative inverts: the volume drops as the control rises.
- RSP1, RSP2: the response curve. At zero, linear; at maximum, exponential, which sounds more natural for volume.
- DRIFT: a small, slow wobble in both volumes.
Inputs: IN1 and IN2, the sounds; CV1 and CV2, the controls. Outputs: O1, O2 and SUM, the two added together.

## Experimente
1. Connect the SAW output of an OSC to IN1, and O1 to a MIXER input. Silence: LVL1 is at zero.
2. Raise LVL1 slowly and the sound appears.
3. Bring LVL1 back to zero and connect the ENV output of an ENVELOPE to CV1, with CLOCK on the envelope's GATE input. The sound now pulses with the notes.
4. Replace the envelope with the UNI output of a slow FUNCTION: the volume undulates, a tremolo.
