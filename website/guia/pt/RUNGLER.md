## O que é
O RUNGLER tem dois osciladores e um registrador de 8 bits. A cada volta do oscilador A, o registrador anda uma casa e guarda o estado do oscilador B naquele instante. Os três últimos bits viram uma tensão de 8 degraus, que sai em RUNG e volta para modular os dois osciladores.

## Como pensar nele
É o coração do Benjolin, de Rob Hordijk. O laço se fecha: com pouca modulação, um padrão que se repete; com muita, o circuito tropeça entre travar num ritmo e virar ruído. LOOP é a chave de Hordijk: no máximo, o bit que sai do fim volta para o começo e o padrão de 8 passos se repete para sempre; no meio, de vez em quando um bit novo entra e o padrão se transforma.

## Controles
- FRQA: a frequência do oscilador A, o relógio.
- FRQB: a frequência do oscilador B.
- LOOP: a chave do laço.
- MODA: o quanto o rungler modula A.
- MODB: o quanto o rungler modula B.
- LEVEL: o volume.
Entradas: PTA e PTB somam alturas em volts por oitava, LOOP soma na chave do laço. Saídas: TRIA e TRIB são os triângulos, PWM o comparador, RUNG os 8 degraus.

## Experimente
1. Ponha FRQA em 8 Hz e ligue RUNG num QUANTIZER e num OSC: uma melodia que pensa sozinha.
2. Ligue PWM num FILTER e num VCA com as duas frequências no áudio: o som rouco do Benjolin.
3. Suba LOOP até o fim: o padrão se prende. Baixe um pouco: ele começa a mudar.
4. Suba MODA e MODB: o padrão tropeça até o caos.
