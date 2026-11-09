# MOUSSE — deformação de um material mole

Implementação local de 9/11/2026, ideia do autor ("deformações, tipo mousse"), aprovada a partir das propostas
de ideias físicas e ópticas (sessão de 9/11). Fonte:
[Mousse.hpp](../src/dsp/Mousse.hpp). Família MODULATE; 10 HP. Licença autoral:
AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela BIBLIOTECA).

## Contrato

Sólido linear padrão (Zener) da viscoelasticidade, adimensional:
alvo = força / k, com k = (0,25 + 1,75·STIFF)·(1 − amolecimento); parte rápida
segue o alvo em 5 ms (a mola); parte lenta segue em VISC/4 ao afundar e em
VISC·RET ao voltar (o fluido); DEPTH = (1 − SLOW)·rápida + SLOW·lenta, em 0–1.

**Força.** HIT: cada borda de subida é um empurrão de tamanho FORCE que decai
em ~80 ms (×4 na escala da força); PRESS: CV 0–1 somada.

**Saídas.** DEPTH (0–1), SPEED (|Δdepth|·fs·0,05, 0–1), REST (gate, DEPTH < 0,02).

**Desvios Rasgo.** (a) FATIGUE: cada golpe soma 0,25·FATIGUE ao amolecimento
(teto 0,8), que volta a 0 em ~6 s; (b) sem HIT ligado, toques autônomos em
RATE com força FORCE·(0,4 + 0,6·acaso semeado).

**Tempo real.** process() não aloca; NaN/∞ viram neutros; determinístico.

## Testes

`tests/test_mousse.cpp`: pressão de 1 s afunda em ~0,06 s (meia descida) e
volta em ~0,94 s (meia volta), mais que o dobro; REST no repouso; mais rígida
afunda menos; fadiga faz golpes posteriores afundarem mais; autônomo
determinístico; parâmetros extremos ficam em 0–1 e finitos.

## Proveniência e limites

Modelo de livro-texto (Zener, 1948; qualquer manual de reologia). Nenhuma
coleção do Cardinal faz isto; ideia e implementação próprias.

Uma dimensão (a profundidade); sem histerese de material além da fadiga. A escuta do autor ainda não foi feita.
