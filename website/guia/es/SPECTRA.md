## O que é
SPECTRA escucha un sonido, encuentra sus frecuencias más fuertes y las vuelve a tocar con un conjunto de osciladores senoidales. El resultado es una especie de sombra del sonido original: reconocible, pero hecha solo de tonos puros, que usted puede transponer, deformar o congelar.

## Como pensar nele
Está entre los módulos de fuente porque lo que suena es su salida, pero casi siempre necesita algo en la entrada IN: una voz, un acorde, un tambor, el sonido de afuera a través de SIGNAL-IN. Con pocas voces hace una caricatura del sonido; con muchas, una copia fiel. Su gesto más fuerte es FRZ: congela el último espectro y lo deja sonando indefinidamente, como una textura sacada de cualquier instante.

## Controles
- VOICE: cuántas senoidales usa el módulo, de 2 a 24. Pocas simplifican el sonido; muchas lo reproducen con fidelidad.
- BLUR: la rapidez con que cada senoidal sigue al sonido que entra. En cero lo sigue de cerca; al máximo se arrastra y emborrona, y el sonido parece derretirse.
- SHIFT: transpone la reconstrucción hasta dos octavas hacia arriba o hacia abajo, sin tocar la escucha.
- STRCH: separa o acerca las frecuencias entre sí, llevando el sonido hacia lo metálico o la campana sin cambiar la altura percibida.
- TONE: oscurece o realza los agudos de la reconstrucción. En el centro queda fiel a lo que se oyó.
- JITR: hace oscilar levemente cada senoidal, para que la reconstrucción nunca suene quieta.
- FRZ: congela la escucha. Las senoidales siguen tocando el último espectro.
- MIX: la mezcla entre el sonido original y la reconstrucción. Al máximo solo oye la reconstrucción.
Entradas: IN para el sonido a analizar, PIT para transponer y FRZ para congelar con un gate. Salidas L y R.

## Experimente
1. Conecte la salida OUT del CHORD a la entrada IN de SPECTRA, y la salida L a una entrada del MIXER. Oye el acorde rehecho en tonos puros.
2. Baje VOICE a 3 o 4. El acorde se vuelve un boceto de sí mismo.
3. Lleve STRCH hacia un lado y escuche el acorde volverse metálico.
4. Conecte la salida EUC del CLOCK a la entrada FRZ. El sonido se congela y se suelta al ritmo del reloj.
