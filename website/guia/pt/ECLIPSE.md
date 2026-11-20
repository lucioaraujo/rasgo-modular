## O que é
O ECLIPSE é a Lua com os seus dois meses. Um é o das fases, de lua nova a lua nova. O outro é o da volta aos nós, os dois pontos onde a órbita da Lua cruza o plano da Terra. Só há eclipse quando as duas coisas coincidem, e cada eclipse dá um gate em SUN ou em MOON.

## Como pensar nele
Os dois meses quase se encaixam, mas não de todo. Por isso os eclipses vêm em temporadas, mais ou menos a cada seis meses, e a sequência inteira só volta depois de 223 meses: o Saros, que os babilônios já conheciam. É um polirritmo astronômico, raro e previsível. Os números são os do céu do Asterismus. SYN e DRAC trocam os meses e dão outros céus.

## Controles
- SPEED: quantos dias por segundo.
- LIMIT: o limite de eclipse.
- GATE: a duração do gate.
- SYN: o mês das fases.
- DRAC: o mês dos nós.
Entradas: CLK faz cada pulso valer um dia, RST volta ao dia 0. Saídas: SUN e MOON são os eclipses, NEW cada lua nova, NODE cada passagem por um nó, ALIGN a chegada da temporada, PHASE a fase da Lua.

## Experimente
1. Ligue NEW num DRUM e SUN num acorde: o pulso do mês, e de vez em quando um acontecimento raro.
2. Ligue ALIGN no nível de uma voz: ela cresce quando a temporada de eclipses se aproxima.
3. Ligue PHASE no corte de um FILTER: um LFO de um mês.
4. Mude DRAC para perto de SYN: os eclipses ficam frequentes; afaste: ficam raríssimos.
