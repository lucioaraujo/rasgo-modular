## O que é
O TAP dá ao patch o andamento de quem toca. Bata três ou mais vezes na entrada TAP (um botão, uma tecla, palmas pelo SIGNAL-IN): ele mede o intervalo e passa a dar um pulso a cada tempo, sozinho, no seu andamento. A primeira batida é o tempo forte, o um.

## Como pensar nele
É a contagem antes de a banda entrar. O TAP usa a mediana das batidas, então uma batida torta não estraga a conta. Se as batidas forem irregulares demais, ele não inventa um andamento: espera uma contagem melhor. Uma pausa e uma contagem nova trocam o andamento.

## Controles
- COMP: de quantos tempos é o compasso, de 1 a 16. A saída UM volta a cada COMP tempos.
- DISP: quanta irregularidade ele aceita na contagem.
Entradas: TAP recebe as batidas, RST esquece o andamento. Saídas: CLK é o pulso a cada tempo, UM o tempo forte, BPM o andamento como CV, TRAV um gate enquanto o andamento está travado.

## Experimente
1. Ligue o SIGNAL-IN no SCOPE e a saída ONS do SCOPE em TAP. Ligue o ON do SIGNAL-IN.
2. Ligue CLK na entrada de relógio externo do CLOCK: o patch inteiro passa a seguir o seu pulso.
3. Bata um, dois, três, quatro e comece a tocar: o patch já está no andamento.
4. Ponha COMP em 3 e ligue UM no RST de um SEQUENCE: a frase recomeça em cada compasso de valsa.
