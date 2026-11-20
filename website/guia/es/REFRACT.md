## O que é
REFRACT es un rayo de luz que nace dentro de una capa de vidrio, atraviesa otras tres y sale al aire. En cada capa se desvía, y su ángulo en cada una sale como CV en A1 a A4.

## Como pensar nele
Es la ley de Snell: al pasar de un medio a otro, el rayo cambia de dirección según sus índices. Yendo de un medio denso a uno menos denso con un ángulo demasiado rasante, no atraviesa: se refleja por completo. Es la reflexión total, el brillo del diamante y el camino de la luz en la fibra óptica. Cada vez que el rayo pasa a reflejarse, sale un pulso en TIR.

## Controles
- N1, N2, N3, N4: los índices de las cuatro capas.
- RATE: la velocidad del barrido del ángulo.
- RANGE: hasta dónde barre el ángulo.
- SHIM: el espejismo, el temblor de los índices.
Entradas: ANG da el ángulo por CV, en lugar del barrido, IDX se suma a todos los índices. Salidas: A1 a A4 son los ángulos en las capas, TIR el pulso de la reflexión total, EXIT el desvío lateral del rayo.

## Experimente
1. Conecte TIR a un DRUM: el barrido pasa por el ángulo crítico dos veces por ciclo, y el DRUM marca cada paso.
2. Conecte A2 y A4 a dos cortes de FILTER: el mismo rayo da dos movimientos parecidos y distintos.
3. Suba SHIM con RANGE cerca del ángulo crítico: TIR se vuelve un ritmo de espejismo.
4. Conecte un LFO a IDX: el vidrio se calienta y se enfría, y el punto de reflexión total se mueve.
