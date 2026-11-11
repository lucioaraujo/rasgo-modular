## O que é
BLEFE is a game of trust made of signals. Each trigger becomes a signal, which always sounds the same, truth or bluff. A window later, the truth is confirmed and the bluff is not. With each resolution, trust moves: it rises with truths and falls with bluffs. When trust that was built up collapses, that is betrayal, and the response refuses until forgiveness.

## Como pensar nele
A random number has no intention; a bluff does. The module does not draw notes, it creates a relationship between voices: a voice that answers while it trusts, hesitates when it doubts and falls silent once betrayed. VERD is the rate of truth, MEM how fast trust changes, LIM the point of betrayal. The RST input is forgiveness. SEED fixes the game: the same seed gives the same truths and lies.

## Controles
- VERD: the rate of truth, from 0 to 1.
- JAN: the window between the signal and the confirmation, from 0.05 to 4 s.
- MEM: how fast trust moves.
- LIM: below this level, trust that was built becomes betrayal.
- SEED: the seed of the game.
Inputs: IN takes the triggers, RST forgives and starts over. Outputs: SIG is the signal, always the same; VERD gives away the hidden truth at once, for whoever wants to cheat; CONF pulses when the truth is confirmed; CNF is the level of trust; RESP is the response, which goes silent after betrayal; TRAI pulses at the moment of betrayal.

## Experimente
1. Patch the CLOCK into IN, SIG into the gate of an ENVELOPE and CONF into a DRUM: the signal and the confirmation sound as two sounds.
2. Patch RESP into the CV1 of another voice's VCA: it answers while it trusts.
3. Start with VERD at 0.9 and then turn it to 0.2. The answering voice grows, hesitates and falls silent.
4. Patch the EOS of a SEQUENCE into RST: with each phrase, the relationship starts over.
