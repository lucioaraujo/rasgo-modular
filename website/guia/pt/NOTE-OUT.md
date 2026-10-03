## O que é
O NOTE-OUT anota as notas de uma voz. Posto no caminho dos sinais de altura e de disparo, ele registra cada nota que passa, com altura, duração, intensidade e acento, na partitura que o app grava junto com o áudio. Não tem controles e não muda o som.

## Como pensar nele
Num patch generativo, as notas surgem e somem. O NOTE-OUT guarda um registro delas para quem quiser estudar, transcrever ou retocar a peça depois. Ele fica no meio do caminho: a altura entra em PITCH e sai igual em PTHR; o pulso entra em GATE e sai igual em GTHR. Ao gravar com REC, o arquivo de texto que acompanha o áudio inclui essas notas.

## Controles
O NOTE-OUT não tem controles.
Entradas: GATE, o pulso de cada nota; PITCH, a altura; VEL, a intensidade; ACC, o acento. Saídas: GTHR e PTHR, os mesmos sinais, para seguir até a voz.

## Experimente
1. Numa melodia feita com QUANTIZER, OSC e ENVELOPE, ligue PTCH do QUANTIZER em PITCH do NOTE-OUT, e PTHR em 1V/O do OSC.
2. Ligue o pulso que dispara o envelope em GATE do NOTE-OUT, e GTHR em GATE do envelope. O som continua o mesmo.
3. Grave um trecho com REC. Ao lado do arquivo de áudio, o arquivo de texto da partitura traz cada nota tocada.
