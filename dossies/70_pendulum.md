# PENDULUM — pêndulo duplo

Implementação local de 9/11/2026, ideia do autor ("pêndulos"), aprovada a partir das propostas
de ideias físicas e ópticas (sessão de 9/11). Fonte:
[Pendulum.hpp](../src/dsp/Pendulum.hpp). Família MODULATE; 12 HP. Licença autoral:
AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela BIBLIOTECA).

## Contrato

Pêndulo duplo de massas pontuais e hastes rígidas sem massa: braço de cima
1, de baixo RATIO (0,3–2); massa de cima 1, de baixo MASS (0,2–3); g = 9,8.
Equações de Lagrange; Runge-Kutta de 4ª ordem por amostra, com passo
SPEED·2^(cv)/fs; atrito viscoso −DAMP·0,6·ω em cada ângulo.

**Saídas.** X1/Y1, X2/Y2: as pontas, normalizadas por 1/(1 + RATIO) em −1..1
(o pivô no centro); CROSS: pulso de 5 ms quando X2 troca de sinal com a ponta
abaixo do pivô; FLIP: pulso quando θ2 cruza uma volta (θ2 + π)/2π.

**Entradas.** KICK (soma ±4 rad/s a ω2, sinal semeado), SPD (CV, ×2 por +1).

**Lançamento.** Os dois braços parados no ângulo 0,15 + 2,9·ENERGY rad (~9° a
~175°), cada um ±10 % semeado e o de baixo com sinal sorteado.

**Desvio Rasgo.** AUTO: energia abaixo de 1/5 da de lançamento → relança.

**Tempo real.** process() não aloca; NaN/∞ viram neutros; determinístico.

## Testes

`tests/test_pendulum.cpp`: energia conservada sem atrito (30,1944 → 30,1944
em 10 s); ENERGY 0 só oscila (CROSS regular, nenhum FLIP); ENERGY 1 dá voltas
(6 FLIP em 10 s); com DAMP máximo e AUTO segue se movendo depois de 25 s,
determinístico; saídas em −1..1 com parâmetros e entradas absurdos.

## Proveniência e limites

Mecânica analítica de livro-texto (Lagrange); o pêndulo triplo acoplado por
molas estudado no Batruc (`BATRUC/MOVIMENTO_GRAFICO_E_SOM.md`) é a referência
próxima na família. O Cardinal não tem pêndulo; implementação própria.

Sem colisões; hastes rígidas (o acoplamento por mola do Batruc fica como
evolução possível). A escuta do autor ainda não foi feita.
