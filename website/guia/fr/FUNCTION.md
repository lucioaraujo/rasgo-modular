## O que é
FUNCTION génère une rampe qui monte et descend sans arrêt. Lentement, elle sert à faire bouger d'autres réglages, ce qu'on appelle un LFO, un oscillateur basse fréquence ; vite, elle devient un son audible ; déclenchée par une impulsion, elle fonctionne comme une enveloppe simple. C'est la source de mouvement la plus utilisée du Rasgo Modular.

## Como pensar nele
Enveloppe, LFO et oscillateur sont la même chose à des vitesses différentes, et FUNCTION les couvre tous, d'un cycle toutes les cent secondes à des milliers par seconde. Quand un guide dit de brancher un LFO quelque part, il s'agit presque toujours d'un FUNCTION lent. La forme va d'une rampe descendante à une rampe montante en passant par le triangle, et les deux sorties donnent la même forme à des échelles différentes : UNI seulement au-dessus de zéro, BI au-dessus et en dessous.

## Controles
- RATE : la vitesse, de 0,01 à 12000 cycles par seconde.
- SLOPE : la forme. À un bout, elle monte d'un coup et descend lentement ; au milieu, un triangle ; à l'autre bout, elle monte lentement et retombe d'un coup.
- DRIFT : fait varier la vitesse peu à peu, pour que le mouvement ne sonne pas comme un métronome.
- SYNC : active l'entrée SYNC, pour que chaque impulsion relance la rampe.
Entrées : RATE et SLOPE, pour moduler ces réglages ; SYNC, l'impulsion qui relance. Sorties : UNI et BI.

## Experimente
1. Montez une voix : la SAW d'un OSC sur l'entrée IN d'un FILTER, LO du FILTER dans le MIXER. Baissez CUT.
2. Branchez BI de FUNCTION sur l'entrée FC du FILTER, avec RATE bas. Le timbre s'ouvre et se ferme lentement.
3. Montez RATE vers 5 Hz et branchez BI sur l'entrée FM de l'OSC, avec le FM de l'OSC très bas. La hauteur tremble, en vibrato.
4. Montez RATE jusqu'à la zone audible et branchez BI directement dans le MIXER : FUNCTION devient lui-même un son.
