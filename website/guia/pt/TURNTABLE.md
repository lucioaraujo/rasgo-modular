## O que é
O TURNTABLE toca uma gravação como se ela estivesse num disco de vinil. Ele grava um trecho, como o SAMPLER, mas a leitura imita um prato de verdade: o disco tem peso, demora para ganhar velocidade, freia aos poucos e pode ser empurrado para a frente e para trás, como no scratch de um DJ.

## Como pensar nele
O SAMPLER salta de um ponto a outro na hora. O TURNTABLE tem inércia, e é isso que dá o som de vinil: a afinação escorrega quando o disco acelera ou freia. A entrada SCR é a mão sobre o disco: uma onda lenta ali faz o disco ir e voltar no ritmo. A entrada BRK desliga o motor, e o som desce até parar, no efeito conhecido como tape stop. Não há sincronia automática de andamento: acertar o ritmo é parte do gesto.

## Controles
- SPEED: a velocidade que o motor tenta alcançar, de metade a o dobro da original. Negativo gira o disco para trás.
- TORQ: a força do motor, ou seja, quão rápido o disco chega à velocidade. Baixo, a partida tem uma oscilação longa de afinação; no mínimo, o disco só se move pela mão, na entrada SCR.
- FRIC: o atrito. Decide quão rápido o disco para ao frear e quanto ele volta sozinho ao ritmo depois de um scratch.
- GRAB: a firmeza da mão, quanto o sinal em SCR move o disco.
- START: onde a agulha cai a cada pulso em TRIG.
- WEAR: o desgaste do disco: estalos e pequenas irregularidades de rotação.
- LOOP: desligado, o disco acaba e para; ligado, a leitura dá a volta e recomeça.
Entradas: TRIG, para recolocar a agulha; IN, o som a gravar; REC, o sinal que grava enquanto está ligado; SCR, a mão no disco; BRK, o freio. Saída: OUT.

## Experimente
1. Grave uma frase como no SAMPLER: a voz em IN e a saída DIV de um LOGIC, com DIV em 16, em REC. Ligue OUT no MIXER e LOOP.
2. Ligue a saída BI de um FUNCTION a uns 2 Hz em SCR e suba GRAB até o meio. O disco vai e volta, num scratch.
3. Retire o cabo de SCR. Ligue a saída GATE de um DECISION com BIAS baixo em BRK. De vez em quando, o disco freia e volta a girar.
