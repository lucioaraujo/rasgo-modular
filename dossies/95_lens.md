# 95 — LENS — foco e desfoque

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Profundidade de campo da óptica fotográfica, levada ao espectro: oito faixas (SVF de Zavalishin) e difusores passa-tudo de Schroeder (1962). Território próprio do Rasgo. Desvio RASGO: o desfoque abre o estéreo (bokeh).

Código: `src/dsp/Lens.hpp`. Família SPACE.

Testes (`test_lens`): com BLUR 0 a saída é a entrada, amostra a amostra (diferença 0); um clique fora de foco espalha 18 % da energia para depois de 20 ms (nítido: 0 %); a profundidade de campo segue a distância ao foco; o ruído fora de foco sai com correlação L·R 0,76 (o desfoque abre o estéreo).

## Na seed

Entra no rack da seed (família SPACE), pela passada genérica. Um papel dedicado fica para depois da escuta.
