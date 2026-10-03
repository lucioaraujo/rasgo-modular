## O que é
SH, short for sample and hold, captures the value of a signal at the moment of a pulse and holds it until the next pulse. With nothing on its input, it draws a new random value on each pulse. It is the traditional way of turning chance into steps: each pulse, a new note, a new tone. SH has two channels.

## Como pensar nele
Two independent draws have nothing to do with each other, and sometimes that is what you want. But a melody often sounds better when the tone follows the note without copying it. The CORR control sets exactly that relationship between the two channels: from mirrored, one rising as the other falls, to independent, to twins that draw the same value. The slew controls turn the steps into glides.

## Controles
- RATE: the internal draw rate, from 0.02 to 40 times per second. It only applies to a channel with no pulse on T1 or T2.
- SLW1, SLW2: how long each channel takes to glide to the new value. At zero it jumps instantly.
- SLOPE: makes the way up faster than the way down, or the other way round.
- TRK1, TRK2: instead of capturing only at the moment of the pulse, the channel follows the input while the pulse is high and holds when it falls.
- SPRD: changes the kind of draw, making small variations more frequent than large ones.
- CORR: the relationship between the two channels' draws: mirrored, independent or identical. It only applies to channels with nothing on their input.
Inputs: IN1 and IN2, the signals to capture; T1 and T2, the pulses. Outputs: O1 and O2.

## Experimente
1. Build a voice: the SAW of an OSC into the IN input of a FILTER, the FILTER's LO into the MIXER.
2. Connect the CLK output of CLOCK to T1 and to T2.
3. Connect O1 to the CV input of a QUANTIZER, the QUANTIZER's PTCH to the OSC's 1V/O, and O2 to the FILTER's FC input. Each pulse brings a note and a tone.
4. Turn CORR from one end to the other. On one side, high notes come out dark; in the middle, no relationship; on the other, high notes come out bright.
