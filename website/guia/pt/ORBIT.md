## O que é
O ORBIT é um pequeno sistema solar: um sol no centro e dois ou três planetas em gravidade. As posições dos planetas saem como CV, e os encontros entre eles saem como pulsos.

## Como pensar nele
São as leis de Newton e de Kepler tocando o patch. Órbitas mais afastadas demoram mais, então os planetas andam em ritmos que quase se alinham e se desencontram. MASS faz os planetas se puxarem uns aos outros: baixa, as órbitas são regulares; alta, elas se desfazem em caos. Um planeta que escapa volta à sua órbita, então o sistema nunca se perde. LIGHT atrasa o pulso de conjunção pela distância, como a luz que demora a chegar.

## Controles
- PLNTS: quantos planetas, 2 ou 3.
- RATIO: o quanto as órbitas se afastam umas das outras.
- ECC: o achatamento das elipses.
- MASS: o quanto os planetas se puxam entre si.
- SPEED: a velocidade do tempo do sistema.
- LIGHT: o atraso da conjunção pela distância, até cerca de 1 s.
Entradas: NUDGE empurra o primeiro planeta e muda a forma da órbita, SPD move a velocidade por CV. Saídas: X1 e Y1 são a posição do primeiro planeta, X2 e Y2 a do segundo, DIST a distância entre eles, CONJ um pulso na conjunção, PERI um pulso quando o primeiro planeta passa mais perto do sol.

## Experimente
1. Ligue CONJ num DRUM e PERI noutro: dois ritmos lentos que quase se repetem.
2. Ligue X1 num QUANTIZER e o QUANTIZER num OSC: uma melodia em órbita.
3. Suba MASS devagar: as órbitas se desfazem e o ritmo fica imprevisível.
4. Ligue DIST no MIX de um SPACE: quando os planetas se afastam, o espaço se abre.
