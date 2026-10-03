## O que é
CONTROL ajusta señales de control antes de que lleguen a su destino. Puede achicar, agrandar o invertir una señal, desplazarla hacia arriba o hacia abajo, suavizar sus escalones y sumar dos señales. Son operaciones discretas, pero aparecen en casi todos los patches que van más allá de lo básico.

## Como pensar nele
A veces una modulación tiene la forma justa y el tamaño equivocado, o el sentido equivocado: una envolvente que debería cerrar el filtro en lugar de abrirlo, una onda que debería oscilar solo por encima de cero. La ganancia del cable achica una señal, pero no la invierte ni la desplaza. CONTROL hace todo eso en un panel, con dos canales iguales y una salida que los junta. Con un sonido real en la entrada, también puede seguir el volumen de ese sonido y convertirlo en control, lo que se llama seguidor de envolvente.

## Controles
- SCALE: el tamaño de la señal, en cada canal. En 1 pasa igual; en 0,5, a la mitad; negativo, invertida. En cero solo queda el desplazamiento de OFF, y el canal se vuelve un control manual.
- OFF: un valor que se suma a la señal y la desplaza hacia arriba o hacia abajo.
- RECT: pliega hacia arriba la parte negativa de la señal. En el centro, la parte negativa desaparece; al máximo, se refleja.
- SLEW: suaviza los cambios bruscos, en hasta dos segundos. Los escalones se vuelven rampas.
- CRV: la forma de ese suavizado, de rampa recta a curva que se va frenando.
- SUM: si la salida SUM suma los dos canales o saca su promedio.
- DRIFT: un paseo lento en los desplazamientos, que quita rigidez a un valor fijo.
Entradas: IN1 e IN2. Salidas: O1, O2 y SUM.

## Experimente
1. Arme una voz: la SAW de un OSC a la entrada IN de un FILTER, y LO del FILTER al MIXER. Baje CUT.
2. Conecte la salida ENV de un ENVELOPE, tocado por el CLOCK, a IN1 de CONTROL, y O1 a la entrada FC del FILTER. Cada nota abre el filtro.
3. Lleve SCALE del primer canal a −1 y suba CUT. Ahora cada nota cierra el filtro.
4. Conecte la salida BI de un FUNCTION a IN2 y escuche la salida SUM en lugar de O1: las dos modulaciones actúan juntas.
