## O que é
CONTROL adjusts control signals before they reach their destination. It can shrink, enlarge or invert a signal, shift it up or down, smooth its steps, and add two signals together. These are modest operations, but they show up in almost every patch that goes beyond the basics.

## Como pensar nele
Sometimes a modulation has the right shape but the wrong size, or the wrong direction: an envelope that should close the filter instead of opening it, a wave that should swing only above zero. A cable's gain can make a signal smaller, but it cannot invert or shift it. CONTROL does all of that on one panel, with two identical channels and an output that combines them. With real audio on the input, it can also follow that sound's volume and turn it into a control signal, which is called an envelope follower.

## Controles
- SCALE: the size of the signal, on each channel. At 1 it passes unchanged; at 0.5, halved; negative, inverted. At zero only the OFF offset remains, and the channel becomes a manual control.
- OFF: a value added to the signal, shifting it up or down.
- RECT: folds the negative part of the signal upward. In the middle the negative part disappears; at maximum it is mirrored.
- SLEW: smooths sudden changes, over up to two seconds. Steps become ramps.
- CRV: the shape of that smoothing, from a straight ramp to a curve that slows down.
- SUM: whether the SUM output adds the two channels or averages them.
- DRIFT: a slow wander in the offsets, which takes the stiffness out of a fixed value.
Inputs: IN1 and IN2. Outputs: O1, O2 and SUM.

## Experimente
1. Build a voice: the SAW of an OSC into the IN input of a FILTER, and the FILTER's LO into the MIXER. Lower CUT.
2. Connect the ENV output of an ENVELOPE, played by CLOCK, to IN1 of CONTROL, and O1 to the FILTER's FC input. Each note opens the filter.
3. Set the first channel's SCALE to −1 and raise CUT. Now each note closes the filter.
4. Connect the BI output of a FUNCTION to IN2 and listen through the SUM output instead of O1: both modulations act together.
