## O que é
DYNAMICS is a compressor. It holds down peaks so the sound sits closer together, and it lets one sound duck another, like the pad that breathes when the kick hits. Here it does a third thing: what it hears and how much it reduces come out as control signals, to move any other module.

## Como pensar nele
Think of a sound engineer with a hand on the fader: when the sound goes over the threshold, the hand pulls down; when the sound drops, it gives the level back. RATIO is the strength of the hand, ATK and REL are its haste. The SIDE input changes what it listens to: with a kick there, the kick decides when everything else ducks. Two Rasgo twists work on time: BRTH makes the release follow the interval between hits, and DRIFT varies attack and release on each hit.

## Controles
- THRES: the threshold, from −48 to 0 dB. Above it, the sound is turned down.
- RATIO: how much it turns down. At 4, every 4 dB above the threshold becomes 1; at 20, it becomes a limiter.
- KNEE: all at once (0) or gradually (up to 12 dB).
- ATK, REL: the time to go down (0.1 to 200 ms) and to come back (5 ms to 2 s).
- GAIN: gives back the lost level, up to 24 dB.
- SIDE: the detector listens to the IN input (0) or the SIDE input (1).
- RMS: measures average energy instead of peaks; it sounds smoother.
- GATE: inverts the curve into a noise gate, which closes what stays below the threshold.
- BRTH: the release follows the tempo of the hits it hears.
- DRIFT: seeded chance in attack and release, hit by hit.
Inputs: IN takes the sound, SIDE what the detector should hear, THR moves the threshold. Outputs: OUT is the compressed sound, ENV the level it hears, GR how much it is reducing right now.

## Experimente
1. Patch a TRIGSEQ into a DRUM and the DRUM into the SIDE input. Patch a CHORD into IN and OUT into the MIXER.
2. Leave SIDE at 1, RATIO at 6, ATK short and REL near 200 ms: the chord ducks on every kick.
3. Raise BRTH: the release starts to breathe with the rhythm.
4. Patch the ENV output into the CUT of a FILTER on another sound: the filter opens with the strength of what the compressor hears.
