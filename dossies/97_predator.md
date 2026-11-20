# 97 — PREDATOR — predador e presa

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Lotka (1925) e Volterra (1926). Integração copiada e adaptada do Aquorbium (`PopulationCycles`, mesmo autor, AGPL), com a captura de lá. Desvios RASGO: CTCH como entrada (o patch caça), WILD (ruído do ambiente) e CANON (o cânone que fecha em stretto, do diretor ecológico do Aquorbium).

Código: `src/dsp/Predator.hpp`. Família MODULATE.

Testes (`test_predator`): sem caça nem ruído, em 10 ciclos o invariante de Lotka–Volterra fica em 2,7704 → 2,7702 (a órbita fecha) e há um pico de presa por ciclo; o predador vem depois da presa; HUNT 0 → 0 capturas, HUNT 1 → 63 em 60 s; determinístico com WILD.

## Na seed

Entra no rack da seed (família MODULATE), pela passada genérica. Um papel dedicado fica para depois da escuta.
