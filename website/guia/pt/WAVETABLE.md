## O que é
O WAVETABLE é um oscilador que guarda uma sequência de 16 formas de onda diferentes, como os quadros de uma animação, da mais brilhante (uma serra) à mais suave (um seno). O knob POS escolhe em que ponto dessa sequência você está. Movendo POS, o timbre muda de forma contínua, sem precisar de filtro.

## Como pensar nele
Enquanto o OSC dá um som fixo que você esculpe depois, o WAVETABLE já nasce em movimento: basta algo mexendo em POS. Um LFO lento faz um pad que respira; um envelope faz cada nota começar brilhante e terminar escura. Ele também sabe ouvir: ligado ao SIGNAL-IN, captura um ciclo do som que entra e passa a usá-lo como forma de onda.

## Controles
- FREQ: a altura da nota, de 8 a 8000 Hz.
- FINE: afinação fina, até um semitom para cada lado.
- POS: a posição na tabela. No começo, o som é cheio de harmônicos; no fim, é quase um seno. A entrada POS soma a este knob, e é aí que o módulo ganha vida.
- WARP: deforma a leitura de cada ciclo e acrescenta harmônicos com um sotaque digital, sem trocar de quadro.
- FM: quanto o sinal na entrada FM mexe na altura.
- DRIFT: uma pequena oscilação lenta na afinação.
Entradas: 1V/O para a altura, POS para mover a posição, FM, CAP para o som a ser capturado e GRAB, um pulso que manda capturar um ciclo novo.

## Experimente
1. Ligue OUT numa entrada do MIXER e gire POS de ponta a ponta. Ouça o timbre passar de áspero a liso.
2. Ligue a saída de um FUNCTION, em velocidade lenta, na entrada POS. Agora o som muda sozinho, em ciclos.
3. Para capturar: ligue o SIGNAL-IN na entrada CAP e um pulso do CLOCK em GRAB. A cada pulso, o oscilador passa a tocar um pedaço do som que está entrando.
