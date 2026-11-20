# 94 — INTERFERENCE — franjas de interferência

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Experiência de Young (1801); interferência de três fontes coerentes. Território próprio do Rasgo (o Cardinal não tem óptica). Desvio RASGO: COHER (coerência parcial).

Código: `src/dsp/Interference.hpp`. Família TIME.

Testes (`test_interference`): duas fontes: I = 1 no centro e 0 na meia franja; SPEED 4 dá 41 franjas claras e 40 escuras em 10 s; a terceira fonte agrupa o padrão (81 → 56 franjas em 20 s); a coerência parcial muda o padrão (56 → 66) sem virar chiado.

## Na seed

Entra no rack da seed (família TIME), pela passada genérica. Um papel dedicado fica para depois da escuta.
