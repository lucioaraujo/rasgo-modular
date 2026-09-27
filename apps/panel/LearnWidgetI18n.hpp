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
