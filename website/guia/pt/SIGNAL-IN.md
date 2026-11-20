## O que é
O SIGNAL-IN traz o mundo de fora para dentro do patch. Ele recebe o som da entrada de áudio do computador, como um microfone ou outro instrumento, e as notas de um teclado ou controlador MIDI, e transforma tudo em sinais que os outros módulos entendem.

## Como pensar nele
Com ele, alguém de fora pode tocar o Rasgo Modular: a altura da tecla vai para um oscilador, o aperto da tecla abre um envelope, e a roda de modulação do teclado pode abrir um filtro. Ele também deixa o patch processar som de fora, passando uma voz ou uma guitarra por filtros e espaços. O módulo começa desligado: o app só abre o microfone e o MIDI depois que você liga o ON e cabeia alguma saída.

## Controles
- GAIN: o volume do áudio que entra, de zero ao dobro.
- BEND: quantos semitons a alavanca de pitch bend do teclado move a altura, de zero a duas oitavas.
- CC#: qual controle do teclado a saída CC segue. O número 1 costuma ser a roda de modulação.
- ON: liga a entrada de fora. Desligado, que é como o módulo começa, nada entra no patch.
- DIV: a divisão do relógio MIDI na saída CLK: semínima, colcheia (o valor de partida, que casa com o MULT 2 do CLOCK), semicolcheia ou os 24 pulsos por semínima.
Saídas: L e R, o áudio que entra; 1V/O, a altura da tecla tocada; GATE, aberto enquanto a tecla está pressionada; VEL, a força com que ela foi tocada; CC, o valor do controle escolhido; CLK, os pulsos no andamento do relógio MIDI que chega de um programa de gravação ou de uma máquina; RUN, aberto enquanto esse programa toca; RST, um pulso no primeiro tempo depois do start.

## Experimente
1. Com um microfone ou instrumento ligado ao computador, ligue L numa entrada do MIXER, ligue o ON e ajuste GAIN até ouvir o som.
2. Passe esse som por um FILTER ou pelo SPACE antes do MIXER para transformá-lo.
3. Com um teclado MIDI e o ON ligado, ligue 1V/O na entrada 1V/O de um OSC, e GATE na entrada GATE de um ENVELOPE que tenha o som do OSC na entrada IN. O teclado passa a tocar o patch.
4. Num programa de gravação, ligue o envio de relógio MIDI para o Rasgo Modular. Ligue CLK na entrada EXT do CLOCK e RST na entrada RST do CLOCK, e aperte o play do programa: o patch anda no andamento dele e começa no mesmo tempo.
