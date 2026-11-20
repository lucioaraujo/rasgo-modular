## O que é
GENDYN es una fuente sonora en la que la onda es un polígono de pocos puntos. En cada vuelta, cada punto cambia un poco de altura y de distancia hasta el siguiente, al azar. La onda nunca se repite, pero tampoco se escapa: un sonido que vive.

## Como pensar nele
Es la síntesis estocástica dinámica de Iannis Xenakis, la de Gendy3, de 1991. Cada punto da un paso al azar, como un borracho, y los bordes lo reflejan de vuelta, para que nunca se pegue a ellos. Con BAND en 0, la altura queda sujeta y solo el dibujo de la onda respira: una nota afinada y viva. Con BAND en 1, el período también vaga, y el sonido llega hasta el ruido. DIST elige el tipo de paso; la distribución de Cauchy, la preferida de Xenakis, da saltos raros y grandes.

## Controles
- PTS: cuántos puntos tiene el polígono.
- FREQ: la altura central.
- BAND: cuánto puede escaparse la altura.
- AMP: el paso de las amplitudes.
- DUR: el paso de las duraciones.
- DIST: el tipo de paso.
- LEVEL: el nivel.
Entradas: PTCH suma una altura en voltios por octava, FRZ congela la onda mientras el gate está alto, AMP se suma al paso de las amplitudes. Salida: OUT es el sonido.

## Experimente
1. Ponga BAND en 0,1 y conecte OUT a un FILTER y a un VCA: un bajo que respira.
2. Suba BAND y AMP hasta 1: la masa de ruido de Xenakis.
3. Conecte un gate lento a FRZ: la onda se congela y se descongela.
4. Conecte una secuencia a PTCH con BAND bajo: una melodía de timbres vivos.
