## O que é
SPECTRA listens to a sound, finds its strongest frequencies, and plays them again with a set of sine oscillators. The result is a kind of shadow of the original: recognizable, but made only of pure tones, and you can transpose it, bend it, or freeze it.

## Como pensar nele
It sits among the source modules because its output is what you hear, but it almost always needs something on the IN input: a voice, a chord, a drum, or outside sound through SIGNAL-IN. With few voices it draws a caricature of the sound; with many, a faithful copy. Its strongest move is FRZ: it freezes the last spectrum and keeps it ringing indefinitely, like a pad lifted from any moment.

## Controles
- VOICE: how many sines the module uses, from 2 to 24. Few simplify the sound; many reproduce it faithfully.
- BLUR: how quickly each sine follows the incoming sound. At zero it tracks closely; at maximum it drags and smears, and the sound seems to melt.
- SHIFT: transposes the reconstruction up to two octaves up or down, without touching the listening.
- STRCH: pushes the frequencies apart or together, taking the sound towards metal or bell without changing the perceived pitch.
- TONE: darkens or brings out the highs of the reconstruction. In the middle it stays true to what was heard.
- JITR: makes each sine wobble slightly, so the reconstruction never sounds static.
- FRZ: freezes the listening. The sines keep playing the last spectrum.
- MIX: the blend between the original sound and the reconstruction. At maximum you hear only the reconstruction.
Inputs: IN for the sound to analyze, PIT to transpose and FRZ to freeze with a gate. Outputs L and R.

## Experimente
1. Connect the OUT output of CHORD to the IN input of SPECTRA, and the L output to a MIXER input. You hear the chord rebuilt in pure tones.
2. Lower VOICE to 3 or 4. The chord becomes a sketch of itself.
3. Move STRCH to one side and hear the chord turn metallic.
4. Connect the EUC output of CLOCK to the FRZ input. The sound freezes and lets go in time with the clock.
