## O que é
O SPECTRA escuta um som, encontra as frequências mais fortes que há nele e as toca de novo com um conjunto de osciladores de seno. O resultado é uma espécie de sombra do som original: reconhecível, mas feita só de tons puros, e que você pode transpor, deformar ou congelar.

## Como pensar nele
Ele fica entre os módulos de fonte porque é a sua saída que soa, mas quase sempre precisa de algo na entrada IN: uma voz, um acorde, um tambor, o som de fora pelo SIGNAL-IN. Com poucas vozes, ele faz uma caricatura do som; com muitas, uma cópia fiel. O gesto mais forte é o FRZ: congela o último espectro e o deixa soando indefinidamente, como um pad tirado de qualquer instante.

## Controles
- VOICE: quantos senos o módulo usa, de 2 a 24. Poucos simplificam o som; muitos o reproduzem com fidelidade.
- BLUR: a rapidez com que cada seno segue o som que entra. Em zero, acompanha de perto; no máximo, arrasta e borra, e o som parece derreter.
- SHIFT: transpõe a reconstrução até duas oitavas para cima ou para baixo, sem mexer na escuta.
- STRCH: afasta ou aproxima as frequências entre si, levando o som para algo metálico, de sino, sem mudar a altura percebida.
- TONE: escurece ou realça os agudos da reconstrução. No meio, fica fiel ao que foi ouvido.
- JITR: faz cada seno oscilar de leve, para a reconstrução nunca soar parada.
- FRZ: congela a escuta. Os senos continuam tocando o último espectro.
- MIX: a mistura entre o som original e a reconstrução. No máximo, você ouve só a reconstrução.
Entradas: IN para o som a analisar, PIT para transpor e FRZ para congelar com um gate. Saídas L e R.

## Experimente
1. Ligue a saída OUT do CHORD na entrada IN do SPECTRA, e a saída L numa entrada do MIXER. Você ouve o acorde refeito em tons puros.
2. Baixe VOICE até 3 ou 4. O acorde vira um esboço de si mesmo.
3. Leve STRCH para um dos lados e ouça o acorde ficar metálico.
4. Ligue a saída EUC do CLOCK na entrada FRZ. O som congela e se solta no ritmo do relógio.
