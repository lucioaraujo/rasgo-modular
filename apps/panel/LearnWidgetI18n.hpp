#pragma once

// Tradução dos verbetes de WIDGET do LEARN, por LOTES.
//
// ARQUIVO GERADO por `tools/traducao/gerar.py` a partir dos JSON em
// `tools/traducao/`. Não editar à mão: edite o JSON da família e rode o
// gerador.
//
// ---- por que por lotes -------------------------------------------------
//
// Sao 803 verbetes, 986 campos preenchidos, 73.203 caracteres — ~220.000
// nos tres idiomas. Traduzir tudo de uma vez produziria uma mudanca
// impossivel de revisar e facil de abandonar pela metade. Por FAMILIA de
// modulos, cada lote e verificavel, commitavel, e o medidor
// (`rasgo_modular_learn_coverage`) mostra exatamente o que falta.
//
// ---- regras do lote ----------------------------------------------------
//
// 1. quando um verbete e traduzido, TODOS os campos preenchidos no
//    portugues vao juntos. A queda e por VERBETE, nao por campo: traduzir
//    so o `quick` daria uma caixa com duas linguas dentro, pior que uma
//    caixa numa lingua so;
// 2. nomes de modulo, siglas, rotulos de knob e unidades ficam como estao
//    (OSC, VCA, 1 V/oct, TORQ, S&H) — sao o vocabulario do modular em
//    qualquer idioma, e traduzi-los esconde o que o painel mostra;
// 3. a mao, nao por maquina. O conteudo descreve comportamento real de
//    cada parametro naquele modulo; traducao automatica produz texto que
//    PARECE explicacao sem ser, e num instrumento didatico isso e pior
//    que nao ter texto.

#include "panel/LearnCatalog.hpp"

