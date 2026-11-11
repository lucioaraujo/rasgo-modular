## O que é
O COMPARADOR mistura dois sinais com polaridade e decide quando a mistura passa de um limiar. Os knobs A e B têm o zero no centro: para a direita a entrada soma, para a esquerda entra invertida. A saída COMP é um gate que fica alto enquanto a mistura está acima do limiar.

## Como pensar nele
É uma balança. Com A à direita e B à esquerda, a mistura é a diferença entre os dois sinais; com o mesmo sinal nos dois, ela se anula. A decisão vem depois: o gate abre quando a balança pende para um lado. A histerese impede que ele tremule quando o sinal tem ruído perto do limiar.

## Controles
- A, B: o quanto cada entrada entra, de −1 (invertida) a 1.
- OFS: desloca a mistura toda para cima ou para baixo.
- LIM: o limiar da decisão.
- HIST: a histerese, uma folga em torno do limiar.
Entradas: A e B recebem os sinais, LIM move o limiar por CV. Saídas: MIX é a mistura, INV a mistura de cabeça para baixo, COMP o gate da decisão, CRUZ um pulso a cada mudança de lado.

## Experimente
1. Ligue dois FUNCTION lentos, em velocidades diferentes, em A e B. Ponha A à direita e B à esquerda.
2. Ligue COMP no gate de um ENVELOPE e CRUZ num DRUM: dois movimentos lentos que se cruzam viram ritmo.
3. Ligue o mesmo sinal em A e B com A em 1 e B em −1: a mistura some.
4. Ligue o LVL de um SCOPE em A e ponha LIM no meio: quando quem toca passa de um volume, outra voz entra.
