## O que é
PRESSURE is a box of gas. The particles move in straight lines and bounce off the walls. The right wall is a piston, and each hit on it gives a gate at HIT. The average force of the hits is the pressure, which comes out at PRES.

## Como pensar nele
It is the kinetic theory of gases, from Maxwell and Boltzmann. More heat, the particles run faster and hit more. Less volume, they hit more often, because the wall is closer: the pressure rises, as in Boyle's law. With a positive ATTR, the gas becomes a real gas: the particles attract each other and, when strong, condense into a droplet, and the pressure drops.

## Controles
- N: how many particles are in the box.
- TEMP: the temperature, the mean speed.
- VOL: the volume; the position of the piston.
- ATTR: attraction (positive) or repulsion (negative) between the particles.
- GATE: the length of each gate.
Inputs: HEAT gives a burst of heat to all of them, VOL and TEMP move volume and temperature by CV. Outputs: HIT is the gate of the hits on the piston, PRES the pressure, TEMP the measured temperature, X and Y the position of one particle.

## Experimente
1. Patch HIT into a DRUM with N at 3: a sparse, irregular rhythm. Raise N to 20: it becomes a hiss of collisions.
2. Patch a slow LFO into VOL: when the piston compresses, the rhythm speeds up and PRES rises.
3. Patch X and Y into the PAN and the cutoff of a FILTER: the particle draws the timbre.
4. Raise ATTR slowly: the gas condenses, the hits thin out and the pressure drops.
