# 93 — REFRACT — um raio atravessando camadas

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Lei da refração (Ibn Sahl, 984; Snell, 1621; Descartes, 1637) e reflexão total. Território próprio do Rasgo (o Cardinal não tem óptica). Desvios RASGO: SHIM (miragem) e IDX somado a todas as camadas.

Código: `src/dsp/Refract.hpp`. Família MODULATE.

Testes (`test_refract`): Snell se conserva (n·sen θ igual em todas as camadas, erro < 10⁻⁴); vidro → ar: a 41° o raio sai, a 43° reflete (ângulo crítico 41,8°); a varredura ±45° a 0,5 Hz entra em reflexão total duas vezes por ciclo (10 em 10 s), e a miragem a faz piscar (18).

## Na seed

Entra no rack da seed (família MODULATE), pela passada genérica. Um papel dedicado fica para depois da escuta.
