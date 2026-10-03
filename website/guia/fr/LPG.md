## O que é
LPG, pour low pass gate, est une porte qui ouvre et ferme le son à chaque frappe, en baissant à la fois le volume et la brillance. Il imite un composant ancien, le vactrol, qui réagit vite mais relâche lentement, si bien que chaque note sonne frappée : un marimba, une kalimba, une goutte d'eau.

## Como pensar nele
On pourrait monter quelque chose de semblable avec FILTER, VCA et ENVELOPE, mais il manquerait la manière du vactrol : une montée instantanée et une chute qui ralentit vers la fin. LPG l'a d'origine. Il suffit d'un son continu sur IN et d'impulsions sur STRK : chaque impulsion devient une note jouée. MODE choisit s'il agit plutôt comme un filtre, plutôt comme un contrôle de volume, ou les deux.

## Controles
- MODE : à gauche, filtre seulement ; à droite, volume seulement ; au milieu, les deux ensemble, le son caractéristique du LPG.
- RESP : le temps que met la note à mourir après la frappe, d'une touche très brève à environ deux secondes et demie. La montée est toujours rapide.
- OFST : à quel point la porte reste ouverte au repos. À zéro, elle se ferme complètement entre les frappes.
- RESO : la résonance du filtre, qui donne un ton plus marqué.
- BNCE : ajoute un petit rebond juste après chaque frappe.
- DRIFT : fait varier légèrement et lentement la durée de chaque note.
Entrées : IN, le son ; STRK, l'impulsion qui frappe ; CV, pour ouvrir la porte avec un signal continu.

## Experimente
1. Branchez la sortie TRI d'un OSC sur l'entrée IN, et OUT sur une entrée du MIXER. Silence pour l'instant : la porte est fermée.
2. Branchez la sortie EUC du CLOCK sur l'entrée STRK. Chaque impulsion devient une note au timbre frappé.
3. Tournez RESP pour entendre les notes se raccourcir ou s'allonger.
4. Branchez la sortie PTCH d'une SEQUENCE, menée par le même CLOCK, sur l'entrée 1V/O de l'OSC. Vous avez une ligne de marimba.
