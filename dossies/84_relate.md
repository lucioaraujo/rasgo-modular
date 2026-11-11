# 84 — RELATE: os atos entre duas vozes

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: o Asterismus (sombra, hoquetus, contágio, atrito/reparo) e o `InteractionProcessor` do Antitotem — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

Duas vozes entram e saem transformadas pela relação entre elas. Um seguidor de envelope por voz (5 ms / 80 ms) mede quem soa.

## Comportamento

- **VERBO:**
  - SOMBRA: B soa onde A soou, um TEMPO depois;
  - HOQUETO: uma cede quando a outra soa;
  - CONTÁGIO: os envelopes lentos se contaminam;
  - ATRITO: o produto A·B onde as duas se encontram, com reparo lento;
  - ANEL: o produto A·B.
- **ENC:** o encontro, de 0 a 1.

## Teste (`tests/test_relate.cpp`)

- SOMBRA com TEMPO 0,3: B soa de ~0,8 a 1,3 s, para A soando de 0,5 a 1,0 s;
- HOQUETO: as duas cedem quando soam juntas;
- ENC e ANEL só existem quando as duas soam.

## Na seed

Entra no rack da seed (regra de 9/11) na família ROUTE. Um papel dedicado
fica para depois da escuta do autor.
