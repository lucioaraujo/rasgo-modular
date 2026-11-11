## O que é
O DYNAMICS é um compressor. Ele segura os picos para o som ficar mais junto, e deixa um som abaixar outro, como o pad que respira quando o bumbo bate. Aqui ele faz uma terceira coisa: o que ele ouve e quanto ele abaixa saem como sinais de controle, para mover qualquer outro módulo.

## Como pensar nele
Pense num técnico de som com a mão no fader: quando o som passa do limiar, ele abaixa; quando o som cai, ele devolve. RATIO é a força da mão, ATK e REL são a pressa dela. A entrada SIDE troca o que ele escuta: com um bumbo ali, é o bumbo que decide quando o resto abaixa. Dois desvios Rasgo mexem no tempo: BRTH faz a volta seguir o intervalo entre os golpes, e DRIFT varia ataque e volta a cada golpe.

## Controles
- THRES: o limiar, de −48 a 0 dB. Acima dele, o som é abaixado.
- RATIO: quanto abaixa. Com 4, cada 4 dB acima do limiar viram 1; com 20, vira um limitador.
- KNEE: de uma vez (0) ou aos poucos (até 12 dB).
- ATK, REL: o tempo de descer (0,1 a 200 ms) e o de voltar (5 ms a 2 s).
- GAIN: devolve o nível perdido, até 24 dB.
- SIDE: o detector ouve a entrada IN (0) ou a entrada SIDE (1).
- RMS: mede a energia média em vez do pico; soa mais suave.
- GATE: inverte a curva e vira um noise gate, que fecha o que fica abaixo do limiar.
- BRTH: a volta acompanha o andamento dos golpes que ele ouve.
- DRIFT: um acaso semeado no ataque e na volta, golpe a golpe.
Entradas: IN recebe o som, SIDE o que o detector deve ouvir, THR move o limiar. Saídas: OUT é o som comprimido, ENV o nível que ele ouve, GR quanto ele abaixa agora.

## Experimente
1. Ligue um TRIGSEQ num DRUM e o DRUM na entrada SIDE. Ligue um CHORD em IN e OUT no MIXER.
2. Deixe SIDE em 1, RATIO em 6, ATK curto e REL perto de 200 ms: o acorde abaixa a cada bumbo.
3. Suba BRTH: a volta passa a respirar no ritmo.
4. Ligue a saída ENV no CUT de um FILTER noutro som: o filtro abre com a força do que o compressor ouve.
