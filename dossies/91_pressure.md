# 91 — PRESSURE — gás numa caixa

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Teoria cinética (Maxwell, 1860; Boltzmann); conceito do Sapphire Nucleus (GPL), nenhum código copiado. Desvios RASGO: ATTR (gás de van der Waals: condensa) e HEAT (estalo de calor); o pistão que avança aquece o gás.

Código: `src/dsp/Pressure.hpp`. Família MODULATE.

Testes (`test_pressure`): mais quente, mais choques (T 0,15 → 28, T 0,8 → 146 em 20 s) e mais pressão; comprimir de VOL 1 a 0,4 sobe a pressão (0,55 → 0,92, Boyle); com ATTR 1 o gás condensa e a pressão cai a ~0; determinístico.

## Na seed

Entra no rack da seed (família MODULATE), pela passada genérica. Um papel dedicado fica para depois da escuta.
