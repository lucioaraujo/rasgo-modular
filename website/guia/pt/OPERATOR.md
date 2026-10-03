## O que é
O OPERATOR é um sintetizador de FM com quatro osciladores de seno, chamados operadores. Em FM, um operador não soa sozinho: ele empurra a frequência de outro para frente e para trás muito depressa, e disso nascem harmônicos novos. É assim que se fazem pianos elétricos, sinos, baixos estalados e timbres metálicos, como nos sintetizadores digitais dos anos 1980.

## Como pensar nele
Três decisões fazem o timbre: quem modula quem (ALGO), a relação de frequência entre os operadores (RB, RC, RD) e a força da modulação (INDEX). Comece com INDEX baixo e suba aos poucos: o som vai de um seno limpo a algo brilhante e áspero. O gesto mais útil é ligar um envelope na entrada IDX, para cada nota começar brilhante e ir escurecendo, como uma tecla de piano elétrico.

## Controles
- FREQ: a altura da nota, de 8 a 8000 Hz.
- FINE: afinação fina, até um semitom para cada lado.
- ALGO: escolhe um entre oito arranjos de quem modula quem. No começo, os operadores ficam em cadeia e o som é mais áspero; no fim, soam lado a lado, mais perto de um órgão.
- RB, RC, RD: a relação de frequência de cada operador com o primeiro. Valores inteiros soam afinados; valores quebrados, como 2,5, soam como sino; valores altos, como metal.
- INDEX: a força da modulação. Em zero, você ouve só senos puros; no máximo, um timbre muito brilhante.
- FBK: faz o primeiro operador modular a si mesmo. Sozinho, já transforma o seno em algo parecido com um dente de serra.
- DRIFT: desafina cada operador um pouquinho, de forma lenta. Em zero, tudo fica estável.
Entradas: 1V/O para a altura e IDX para modular a força da FM.

## Experimente
1. Ligue OUT numa entrada do MIXER com INDEX em zero: um seno simples.
2. Suba INDEX devagar e ouça os harmônicos aparecerem.
3. Mude RB para um valor quebrado e o som vira sino.
4. Ligue a saída ENV de um ENVELOPE na entrada IDX, e a saída CLK do CLOCK na entrada GATE do ENVELOPE. Cada nota abre brilhante e se apaga, como um piano elétrico.
