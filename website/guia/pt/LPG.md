## O que é
O LPG, de low pass gate, é um portão que abre e fecha o som a cada golpe, ao mesmo tempo baixando o volume e escurecendo o timbre. Ele imita um componente antigo, o vactrol, que reage rápido mas solta devagar, e por isso cada nota soa como algo percutido: uma marimba, uma kalimba, uma gota.

## Como pensar nele
Dá para montar algo parecido com FILTER, VCA e ENVELOPE, mas falta o jeito do vactrol: subida instantânea e uma queda que vai freando perto do fim. O LPG traz isso pronto. Basta um som contínuo na entrada IN e pulsos na entrada STRK: cada pulso vira uma nota tocada. MODE escolhe se ele age mais como filtro, mais como controle de volume ou como os dois.

## Controles
- MODE: para a esquerda, só filtro; para a direita, só volume; no meio, os dois juntos, que é o som característico do LPG.
- RESP: quanto tempo a nota leva para morrer depois do golpe, de um toque curtíssimo a uns dois segundos e meio. A subida é sempre rápida.
- OFST: quanto o portão fica aberto em repouso. Em zero, fecha totalmente entre os golpes.
- RESO: a ressonância do filtro, que dá um tom mais marcado.
- BNCE: acrescenta um pequeno repique logo depois de cada golpe.
- DRIFT: varia de leve, e de forma lenta, a duração de cada nota.
Entradas: IN, o som; STRK, o pulso que golpeia; CV, para abrir o portão com um sinal contínuo.

## Experimente
1. Ligue a saída TRI de um OSC na entrada IN, e OUT numa entrada do MIXER. Por enquanto, silêncio: o portão está fechado.
2. Ligue a saída EUC do CLOCK na entrada STRK. Cada pulso vira uma nota de timbre percutido.
3. Gire RESP para ouvir as notas ficarem curtas ou longas.
4. Ligue a saída PTCH de uma SEQUENCE, movida pelo mesmo CLOCK, na entrada 1V/O do OSC. Agora você tem uma linha de marimba.
