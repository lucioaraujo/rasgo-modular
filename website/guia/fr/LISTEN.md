## O que é
LISTEN suit l'énergie d'un son dans trois régions : grave, médium et aigu. Chaque région fournit un niveau continu et une impulsion lorsqu'elle détecte une attaque. Ces signaux permettent à une autre voix de répondre à ce qui joue.

## Como pensar nele
Un coup grave peut appeler une note ; les cymbales peuvent changer la brillance d'une voix. Les régions se recouvrent : un instrument peut apparaître sur plusieurs sorties. LISTEN mesure des fréquences et des contrastes d'énergie, sans séparer les instruments ni trouver le tempo. THRU conserve le son reçu pour que vous puissiez aussi l'écouter. Les indicateurs montrent les niveaux et signalent les attaques récentes.

## Controles
- LOW : séparation grave/médium, de 60 à 800 Hz. Déplacer une séparation suspend les attaques pendant 50 ms pour éviter les déclenchements dus au réglage. Une modulation continue peut prolonger cette suspension.
- HIGH : séparation médium/aigu, de 1 à 8 kHz, limitée par la fréquence d'échantillonnage.
- RESP : lisse les niveaux. Augmentez-le pour prolonger l'énergie après chaque coup. La détection des attaques reste indépendante.
- SENS : sensibilité aux attaques. Augmentez-la pour détecter de plus petits contrastes ; réduisez-la si un coup provoque trop d'événements.
- FLOOR : seuil d'amplitude pour déclencher, de −72 à −18 dBFS. Le relever aide à écarter les bruits faibles.
- REARM : attente minimale entre deux attaques de la même région, de 20 à 250 ms. Un réglage élevé peut ignorer des coups rapides.
Entrée : IN, l'audio. Sorties : THRU, l'audio conservé sur chaque canal ; LOW, MID et HIGH, niveaux de 0 à 1 ; L-HIT, M-HIT et H-HIT, impulsions de 5 ms. Le mouvement automatique VARIA laisse les réglages de calibration inchangés.

## Experimente
1. Désactivez VARIA. Reliez CLOCK.EUC à DRUM.GATE, DRUM.OUT à LISTEN.IN et LISTEN.THRU à une entrée du MIXER. Commencez à faible volume et observez les indicateurs.
2. Reliez L-HIT à PHRASE.CLK. Pour entendre la réponse, reliez PHRASE.PITCH à QUANTIZER.CV, avec RANGE=1 ; QUANTIZER.PITCH à OSC.PITCH ; OSC.SAW à ENVELOPE.IN ; PHRASE.GATE à ENVELOPE.GATE ; ENVELOPE.OUT au MIXER.
3. Modifiez TONE et SNAP du DRUM. Réglez FLOOR et REARM jusqu'à ce que la voix réponde aux coups souhaités. Remplacez L-HIT par H-HIT et écoutez le dialogue changer.
4. Pour varier l'intensité, placez un VCA entre ENVELOPE et MIXER, avec LVL1=0 et CV1=1 ; reliez PHRASE.ACC à VCA.CV1 et LISTEN.LOW à PHRASE.STIM. Pour l'audio externe, envoyez SIGNAL-IN.OUT vers LISTEN.IN et activez vous-même ON sur SIGNAL-IN.
