# 88 — SUPPLY: a alimentação variável, com cabos

Data: 11/11/2026. Estado: **em implementação**. O estudo da família §4.5c
(item 2) pedia "a decidir com o autor; não implementar sem conversa". A
decisão do autor (11/11): **"módulo com cabos"**, não um barramento global.
Origem: a ENERGIA do Antitotem (mesmo autor; conceito, não código): "uma
interpretação musical segura, não um modelo de alimentação elétrica real".

## Comportamento

- **O trilho:** ENERGIA dá a tensão. A CARGA (|som| suavizado em 50 ms)
  a faz ceder na medida de QUEDA, com a demora de TEMPO: trilho = ENERGIA ·
  (1 − QUEDA · carga).
- **TRILHO** (0–1) é a tensão que sobra.
- **D1–D4:** quatro derivas em V/oct (até ±60 cents) que só aparecem com o
  trilho baixo (o `railWander`). Ligadas em vozes diferentes, fazem as vozes
  se desencontrarem.
- **FALHA:** um pulso abaixo de 0,25 e outro acima de 0,35 (com
  histerese).
- **Só onde há cabo:** o SUPPLY só age onde há cabo (regra do Antitotem).

## Teste (`tests/test_supply.cpp`)

- Sem carga, o trilho é a ENERGIA (0,80). Com carga, cede para 0,28; com
  ENERGIA 0,5, passa abaixo de 0,25 e dá a FALHA.
- As derivas: com o trilho cheio, D1 − D2 = 0; com o trilho baixo, até
  0,015 oct (~18 cents) em 30 s.

## Na seed

Entra no rack (família MODULATE). Um papel dedicado fica para depois da
escuta.
