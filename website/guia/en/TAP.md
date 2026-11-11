## O que é
TAP gives the patch the tempo of whoever is playing. Tap three or more times on the TAP input (a button, a key, handclaps through SIGNAL-IN): it measures the interval and starts giving a pulse on every beat, by itself, at your tempo. The first tap is the downbeat, the one.

## Como pensar nele
It is the count-in before the band comes in. TAP uses the median of the taps, so one crooked tap does not spoil the count. If the taps are too irregular, it does not invent a tempo: it waits for a better count. A pause and a new count change the tempo.

## Controles
- COMP: how many beats the bar has, from 1 to 16. The UM output comes back every COMP beats.
- DISP: how much irregularity it accepts in the count.
Inputs: TAP takes the taps, RST forgets the tempo. Outputs: CLK is the pulse on each beat, UM the downbeat, BPM the tempo as CV, TRAV a gate while there is a tempo.

## Experimente
1. Patch SIGNAL-IN into SCOPE and the ONS output of SCOPE into TAP. Turn on the ON of SIGNAL-IN.
2. Patch CLK into the external clock input of CLOCK: the whole patch follows your pulse.
3. Tap one, two, three, four and start playing: the patch is already at your tempo.
4. Set COMP to 3 and patch UM into the RST of a SEQUENCE: the phrase restarts on every waltz bar.
