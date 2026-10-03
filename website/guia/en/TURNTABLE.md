## O que é
TURNTABLE plays a recording as if it were on a vinyl record. It records a stretch, like SAMPLER, but playback imitates a real platter: the record has weight, takes time to get up to speed, slows down gradually, and can be pushed back and forth, as in a DJ's scratch.

## Como pensar nele
SAMPLER jumps from one point to another instantly. TURNTABLE has inertia, and that is what gives it the vinyl sound: the pitch slides when the record speeds up or slows down. The SCR input is the hand on the record: a slow wave there makes the record go back and forth in time. The BRK input switches the motor off, and the sound drops until it stops, the effect known as tape stop. There is no automatic tempo sync: getting the rhythm right is part of the gesture.

## Controles
- SPEED: the speed the motor tries to reach, from half to double the original. Negative spins the record backwards.
- TORQ: the strength of the motor, that is, how quickly the record reaches speed. Low, the start has a long wavering of pitch; at minimum, the record moves only by hand, through the SCR input.
- FRIC: the friction. It decides how quickly the record stops when braking and how much it drifts back into rhythm on its own after a scratch.
- GRAB: how firm the hand is, how much the signal on SCR moves the record.
- START: where the needle drops on each pulse at TRIG.
- WEAR: wear on the record: crackles and small irregularities in rotation.
- LOOP: off, the record runs out and stops; on, playback wraps around and starts again.
Inputs: TRIG, to drop the needle again; IN, the sound to record; REC, the signal that records while it is on; SCR, the hand on the record; BRK, the brake. Output: OUT.

## Experimente
1. Record a phrase as with SAMPLER: the voice on IN and the DIV output of a LOGIC, with DIV at 16, on REC. Connect OUT to the MIXER and turn LOOP on.
2. Connect the BI output of a FUNCTION at about 2 Hz to SCR and raise GRAB to the middle. The record goes back and forth, a scratch.
3. Remove the cable from SCR. Connect the GATE output of a DECISION with low BIAS to BRK. Every now and then, the record brakes and spins up again.
