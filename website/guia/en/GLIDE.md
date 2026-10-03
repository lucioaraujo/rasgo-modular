## O que é
GLIDE makes one note slide into the next instead of jumping. It sits in the path of the pitch signal, between whatever chooses the notes, such as SEQUENCE or QUANTIZER, and the oscillator. That slide, called portamento, is what gives acid bass lines and legato leads their singing quality.

## Como pensar nele
A slide that is always on gets tiresome fast. The interesting part is choosing when to slide, and GLIDE decides note by note: it can always slide, only when the SLIDE input is high, or only when notes are tied, that is, when the new note arrives before the previous one is released. The MOV and DONE outputs tell you when a slide is happening and when it ends, and can trigger other things.

## Controles
- TIME: how long it takes to rise an octave, from zero, a clean jump, to two seconds.
- FALL: makes the way down faster or slower than the way up. In the middle they are equal.
- CURVE: the shape of the slide. At zero the speed is constant and the note arrives exactly on time; at maximum it slows down as it approaches and takes longer to settle.
- MODE: when to slide: always; only with SLIDE high, as on the TB-303 bass synthesizer; or only between tied notes, reading the GATE input.
Inputs: PITCH, the incoming pitch; SLIDE and GATE, which decide when to slide. Outputs: OUT, the shaped pitch; MOV, high during a slide; DONE, a pulse when it ends.

## Experimente
1. Build a line: the CLK output of CLOCK into the CLK input of a SEQUENCE, and the SAW output of an OSC into the MIXER.
2. Connect the SEQUENCE's PTCH to GLIDE's PITCH, and GLIDE's OUT to the OSC's 1V/O.
3. Raise TIME: the notes start sliding into one another.
4. Set MODE to the third position and connect the SEQUENCE's GATE to GLIDE's GATE. Now only tied notes slide.
