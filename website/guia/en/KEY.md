## O que é
KEY listens to whoever is playing, estimates the key and hands the tonic and scale to QUANTIZER. The patch then answers in the key of whoever sings or plays over it, instead of staying in the scale the seed chose.

## Como pensar nele
It counts how long each of the twelve notes has sounded (a long note weighs more than a passing one) and compares that picture with the Krumhansl and Kessler key profiles. The key only changes when another wins for ESTAB notes in a row, so it does not flicker between neighbouring keys. The CERT output says when the key is ambiguous, instead of hiding it. The display shows the twelve notes, the tonic and the key name.

## Controles
- MEM: how long it remembers, from 2 to 60 s. Short follows modulations; long stays firm.
- ESTAB: how many notes in a row a new key must win to count.
Inputs: PTCH takes the pitch, GATE marks each note, RST forgets everything. Outputs: ROOT is the tonic, SCL the scale, CERT the certainty, MUD a pulse on each key change.

## Experimente
1. Patch SIGNAL-IN into SCOPE, the PIT output of SCOPE into PTCH and the ONS output into GATE. Patch ROOT and SCL into the tonic and scale inputs of QUANTIZER.
2. Turn on the ON of SIGNAL-IN and sing a melody in D minor: the display shows Dm and the patch starts playing in that key.
3. Patch MUD into the FRASE input of the MIXER, with the stage in gesture mode: each key change moves the stage.
4. Without a microphone: a MARKOV into a QUANTIZER, that QUANTIZER into KEY and KEY into a second QUANTIZER. The second voice follows the key the first one seems to be in.
