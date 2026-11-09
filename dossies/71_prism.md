# PRISM — o som através de um prisma

Implementação local de 9/11/2026, ideia do autor ("ideias ópticas, refração"), aprovada a partir das propostas
de ideias físicas e ópticas (sessão de 9/11). Fonte:
[Prism.hpp](../src/dsp/Prism.hpp). Família SPACE; 10 HP. Licença autoral:
AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela BIBLIOTECA).

## Contrato

6 bandas por passa-faixas SVF (Zavalishin, TPT; Q ≈ 1,4, saída normalizada
k·bp) em 120, 300, 750, 1900, 4700 e 11 000 Hz. Cada banda tem sua linha de
atraso (até 420 ms, alocada no prepare()); atraso = DISP·forma(b) + 1 amostra,
suavizado em 50 ms. forma(b): CURVE ≥ 0 → CURVE·x + (1 − CURVE)/2 (x = 0 grave
… 1 agudo); CURVE < 0 → −CURVE·(1 − x) + (1 + CURVE)/2.

**Realimentação.** A entrada de cada banda é IN + FDBK·(a sua saída
atrasada): ecos que se separam em cores (FDBK ≤ 0,85, ganho do passa-faixa ≤ 1
→ estável).

**Estéreo.** Leque de potência constante: banda b em (b/5 − 0,5)·SPRD.
OUT = (1 − MIX)·seco + MIX·molhado; WET só o molhado; ±4.

**Desvios Rasgo.** Dispersão anômala (CURVE < 0) e FDBK por cor.

**Tempo real.** process() não aloca; NaN/∞ viram neutros; determinístico.

## Testes

`tests/test_prism.cpp`: forma nos extremos; uma rajada de 120 Hz chega a
0,035 s e uma de 4,7 kHz a 0,27 s com CURVE +1 — o contrário com CURVE −1;
grave mais à esquerda e agudo mais à direita com SPRD 1; MIX 0 = entrada;
FDBK 0,85 decai; parâmetros e entradas absurdos ficam finitos e limitados.

## Proveniência e limites

Dispersão óptica (índice de refração que varia com o comprimento de onda) —
física pública. O Cardinal não tem módulos de óptica; ideia e implementação
próprias. O SVF segue a formulação pública de V. Zavalishin, *The Art of VA
Filter Design* (2012).

Bandas fixas (6); sem cruzamento de fase perfeito entre elas — é efeito, não
divisor de fase linear. A escuta do autor ainda não foi feita.
