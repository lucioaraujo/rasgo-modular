## O que é
SUM joins signals. Four inputs, each with its own attenuverter, become a single output: the sum, the inverted sum and the average.

## Como pensar nele
In Rasgo Modular each input takes a single cable. For a filter to move with two motions at once, both go through SUM first. MULT copies one signal to many places; SUM joins many signals in one place.

## Controles
- A1 to A4: how much of each input goes into the sum, from −2 to +2. At the centre the input is left out; below the centre it comes in inverted.
- OFS: a fixed value added to the result.
- SAT: on, the sum saturates gently and never exceeds ±1, good for summing audio.
Inputs: IN1 to IN4. Outputs: SUM, the sum; INV, the inverted sum; AVG, the average of the connected inputs.

## Experimente
1. Connect a FUNCTION's BI to IN1 and a DRIFT output to IN2. Connect SUM to a FILTER's cutoff: the filter moves with both.
2. Turn A2 below the centre: DRIFT now pushes the filter the other way.
3. Connect two oscillators to IN1 and IN2, switch SAT on and send SUM to the MIXER: both play together without clipping.
