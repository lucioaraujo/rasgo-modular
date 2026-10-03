## O que é
O CHAOS gera um movimento imprevisível a partir de um sistema físico simulado: algo como uma bola que rola entre dois vales, ora se acomodando num deles, ora pulando para o outro. Você não consegue prever quando ela vai pular, mas com a mesma semente tudo se repete do mesmo jeito.

## Como pensar nele
Outros módulos sorteiam valores, como o TURING e o DECISION, ou passeiam devagar, como o DRIFT. O CHAOS tem um comportamento próprio: passa um tempo rondando uma região e de repente muda para outra. Num filtro, isso dá um timbre que hesita e depois salta; numa melodia, frases que ficam numa região e se mudam de repente. Em velocidades altas, ele mesmo vira uma textura sonora.

## Controles
- RATE: a velocidade do movimento, de 0,02 a 400 ciclos por segundo. Lento para mover controles; rápido para ouvir direto.
- DRIVE: a força que empurra a bola entre os vales. Mais alto, saltos maiores e mais frequentes.
- DAMP: o atrito. Alto, a bola assenta num vale e quase para; baixo, ela oscila muito e pula de um lado para o outro.
- FRZ: congela o movimento no valor atual.
Entradas: RSD, um pulso que dá um empurrão novo; RTM, para modular a velocidade. Saída: OUT.

## Experimente
1. Faça uma voz: SAW de um OSC na entrada IN de um FILTER, LO do FILTER no MIXER.
2. Ligue OUT do CHAOS na entrada FC do FILTER. Ponha RATE em torno de 0,3, DRIVE alto e DAMP baixo.
3. Ouça o timbre ficar um tempo numa região e saltar para outra.
4. Ligue OUT do CHAOS também na entrada CV de um QUANTIZER, e PTCH dele em 1V/O do OSC. Agora a melodia muda de região junto com o timbre.
