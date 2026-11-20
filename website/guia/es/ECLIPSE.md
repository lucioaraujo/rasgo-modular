## O que é
ECLIPSE es la Luna con sus dos meses. Uno es el de las fases, de luna nueva a luna nueva. El otro es el regreso a los nodos, los dos puntos donde la órbita de la Luna cruza el plano de la Tierra. Solo hay eclipse cuando las dos cosas coinciden, y cada eclipse da un gate en SUN o en MOON.

## Como pensar nele
Los dos meses casi encajan, pero no del todo. Por eso los eclipses vienen en temporadas, más o menos cada seis meses, y la secuencia entera solo vuelve tras 223 meses: el Saros, que ya conocían los babilonios. Es un polirritmo astronómico, raro y previsible. Los números son los del cielo del Asterismus. SYN y DRAC cambian los meses y dan otros cielos.

## Controles
- SPEED: cuántos días por segundo.
- LIMIT: el límite de eclipse.
- GATE: la duración del gate.
- SYN: el mes de las fases.
- DRAC: el mes de los nodos.
Entradas: CLK hace que cada pulso valga un día, RST vuelve al día 0. Salidas: SUN y MOON son los eclipses, NEW cada luna nueva, NODE cada paso por un nodo, ALIGN la llegada de la temporada, PHASE la fase de la Luna.

## Experimente
1. Conecte NEW a un DRUM y SUN a un acorde: el pulso del mes, y de vez en cuando un acontecimiento raro.
2. Conecte ALIGN al nivel de una voz: crece cuando se acerca la temporada de eclipses.
3. Conecte PHASE al corte de un FILTER: un LFO de un mes.
4. Acerque DRAC a SYN: los eclipses se vuelven frecuentes; aléjelo: se vuelven rarísimos.
