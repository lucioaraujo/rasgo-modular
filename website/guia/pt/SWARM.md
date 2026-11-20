## O que é
O SWARM é um cardume de seis peixes, e cada peixe é um relógio com o seu próprio andamento. Eles se ouvem e se puxam uns aos outros. Quatro deles saem em G1 a G4, e o pulso do cardume inteiro, em ALL.

## Como pensar nele
É o modelo de Kuramoto, o mesmo das rãs que se sincronizam no coro de fases do Aquorbium e dos vagalumes que piscam juntos. Um só controle, COHES, vai do solto ao coeso: em 0, cada peixe toca no seu tempo, como uma heterofonia; em 1, todos no mesmo pulso, como uma homorritmia; no meio, o cardume se forma e se desfaz. SYNC mede o quanto estão juntos.

## Controles
- RATE: o andamento médio.
- SPRD: o quanto os andamentos naturais diferem.
- COHES: a coesão do cardume.
- GATE: a duração de cada gate.
Entradas: SCAR é o susto que espalha o cardume, LEAD é um líder externo que todos seguem, COH soma no COHES. Saídas: G1 a G4 são os pulsos de quatro peixes, ALL o pulso do cardume, SYNC a coesão medida.

## Experimente
1. Ligue G1 a G4 em quatro DRUM com COHES em 0: quatro andamentos soltos. Suba COHES devagar e ouça o cardume se formar.
2. Ligue um gatilho raro em SCAR: o cardume se espalha e se reagrupa.
3. Ligue um CLOCK em LEAD: o cardume passa a seguir o líder.
4. Ligue SYNC no nível de um VCA: o som cresce quando o cardume se junta.
