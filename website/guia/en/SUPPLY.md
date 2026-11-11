## O que é
SUPPLY is the patch's power supply, put into a module with cables. In a circuit, when the battery weakens, everything changes together: the clock slows, the sound loses body and the oscillators drift out of tune with each other. SUPPLY brings that common cause into the patch.

## Como pensar nele
ENERG is the supply voltage. The load, the patch's own sound, makes that voltage sag, like a battery under strain: the more the patch sounds, the further the rail sinks. What is left comes out at TRILHO, to patch wherever you like. The drifts D1 to D4 only appear when the rail is low and make the voices drift apart; with a full rail, they come back together. Nothing is hidden: SUPPLY only acts where there is a cable.

## Controles
- ENERG: the supply voltage.
- QUEDA: how much the load makes the rail sag.
- TEMPO: how long the rail takes to sag and recover, from 0.05 to 5 s.
- DERIVA: how far the voices drift apart when the rail is low.
Inputs: CARGA takes the patch's sound, ENERG moves the voltage by CV. Outputs: TRILHO is the remaining voltage, D1 to D4 four tuning drifts (up to 60 cents), FALHA a pulse at the blackout and another at the recovery.

## Experimente
1. Patch the MIXER output into CARGA and TRILHO into the CV1 of a voice's VCA: the patch loses body when it plays loud.
2. Patch D1 into the 1V/O of one OSC and D2 into another's. Lower ENERG: the voices go out of tune with each other.
3. Raise ENERG again: they come back together.
4. Patch FALHA into the CIT of APOCRIFO: the blackout tears through time.
