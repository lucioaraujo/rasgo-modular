## O que é
SWARM is a school of six fish, and each fish is a clock with its own tempo. They listen to and pull on one another. Four of them come out at G1 to G4, and the pulse of the whole school at ALL.

## Como pensar nele
It is the Kuramoto model, the same one behind the frogs that synchronise in the Aquorbium's phase chorus and the fireflies that flash together. A single control, COHES, goes from loose to tight: at 0, each fish plays in its own time, like a heterophony; at 1, all on the same pulse, like a homorhythm; in between, the school forms and breaks up. SYNC measures how together they are.

## Controles
- RATE: the average tempo.
- SPRD: how much the natural tempos differ.
- COHES: the cohesion of the school.
- GATE: the length of each gate.
Inputs: SCAR is the scare that scatters the school, LEAD is an external leader that everyone follows, COH adds to COHES. Outputs: G1 to G4 are the pulses of four fish, ALL the pulse of the school, SYNC the measured cohesion.

## Experimente
1. Patch G1 to G4 into four DRUM with COHES at 0: four loose tempos. Raise COHES slowly and hear the school form.
2. Patch a rare trigger into SCAR: the school scatters and regroups.
3. Patch a CLOCK into LEAD: the school starts to follow the leader.
4. Patch SYNC into the level of a VCA: the sound grows when the school comes together.
