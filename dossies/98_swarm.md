# 98 — SWARM — o cardume

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Kuramoto (1975); o coro de fases do Aquorbium (`ChorusPhase`, mesmo autor); o cardume coeso/disperso de Blackwell e Bentley (2002, swarm music). Desvios RASGO: SCAR (o susto) e LEAD (o líder externo).

Código: `src/dsp/Swarm.hpp`. Família TIME.

Testes (`test_swarm`): SYNC médio 0,37 com COHES 0 e 0,98 com COHES 1; soltos, G1 e G2 dão 53 e 64 pulsos em 30 s; coesos, 60 e 59; o susto derruba o SYNC de 0,98 para 0,48 e o cardume se reagrupa.

## Na seed

Entra no rack da seed (família TIME), pela passada genérica. Um papel dedicado fica para depois da escuta.
