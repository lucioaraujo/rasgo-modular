## O que é
O FILTER deixa passar parte das frequências de um som e corta o resto. Ele tem três saídas que funcionam ao mesmo tempo: LO, que deixa passar os graves; CTR, a faixa do meio; e HI, os agudos. Girando o corte, você ouve o som escurecer ou clarear, como quando se cobre a boca ao falar.

## Como pensar nele
É o segundo módulo de quase todo patch: depois de uma fonte rica em harmônicos, como a serra de um OSC, o filtro esculpe o timbre. O que ele tem de particular é o SPRD: em zero, as três saídas usam o mesmo corte; subindo, elas se afastam e viram três filtros diferentes, cada um numa região do som. Com a ressonância no máximo, o filtro passa a soar sozinho, como um oscilador de seno.

## Controles
- CUT: a frequência de corte, de 20 a 20000 Hz. É o controle principal: gire e ouça o timbre abrir e fechar.
- RESO: realça as frequências perto do corte, dando um timbre mais nasal. No máximo, o filtro começa a soar sozinho.
- SPRD: afasta as três saídas em frequência. Em zero, as três seguem o mesmo corte; no máximo, cada uma fica numa região do som.
- DRIVE: satura o sinal antes de filtrar, engordando o som.
Entradas: IN, o som a filtrar; FC, para mover o corte (com 1 V por oitava); Q, para mover a ressonância; SPR, para mover o afastamento. Saídas: LO, CTR, HI e ALL, que soma as três.

## Experimente
1. Ligue a saída SAW de um OSC na entrada IN, e a saída ALL numa entrada do MIXER.
2. Gire CUT devagar de um lado ao outro e ouça o brilho entrar e sair.
3. Suba RESO até a metade e repita: o corte fica marcado, quase uma voz.
4. Ligue a saída ENV de um ENVELOPE na entrada FC, com o CLOCK na entrada GATE do envelope. O filtro abre a cada pulso, no efeito que se ouve em tanta música eletrônica.
