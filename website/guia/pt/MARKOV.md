## O que é
O MARKOV é uma cadeia de Markov: ele aprende de onde se vai para onde numa melodia (depois desta nota veio aquela três vezes, aquela outra uma vez) e improvisa sorteando a próxima nota pelo que aprendeu. Sem nada ligado, ele já toca, começando numa caminhada pentatônica.

## Como pensar nele
É um aluno que imita o estilo de quem ouve, não as notas exatas. TEMP é a ousadia: perto de 0 ele repete o caminho mais comum, perto de 2 ele inventa. MEM é quanto ele lembra. A saída SURP diz quão inesperada foi cada nota, e isso também pode mover o patch.

## Controles
- RATE: o relógio interno, quando nada está ligado em CLK.
- GATE: a duração de cada nota.
- TEMP: a ousadia do sorteio, de 0 a 2.
- MEM: quanto do que aprendeu ele guarda.
- LEARN: liga ou desliga o aprendizado. Desligado, ele congela o vocabulário.
- OCT: transpõe a saída em oitavas.
Entradas: CLK avança uma nota, LRN.P e LRN.G recebem a melodia a aprender (altura e gate), RST recomeça. Saídas: PTCH é a nota, GT o gate, SURP o grau de surpresa.

## Experimente
1. Ligue PTCH num OSC e GT num ENVELOPE que controla um VCA. Ouça a caminhada inicial.
2. Ligue a altura e o gate de um PHRASE em LRN.P e LRN.G: o MARKOV passa a responder no estilo daquela voz.
3. Depois de ensinar, desligue LEARN e suba TEMP: ele improvisa com o vocabulário que guardou.
4. Ligue SURP no MIX de um SPACE: as notas inesperadas ganham eco.
