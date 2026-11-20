## O que é
SWARM es un cardumen de seis peces, y cada pez es un reloj con su propio tempo. Se escuchan y se arrastran unos a otros. Cuatro de ellos salen en G1 a G4, y el pulso del cardumen entero en ALL.

## Como pensar nele
Es el modelo de Kuramoto, el mismo de las ranas que se sincronizan en el coro de fases del Aquorbium y de las luciérnagas que destellan juntas. Un solo control, COHES, va de lo suelto a lo cohesionado: en 0, cada pez toca en su tiempo, como una heterofonía; en 1, todos en el mismo pulso, como una homorritmia; en el medio, el cardumen se forma y se deshace. SYNC mide cuán juntos están.

## Controles
- RATE: el tempo medio.
- SPRD: cuánto difieren los tempos naturales.
- COHES: la cohesión del cardumen.
- GATE: la duración de cada gate.
Entradas: SCAR es el susto que dispersa el cardumen, LEAD un líder externo que todos siguen, COH se suma a COHES. Salidas: G1 a G4 son los pulsos de cuatro peces, ALL el pulso del cardumen, SYNC la cohesión medida.

## Experimente
1. Conecte G1 a G4 a cuatro DRUM con COHES en 0: cuatro tempos sueltos. Suba COHES despacio y escuche formarse el cardumen.
2. Conecte un disparo raro a SCAR: el cardumen se dispersa y se reagrupa.
3. Conecte un CLOCK a LEAD: el cardumen pasa a seguir al líder.
4. Conecte SYNC al nivel de un VCA: el sonido crece cuando el cardumen se junta.
