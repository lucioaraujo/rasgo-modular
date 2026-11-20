## O que é
O PREDATOR é um pequeno ecossistema: presas que crescem sozinhas e predadores que crescem comendo. As duas populações sobem e descem em ciclo, o predador sempre um pouco atrás, e saem como CV em PREY e PRED.

## Como pensar nele
São as equações de Lotka e Volterra, dos anos 1920, a mesma física do ecossistema do Aquorbium. Quando há muita presa, o predador cresce; quando há muito predador, a presa cai; então o predador passa fome, e tudo recomeça. As capturas dão pulsos em CTCH, e o pico da presa, a fuga, dá um pulso em FLEE. CANON repete a linha da presa com um atraso que encolhe quando o predador se aproxima: um cânone que se aperta até o stretto.

## Controles
- PERIOD: a duração de um ciclo.
- SIZE: o tamanho da órbita.
- HUNT: quanto o predador caça.
- WILD: o ruído do ambiente.
- DELAY: o atraso máximo do cânone.
Entradas: CTCH é uma captura vinda de fora, RATE move o andamento em volts por oitava. Saídas: PREY e PRED são as populações, CTCH e FLEE os pulsos da caça e da fuga, CANON o cânone da presa.

## Experimente
1. Ligue PREY no corte de um FILTER e PRED no nível de um VCA: o timbre e a dinâmica caçam um ao outro.
2. Ligue CTCH num DRUM com HUNT alto: as capturas viram batidas nos momentos de fartura.
3. Ligue PREY e CANON nas alturas de duas vozes: a segunda imita a primeira cada vez mais perto.
4. Suba WILD: os ciclos deixam de ser iguais, como num ecossistema real.
