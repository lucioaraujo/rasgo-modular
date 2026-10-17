## O que é
LISTEN sigue la energía de un sonido en tres regiones: graves, medios y agudos. Cada región ofrece un nivel continuo y un pulso cuando detecta un ataque. Puede usar estas señales para que otra voz responda a lo que está sonando.

## Como pensar nele
Un golpe grave puede llamar a una nota; los platos pueden cambiar el brillo de una voz. Las regiones se solapan: un instrumento puede aparecer en varias salidas. LISTEN mide frecuencias y contrastes de energía, sin separar instrumentos ni descubrir el tempo. THRU conserva el sonido recibido para que también pueda escucharlo. Los medidores muestran niveles y destacan ataques recientes.

## Controles
- LOW: separación entre graves y medios, de 60 a 800 Hz. Mover un corte suspende los ataques durante 50 ms para evitar disparos causados por el ajuste. Una modulación continua puede mantenerlos suspendidos.
- HIGH: separación entre medios y agudos, de 1 a 8 kHz, limitada por la frecuencia de muestreo.
- RESP: suaviza los niveles. Súbalo para prolongar la energía después de cada golpe. No cambia la detección de ataques.
- SENS: sensibilidad a los ataques. Súbala para detectar contrastes menores; bájela si un golpe provoca demasiados eventos.
- FLOOR: umbral de amplitud para disparar, de −72 a −18 dBFS. Subirlo ayuda a rechazar ruidos débiles.
- REARM: espera mínima entre ataques de una misma región, de 20 a 250 ms. Un valor alto puede omitir golpes rápidos.
Entrada: IN, el audio. Salidas: THRU, audio conservado por canal; LOW, MID y HIGH, niveles de 0 a 1; L-HIT, M-HIT y H-HIT, pulsos de 5 ms. El movimiento automático VARIA no altera los controles de calibración.

## Experimente
1. Desactive VARIA. Conecte CLOCK.EUC a DRUM.GATE, DRUM.OUT a LISTEN.IN y LISTEN.THRU a una entrada del MIXER. Empiece con volumen bajo y observe los medidores.
2. Conecte L-HIT a PHRASE.CLK. Para oír la respuesta, conecte PHRASE.PITCH a QUANTIZER.CV, con RANGE=1; QUANTIZER.PITCH a OSC.PITCH; OSC.SAW a ENVELOPE.IN; PHRASE.GATE a ENVELOPE.GATE; ENVELOPE.OUT al MIXER.
3. Cambie TONE y SNAP del DRUM. Ajuste FLOOR y REARM hasta que la voz responda a los golpes deseados. Sustituya L-HIT por H-HIT y escuche cómo cambia el diálogo.
4. Para variar la intensidad, coloque un VCA entre ENVELOPE y MIXER, con LVL1=0 y CV1=1; conecte PHRASE.ACC a VCA.CV1 y LISTEN.LOW a PHRASE.STIM. Para audio externo, envíe SIGNAL-IN.OUT a LISTEN.IN y active ON en SIGNAL-IN mediante un gesto propio.
