## O que é
TAP da al patch el tempo de quien toca. Golpee tres o más veces en la entrada TAP (un botón, una tecla, palmas por el SIGNAL-IN): mide el intervalo y empieza a dar un pulso en cada tiempo, solo, a su tempo. El primer golpe es el tiempo fuerte, el uno.

## Como pensar nele
Es la cuenta antes de que entre la banda. TAP usa la mediana de los golpes, así que un golpe torcido no estropea la cuenta. Si los golpes son demasiado irregulares, no inventa un tempo: espera una cuenta mejor. Una pausa y una cuenta nueva cambian el tempo.

## Controles
- COMP: de cuántos tiempos es el compás, de 1 a 16. La salida UM vuelve cada COMP tiempos.
- DISP: cuánta irregularidad acepta en la cuenta.
Entradas: TAP recibe los golpes, RST olvida el tempo. Salidas: CLK es el pulso en cada tiempo, UM el tiempo fuerte, BPM el tempo como CV, TRAV un gate mientras hay tempo.

## Experimente
1. Conecte el SIGNAL-IN al SCOPE y la salida ONS del SCOPE a TAP. Active el ON del SIGNAL-IN.
2. Conecte CLK a la entrada de reloj externo del CLOCK: todo el patch sigue su pulso.
3. Golpee uno, dos, tres, cuatro y empiece a tocar: el patch ya está a su tempo.
4. Ponga COMP en 3 y conecte UM al RST de un SEQUENCE: la frase vuelve a empezar en cada compás de vals.
