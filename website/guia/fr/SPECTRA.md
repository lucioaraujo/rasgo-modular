## O que é
SPECTRA écoute un son, trouve ses fréquences les plus fortes et les rejoue avec un ensemble d'oscillateurs sinusoïdaux. Le résultat est une sorte d'ombre du son original : reconnaissable, mais faite uniquement de sons purs, que vous pouvez transposer, déformer ou figer.

## Como pensar nele
Il est rangé parmi les sources parce que c'est sa sortie qu'on entend, mais il a presque toujours besoin de quelque chose sur l'entrée IN : une voix, un accord, un tambour, le son extérieur via SIGNAL-IN. Avec peu de voix, il fait une caricature du son ; avec beaucoup, une copie fidèle. Son geste le plus fort est FRZ : il fige le dernier spectre et le laisse sonner indéfiniment, comme une nappe tirée de n'importe quel instant.

## Controles
- VOICE : combien de sinus le module utilise, de 2 à 24. Peu simplifient le son ; beaucoup le reproduisent fidèlement.
- BLUR : la rapidité avec laquelle chaque sinus suit le son entrant. À zéro, il le suit de près ; au maximum, il traîne et brouille, et le son semble fondre.
- SHIFT : transpose la reconstruction jusqu'à deux octaves vers le haut ou le bas, sans toucher à l'écoute.
- STRCH : écarte ou rapproche les fréquences entre elles, emmenant le son vers le métal ou la cloche sans changer la hauteur perçue.
- TONE : assombrit ou fait ressortir les aigus de la reconstruction. Au milieu, elle reste fidèle à ce qui a été entendu.
- JITR : fait légèrement osciller chaque sinus, pour que la reconstruction ne sonne jamais figée.
- FRZ : fige l'écoute. Les sinus continuent de jouer le dernier spectre.
- MIX : le mélange entre le son original et la reconstruction. Au maximum, vous n'entendez que la reconstruction.
Entrées : IN pour le son à analyser, PIT pour transposer et FRZ pour figer avec un gate. Sorties L et R.

## Experimente
1. Branchez la sortie OUT du CHORD sur l'entrée IN de SPECTRA, et la sortie L sur une entrée du MIXER. Vous entendez l'accord refait en sons purs.
2. Descendez VOICE à 3 ou 4. L'accord devient une esquisse de lui-même.
3. Poussez STRCH d'un côté et écoutez l'accord devenir métallique.
4. Branchez la sortie EUC du CLOCK sur l'entrée FRZ. Le son se fige et se libère au rythme de l'horloge.
