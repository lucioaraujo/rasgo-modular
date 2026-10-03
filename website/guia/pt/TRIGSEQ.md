## O que é
O TRIGSEQ gera ritmos de bateria em quatro linhas ao mesmo tempo, pensadas para bumbo, caixa, chimbal e uma percussão extra. Você não marca as batidas uma por uma: escolhe um estilo, decide a densidade de cada linha, e ele cria o padrão. Ao carregar, já está tocando.

## Como pensar nele
Quatro CLOCK, um por instrumento, gerariam linhas que não conversam entre si. No TRIGSEQ as quatro nascem do mesmo estilo, e por isso se encaixam como numa bateria tocada por uma pessoa. O controle MAP passa por quatro caracteres: reto, como no rock e na house; quebrado, como no breakbeat; com balanço, como no hip-hop; e esparso, como no dub. Girando devagar, o groove muda de personalidade sem perder o pulso.

## Controles
- LEN: quantos dos 16 passos entram no ciclo.
- RATE: o relógio interno, usado quando nada chega em CLK.
- MAP: o estilo, do reto ao esparso.
- DNS1, DNS2, DNS3, DNS4: a densidade de cada linha. Mais alto, mais batidas.
- SWING: atrasa um passo a cada dois, dando balanço.
- CHAOS: a chance de surgirem batidas fantasmas ou de faltarem batidas previstas, o que torna o ritmo menos mecânico.
- RATCH: a chance de uma batida virar uma rajada rápida de repetições, um rufo.
- FILL: quanto a entrada FILL aumenta as densidades, para viradas.
- DRIFT: deixa o estilo e as densidades variarem aos poucos, e o groove evolui sozinho.
Entradas: CLK, o pulso; RST, para voltar ao começo; FILL, um sinal que dispara a virada; MAP, para modular o estilo. Saídas: T1 a T4, as quatro linhas; ACC, os acentos; ANY, um pulso sempre que qualquer linha toca.

## Experimente
1. Ligue a saída CLK do CLOCK na entrada CLK do TRIGSEQ.
2. Ponha três DRUM no rack. Ligue T1, T2 e T3 nas entradas GATE de cada um, e as saídas OUT no MIXER.
3. Ajuste TONE e DECAY de cada DRUM para soar como bumbo, caixa e chimbal.
4. Gire MAP devagar e ouça o groove mudar de estilo. Depois mexa nas densidades.
