## O que é
A VOLTA é um laço de passos que varia sem se perder. Ela guarda uma sequência-base e a transforma por cima; quando você volta o knob, a sequência original volta exatamente como era.

## Como pensar nele
O TURING reescreve o próprio laço, e o que muda se perde. A VOLTA nunca reescreve a base: cada passo tem a sua vez de mudar, sempre na mesma ordem, e por isso dá para ir e voltar. Só um pulso em NEW troca a base por uma nova.

## Controles
- RATE: o andamento do relógio interno, quando não há CLK.
- LEN: quantos passos tem o laço.
- MUT: quantos passos trocam pelo valor alternativo.
- ERODE: quantos passos calam.
- MEM: os passos protegidos, que não mudam nem calam.
- RANGE: a amplitude da CV.
- STEPS: em quantos degraus a CV é quantizada.
- OFST: um deslocamento somado à CV.
- SEED: a semente do laço e da ordem das mudanças.
Entradas: CLK, o relógio; RST, volta ao primeiro passo; NEW, uma base nova; MUT e ERO, sinais somados aos knobs. Saídas: CV e CV2, as alturas; PULSE, os gates; POS, a posição no laço.

## Experimente
1. Ligue o CLK do CLOCK no CLK da VOLTA, a CV num QUANTIZER e dali num OSC, e PULSE num ENVELOPE.
2. Suba o MUT devagar: a melodia varia. Volte o MUT a zero: ela volta inteira.
3. Ligue um FUNCTION lento em MUT: a melodia se afasta e retorna sozinha.
