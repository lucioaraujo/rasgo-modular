## O que é
O HARMONY decide por quais tonalidades a música passa. De tempos em tempos, ou a cada pulso na entrada ADV, ele escolhe um novo centro tonal, seguindo um entre seis movimentos que músicos de jazz e de cinema usam há décadas. A saída ROOT informa a tônica nova na mesma escala de altura que os osciladores entendem.

## Como pensar nele
O QUANTIZER deixa a melodia afinada, mas sempre no mesmo tom. Músicas costumam viajar, e cada estilo viaja de um jeito. O controle MOVE escolhe o percurso: Coltrane, que salta uma terça maior a cada troca e fecha um ciclo de três tons, como em Giant Steps; substituição de trítono, que chega ao tom seguinte por um caminho inesperado; mediante cromática, mudanças de terça com efeito de trilha de cinema; intercâmbio modal, que mantém a tônica e troca o modo; jazz modal, que quase nunca sai do lugar; e backdoor, que sobe um tom inteiro.

Nesta versão, a forma de ouvir as mudanças é somar a saída ROOT à melodia: dois cabos chegando na mesma entrada 1V/O se somam, e a melodia inteira se transpõe para o centro novo. A troca de escala, da saída SCALE, ainda não chega ao QUANTIZER por cabo.

## Controles
- MOVE: o tipo de percurso, entre os seis descritos acima.
- RATE: o ritmo das trocas, quando nada chega em ADV. Vai de uma troca a cada poucos minutos a duas por segundo.
- ROOT: a tônica de partida, para onde o módulo volta quando recebe um pulso em RST.
- S-LO, S-HI: a faixa de escalas que podem ser sorteadas nas trocas.
- HOLD: a chance de uma troca ser ignorada, o que alonga algumas seções.
Entradas: ADV, o pulso que pede uma troca; RST, para voltar ao começo. Saídas: ROOT, a tônica como altura; SCALE, o número da escala; CHG, um pulso a cada troca.

## Experimente
1. Monte a melodia do QUANTIZER: TURING no CV do QUANTIZER, PTCH em 1V/O de um OSC, o OSC passando por um ENVELOPE até o MIXER.
2. Ligue ROOT do HARMONY também em 1V/O do OSC. Os dois cabos se somam.
3. Ponha MOVE na primeira posição, Coltrane, e suba RATE. A cada troca, a melodia salta para outro centro, em ciclos de três.
4. Ligue CHG na entrada ADV de um DRIFT para que o resto do patch também mude de seção a cada troca.
