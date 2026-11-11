## O que é
O ARP é um arpejador: toca um acorde nota por nota, e assim um acorde vira melodia. Ele escolhe as notas (um dos dez formatos de acorde do CHORD, com inversão e oitavas) e o caminho por elas. Sem nada ligado, toca no próprio relógio.

## Como pensar nele
Separe duas perguntas: quais notas, e em que ordem. CHORD, INV e OCT respondem a primeira; MODE responde a segunda (subindo, descendo, ida e volta, ao acaso, embaralhado, do centro para fora, de fora para o centro). SKIP abre buracos na linha, e os buracos são sempre os mesmos para o mesmo patch, então a linha soa tocada e se repete igual.

## Controles
- CHORD: o formato do acorde, de 0 a 9.
- INV: a inversão, de 0 a 3.
- OCT: por quantas oitavas o arpejo se espalha, de 1 a 4.
- MODE: o caminho pelas notas, de 0 a 7.
- SKIP: a chance de um passo ficar em silêncio.
- RATE: o relógio interno, usado quando nada está ligado em CLK.
- GATE: a duração de cada nota, de 5 a 95 % do passo.
Entradas: CLK avança um passo por pulso, RST volta ao começo, ROOT recebe a tônica (1 volt por oitava), CHD troca o formato do acorde. Saídas: PTCH é a nota, GT o gate de cada nota, EOS um pulso a cada volta completa.

## Experimente
1. Ligue PTCH no 1V/O de um OSC, a saída do OSC num VCA e o VCA no MIXER. Ligue GT num ENVELOPE e o ENVELOPE no VCA.
2. Gire MODE e ouça o mesmo acorde ganhar outros desenhos.
3. Ponha MODE em 5, SKIP em 0,3 e OCT em 3: parece uma linha improvisada.
4. Ligue a saída root do HARMONY em ROOT: o arpejo passa a andar com a progressão.
