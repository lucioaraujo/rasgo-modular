# 90 — ORBIT — planetas em gravidade

Data: 11/11/2026. Estado: **em implementação**.

Rascunho de 9/11 (outra sessão), lido por inteiro e aceito em 11/11. Gravitação de Newton com um sol e 2–3 planetas, integrada por leapfrog simplético (kick-drift-kick), a física do céu do Asterismus (conceito, não código). Conceito do AlgoritmArte Planetz (GPL); nenhum código copiado. Desvios RASGO: o tempo-luz (LIGHT), NUDGE, e o planeta que escapa volta à órbita.

Testes (`test_orbit`):
- com MASS 0, a energia deriva só 6,3·10⁻¹¹ em 20 s;
- o período com ECC 0,3 é 1,35 s, o que a 3ª lei de Kepler prevê (o planeta parte do afélio, e a órbita elíptica tem semieixo r/1,3). Os "~2 s" do comentário valem para a órbita circular; o comentário foi corrigido para dizer isso;
- há conjunções;
- com MASS 1 e ECC 0,8, há caos, mas o sistema fica limitado.

## Na seed

Entra no rack (família MODULATE). Um papel dedicado fica para depois da escuta.
