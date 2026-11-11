## O que é
BOUNCE drops balls that bounce, lower and faster each time, until they stop. There are up to three balls. Each bounce is a gate, the force of the bounce is an accent, and the height of each ball is a CV that rises and falls.

## Como pensar nele
It is a rhythm that speeds up by itself, like a ping-pong ball dropped on a table. ELAST decides how much the ball gives back on each bounce, GRAV is its haste. With AUTO, the balls that stop are thrown again, and WIND nudges them gently, so the module plays forever without repeating.

## Controles
- BALLS: how many balls, from 1 to 3.
- GRAV: gravity. Higher, faster bounces.
- ELAST: how much each bounce gives back. Near 1, the ball takes long to stop.
- SPRD: how much the balls are dropped from different heights.
- WIND: a light wind that prevents exact repeats.
- AUTO: throws again the balls that have stopped.
Inputs: DROP drops the balls again, GRAV moves gravity by CV. Outputs: GT is the gate of each bounce, ACC the force of the bounce, H1, H2 and H3 the height of each ball.

## Experimente
1. Patch GT into a DRUM and the DRUM into the MIXER: hear the bouncing speed up until it stops.
2. Patch ACC into the level of the DRUM: the first bounces sound stronger.
3. Patch H1 into a QUANTIZER and the QUANTIZER into an OSC: a falling melody.
4. Patch GT into the CLK of an ARP: the arpeggio speeds up with the ball.