namespace rasgo::panel::detail {

inline void registrarLotesTraduzidos() {
    // ---- familia OUT (47 verbetes) ----
    {
        LearnTable& t = learnTableEnMutable();
        t["MIXER"]["gain1"] = LearnEntry{"Gain of channel 1, in dB.", "", ""};
        t["MIXER"]["pan1"] = LearnEntry{"Stereo position of channel 1 (constant-power law).", "", ""};
        t["MIXER"]["mute1"] = LearnEntry{"Silences channel 1 without touching GAIN.", "", ""};
        t["MIXER"]["in:ch1"] = LearnEntry{"Audio input of channel 1.", "", ""};
        t["MIXER"]["gain2"] = LearnEntry{"Gain of channel 2, in dB.", "", ""};
        t["MIXER"]["pan2"] = LearnEntry{"Stereo position of channel 2.", "", ""};
        t["MIXER"]["mute2"] = LearnEntry{"Silences channel 2.", "", ""};
        t["MIXER"]["in:ch2"] = LearnEntry{"Audio input of channel 2.", "", ""};
        t["MIXER"]["gain3"] = LearnEntry{"Gain of channel 3, in dB.", "", ""};
        t["MIXER"]["pan3"] = LearnEntry{"Stereo position of channel 3.", "", ""};
        t["MIXER"]["mute3"] = LearnEntry{"Silences channel 3.", "", ""};
        t["MIXER"]["in:ch3"] = LearnEntry{"Audio input of channel 3.", "", ""};
        t["MIXER"]["gain4"] = LearnEntry{"Gain of channel 4, in dB.", "", ""};
        t["MIXER"]["pan4"] = LearnEntry{"Stereo position of channel 4.", "", ""};
        t["MIXER"]["mute4"] = LearnEntry{"Silences channel 4.", "", ""};
        t["MIXER"]["in:ch4"] = LearnEntry{"Audio input of channel 4.", "", ""};
        t["MIXER"]["out_gain"] = LearnEntry{"Output bus gain — after the sum of the 4 channels.", "", ""};
        t["MIXER"]["out:out"] = LearnEntry{"Stereo sum of the 4 channels — normally goes to the MASTER.", "", ""};
        t["MASTER"]["gain"] = LearnEntry{"Final gain — after it, only the output protection (DC block + limiter) and the speakers.", "The VU meter shows the recent peak relative to the −1 dBFS ceiling; the square on the right lights up when the limiter really did have to hold something back.", ""};
        t["MASTER"]["width"] = LearnEntry{"Stereo width (mid/side) — 0 is mono, 1 is normal, 2 is double the side.", "It does nothing when the source is MONO, and that is not a fault: width works on the SIDE (the difference between the channels), and in a signal where L = R the side is zero. Most modules are mono — to hear WIDTH, feed the MASTER with two different sources, one per channel, or with a stereo output module such as the HALL.", ""};
        t["MASTER"]["mono"] = LearnEntry{"Forces L=R=(L+R)/2 — mono compatibility check.", "", ""};
        t["MASTER"]["dc_block"] = LearnEntry{"DC block (high-pass ~5 Hz) — removes offset that is not sound, it would only heat the limiter for nothing.", "", ""};
        t["MASTER"]["limit"] = LearnEntry{"Look-ahead limiter — it holds the peak BEFORE it arrives, without distorting the transient.", "The detector looks at the LARGER of the sample peak and an estimate of the peak BETWEEN samples — a peak that the sample alone would not see is caught too.", ""};
        t["MASTER"]["body_guard"] = LearnEntry{"BODY — body governor. It only acts on sound that is LOUD, SUSTAINED and CONCENTRATED in the 2.5 to 8 kHz band, which is where the ear tires and hurts.", "Slow attack (~250 ms) and a high threshold, on purpose: transient, rhythm and bursts of noise go through untouched — only what STAYS PUT, loud and concentrated there, trips it (a self-oscillating filter, a held square, locked folding). When it trips, it applies a few dB of high-shelf, following along. On the overwhelming majority of material it does NOT act, and that is why turning the knob usually changes nothing. The orange mark on the LEFT tip of the VU lights up when it really is acting — without it, a control that works in silence is indistinguishable from a broken one.", "put a FILTER into self-oscillation in the 3–5 kHz band and let it ring: that is where BODY shows what it is for. Compare it at 0 (exact bypass) and at 1."};
        t["MASTER"]["in:in"] = LearnEntry{"Stereo input — normally comes from the MIXER.", "", ""};
        t["MASTER"]["out:out"] = LearnEntry{"Final output, stereo.", "", ""};
        t["MASTER"]["out:level"] = LearnEntry{"Output peak with decay (~300 ms), 0..1 — CV for a module to FOLLOW the master's own volume.", "", ""};
        t["SCOPE"]["trigger"] = LearnEntry{"Trigger level of the comparator.", "", ""};
        t["SCOPE"]["edge"] = LearnEntry{"Triggers on the falling edge (on) or the rising edge (off).", "", ""};
        t["SCOPE"]["reject"] = LearnEntry{"Hysteresis zone around the trigger level — avoids multiple triggering from noise near the threshold.", "", ""};
        t["SCOPE"]["response"] = LearnEntry{"Speed of the LVL/BRT/PIT followers — attack always much faster than the release.", "", ""};
        t["SCOPE"]["hold"] = LearnEntry{"Freezes LVL/BRT/PIT at the current value.", "", ""};
        t["SCOPE"]["sens"] = LearnEntry{"Sensitivity of the ONSET (attack) detector: high = triggers on any rise in amplitude; low = only on strong attacks. Fast envelope vs slow.", "", ""};
        t["SCOPE"]["in:in"] = LearnEntry{"Signal to analyse/display.", "", ""};
        t["SCOPE"]["in:ext"] = LearnEntry{"External trigger source — with no cable, TRIG uses IN itself.", "", ""};
        t["SCOPE"]["out:thru"] = LearnEntry{"IN copied unchanged — it is what the panel Display draws.", "", ""};
        t["SCOPE"]["out:trig"] = LearnEntry{"Pulse when IN crosses TRIGGER in the direction of EDGE.", "", ""};
        t["SCOPE"]["out:level"] = LearnEntry{"Signal peak, as CV — the scope's measurement goes back into the patch.", "", ""};
        t["SCOPE"]["out:bright"] = LearnEntry{"Spectral centroid (treble↔bass), as CV — the higher it is, the brighter the signal.", "Computed by a simple differentiator (derivative energy / signal energy, Parseval) — no FFT, RT-safe.", ""};
        t["SCOPE"]["out:pitch"] = LearnEntry{"Detected pitch in 1 V/oct — autocorrelation (YIN) on a decimated signal, robust to harmonics (saw, chord). Range ~53–1000 Hz; noise → 0.", "The earlier ZCR reported the wrong octave on rich sound; YIN finds the period through the normalised cumulative difference.", "SCOPE.pitch → OSC.pitch: a second oscillator tunes to the pitch of the input."};
        t["SCOPE"]["out:onset"] = LearnEntry{"Pulse at every ATTACK/transient of the signal (fast envelope > slow × SENS). A gate of ~2 ms; it does not trigger on a steady tone.", "It is the classic transient detector (Maths ch2/3 for audio). Steady noise does not trigger it.", "SCOPE.onset ← a drum line → triggers an ENVELOPE/LPG in the metre of the audio; or re-triggers the SAMPLER."};
        t["NOTE-OUT"]["in:gate"] = LearnEntry{"Gate/trigger of the voice to capture — the falling edge closes the complete note.", "", ""};
        t["NOTE-OUT"]["in:pitch"] = LearnEntry{"1 V/oct CV of the voice — captured at the instant the gate turns on.", "", ""};
        t["NOTE-OUT"]["in:velocity"] = LearnEntry{"Velocity (0-1) — captured at the instant the gate turns on; with no cable, 1.0.", "", ""};
        t["NOTE-OUT"]["in:accent"] = LearnEntry{"Accent — captured at the instant the gate turns on.", "", ""};
        t["NOTE-OUT"]["out:gate_thru"] = LearnEntry{"Exact copy of GATE — you need to chain through here for the NOTE-OUT to sit on the patch's real path.", "The engine only processes modules that reach the active sink (orphans cost no DSP) — without that chaining, the NOTE-OUT never runs.", "Patch SOMETHING.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate: the note keeps sounding the same, and now it is recorded in the MUSICAL SCORE."};
        t["NOTE-OUT"]["out:pitch_thru"] = LearnEntry{"Exact copy of PITCH — same reason as GTHR.", "", ""};
    }
    {
        LearnTable& t = learnTableFrMutable();
        t["MIXER"]["gain1"] = LearnEntry{"Gain du canal 1, en dB.", "", ""};
        t["MIXER"]["pan1"] = LearnEntry{"Position stéréo du canal 1 (loi de puissance constante).", "", ""};
        t["MIXER"]["mute1"] = LearnEntry{"Coupe le canal 1 sans toucher au GAIN.", "", ""};
        t["MIXER"]["in:ch1"] = LearnEntry{"Entrée audio du canal 1.", "", ""};
        t["MIXER"]["gain2"] = LearnEntry{"Gain du canal 2, en dB.", "", ""};
        t["MIXER"]["pan2"] = LearnEntry{"Position stéréo du canal 2.", "", ""};
        t["MIXER"]["mute2"] = LearnEntry{"Coupe le canal 2.", "", ""};
        t["MIXER"]["in:ch2"] = LearnEntry{"Entrée audio du canal 2.", "", ""};
        t["MIXER"]["gain3"] = LearnEntry{"Gain du canal 3, en dB.", "", ""};
        t["MIXER"]["pan3"] = LearnEntry{"Position stéréo du canal 3.", "", ""};
        t["MIXER"]["mute3"] = LearnEntry{"Coupe le canal 3.", "", ""};
        t["MIXER"]["in:ch3"] = LearnEntry{"Entrée audio du canal 3.", "", ""};
        t["MIXER"]["gain4"] = LearnEntry{"Gain du canal 4, en dB.", "", ""};
        t["MIXER"]["pan4"] = LearnEntry{"Position stéréo du canal 4.", "", ""};
        t["MIXER"]["mute4"] = LearnEntry{"Coupe le canal 4.", "", ""};
        t["MIXER"]["in:ch4"] = LearnEntry{"Entrée audio du canal 4.", "", ""};
        t["MIXER"]["out_gain"] = LearnEntry{"Gain du bus de sortie — après la somme des 4 canaux.", "", ""};
        t["MIXER"]["out:out"] = LearnEntry{"Somme stéréo des 4 canaux — va normalement au MASTER.", "", ""};
        t["MASTER"]["gain"] = LearnEntry{"Gain final — après lui, seulement la protection de sortie (blocage DC + limiteur) et les enceintes.", "Le vumètre montre le pic récent par rapport au plafond de −1 dBFS ; le carré à droite s’allume quand le limiteur a vraiment dû retenir quelque chose.", ""};
        t["MASTER"]["width"] = LearnEntry{"Largeur stéréo (mid/side) — 0 c’est mono, 1 c’est normal, 2 c’est le double de côté.", "Elle ne fait rien quand la source est MONO, et ce n’est pas un défaut : la largeur agit sur le CÔTÉ (la différence entre les canaux), et dans un signal où L = R le côté vaut zéro. La plupart des modules sont mono — pour entendre le WIDTH, alimentez le MASTER avec deux sources différentes, une par canal, ou avec un module de sortie stéréo comme le HALL.", ""};
        t["MASTER"]["mono"] = LearnEntry{"Force L=R=(L+R)/2 — vérification de compatibilité mono.", "", ""};
        t["MASTER"]["dc_block"] = LearnEntry{"Blocage DC (passe-haut ~5 Hz) — retire l’offset qui n’est pas du son, il ne ferait que chauffer le limiteur pour rien.", "", ""};
        t["MASTER"]["limit"] = LearnEntry{"Limiteur à look-ahead — il retient le pic AVANT son arrivée, sans déformer le transitoire.", "Le détecteur regarde le PLUS GRAND entre le pic d’échantillon et une estimation du pic ENTRE les échantillons — un pic que l’échantillon seul ne verrait pas est attrapé aussi.", ""};
        t["MASTER"]["body_guard"] = LearnEntry{"BODY — gouverneur de corps. Il n’agit que sur un son FORT, SOUTENU et CONCENTRÉ dans la bande de 2,5 à 8 kHz, là où l’oreille se fatigue et souffre.", "Attaque lente (~250 ms) et seuil élevé, exprès : transitoire, rythme et rafale de bruit passent intacts — seul ce qui RESTE FIXE, fort et concentré là, le déclenche (filtre en auto-oscillation, carrée tenue, folding bloqué). Quand il se déclenche, il applique quelques dB de high-shelf, en accompagnant. Sur l’immense majorité du matériau il n’agit PAS, et c’est pourquoi tourner le potentiomètre ne change souvent rien. La marque orange à la pointe GAUCHE du vumètre s’allume quand il agit vraiment — sans elle, un contrôle qui travaille en silence est indiscernable d’un contrôle cassé.", "mettez un FILTER en auto-oscillation dans la bande 3–5 kHz et laissez-le sonner : c’est là que le BODY montre à quoi il sert. Comparez-le à 0 (bypass exact) et à 1."};
        t["MASTER"]["in:in"] = LearnEntry{"Entrée stéréo — vient normalement du MIXER.", "", ""};
        t["MASTER"]["out:out"] = LearnEntry{"Sortie finale, stéréo.", "", ""};
        t["MASTER"]["out:level"] = LearnEntry{"Pic de sortie avec décroissance (~300 ms), 0..1 — CV pour qu’un module SUIVE le volume du master lui-même.", "", ""};
        t["SCOPE"]["trigger"] = LearnEntry{"Niveau de déclenchement du comparateur.", "", ""};
        t["SCOPE"]["edge"] = LearnEntry{"Déclenche sur le front descendant (activé) ou montant (désactivé).", "", ""};
        t["SCOPE"]["reject"] = LearnEntry{"Zone d’hystérésis autour du niveau de déclenchement — évite les déclenchements multiples dus au bruit près du seuil.", "", ""};
        t["SCOPE"]["response"] = LearnEntry{"Vitesse des suiveurs LVL/BRT/PIT — attaque toujours bien plus rapide que le relâchement.", "", ""};
        t["SCOPE"]["hold"] = LearnEntry{"Gèle LVL/BRT/PIT à la valeur actuelle.", "", ""};
        t["SCOPE"]["sens"] = LearnEntry{"Sensibilité du détecteur d’ONSET (attaque) : élevée = déclenche à toute montée d’amplitude ; basse = seulement sur les attaques fortes. Enveloppe rapide contre lente.", "", ""};
        t["SCOPE"]["in:in"] = LearnEntry{"Signal à analyser/afficher.", "", ""};
        t["SCOPE"]["in:ext"] = LearnEntry{"Source de déclenchement externe — sans câble, TRIG utilise IN lui-même.", "", ""};
        t["SCOPE"]["out:thru"] = LearnEntry{"IN copiée sans modification — c’est ce que dessine l’afficheur du panneau.", "", ""};
        t["SCOPE"]["out:trig"] = LearnEntry{"Impulsion quand IN traverse TRIGGER dans le sens d’EDGE.", "", ""};
        t["SCOPE"]["out:level"] = LearnEntry{"Pic du signal, en CV — la mesure du scope retourne dans le patch.", "", ""};
        t["SCOPE"]["out:bright"] = LearnEntry{"Centroïde spectral (aigu↔grave), en CV — plus il est haut, plus le signal est brillant.", "Calculé par un différenciateur simple (énergie de la dérivée / énergie du signal, Parseval) — sans FFT, sûr en temps réel.", ""};
        t["SCOPE"]["out:pitch"] = LearnEntry{"Hauteur détectée en 1 V/oct — autocorrélation (YIN) sur un signal décimé, robuste aux harmoniques (scie, accord). Plage ~53–1000 Hz ; bruit → 0.", "Le ZCR précédent annonçait la mauvaise octave sur un son riche ; le YIN trouve la période par la différence cumulée normalisée.", "SCOPE.pitch → OSC.pitch : un second oscillateur s’accorde sur la hauteur de l’entrée."};
        t["SCOPE"]["out:onset"] = LearnEntry{"Impulsion à chaque ATTAQUE/transitoire du signal (enveloppe rapide > lente × SENS). Un gate d’environ 2 ms ; il ne déclenche pas sur un son stable.", "C’est le détecteur de transitoire classique (Maths ch2/3 pour l’audio). Un bruit stable ne le déclenche pas.", "SCOPE.onset ← une ligne de batterie → déclenche une ENVELOPE/LPG dans la mesure de l’audio ; ou redéclenche le SAMPLER."};
        t["NOTE-OUT"]["in:gate"] = LearnEntry{"Gate/trigger de la voix à capturer — le front descendant ferme la note complète.", "", ""};
        t["NOTE-OUT"]["in:pitch"] = LearnEntry{"CV 1 V/oct de la voix — capturée à l’instant où le gate s’ouvre.", "", ""};
        t["NOTE-OUT"]["in:velocity"] = LearnEntry{"Vélocité (0-1) — capturée à l’instant où le gate s’ouvre ; sans câble, 1,0.", "", ""};
        t["NOTE-OUT"]["in:accent"] = LearnEntry{"Accent — capturé à l’instant où le gate s’ouvre.", "", ""};
        t["NOTE-OUT"]["out:gate_thru"] = LearnEntry{"Copie exacte de GATE — il faut enchaîner ici pour que le NOTE-OUT soit sur le vrai chemin du patch.", "Le moteur ne traite que les modules qui atteignent le sink actif (les orphelins ne coûtent pas de DSP) — sans cet enchaînement, le NOTE-OUT ne tourne jamais.", "Câblez QUELQUE CHOSE.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate : la note continue de sonner pareil, et la voilà consignée dans la MUSICAL SCORE."};
        t["NOTE-OUT"]["out:pitch_thru"] = LearnEntry{"Copie exacte de PITCH — même raison que GTHR.", "", ""};
    }
    {
        LearnTable& t = learnTableEsMutable();
        t["MIXER"]["gain1"] = LearnEntry{"Ganancia del canal 1, en dB.", "", ""};
        t["MIXER"]["pan1"] = LearnEntry{"Posición estéreo del canal 1 (ley de potencia constante).", "", ""};
        t["MIXER"]["mute1"] = LearnEntry{"Silencia el canal 1 sin tocar el GAIN.", "", ""};
        t["MIXER"]["in:ch1"] = LearnEntry{"Entrada de audio del canal 1.", "", ""};
        t["MIXER"]["gain2"] = LearnEntry{"Ganancia del canal 2, en dB.", "", ""};
        t["MIXER"]["pan2"] = LearnEntry{"Posición estéreo del canal 2.", "", ""};
        t["MIXER"]["mute2"] = LearnEntry{"Silencia el canal 2.", "", ""};
        t["MIXER"]["in:ch2"] = LearnEntry{"Entrada de audio del canal 2.", "", ""};
        t["MIXER"]["gain3"] = LearnEntry{"Ganancia del canal 3, en dB.", "", ""};
        t["MIXER"]["pan3"] = LearnEntry{"Posición estéreo del canal 3.", "", ""};
        t["MIXER"]["mute3"] = LearnEntry{"Silencia el canal 3.", "", ""};
        t["MIXER"]["in:ch3"] = LearnEntry{"Entrada de audio del canal 3.", "", ""};
        t["MIXER"]["gain4"] = LearnEntry{"Ganancia del canal 4, en dB.", "", ""};
        t["MIXER"]["pan4"] = LearnEntry{"Posición estéreo del canal 4.", "", ""};
        t["MIXER"]["mute4"] = LearnEntry{"Silencia el canal 4.", "", ""};
        t["MIXER"]["in:ch4"] = LearnEntry{"Entrada de audio del canal 4.", "", ""};
        t["MIXER"]["out_gain"] = LearnEntry{"Ganancia del bus de salida — después de la suma de los 4 canales.", "", ""};
        t["MIXER"]["out:out"] = LearnEntry{"Suma estéreo de los 4 canales — normalmente va al MASTER.", "", ""};
        t["MASTER"]["gain"] = LearnEntry{"Ganancia final — después de ella sólo la protección de salida (bloqueo de DC + limitador) y los altavoces.", "El vúmetro muestra el pico reciente respecto al techo de −1 dBFS; el cuadrado de la derecha se enciende cuando el limitador realmente tuvo que retener algo.", ""};
        t["MASTER"]["width"] = LearnEntry{"Anchura estéreo (mid/side) — 0 es mono, 1 es normal, 2 es el doble de lado.", "No hace nada cuando la fuente es MONO, y eso no es un defecto: la anchura actúa sobre el LADO (la diferencia entre los canales), y en una señal donde L = R el lado es cero. La mayoría de los módulos son mono — para oír el WIDTH, alimente el MASTER con dos fuentes distintas, una por canal, o con un módulo de salida estéreo como el HALL.", ""};
        t["MASTER"]["mono"] = LearnEntry{"Fuerza L=R=(L+R)/2 — comprobación de compatibilidad mono.", "", ""};
        t["MASTER"]["dc_block"] = LearnEntry{"Bloqueo de DC (paso alto ~5 Hz) — quita el offset que no es sonido, sólo calentaría el limitador en vano.", "", ""};
        t["MASTER"]["limit"] = LearnEntry{"Limitador con look-ahead — retiene el pico ANTES de que llegue, sin distorsionar el transitorio.", "El detector mira el MAYOR entre el pico de muestra y una estimación del pico ENTRE muestras — un pico que la muestra sola no vería también se atrapa.", ""};
        t["MASTER"]["body_guard"] = LearnEntry{"BODY — gobernador de cuerpo. Sólo actúa sobre sonido FUERTE, SOSTENIDO y CONCENTRADO en la banda de 2,5 a 8 kHz, que es donde el oído se cansa y duele.", "Ataque lento (~250 ms) y umbral alto, a propósito: transitorio, ritmo y ráfaga de ruido pasan intactos — sólo lo que se QUEDA QUIETO, fuerte y concentrado ahí, lo dispara (filtro autooscilando, cuadrada sostenida, folding trabado). Cuando dispara, aplica unos pocos dB de high-shelf, acompañando. En la inmensa mayoría del material NO actúa, y por eso girar el mando suele no cambiar nada. La marca naranja en la punta IZQUIERDA del vúmetro se enciende cuando está actuando de verdad — sin ella, un control que trabaja en silencio es indistinguible de un control roto.", "ponga un FILTER en autooscilación en la banda de 3–5 kHz y déjelo sonar: ahí es donde el BODY muestra para qué vino. Compárelo en 0 (bypass exacto) y en 1."};
        t["MASTER"]["in:in"] = LearnEntry{"Entrada estéreo — normalmente viene del MIXER.", "", ""};
        t["MASTER"]["out:out"] = LearnEntry{"Salida final, estéreo.", "", ""};
        t["MASTER"]["out:level"] = LearnEntry{"Pico de salida con caída (~300 ms), 0..1 — CV para que un módulo SIGA el propio volumen del master.", "", ""};
        t["SCOPE"]["trigger"] = LearnEntry{"Nivel de disparo del comparador.", "", ""};
        t["SCOPE"]["edge"] = LearnEntry{"Dispara en el flanco de bajada (activado) o de subida (desactivado).", "", ""};
        t["SCOPE"]["reject"] = LearnEntry{"Zona de histéresis alrededor del nivel de disparo — evita disparos múltiples por ruido cerca del umbral.", "", ""};
        t["SCOPE"]["response"] = LearnEntry{"Velocidad de los seguidores LVL/BRT/PIT — ataque siempre mucho más rápido que el relajo.", "", ""};
        t["SCOPE"]["hold"] = LearnEntry{"Congela LVL/BRT/PIT en el valor actual.", "", ""};
        t["SCOPE"]["sens"] = LearnEntry{"Sensibilidad del detector de ONSET (ataque): alta = dispara en cualquier subida de amplitud; baja = sólo en los ataques fuertes. Envolvente rápida frente a lenta.", "", ""};
        t["SCOPE"]["in:in"] = LearnEntry{"Señal a analizar/mostrar.", "", ""};
        t["SCOPE"]["in:ext"] = LearnEntry{"Fuente externa de disparo — sin cable, TRIG usa el propio IN.", "", ""};
        t["SCOPE"]["out:thru"] = LearnEntry{"IN copiado sin alteración — es lo que dibuja el Display del panel.", "", ""};
        t["SCOPE"]["out:trig"] = LearnEntry{"Pulso cuando IN cruza TRIGGER en la dirección de EDGE.", "", ""};
        t["SCOPE"]["out:level"] = LearnEntry{"Pico de la señal, como CV — la medición del scope vuelve al patch.", "", ""};
        t["SCOPE"]["out:bright"] = LearnEntry{"Centroide espectral (agudo↔grave), como CV — cuanto más alto, más brillante la señal.", "Calculado por un diferenciador simple (energía de la derivada / energía de la señal, Parseval) — sin FFT, seguro en tiempo real.", ""};
        t["SCOPE"]["out:pitch"] = LearnEntry{"Altura detectada en 1 V/oct — autocorrelación (YIN) en una señal diezmada, robusta a los armónicos (sierra, acorde). Rango ~53–1000 Hz; ruido → 0.", "El ZCR anterior informaba la octava equivocada en sonido rico; el YIN halla el período por la diferencia acumulada normalizada.", "SCOPE.pitch → OSC.pitch: un segundo oscilador se afina por la altura de la entrada."};
        t["SCOPE"]["out:onset"] = LearnEntry{"Pulso en cada ATAQUE/transitorio de la señal (envolvente rápida > lenta × SENS). Un gate de ~2 ms; no dispara en un tono estable.", "Es el detector de transitorio clásico (Maths ch2/3 para audio). El ruido estable no lo dispara.", "SCOPE.onset ← una línea de batería → dispara una ENVELOPE/LPG en el compás del audio; o redispara el SAMPLER."};
        t["NOTE-OUT"]["in:gate"] = LearnEntry{"Gate/trigger de la voz a capturar — el flanco de bajada cierra la nota completa.", "", ""};
        t["NOTE-OUT"]["in:pitch"] = LearnEntry{"CV 1 V/oct de la voz — capturada en el instante en que el gate se activa.", "", ""};
        t["NOTE-OUT"]["in:velocity"] = LearnEntry{"Velocidad (0-1) — capturada en el instante en que el gate se activa; sin cable, 1,0.", "", ""};
        t["NOTE-OUT"]["in:accent"] = LearnEntry{"Acento — capturado en el instante en que el gate se activa.", "", ""};
        t["NOTE-OUT"]["out:gate_thru"] = LearnEntry{"Copia exacta de GATE — hay que encadenar por aquí para que el NOTE-OUT quede en el camino real del patch.", "El motor sólo procesa módulos que llegan al sink activo (los huérfanos no cuestan DSP) — sin ese encadenamiento, el NOTE-OUT nunca se ejecuta.", "Cablee ALGO.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate: la nota sigue sonando igual, y ahora queda registrada en la MUSICAL SCORE."};
        t["NOTE-OUT"]["out:pitch_thru"] = LearnEntry{"Copia exacta de PITCH — el mismo motivo que GTHR.", "", ""};
    }
    // ---- familia ROUTE (78 verbetes) ----
    {
        LearnTable& t = learnTableEnMutable();
        t["SWITCH"]["steps"] = LearnEntry{"How many positions the switch uses (2-4).", "", ""};
        t["SWITCH"]["mode"] = LearnEntry{"Advance order: forward / ping-pong / random (never the same twice in a row) / by ADR only (ignores CLK).", "", ""};
        t["SWITCH"]["glide"] = LearnEntry{"Crossfade at the switching point — MUX mode only.", "", ""};
        t["SWITCH"]["slew"] = LearnEntry{"Always smooths the step of the switch (~1 ms more), anti-click in any mode.", "", ""};
        t["SWITCH"]["dir"] = LearnEntry{"DEMUX on: 1 input (A) distributed to N outputs according to the step. Off (MUX): N inputs to 1 output.", "", ""};
        t["SWITCH"]["in:a"] = LearnEntry{"Input A (MUX), or the single distributed input (DEMUX).", "", ""};
        t["SWITCH"]["in:b"] = LearnEntry{"Input B (MUX only).", "", ""};
        t["SWITCH"]["in:c"] = LearnEntry{"Input C (MUX only).", "", ""};
        t["SWITCH"]["in:d"] = LearnEntry{"Input D (MUX only).", "", ""};
        t["SWITCH"]["in:clock"] = LearnEntry{"Advances one step on the rising edge (ignored if ADR is connected).", "", ""};
        t["SWITCH"]["in:reset"] = LearnEntry{"Back to step 0.", "", ""};
        t["SWITCH"]["in:addr"] = LearnEntry{"CV that picks the step directly (0..1 mapped over STEPS) — when present it rules, and CLK/MODE are ignored.", "", ""};
        t["SWITCH"]["out:out"] = LearnEntry{"MUX output (the selected A/B/C/D) — or, in DEMUX, the slice of step 0.", "", ""};
        t["SWITCH"]["out:out_b"] = LearnEntry{"In DEMUX, the slice of step 1 (otherwise always 0).", "", ""};
        t["SWITCH"]["out:out_c"] = LearnEntry{"In DEMUX, the slice of step 2.", "", ""};
        t["SWITCH"]["out:out_d"] = LearnEntry{"In DEMUX, the slice of step 3.", "", ""};
        t["SWITCH"]["out:step"] = LearnEntry{"Current position, normalised (0..1).", "", ""};
        t["MATRIX"]["g11"] = LearnEntry{"Gain of IN1 → OUT1 (the diagonal — default 1, straight through).", "Each cell is an attenuverter (−1..1); negative inverts the phase. Two non-zero cells in the same COLUMN (same destination) add — or, with RING high, multiply (four-quadrant ring-mod).", ""};
        t["MATRIX"]["g12"] = LearnEntry{"Gain of IN1 → OUT2.", "", ""};
        t["MATRIX"]["g13"] = LearnEntry{"Gain of IN1 → OUT3.", "", ""};
        t["MATRIX"]["g14"] = LearnEntry{"Gain of IN1 → OUT4.", "", ""};
        t["MATRIX"]["in:in1"] = LearnEntry{"Matrix input 1.", "", ""};
        t["MATRIX"]["g21"] = LearnEntry{"Gain of IN2 → OUT1.", "", ""};
        t["MATRIX"]["g22"] = LearnEntry{"Gain of IN2 → OUT2 (the diagonal — default 1).", "", ""};
        t["MATRIX"]["g23"] = LearnEntry{"Gain of IN2 → OUT3.", "", ""};
        t["MATRIX"]["g24"] = LearnEntry{"Gain of IN2 → OUT4.", "", ""};
        t["MATRIX"]["in:in2"] = LearnEntry{"Matrix input 2.", "", ""};
        t["MATRIX"]["g31"] = LearnEntry{"Gain of IN3 → OUT1.", "", ""};
        t["MATRIX"]["g32"] = LearnEntry{"Gain of IN3 → OUT2.", "", ""};
        t["MATRIX"]["g33"] = LearnEntry{"Gain of IN3 → OUT3 (the diagonal — default 1).", "", ""};
        t["MATRIX"]["g34"] = LearnEntry{"Gain of IN3 → OUT4.", "", ""};
        t["MATRIX"]["in:in3"] = LearnEntry{"Matrix input 3.", "", ""};
        t["MATRIX"]["g41"] = LearnEntry{"Gain of IN4 → OUT1.", "", ""};
        t["MATRIX"]["g42"] = LearnEntry{"Gain of IN4 → OUT2.", "", ""};
        t["MATRIX"]["g43"] = LearnEntry{"Gain of IN4 → OUT3.", "", ""};
        t["MATRIX"]["g44"] = LearnEntry{"Gain of IN4 → OUT4 (the diagonal — default 1).", "", ""};
        t["MATRIX"]["in:in4"] = LearnEntry{"Matrix input 4.", "", ""};
        t["MATRIX"]["out:out1"] = LearnEntry{"Weighted sum of column 1 (every input × its g_1 gains).", "", ""};
        t["MATRIX"]["out:out2"] = LearnEntry{"Weighted sum of column 2.", "", ""};
        t["MATRIX"]["out:out3"] = LearnEntry{"Weighted sum of column 3.", "", ""};
        t["MATRIX"]["out:out4"] = LearnEntry{"Weighted sum of column 4.", "", ""};
        t["MATRIX"]["level"] = LearnEntry{"Overall output gain, before SAT.", "", ""};
        t["MATRIX"]["norm"] = LearnEntry{"Per-column normalisation — keeps the output level steady even with several high gains summed into the same destination.", "", ""};
        t["MATRIX"]["ring"] = LearnEntry{"Crosses each column between linear sum (0) and four-quadrant ring-mod (1) — two non-zero inputs into the same destination start to multiply instead of adding.", "", ""};
        t["MATRIX"]["sat"] = LearnEntry{"Saturates the matrix gently — it holds even inside a feedback loop.", "", ""};
        t["MATRIX"]["drift"] = LearnEntry{"The 16 gains breathe slowly (a sum of sines, no RNG) — the matrix is never 100% static.", "", ""};
        t["MULT"]["in:in"] = LearnEntry{"Input — feeds the 4 outputs (or only out1/2 in DUAL mode).", "", ""};
        t["MULT"]["in:in2"] = LearnEntry{"2nd input — used only in DUAL mode, feeds out3/4.", "", ""};
        t["MULT"]["dual"] = LearnEntry{"1→4 mode (off) or two independent 1→2 multiples (on, A-180-2): out1/2 follow IN, out3/4 follow IN2.", "", ""};
        t["MULT"]["slew"] = LearnEntry{"Smooths the 4 outputs (shared).", "", ""};
        t["MULT"]["scale1"] = LearnEntry{"Attenuverter for output 1 (negative inverts).", "", ""};
        t["MULT"]["offset1"] = LearnEntry{"Constant added to output 1 — with no IN connected, it becomes a manual voltage source.", "", ""};
        t["MULT"]["scale2"] = LearnEntry{"Attenuverter for output 2.", "", ""};
        t["MULT"]["offset2"] = LearnEntry{"Constant added to output 2.", "", ""};
        t["MULT"]["scale3"] = LearnEntry{"Attenuverter for output 3 (in DUAL, it follows IN2).", "", ""};
        t["MULT"]["offset3"] = LearnEntry{"Constant added to output 3.", "", ""};
        t["MULT"]["scale4"] = LearnEntry{"Attenuverter for output 4 (in DUAL, it follows IN2).", "", ""};
        t["MULT"]["offset4"] = LearnEntry{"Constant added to output 4.", "", ""};
        t["MULT"]["out:out1"] = LearnEntry{"Output 1: SCALE1·input + OFFSET1.", "", ""};
        t["MULT"]["out:out2"] = LearnEntry{"Output 2: SCALE2·input + OFFSET2.", "", ""};
        t["MULT"]["out:out3"] = LearnEntry{"Output 3: SCALE3·input + OFFSET3.", "", ""};
        t["MULT"]["out:out4"] = LearnEntry{"Output 4: SCALE4·input + OFFSET4.", "", ""};
        t["PLANAR"]["x"] = LearnEntry{"Horizontal position of the point in the square (0 = the A/C side, 1 = the B/D side). Adds to the X CV.", "With a gesture playing, the knob becomes a bipolar nudge: 0.5 = no offset, below 0.5 pushes to the left.", ""};
        t["PLANAR"]["y"] = LearnEntry{"Vertical position (0 = the A/B side at the top, 1 = the C/D side at the bottom). Adds to the Y CV.", "", ""};
        t["PLANAR"]["curve"] = LearnEntry{"Linear (0) ↔ constant power (1). Linear: the weights sum to 1 — the honest morph for CV. Constant power: scales the weights by 1/√Σw² so the AUDIO output does not dip ~6 dB in the middle of the square.", "", ""};
        t["PLANAR"]["smooth"] = LearnEntry{"One-pole glide on the point — τ from ~1 ms to ~0.5 s. From immediate response to a slew that drags the trajectory.", "", ""};
        t["PLANAR"]["rate"] = LearnEntry{"Speed of the gesture loop AND of the drift. 0.5 = 1×; 2^((rate−0.5)·4), so 1/16× to 16×.", "", ""};
        t["PLANAR"]["drift"] = LearnEntry{"Autonomous 2D walk of the point when there is no gesture — a Lissajous of three slow incommensurable sines. Deterministic, no RNG. 0 = the point stands still.", "", ""};
        t["PLANAR"]["in:a"] = LearnEntry{"Source at the top-left corner.", "", ""};
        t["PLANAR"]["in:b"] = LearnEntry{"Source at the top-right corner.", "", ""};
        t["PLANAR"]["in:c"] = LearnEntry{"Source at the bottom-left corner.", "", ""};
        t["PLANAR"]["in:d"] = LearnEntry{"Source at the bottom-right corner.", "", ""};
        t["PLANAR"]["in:x"] = LearnEntry{"CV added to X (an LFO, an envelope, another x_out…).", "", ""};
        t["PLANAR"]["in:y"] = LearnEntry{"CV added to Y.", "", ""};
        t["PLANAR"]["in:gesture"] = LearnEntry{"Gate: while high, it RECORDS the trajectory of the point (decimated 32×, up to ~4 s).", "On the falling edge, if it recorded enough, the gesture starts playing in a loop. A short tap (under ~2 ms) clears it and returns to live.", ""};
        t["PLANAR"]["out:out"] = LearnEntry{"The bilinear mix of the 4 sources, with a safety softclip.", "", ""};
        t["PLANAR"]["out:x_out"] = LearnEntry{"The effective X position (already smoothed) as CV — patch it into a FILTER cutoff, a WAVETABLE pos… the gesture drives the patch.", "", ""};
        t["PLANAR"]["out:y_out"] = LearnEntry{"The effective Y position as CV.", "", ""};
    }
    {
        LearnTable& t = learnTableFrMutable();
        t["SWITCH"]["steps"] = LearnEntry{"Combien de positions le commutateur utilise (2-4).", "", ""};
        t["SWITCH"]["mode"] = LearnEntry{"Ordre d’avancement : avant / ping-pong / aléatoire (jamais deux fois de suite le même) / par ADR seulement (ignore CLK).", "", ""};
        t["SWITCH"]["glide"] = LearnEntry{"Fondu enchaîné au point de bascule — mode MUX uniquement.", "", ""};
        t["SWITCH"]["slew"] = LearnEntry{"Adoucit toujours la marche de la bascule (~1 ms de plus), anti-clic dans tous les modes.", "", ""};
        t["SWITCH"]["dir"] = LearnEntry{"DEMUX activé : 1 entrée (A) distribuée vers N sorties selon le pas. Désactivé (MUX) : N entrées vers 1 sortie.", "", ""};
        t["SWITCH"]["in:a"] = LearnEntry{"Entrée A (MUX), ou l’unique entrée distribuée (DEMUX).", "", ""};
        t["SWITCH"]["in:b"] = LearnEntry{"Entrée B (MUX uniquement).", "", ""};
        t["SWITCH"]["in:c"] = LearnEntry{"Entrée C (MUX uniquement).", "", ""};
        t["SWITCH"]["in:d"] = LearnEntry{"Entrée D (MUX uniquement).", "", ""};
        t["SWITCH"]["in:clock"] = LearnEntry{"Avance d’un pas sur le front montant (ignoré si ADR est branchée).", "", ""};
        t["SWITCH"]["in:reset"] = LearnEntry{"Retour au pas 0.", "", ""};
        t["SWITCH"]["in:addr"] = LearnEntry{"CV qui choisit le pas directement (0..1 réparti sur STEPS) — présente, elle commande et CLK/MODE sont ignorés.", "", ""};
        t["SWITCH"]["out:out"] = LearnEntry{"Sortie MUX (le A/B/C/D sélectionné) — ou, en DEMUX, la tranche du pas 0.", "", ""};
        t["SWITCH"]["out:out_b"] = LearnEntry{"En DEMUX, la tranche du pas 1 (sinon toujours 0).", "", ""};
        t["SWITCH"]["out:out_c"] = LearnEntry{"En DEMUX, la tranche du pas 2.", "", ""};
        t["SWITCH"]["out:out_d"] = LearnEntry{"En DEMUX, la tranche du pas 3.", "", ""};
        t["SWITCH"]["out:step"] = LearnEntry{"Position actuelle, normalisée (0..1).", "", ""};
        t["MATRIX"]["g11"] = LearnEntry{"Gain de IN1 → OUT1 (la diagonale — par défaut 1, passage direct).", "Chaque cellule est un atténuverseur (−1..1) ; négatif inverse la phase. Deux cellules non nulles dans la même COLONNE (même destination) s’additionnent — ou, avec RING élevé, se multiplient (ring-mod quatre quadrants).", ""};
        t["MATRIX"]["g12"] = LearnEntry{"Gain de IN1 → OUT2.", "", ""};
        t["MATRIX"]["g13"] = LearnEntry{"Gain de IN1 → OUT3.", "", ""};
        t["MATRIX"]["g14"] = LearnEntry{"Gain de IN1 → OUT4.", "", ""};
        t["MATRIX"]["in:in1"] = LearnEntry{"Entrée 1 de la matrice.", "", ""};
        t["MATRIX"]["g21"] = LearnEntry{"Gain de IN2 → OUT1.", "", ""};
        t["MATRIX"]["g22"] = LearnEntry{"Gain de IN2 → OUT2 (la diagonale — par défaut 1).", "", ""};
        t["MATRIX"]["g23"] = LearnEntry{"Gain de IN2 → OUT3.", "", ""};
        t["MATRIX"]["g24"] = LearnEntry{"Gain de IN2 → OUT4.", "", ""};
        t["MATRIX"]["in:in2"] = LearnEntry{"Entrée 2 de la matrice.", "", ""};
        t["MATRIX"]["g31"] = LearnEntry{"Gain de IN3 → OUT1.", "", ""};
        t["MATRIX"]["g32"] = LearnEntry{"Gain de IN3 → OUT2.", "", ""};
        t["MATRIX"]["g33"] = LearnEntry{"Gain de IN3 → OUT3 (la diagonale — par défaut 1).", "", ""};
        t["MATRIX"]["g34"] = LearnEntry{"Gain de IN3 → OUT4.", "", ""};
        t["MATRIX"]["in:in3"] = LearnEntry{"Entrée 3 de la matrice.", "", ""};
        t["MATRIX"]["g41"] = LearnEntry{"Gain de IN4 → OUT1.", "", ""};
        t["MATRIX"]["g42"] = LearnEntry{"Gain de IN4 → OUT2.", "", ""};
        t["MATRIX"]["g43"] = LearnEntry{"Gain de IN4 → OUT3.", "", ""};
        t["MATRIX"]["g44"] = LearnEntry{"Gain de IN4 → OUT4 (la diagonale — par défaut 1).", "", ""};
        t["MATRIX"]["in:in4"] = LearnEntry{"Entrée 4 de la matrice.", "", ""};
        t["MATRIX"]["out:out1"] = LearnEntry{"Somme pondérée de la colonne 1 (chaque entrée × ses gains g_1).", "", ""};
        t["MATRIX"]["out:out2"] = LearnEntry{"Somme pondérée de la colonne 2.", "", ""};
        t["MATRIX"]["out:out3"] = LearnEntry{"Somme pondérée de la colonne 3.", "", ""};
        t["MATRIX"]["out:out4"] = LearnEntry{"Somme pondérée de la colonne 4.", "", ""};
        t["MATRIX"]["level"] = LearnEntry{"Gain de sortie général, avant SAT.", "", ""};
        t["MATRIX"]["norm"] = LearnEntry{"Normalisation par colonne — garde le niveau de sortie stable même avec plusieurs gains élevés additionnés vers la même destination.", "", ""};
        t["MATRIX"]["ring"] = LearnEntry{"Fait passer chaque colonne de la somme linéaire (0) au ring-mod quatre quadrants (1) — deux entrées non nulles vers la même destination se multiplient au lieu de s’additionner.", "", ""};
        t["MATRIX"]["sat"] = LearnEntry{"Sature doucement la matrice — elle tient même dans une boucle de rétroaction.", "", ""};
        t["MATRIX"]["drift"] = LearnEntry{"Les 16 gains respirent lentement (une somme de sinus, sans RNG) — la matrice n’est jamais 100 % statique.", "", ""};
        t["MULT"]["in:in"] = LearnEntry{"Entrée — alimente les 4 sorties (ou seulement out1/2 en mode DUAL).", "", ""};
        t["MULT"]["in:in2"] = LearnEntry{"2ᵉ entrée — utilisée seulement en mode DUAL, alimente out3/4.", "", ""};
        t["MULT"]["dual"] = LearnEntry{"Mode 1→4 (éteint) ou deux multiples 1→2 indépendants (allumé, A-180-2) : out1/2 suivent IN, out3/4 suivent IN2.", "", ""};
        t["MULT"]["slew"] = LearnEntry{"Adoucit les 4 sorties (partagé).", "", ""};
        t["MULT"]["scale1"] = LearnEntry{"Atténuverseur de la sortie 1 (négatif inverse).", "", ""};
        t["MULT"]["offset1"] = LearnEntry{"Constante ajoutée à la sortie 1 — sans IN branchée, elle devient une source de tension manuelle.", "", ""};
        t["MULT"]["scale2"] = LearnEntry{"Atténuverseur de la sortie 2.", "", ""};
        t["MULT"]["offset2"] = LearnEntry{"Constante ajoutée à la sortie 2.", "", ""};
        t["MULT"]["scale3"] = LearnEntry{"Atténuverseur de la sortie 3 (en DUAL, elle suit IN2).", "", ""};
        t["MULT"]["offset3"] = LearnEntry{"Constante ajoutée à la sortie 3.", "", ""};
        t["MULT"]["scale4"] = LearnEntry{"Atténuverseur de la sortie 4 (en DUAL, elle suit IN2).", "", ""};
        t["MULT"]["offset4"] = LearnEntry{"Constante ajoutée à la sortie 4.", "", ""};
        t["MULT"]["out:out1"] = LearnEntry{"Sortie 1 : SCALE1·entrée + OFFSET1.", "", ""};
        t["MULT"]["out:out2"] = LearnEntry{"Sortie 2 : SCALE2·entrée + OFFSET2.", "", ""};
        t["MULT"]["out:out3"] = LearnEntry{"Sortie 3 : SCALE3·entrée + OFFSET3.", "", ""};
        t["MULT"]["out:out4"] = LearnEntry{"Sortie 4 : SCALE4·entrée + OFFSET4.", "", ""};
        t["PLANAR"]["x"] = LearnEntry{"Position horizontale du point dans le carré (0 = côté A/C, 1 = côté B/D). S’ajoute à la CV de X.", "Avec un geste en lecture, le potentiomètre devient un décalage bipolaire : 0,5 = aucun décalage, en dessous de 0,5 il pousse vers la gauche.", ""};
        t["PLANAR"]["y"] = LearnEntry{"Position verticale (0 = côté A/B en haut, 1 = côté C/D en bas). S’ajoute à la CV de Y.", "", ""};
        t["PLANAR"]["curve"] = LearnEntry{"Linéaire (0) ↔ puissance constante (1). Linéaire : les poids font 1 — le morphing honnête pour la CV. Puissance constante : met les poids à l’échelle par 1/√Σw² pour que la sortie AUDIO ne creuse pas d’environ 6 dB au centre du carré.", "", ""};
        t["PLANAR"]["smooth"] = LearnEntry{"Glissement à un pôle sur le point — τ de ~1 ms à ~0,5 s. De la réponse immédiate à un slew qui traîne la trajectoire.", "", ""};
        t["PLANAR"]["rate"] = LearnEntry{"Vitesse de la boucle du geste ET de la dérive. 0,5 = 1× ; 2^((rate−0,5)·4), donc de 1/16× à 16×.", "", ""};
        t["PLANAR"]["drift"] = LearnEntry{"Marche 2D autonome du point en l’absence de geste — une Lissajous de trois sinus lents incommensurables. Déterministe, sans RNG. 0 = le point reste immobile.", "", ""};
        t["PLANAR"]["in:a"] = LearnEntry{"Source du coin supérieur gauche.", "", ""};
        t["PLANAR"]["in:b"] = LearnEntry{"Source du coin supérieur droit.", "", ""};
        t["PLANAR"]["in:c"] = LearnEntry{"Source du coin inférieur gauche.", "", ""};
        t["PLANAR"]["in:d"] = LearnEntry{"Source du coin inférieur droit.", "", ""};
        t["PLANAR"]["in:x"] = LearnEntry{"CV ajoutée à X (un LFO, une enveloppe, un autre x_out…).", "", ""};
        t["PLANAR"]["in:y"] = LearnEntry{"CV ajoutée à Y.", "", ""};
        t["PLANAR"]["in:gesture"] = LearnEntry{"Gate : tant qu’il est haut, il ENREGISTRE la trajectoire du point (décimée 32×, jusqu’à ~4 s).", "Au front descendant, s’il a enregistré assez, le geste se met à jouer en boucle. Une brève impulsion (moins de ~2 ms) l’effface et revient au direct.", ""};
        t["PLANAR"]["out:out"] = LearnEntry{"Le mélange bilinéaire des 4 sources, avec un softclip de sécurité.", "", ""};
        t["PLANAR"]["out:x_out"] = LearnEntry{"La position X effective (déjà lissée) en CV — branchez-la sur le cutoff d’un FILTER, le pos d’un WAVETABLE… le geste conduit le patch.", "", ""};
        t["PLANAR"]["out:y_out"] = LearnEntry{"La position Y effective en CV.", "", ""};
    }
    {
        LearnTable& t = learnTableEsMutable();
        t["SWITCH"]["steps"] = LearnEntry{"Cuántas posiciones usa el conmutador (2-4).", "", ""};
        t["SWITCH"]["mode"] = LearnEntry{"Orden de avance: adelante / ping-pong / aleatorio (nunca el mismo dos veces seguidas) / sólo por ADR (ignora CLK).", "", ""};
        t["SWITCH"]["glide"] = LearnEntry{"Fundido cruzado en el punto de cambio — sólo en modo MUX.", "", ""};
        t["SWITCH"]["slew"] = LearnEntry{"Suaviza siempre el escalón del cambio (~1 ms más), anticlic en cualquier modo.", "", ""};
        t["SWITCH"]["dir"] = LearnEntry{"DEMUX activado: 1 entrada (A) distribuida a N salidas según el paso. Desactivado (MUX): N entradas a 1 salida.", "", ""};
        t["SWITCH"]["in:a"] = LearnEntry{"Entrada A (MUX), o la única entrada distribuida (DEMUX).", "", ""};
        t["SWITCH"]["in:b"] = LearnEntry{"Entrada B (sólo MUX).", "", ""};
        t["SWITCH"]["in:c"] = LearnEntry{"Entrada C (sólo MUX).", "", ""};
        t["SWITCH"]["in:d"] = LearnEntry{"Entrada D (sólo MUX).", "", ""};
        t["SWITCH"]["in:clock"] = LearnEntry{"Avanza un paso en el flanco de subida (ignorado si ADR está conectada).", "", ""};
        t["SWITCH"]["in:reset"] = LearnEntry{"Vuelve al paso 0.", "", ""};
        t["SWITCH"]["in:addr"] = LearnEntry{"CV que elige el paso directamente (0..1 repartido en STEPS) — si está presente manda, y CLK/MODE se ignoran.", "", ""};
        t["SWITCH"]["out:out"] = LearnEntry{"Salida MUX (el A/B/C/D seleccionado) — o, en DEMUX, la rodaja del paso 0.", "", ""};
        t["SWITCH"]["out:out_b"] = LearnEntry{"En DEMUX, la rodaja del paso 1 (si no, siempre 0).", "", ""};
        t["SWITCH"]["out:out_c"] = LearnEntry{"En DEMUX, la rodaja del paso 2.", "", ""};
        t["SWITCH"]["out:out_d"] = LearnEntry{"En DEMUX, la rodaja del paso 3.", "", ""};
        t["SWITCH"]["out:step"] = LearnEntry{"Posición actual, normalizada (0..1).", "", ""};
        t["MATRIX"]["g11"] = LearnEntry{"Ganancia de IN1 → OUT1 (la diagonal — por defecto 1, paso directo).", "Cada celda es un atenuversor (−1..1); negativo invierte la fase. Dos celdas no nulas en la misma COLUMNA (mismo destino) se suman — o, con RING alto, se multiplican (ring-mod de cuatro cuadrantes).", ""};
        t["MATRIX"]["g12"] = LearnEntry{"Ganancia de IN1 → OUT2.", "", ""};
        t["MATRIX"]["g13"] = LearnEntry{"Ganancia de IN1 → OUT3.", "", ""};
        t["MATRIX"]["g14"] = LearnEntry{"Ganancia de IN1 → OUT4.", "", ""};
        t["MATRIX"]["in:in1"] = LearnEntry{"Entrada 1 de la matriz.", "", ""};
        t["MATRIX"]["g21"] = LearnEntry{"Ganancia de IN2 → OUT1.", "", ""};
        t["MATRIX"]["g22"] = LearnEntry{"Ganancia de IN2 → OUT2 (la diagonal — por defecto 1).", "", ""};
        t["MATRIX"]["g23"] = LearnEntry{"Ganancia de IN2 → OUT3.", "", ""};
        t["MATRIX"]["g24"] = LearnEntry{"Ganancia de IN2 → OUT4.", "", ""};
        t["MATRIX"]["in:in2"] = LearnEntry{"Entrada 2 de la matriz.", "", ""};
        t["MATRIX"]["g31"] = LearnEntry{"Ganancia de IN3 → OUT1.", "", ""};
        t["MATRIX"]["g32"] = LearnEntry{"Ganancia de IN3 → OUT2.", "", ""};
        t["MATRIX"]["g33"] = LearnEntry{"Ganancia de IN3 → OUT3 (la diagonal — por defecto 1).", "", ""};
        t["MATRIX"]["g34"] = LearnEntry{"Ganancia de IN3 → OUT4.", "", ""};
        t["MATRIX"]["in:in3"] = LearnEntry{"Entrada 3 de la matriz.", "", ""};
        t["MATRIX"]["g41"] = LearnEntry{"Ganancia de IN4 → OUT1.", "", ""};
        t["MATRIX"]["g42"] = LearnEntry{"Ganancia de IN4 → OUT2.", "", ""};
        t["MATRIX"]["g43"] = LearnEntry{"Ganancia de IN4 → OUT3.", "", ""};
        t["MATRIX"]["g44"] = LearnEntry{"Ganancia de IN4 → OUT4 (la diagonal — por defecto 1).", "", ""};
        t["MATRIX"]["in:in4"] = LearnEntry{"Entrada 4 de la matriz.", "", ""};
        t["MATRIX"]["out:out1"] = LearnEntry{"Suma ponderada de la columna 1 (cada entrada × sus ganancias g_1).", "", ""};
        t["MATRIX"]["out:out2"] = LearnEntry{"Suma ponderada de la columna 2.", "", ""};
        t["MATRIX"]["out:out3"] = LearnEntry{"Suma ponderada de la columna 3.", "", ""};
        t["MATRIX"]["out:out4"] = LearnEntry{"Suma ponderada de la columna 4.", "", ""};
        t["MATRIX"]["level"] = LearnEntry{"Ganancia general de salida, antes de SAT.", "", ""};
        t["MATRIX"]["norm"] = LearnEntry{"Normalización por columna — mantiene el nivel de salida estable incluso con varias ganancias altas sumadas al mismo destino.", "", ""};
        t["MATRIX"]["ring"] = LearnEntry{"Cruza cada columna entre suma lineal (0) y ring-mod de cuatro cuadrantes (1) — dos entradas no nulas al mismo destino pasan a multiplicarse en vez de sumarse.", "", ""};
        t["MATRIX"]["sat"] = LearnEntry{"Satura suavemente la matriz — se sostiene incluso dentro de un lazo de realimentación.", "", ""};
        t["MATRIX"]["drift"] = LearnEntry{"Las 16 ganancias respiran despacio (una suma de senos, sin RNG) — la matriz nunca queda 100% estática.", "", ""};
        t["MULT"]["in:in"] = LearnEntry{"Entrada — alimenta las 4 salidas (o sólo out1/2 en modo DUAL).", "", ""};
        t["MULT"]["in:in2"] = LearnEntry{"2.ª entrada — usada sólo en modo DUAL, alimenta out3/4.", "", ""};
        t["MULT"]["dual"] = LearnEntry{"Modo 1→4 (apagado) o dos múltiples 1→2 independientes (encendido, A-180-2): out1/2 siguen IN, out3/4 siguen IN2.", "", ""};
        t["MULT"]["slew"] = LearnEntry{"Suaviza las 4 salidas (compartido).", "", ""};
        t["MULT"]["scale1"] = LearnEntry{"Atenuversor de la salida 1 (negativo invierte).", "", ""};
        t["MULT"]["offset1"] = LearnEntry{"Constante sumada a la salida 1 — sin IN conectada, se vuelve una fuente de tensión manual.", "", ""};
        t["MULT"]["scale2"] = LearnEntry{"Atenuversor de la salida 2.", "", ""};
        t["MULT"]["offset2"] = LearnEntry{"Constante sumada a la salida 2.", "", ""};
        t["MULT"]["scale3"] = LearnEntry{"Atenuversor de la salida 3 (en DUAL, sigue IN2).", "", ""};
        t["MULT"]["offset3"] = LearnEntry{"Constante sumada a la salida 3.", "", ""};
        t["MULT"]["scale4"] = LearnEntry{"Atenuversor de la salida 4 (en DUAL, sigue IN2).", "", ""};
        t["MULT"]["offset4"] = LearnEntry{"Constante sumada a la salida 4.", "", ""};
        t["MULT"]["out:out1"] = LearnEntry{"Salida 1: SCALE1·entrada + OFFSET1.", "", ""};
        t["MULT"]["out:out2"] = LearnEntry{"Salida 2: SCALE2·entrada + OFFSET2.", "", ""};
        t["MULT"]["out:out3"] = LearnEntry{"Salida 3: SCALE3·entrada + OFFSET3.", "", ""};
        t["MULT"]["out:out4"] = LearnEntry{"Salida 4: SCALE4·entrada + OFFSET4.", "", ""};
        t["PLANAR"]["x"] = LearnEntry{"Posición horizontal del punto en el cuadrado (0 = lado A/C, 1 = lado B/D). Se suma a la CV de X.", "Con un gesto en reproducción, el mando se vuelve un empuje bipolar: 0,5 = sin desvío, por debajo de 0,5 empuja a la izquierda.", ""};
        t["PLANAR"]["y"] = LearnEntry{"Posición vertical (0 = lado A/B arriba, 1 = lado C/D abajo). Se suma a la CV de Y.", "", ""};
        t["PLANAR"]["curve"] = LearnEntry{"Lineal (0) ↔ potencia constante (1). Lineal: los pesos suman 1 — el morfismo honesto para CV. Potencia constante: escala los pesos por 1/√Σw² para que la salida de AUDIO no caiga unos 6 dB en el centro del cuadrado.", "", ""};
        t["PLANAR"]["smooth"] = LearnEntry{"Deslizamiento de un polo en el punto — τ de ~1 ms a ~0,5 s. De la respuesta inmediata a un slew que arrastra la trayectoria.", "", ""};
        t["PLANAR"]["rate"] = LearnEntry{"Velocidad del bucle del gesto Y de la deriva. 0,5 = 1×; 2^((rate−0,5)·4), o sea de 1/16× a 16×.", "", ""};
        t["PLANAR"]["drift"] = LearnEntry{"Paseo 2D autónomo del punto cuando no hay gesto — una Lissajous de tres senos lentos inconmensurables. Determinista, sin RNG. 0 = el punto se queda quieto.", "", ""};
        t["PLANAR"]["in:a"] = LearnEntry{"Fuente de la esquina superior izquierda.", "", ""};
        t["PLANAR"]["in:b"] = LearnEntry{"Fuente de la esquina superior derecha.", "", ""};
        t["PLANAR"]["in:c"] = LearnEntry{"Fuente de la esquina inferior izquierda.", "", ""};
        t["PLANAR"]["in:d"] = LearnEntry{"Fuente de la esquina inferior derecha.", "", ""};
        t["PLANAR"]["in:x"] = LearnEntry{"CV sumada a X (un LFO, una envolvente, otro x_out…).", "", ""};
        t["PLANAR"]["in:y"] = LearnEntry{"CV sumada a Y.", "", ""};
        t["PLANAR"]["in:gesture"] = LearnEntry{"Gate: mientras está alto, GRABA la trayectoria del punto (diezmada 32×, hasta ~4 s).", "En el flanco de bajada, si grabó lo suficiente, el gesto empieza a tocar en bucle. Un toque corto (menos de ~2 ms) lo borra y vuelve al vivo.", ""};
        t["PLANAR"]["out:out"] = LearnEntry{"La mezcla bilineal de las 4 fuentes, con un softclip de seguridad.", "", ""};
        t["PLANAR"]["out:x_out"] = LearnEntry{"La posición X efectiva (ya suavizada) como CV — conéctela al cutoff de un FILTER, al pos de un WAVETABLE… el gesto dirige el patch.", "", ""};
        t["PLANAR"]["out:y_out"] = LearnEntry{"La posición Y efectiva como CV.", "", ""};
    }
}

}  // namespace rasgo::panel::detail
