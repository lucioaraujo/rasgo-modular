## O que é
O STRING simula uma corda: pinçada, como num violão; martelada, como num piano; ou friccionada, como num violino. Ele recria o comportamento de uma corda que vibra e vai perdendo energia, e por isso o ataque e o corpo do som soam naturais, mesmo sendo sintéticos.

## Como pensar nele
O MATTER é o módulo dos objetos que ressoam, como sinos e placas; o STRING é o das cordas. O gesto principal é POS, o ponto onde a corda é tocada: mudando esse ponto, alguns harmônicos somem, como quando um violonista toca mais perto da ponte. Um pulso em PLK pinça a corda; um som contínuo em IN faz a corda cantar como se fosse arcada.

## Controles
- FREQ: a afinação da corda, de 20 a 4000 Hz.
- DECAY: quanto tempo a corda soa depois de pinçada. Baixo é seco; alto quase não acaba.
- DAMP: o brilho. Alto deixa a corda escura, como uma corda velha; baixo mantém o brilho de uma corda nova.
- POS: o ponto onde a corda é tocada. Perto do meio, os harmônicos pares somem e o som fica oco.
- EXCIT: quanto ruído entra no golpe, como o som do dedo ou da palheta.
- DRIVE: satura a corda por dentro, levando-a do som acústico a um som distorcido.
- MIX: a mistura entre o golpe cru e a corda. No máximo, só a corda.
Entradas: IN, um som que faz a corda vibrar; PLK, um pulso que a pinça; 1V/O, para a afinação; DMP, para mudar o brilho.

## Experimente
1. Ligue OUT numa entrada do MIXER, e a saída EUC do CLOCK na entrada PLK. A corda é pinçada no ritmo do relógio.
2. Gire DECAY: das notas secas às que se emendam.
3. Mova POS devagar e ouça o timbre mudar com o ponto do toque.
4. Ligue a saída PTCH de uma SEQUENCE, movida pelo mesmo CLOCK, na entrada 1V/O. Agora a corda toca uma linha melódica.
