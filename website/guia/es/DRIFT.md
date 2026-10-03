## O que é
DRIFT produce movimientos muy lentos, en la escala de los minutos. Conectado a algunos controles de un patch, hace que la música cambie poco a poco por sí sola: el timbre a los treinta segundos es otro a los cuatro minutos, sin saltos y sin volver al mismo punto.

## Como pensar nele
Un patch que nadie toca tiende a dar vueltas en círculo después de un rato. DRIFT es la capa del desarrollo lento. Por dentro hay un único valor que pasea, y las cuatro salidas A, B, C y D son versiones de ese mismo paseo, cada una con su peso. Por eso cuentan la misma historia desde ángulos distintos: cuando una sube, las otras tienden a subir también. El paseo tiene además inercia: cuando toma una dirección, tiende a mantenerla un tiempo.

## Controles
- RATE: el ritmo del paseo, de un paso por segundo a uno cada ocho minutos, más o menos.
- DEPTH: el alcance, cuánto puede alejarse el paseo del punto de reposo.
- MOMT: la inercia. Alta, el camino es suave e insiste en una dirección; baja, cambia de rumbo a cada momento.
- STRD: en cero, las cuatro salidas andan casi juntas; al máximo, cada una va por su lado.
- ANCHR: la memoria. En cero, el paseo nunca vuelve; alta, tiende a regresar a lugares por los que ya pasó, lo que da a la pieza algo así como temas que reaparecen.
- BIAS: desplaza el punto de reposo hacia arriba o hacia abajo.
Entradas: ADV, un pulso que fuerza un paso; RATE, para modular la velocidad. Salidas: A, B, C y D, los cuatro aspectos del paseo; FLD, el paseo sin pesos; EVT, un pulso en cada paso.

## Experimente
1. Arme cualquier voz que pase por un FILTER y por un SPACE antes del MIXER.
2. Conecte A a la entrada FC del FILTER y B a la entrada FBK del SPACE. Suba DEPTH.
3. Déjelo sonar unos minutos. El timbre y el espacio cambian juntos, sin prisa.
4. Para seguir la forma de la música, conecte la salida EOS de una SEQUENCE a ADV: el paseo da un paso al final de cada frase.
