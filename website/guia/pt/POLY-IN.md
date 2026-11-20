## O que é
O POLY-IN reparte as notas de um teclado MIDI ou de uma faixa da DAW em até quatro vozes. Cada voz tem a sua altura, o seu gate e a sua velocidade, e cada uma pode tocar o seu próprio oscilador.

## Como pensar nele
O SIGNAL-IN dá uma voz só; o POLY-IN dá um acorde. O rack aceita até quatro cópias de cada módulo: com quatro OSC e quatro ENVELOPE, cada voz do POLY-IN toca um par, e o teclado passa a tocar acordes no patch. O módulo começa desligado e não pede o microfone: ele só abre o MIDI.

## Controles
- VOZES: quantas vozes recebem notas, de uma a quatro.
- MODE: como as notas se repartem. *Rota* passa cada nota para a próxima voz livre; *Mesma* devolve a nota repetida à voz em que ela tocou; *Altura* põe as vozes em ordem, e a voz 1 fica sempre com a nota mais grave. Sem voz livre, a nota nova toma a voz mais antiga.
- BEND: quantos semitons o pitch bend move todas as vozes.
- ON: liga a entrada MIDI.
Saídas: P1 a P4, a altura de cada voz; G1 a G4, o gate de cada voz, aberto enquanto a tecla ou o pedal segura a nota; V1 a V4, a força de cada nota.

## Experimente
1. Ligue o ON. Ligue P1 na entrada 1V/O de um OSC e G1 na entrada GATE de um ENVELOPE que receba o som desse OSC.
2. Acrescente mais um OSC e mais um ENVELOPE e ligue neles P2 e G2. Toque duas notas juntas: cada uma soa no seu par.
3. Com quatro pares e MODE em *Altura*, a voz 1 é sempre o baixo do acorde: dê a ela um filtro mais fechado ou um volume maior no MIXER.
