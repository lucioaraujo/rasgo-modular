## O que é
DRIFT produces very slow movements, on the scale of minutes. Connected to a few controls in a patch, it makes the music change gradually on its own: the tone at thirty seconds is different from the tone at four minutes, with no jumps and no return to the same point.

## Como pensar nele
A patch nobody is touching tends to go round in circles after a while. DRIFT is the layer of slow development. Inside there is a single value wandering, and the four outputs A, B, C and D are versions of that same walk, each with its own weight. So they tell the same story from different angles: when one rises, the others tend to rise too. The walk also has inertia: once it picks a direction, it tends to keep it for a while.

## Controles
- RATE: the pace of the walk, from one step per second to one every eight minutes or so.
- DEPTH: the reach, how far the walk can stray from its resting point.
- MOMT: the inertia. High, the path is smooth and keeps a direction; low, it changes course constantly.
- STRD: at zero the four outputs move almost together; at maximum each goes its own way.
- ANCHR: the memory. At zero the walk never returns; high, it tends to come back to places it has been, which gives the piece something like recurring themes.
- BIAS: moves the resting point up or down.
Inputs: ADV, a pulse that forces a step; RATE, to modulate the speed. Outputs: A, B, C and D, the four aspects of the walk; FLD, the unweighted walk; EVT, a pulse on each step.

## Experimente
1. Build any voice that goes through a FILTER and a SPACE before the MIXER.
2. Connect A to the FILTER's FC input and B to the SPACE's FBK input. Raise DEPTH.
3. Let it play for a few minutes. The tone and the space change together, unhurried.
4. To follow the shape of the music, connect the EOS output of a SEQUENCE to ADV: the walk takes a step at the end of each phrase.
