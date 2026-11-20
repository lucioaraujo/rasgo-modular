# 100 — SEASON — dia, noite e estação

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Altura do Sol: sen h = sen φ·sen δ + cos φ·cos δ·cos H, com declinação de ±23,44°. O relógio de dia, noite e estação do Aquorbium (mesmo autor), que escolhe modo e andamento. Desvio RASGO: HEMI (o sul inverte as estações).

Código: `src/dsp/Season.hpp`. Família TIME.

Testes (`test_season`): 12 h de dia no equador o ano inteiro; a 60°, 77 % de dia no verão e 23 % no inverno; a 75°, sol da meia-noite e noite polar; 12 h em qualquer latitude no equinócio; um nascer por dia; no norte, junho é verão (67 % de luz), no sul, inverno (33 %).

## Na seed

Entra no rack da seed (família TIME), pela passada genérica. Um papel dedicado fica para depois da escuta.
