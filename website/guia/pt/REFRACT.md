## O que é
O REFRACT é um raio de luz que nasce dentro de uma camada de vidro, atravessa outras três e sai para o ar. Em cada camada ele desvia, e o ângulo dele em cada uma sai como CV em A1 a A4.

## Como pensar nele
É a lei de Snell: ao passar de um meio para outro, o raio muda de direção conforme os índices dos dois. Indo de um meio denso para um menos denso num ângulo raso demais, ele não atravessa: reflete por inteiro. É a reflexão total, o brilho do diamante e o caminho da luz na fibra óptica. A cada vez que o raio passa a refletir, sai um pulso em TIR.

## Controles
- N1, N2, N3, N4: os índices das quatro camadas.
- RATE: a velocidade da varredura do ângulo.
- RANGE: até onde o ângulo varre.
- SHIM: a miragem, o tremor dos índices.
Entradas: ANG dá o ângulo por CV, no lugar da varredura, IDX soma em todos os índices. Saídas: A1 a A4 são os ângulos nas camadas, TIR o pulso da reflexão total, EXIT o desvio lateral do raio.

## Experimente
1. Ligue TIR num DRUM: a varredura passa pelo ângulo crítico duas vezes por ciclo, e o DRUM marca cada passagem.
2. Ligue A2 e A4 em dois cortes de FILTER: o mesmo raio dá dois movimentos parecidos e diferentes.
3. Suba SHIM com RANGE perto do ângulo crítico: o TIR vira um ritmo de miragem.
4. Ligue um LFO em IDX: o vidro esquenta e esfria, e o ponto da reflexão total anda.
