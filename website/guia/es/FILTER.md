## O que é
FILTER deja pasar parte de las frecuencias de un sonido y corta el resto. Tiene tres salidas que funcionan a la vez: LO, que deja pasar los graves; CTR, la banda del medio; y HI, los agudos. Al girar el corte, oye el sonido oscurecerse o aclararse, como cuando uno se tapa la boca al hablar.

## Como pensar nele
Es el segundo módulo de casi todos los patches: después de una fuente rica en armónicos, como la sierra de un OSC, el filtro esculpe el timbre. Lo que lo distingue es SPRD: en cero, las tres salidas comparten el mismo corte; al subirlo, se separan y se vuelven tres filtros distintos, cada uno en una región del sonido. Con la resonancia al máximo, el filtro empieza a sonar solo, como un oscilador senoidal.

## Controles
- CUT: la frecuencia de corte, de 20 a 20000 Hz. Es el control principal: gírelo y escuche el timbre abrirse y cerrarse.
- RESO: realza las frecuencias cercanas al corte, dando un timbre más nasal. Al máximo, el filtro empieza a sonar solo.
- SPRD: separa las tres salidas en frecuencia. En cero, las tres siguen el mismo corte; al máximo, cada una ocupa su región del sonido.
- DRIVE: satura la señal antes de filtrar, engordando el sonido.
Entradas: IN, el sonido a filtrar; FC, para mover el corte (1 V por octava); Q, para mover la resonancia; SPR, para mover la separación. Salidas: LO, CTR, HI y ALL, que suma las tres.

## Experimente
1. Conecte la salida SAW de un OSC a la entrada IN, y la salida ALL a una entrada del MIXER.
2. Gire CUT despacio de un lado al otro y escuche el brillo entrar y salir.
3. Suba RESO hasta la mitad y repita: el corte se marca, casi como una voz.
4. Conecte la salida ENV de un ENVELOPE a la entrada FC, con el CLOCK en la entrada GATE de la envolvente. El filtro se abre con cada pulso, un sonido presente en muchísima música electrónica.
