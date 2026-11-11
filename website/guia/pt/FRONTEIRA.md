## O que é
A FRONTEIRA escuta uma melodia e marca onde começa uma ideia nova. Ouvimos uma fronteira onde algo muda: um salto depois de graus conjuntos, uma nota longa depois de curtas, uma pausa. O módulo mede essas três mudanças nota a nota e pulsa quando a força delas passa do limiar.

## Como pensar nele
É o modelo de Cambouropoulos, a partir das regras de agrupamento de Lerdahl e Jackendoff. Para saber se uma nota é fronteira, ele precisa ouvir a seguinte, então o pulso chega uma nota depois. Um silêncio longo fecha a frase na hora. O PHRASE gera um motivo; a FRONTEIRA escuta.

## Controles
- LIM: o limiar de força acima do qual uma mudança vira fronteira.
- MEM: quantas notas ele usa como referência do que é normal.
- MIN: o mínimo de notas entre duas fronteiras.
- SIL: quanto silêncio fecha a frase, de 0,3 a 4 s.
Entradas: PTCH recebe a altura, GATE marca cada nota. Saídas: FRONT pulsa no começo de cada célula, FORCA é a força da última fronteira, NOTAS quantas notas teve a célula, FIM pulsa quando o silêncio fecha a frase.

## Experimente
1. Ligue o SIGNAL-IN no SCOPE, a saída PIT do SCOPE em PTCH e a saída ONS em GATE. Ligue o ON do SIGNAL-IN.
2. Ligue FRONT na entrada FRASE do MIXER, com o palco no modo gesto: cada ideia nova de quem toca muda o palco.
3. Ligue FIM no RST de um SEQUENCE: quando quem toca para, o acompanhamento recomeça com a próxima frase.
4. Ligue NOTAS na duração de um ENVELOPE: frases longas pedem notas mais longas.
