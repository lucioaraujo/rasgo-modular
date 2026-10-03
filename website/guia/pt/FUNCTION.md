## O que é
O FUNCTION gera uma rampa que sobe e desce sem parar. Devagar, ela serve para mover outros controles, o que se chama LFO, oscilador de baixa frequência; rápido, vira um som audível; disparada por um pulso, funciona como um envelope simples. É a fonte de movimento mais usada do Rasgo Modular.

## Como pensar nele
Envelope, LFO e oscilador são a mesma coisa em velocidades diferentes, e o FUNCTION cobre todas, de um ciclo a cada cem segundos a milhares por segundo. Quando um guia diz para ligar um LFO em algum lugar, quase sempre é um FUNCTION lento. A forma vai de uma rampa que cai a uma que sobe, passando pelo triângulo, e as duas saídas entregam a mesma forma em escalas diferentes: UNI só acima de zero, BI acima e abaixo.

## Controles
- RATE: a velocidade, de 0,01 a 12000 ciclos por segundo.
- SLOPE: a forma. Num extremo, sobe de repente e desce devagar; no meio, triângulo; no outro extremo, sobe devagar e cai de repente.
- DRIFT: varia a velocidade aos poucos, para que o movimento não soe como um metrônomo.
- SYNC: liga a entrada SYNC, para que cada pulso recomece a rampa.
Entradas: RATE e SLOPE, para modular os controles; SYNC, o pulso que recomeça. Saídas: UNI e BI.

## Experimente
1. Faça uma voz: SAW de um OSC na entrada IN de um FILTER, LO do FILTER no MIXER. Baixe CUT.
2. Ligue BI do FUNCTION na entrada FC do FILTER, com RATE baixo. O timbre abre e fecha devagar.
3. Suba RATE até uns 5 Hz e ligue BI na entrada FM do OSC, com o FM do OSC bem baixo. A afinação treme, num vibrato.
4. Suba RATE até a faixa audível e ligue BI direto no MIXER: o próprio FUNCTION vira som.
