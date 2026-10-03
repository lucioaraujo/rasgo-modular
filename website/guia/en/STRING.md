## O que é
STRING simulates a string: plucked, as on a guitar; hammered, as on a piano; or bowed, as on a violin. It recreates the behavior of a string that vibrates and gradually loses energy, which is why the attack and body of the sound feel natural even though they are synthetic.

## Como pensar nele
MATTER is the module for objects that ring, like bells and plates; STRING is the one for strings. Its main gesture is POS, the point where the string is played: as that point moves, some harmonics disappear, just as when a guitarist plays closer to the bridge. A pulse on PLK plucks the string; a continuous sound on IN makes it sing as if bowed.

## Controles
- FREQ: the tuning of the string, from 20 to 4000 Hz.
- DECAY: how long the string rings after being plucked. Low is dry; high barely ends.
- DAMP: brightness. High makes the string dark, like an old string; low keeps the brightness of a new one.
- POS: the point where the string is played. Near the middle, the even harmonics vanish and the sound turns hollow.
- EXCIT: how much noise goes into the strike, like the sound of a finger or pick.
- DRIVE: saturates the string from inside, taking it from an acoustic sound to a distorted one.
- MIX: the blend between the raw strike and the string. At maximum, only the string.
Inputs: IN, a sound that makes the string vibrate; PLK, a pulse that plucks it; 1V/O, for tuning; DMP, to change the brightness.

## Experimente
1. Connect OUT to a MIXER input, and the EUC output of CLOCK to the PLK input. The string is plucked in time with the clock.
2. Turn DECAY: from dry notes to notes that run into each other.
3. Move POS slowly and hear the tone change with the playing point.
4. Connect the PTCH output of a SEQUENCE, driven by the same CLOCK, to the 1V/O input. Now the string plays a melodic line.
