## O que é
O CELL é um autômato celular: uma linha de casas acesas ou apagadas que se reescreve a cada pulso. Cada casa olha para si e para as duas vizinhas, e uma regra (um número de 0 a 255) decide se ela fica acesa. O módulo lê esse desenho com uma cabeça: a casa sob ela dá o gate, e as três seguintes escolhem a nota.

## Como pensar nele
É uma partitura que se escreve sozinha por uma lei muito simples. Algumas regras dão desenhos que nunca se repetem (30, 110), outras dão triângulos (90) ou coisas que andam (184). Você não compõe as notas, compõe a lei. MUT acrescenta um pequeno acaso, para a linha não morrer nem travar.

## Controles
- RATE: o relógio interno, quando nada está ligado em CLK.
- RULE: a regra, de 0 a 255. Comece por 30, 90, 110 e 184.
- CELLS: quantas casas tem a linha, de 8 a 24.
- HEAD: onde fica a cabeça de leitura.
- RANGE: o alcance das notas, até 2 oitavas.
- MUT: a chance de uma casa trocar ao acaso.
- GATE: a duração de cada nota.
Entradas: CLK avança uma geração, RST recomeça a linha, RULE muda a regra por CV. Saídas: PTCH é a nota, GT o gate, DENS a proporção de casas acesas, EOS um pulso quando a linha renasce.

## Experimente
1. Ligue GT num DRUM e o DRUM no MIXER. Com RULE em 30, ouça um ritmo que não se repete igual.
2. Ligue PTCH num OSC e GT num ENVELOPE que controla um VCA depois do OSC. Gire RULE devagar.
3. Ligue DENS no CUT de um FILTER: quanto mais vida na linha, mais brilho.
4. Ligue EOS no ADV do HARMONY: o acorde muda quando a linha renasce.
