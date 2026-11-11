# 89 — MAGNET — uma grade de ímãs

Data: 11/11/2026. Estado: **em implementação**.

Rascunho de 9/11 (outra sessão; ideia do autor: "imãs"), lido por inteiro e aceito em 11/11 ("magnet e orbit entram"). Modelo de Ising (E. Ising, 1925; L. Onsager, 1944), com 8×8 spins numa grade circular e dinâmica de Metropolis (1953) semeada. Conceito lido no CVfunk Magnets (MIT); nenhum código copiado. Desvios RASGO: QUENCH (esquenta tudo de uma vez) e FIELD por CV.

Testes (`test_magnet`):
- |MAG| médio: 1,00 a T 1,0 e 0,16 a T 5,5 (a transição de fase);
- o antiferromagneto (COUPL −1) fica com |MAG| 0,00 e energia 0,00 (o tabuleiro de xadrez);
- o GT é determinístico.

## Na seed

Entra no rack (família MODULATE). Um papel dedicado fica para depois da escuta.
