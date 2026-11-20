## O que é
O PRESSURE é uma caixa com gás. As partículas andam em linha reta e batem nas paredes. A parede da direita é um pistão, e cada choque nela dá um gate em HIT. A força média dos choques é a pressão, que sai em PRES.

## Como pensar nele
É a teoria cinética dos gases, de Maxwell e Boltzmann. Mais calor, as partículas correm mais e batem mais. Menos volume, elas batem mais vezes, porque a parede está mais perto: a pressão sobe, como na lei de Boyle. Com ATTR positivo, o gás vira um gás real: as partículas se atraem e, com força, condensam numa gota, e a pressão cai.

## Controles
- N: quantas partículas há na caixa.
- TEMP: a temperatura, a velocidade média.
- VOL: o volume; a posição do pistão.
- ATTR: atração (positivo) ou repulsão (negativo) entre as partículas.
- GATE: a duração de cada gate.
Entradas: HEAT dá um estalo de calor em todas, VOL e TEMP movem volume e temperatura por CV. Saídas: HIT é o gate dos choques no pistão, PRES a pressão, TEMP a temperatura medida, X e Y a posição de uma partícula.

## Experimente
1. Ligue HIT num DRUM com N em 3: um ritmo esparso e irregular. Suba N até 20: vira um chiado de choques.
2. Ligue um LFO lento em VOL: quando o pistão comprime, o ritmo acelera e PRES sobe.
3. Ligue X e Y no PAN e no corte de um FILTER: a partícula desenha o timbre.
4. Suba ATTR devagar: o gás condensa, os choques rareiam e a pressão cai.
