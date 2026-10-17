## O que é
PHRASE propõe um motivo e o toca quando recebe pulsos em CLK. A ideia retorna a cada frase, com mudanças de direção, pausas e acentos que você regula. Suas saídas comandam outras vozes; para ouvi-lo, é preciso ligá-lo a um oscilador e a um envelope.

## Como pensar nele
Use CLOCK para um pulso regular ou os ataques do LISTEN para responder a outro som. SEED identifica o motivo. VAR transforma suas passagens; em zero, fica mais fácil reconhecer o desenho que volta. A entrada PITCH oferece um apoio, capturado no início de cada frase. Para escolher uma escala, passe a saída PITCH pelo QUANTIZER com RANGE=1. PHRASE não inclui por conta própria um repertório de estilos ou tradições musicais.

## Controles
- LEN: duração da frase em acontecimentos, de 2 a 16 pulsos. Não é uma duração fixa em segundos.
- SPAN: âmbito do desenho, de 1 a 36 semitons, antes da quantização.
- DIR: inclina o desenho para baixo ou para cima. No centro, mantém a direção do motivo de base.
- VAR: quanto cada passagem se transforma. Comece em zero e aumente ouvindo o que ainda reconhece.
- SPACE: probabilidade de pausas nos passos intermediários. O início e o fechamento são preservados.
- CAD: força do retorno ao apoio no final.
- GATE: duração de cada articulação, de 5 a 400 ms. A nota só fica audível se GATE estiver ligado ao envelope da voz.
- SEED: identidade numérica do motivo, de 1 a 65535. Mudar recria o desenho e recomeça; VARIA não move esse controle.
Entradas: PITCH, apoio em 1 V por oitava; CLK, um passo por borda ascendente; RST, reinício; STIM, intensidade de 0 a 1; RET, pedido de conclusão no próximo passo. Saídas: PITCH, altura mantida; GATE, articulação; ACC, acento de 0 a 1; END, pulso de 5 ms no fechamento; PHASE, posição de 0 a 1 na frase. Sem STIM conectado, a intensidade é 1. Ao reabrir o patch, a frase recomeça: não retoma a posição exata da execução anterior.

## Experimente
1. Desligue VARIA. Ligue CLOCK.EUC a PHRASE.CLK, PHRASE.PITCH a QUANTIZER.CV (RANGE=1), e QUANTIZER.PITCH a OSC.PITCH. Escolha uma escala no QUANTIZER.
2. Ligue OSC.SAW a ENVELOPE.IN, PHRASE.GATE a ENVELOPE.GATE e ENVELOPE.OUT ao MIXER. Comece com VAR=0 e SPACE=0. Ouça a ideia repetir; altere DIR e SPAN, um por vez.
3. Ponha um VCA entre a voz e o MIXER: ENVELOPE.OUT em VCA.IN1, VCA.O1 no MIXER, LVL1=0 e CV1=1. Ligue PHRASE.ACC a VCA.CV1 para ouvir os acentos. Aumente VAR e depois SPACE.
4. Troque CLOCK por LISTEN.L-HIT em CLK e ligue LISTEN.LOW a STIM. Agora o som escutado chama e intensifica as notas. Um pulso em RET pede um fechamento no próximo CLK; END pode chamar uma segunda voz.
