## O que é
FUNCTION generates a ramp that rises and falls without stopping. Slowly, it moves other controls, which is called an LFO, a low-frequency oscillator; fast, it becomes an audible sound; triggered by a pulse, it works as a simple envelope. It is the most used source of movement in Rasgo Modular.

## Como pensar nele
Envelope, LFO and oscillator are the same thing at different speeds, and FUNCTION covers all of them, from one cycle every hundred seconds to thousands per second. When a guide tells you to connect an LFO somewhere, it almost always means a slow FUNCTION. The shape goes from a falling ramp to a rising one, passing through a triangle, and the two outputs give the same shape on different scales: UNI only above zero, BI above and below.

## Controles
- RATE: the speed, from 0.01 to 12000 cycles per second.
- SLOPE: the shape. At one end it rises suddenly and falls slowly; in the middle, a triangle; at the other end, it rises slowly and drops suddenly.
- DRIFT: varies the speed gradually, so the movement doesn't sound like a metronome.
- SYNC: enables the SYNC input, so each pulse restarts the ramp.
Inputs: RATE and SLOPE, to modulate those controls; SYNC, the pulse that restarts. Outputs: UNI and BI.

## Experimente
1. Build a voice: the SAW of an OSC into the IN input of a FILTER, the FILTER's LO into the MIXER. Lower CUT.
2. Connect FUNCTION's BI to the FILTER's FC input, with RATE low. The tone opens and closes slowly.
3. Raise RATE to about 5 Hz and connect BI to the OSC's FM input, with the OSC's FM knob very low. The pitch trembles, a vibrato.
4. Raise RATE into the audible range and connect BI straight to the MIXER: FUNCTION itself becomes sound.
