# 83 — VIA: o cabo com matéria

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: as PORTAS do Antitotem (`CmosVoice::feedbackSample`, código do autor, AGPL) — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

Sete portas, combinadas pela média das abertas: RES, DIODO, CAP, PULSO, TRANS, REFLUXO e FALANTE. As seis primeiras são as do Antitotem; o FALANTE, um corpo ressonante em CORPO que satura, vem do estudo. Desvio obrigatório: as portas servem a qualquer cabo do patch, não só ao retorno de uma voz, que é o próximo passo que o próprio autor deixou escrito.

## Comportamento

- **2× de sobreamostragem:** as portas rodam a 2× (o diodo e o pulso geram harmônicos).
- **O tempo do capacitor:** MEM, de 1 ms a 2 s, mais a entrada de CV.
- **CAP como saída:** o estado do capacitor sai como CV.
- **Nenhuma porta aberta:** silêncio.

## Teste (`tests/test_via.cpp`)

- RES passa o sinal;
- DIODO dobra a frequência;
- CAP alisa o agudo e carrega com um nível constante;
- PULSO fortalece os ímpares;
- FALANTE ressoa em CORPO;
- portas compostas ficam limitadas.

## Na seed

Entra no rack da seed (regra de 9/11) na família ROUTE. Um papel dedicado
fica para depois da escuta do autor.
