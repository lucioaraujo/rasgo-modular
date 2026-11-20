# 92 — ELASTIC — massas e molas

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Corda discreta de massas e molas (modos de Lagrange); conceito do Sapphire Elastika (GPL), nenhum código copiado. Euler simplético por amostra. Desvios RASGO: PLUK (dedilha sozinho) e TUNE por CV.

Código: `src/dsp/Elastic.hpp`. Família TRANSFORM.

Testes (`test_elastic`): o dedilhado soa nos modos teóricos da corrente (modo 1 de N 4, TUNE 200: 123,6 Hz, energia 1600× acima de fora do modo); DAMP 0,9 encurta a cauda mais de 10×; ruído 0,5 na entrada sai com RMS 0,18, finito e dentro de ±1 com STRCH 1.

## Na seed

Entra no rack da seed (família TRANSFORM), pela passada genérica. Um papel dedicado fica para depois da escuta.
