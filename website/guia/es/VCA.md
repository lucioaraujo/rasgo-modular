## O que é
VCA controla el volumen de un sonido mediante otra señal. Tiene dos canales iguales: el sonido entra en IN, una señal de control entra en CV, y el volumen de salida sube y baja según ese control. Es el módulo más usado de cualquier modular.

## Como pensar nele
Piense en él como una mano en la perilla de volumen, movida por otro módulo. Con una envolvente en el control, cada nota tiene principio, medio y fin. Con una onda lenta, el volumen ondula, en el efecto llamado trémolo. También sirve para dosificar una señal de control antes de enviarla a otro lugar, y la salida SUM suma los dos canales, como un mezclador pequeño. Atención: el nivel empieza en cero, así que el canal queda mudo hasta que algo llegue a CV o usted suba LVL.

## Controles
- LVL1, LVL2: el volumen de cada canal. La señal de control se suma a este valor.
- CV1, CV2: cuánto actúa la señal de control, y en qué sentido. Negativo invierte: el volumen baja cuando el control sube.
- RSP1, RSP2: la curva de respuesta. En cero, lineal; al máximo, exponencial, que suena más natural para el volumen.
- DRIFT: una oscilación lenta y pequeña en ambos volúmenes.
Entradas: IN1 e IN2, los sonidos; CV1 y CV2, los controles. Salidas: O1, O2 y SUM, la suma de los dos.

## Experimente
1. Conecte la salida SAW de un OSC a IN1, y O1 a una entrada del MIXER. Silencio: LVL1 está en cero.
2. Suba LVL1 despacio y el sonido aparece.
3. Vuelva LVL1 a cero y conecte la salida ENV de un ENVELOPE a CV1, con el CLOCK en la entrada GATE de la envolvente. El sonido ahora late con las notas.
4. Cambie la envolvente por la salida UNI de un FUNCTION lento: el volumen ondula, en trémolo.
