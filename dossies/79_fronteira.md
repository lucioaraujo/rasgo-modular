# 79 — FRONTEIRA: onde a célula melódica começa

Data: 11/11/2026. Estado: **em implementação** (estudo da família §4.9,
"FRASE", aprovado; "pode avançar"). O nome é FRONTEIRA porque o PHRASE
(#62) do Modular já é outro módulo, um motivo interpretado. Origem:
`phrasing.py` do Maravi (mesmo autor; conceito, não código).

## Portão conceitual

1. **Território.**
   - O LBDM (*Local Boundary Detection Model*, E. Cambouropoulos, 2001), a
     forma computável das regras de agrupamento de Lerdahl & Jackendoff
     (GTTM, 1983).
   - A Gestalt: percebemos fronteira onde há **mudança**.
2. **Tensão.** O Antitotem sabe onde a frase começa porque a gerou; o Maravi
   precisa inferir do áudio; o Modular, ao vivo, não sabia. Com a FRONTEIRA,
   o patch sabe **quando quem toca começou uma ideia nova**.
3. **Unidade.** A **célula melódica**.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe notas (PTCH + GATE).
   - Conserva as 4 últimas.
   - Transforma em força de fronteira em três dimensões: salto, intervalo
     entre ataques e pausa.
   - Esquece: os máximos de referência desbotam em MEM notas.
5. **Regra, recurso, risco, limite.**
   - Regra: fronteira = força ≥ LIM, maior que a anterior, com a célula de
     pelo menos MIN notas.
   - Risco: as primeiras notas não têm referência, então as 5 primeiras só
     formam a referência (a 1ª versão marcava fronteiras falsas nas notas 2
     e 4).
   - Limite: **uma nota de atraso**. A força de uma nota precisa da
     seguinte, e o atraso é dito, não escondido.
6. **Consequência.**
   - FRONT no GESTO do PALCO (cada ideia nova muda o palco).
   - FRONT no RST de um SEQUENCE (o acompanhamento recomeça com a frase de
     quem toca).
   - FIM (o silêncio) para o patch respirar junto.
7. **Diferença no Modular.** O SCOPE dá ataques, o COHERENCE mede a ordem,
   o TOM o tom. Nenhum segmenta a frase.
8. **Diferença para o Maravi.** Lá, offline, normaliza pela melodia
   inteira e exige pico local dos dois lados. Aqui, causal: normaliza pelo
   máximo recente e compara só com a nota anterior. Mais o FIM por
   silêncio, que fecha na hora.
9. **Escuta.**
   - Êxito: cantar frases com respiração e ouvir o patch mudar no começo de
     cada uma.
   - Fracasso: fronteira a cada nota, ou nunca.
10. **Proveniência.** Cambouropoulos 2001; Lerdahl & Jackendoff 1983; pesos
    0,25 / 0,50 / 0,25 da literatura (como no Maravi). Código próprio.

## Comportamento

- **Os valores:** por nota, o salto em semitons, o IOI (s) e a pausa (s,
  do fim da anterior ao ataque).
- **A força:** v·(r(antes, v) + r(v, depois)), com r(a,b) = |a−b|/(a+b),
  normalizada pelo máximo recente de cada dimensão (desbota com
  exp(−1/MEM) por nota).
- **O silêncio:** sem ataque por SIL s, sai um FIM, e a próxima nota começa
  uma célula.
- **Saídas:** FRONT (pulso), FORÇA (0–1), NOTAS (tamanho da última célula
  fechada, /16) e FIM.

**Teste** (`test_fronteira`):
- grupos de 3 colcheias + 1 nota longa + pausa, repetidos 5 vezes, dão
  fronteiras exatamente no início de cada grupo, a partir do 2º;
- uma linha regular não tem fronteiras;
- o silêncio longo dá um FIM.
