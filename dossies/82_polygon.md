# 82 — POLYGON: polígonos inscritos num ciclo

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: o Batruc (a geometria rítmica: ataques como vértices de polígonos) — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

Um ciclo comum e até 4 camadas. A camada k é um polígono de Nₖ vértices, girado pela fase φₖ (até um lado). É o polímetro como figura.

## Comportamento

- **COINC:** dois ou mais vértices caem juntos (±1,5 ms).
- **EOS:** o fim de cada ciclo.
- **FASE:** a posição no ciclo, como CV.
- **O ciclo:** dura CICLO s; com CLK, dura PASSOS pulsos.

## Teste (`tests/test_polygon.cpp`)

- 3:2 em 3 s dá 10 e 7 pulsos (contando o vértice do recomeço);
- COINC só no vértice comum;
- girar o triângulo ¼ de lado tira as coincidências;
- com relógio, o ciclo dura PASSOS pulsos.
- Na largada, uma camada girada só dispara se estiver de fato num vértice (a 1ª versão disparava em falso).

## Na seed

Entra no rack da seed (regra de 9/11) na família TIME. Um papel dedicado
fica para depois da escuta do autor.
