#pragma once

#include <string>
#include <unordered_map>
#include "panel/UiLanguage.hpp"   // Lang, para o LEARN de módulo traduzido

// ============================================================================
// LearnCatalog — hover-learn (protótipo)
// ============================================================================
//
// Ver o estudo: `RASGO_MODULAR/dossies/ESTUDO_seed_composicao_generativa.md`
// §6. Precedente citado pelo autor: Antitotem e Navalha 2 (hover sobre
// controles/jacks → explicação). Silencioso por padrão — só com o modo
// Learn ligado (`[l]`). O texto vai numa CAIXA FIXA estilo terminal no
// rodapé da coluna esquerda do painel (não flutua sobre os módulos —
// modelo do `learnEditor` do ANTITOTEM), com os 3 níveis empilhados.
//
// 3 níveis por widget (mapeando short/basic/listening/experiment da
// conversa de origem):
//   `quick`      — nome + função imediata (1 linha)
//   `understand` — como funciona por baixo (1-2 linhas)
//   `explore`    — um experimento concreto pra fazer/ouvir
//
// Conteúdo é PRECISO POR MÓDULO — a mesma justificativa do
// `MODULE_DEVELOPMENT_STANDARD`: cada texto descreve o comportamento
// REAL daquele parâmetro/porta naquele módulo, não uma explicação
// genérica de "o que é um filtro" copiada de lugar nenhum.
//
// **Catálogo completo (2026-09-05)**: os 37 tipos de módulo do
// `moduleCatalog()` (`ModuleCatalog.hpp`) têm entrada — todo knob e todo
// jack de todo módulo instanciável no painel. `quick` está preenchido em
// tudo; `understand`/`explore` são mais densos nos mecanismos que valem
// explicar (ex.: por que `CLOCK.feel` precisa de RNG isolado, o alcance
// de captura do `PLL`, a assimetria de vactrol do `LPG`) e ficam vazios
// em parâmetros autoexplicativos (ex.: `MIXER.gain2`) — o padrão do
// `MODULE_DEVELOPMENT_STANDARD` de não escrever prosa por escrever.
// `rasgo_modular_core` continua sem saber o que é "aprendizado" — isto
// vive em `apps/panel/`.
//
// ---- IDIOMA: este catálogo é MONOLÍNGUE (português) --------------------
//
// Achado da revisão de idiomas em 26 set. 2026. O app tem interface,
// tutorial e cartões em QUATRO idiomas (en/pt/fr/es), mas o LEARN não tem
// suporte a idioma nenhum: quem roda em inglês recebe interface inglesa,
// tutorial inglês e explicações em PORTUGUÊS.
//
// Tamanho medido: 803 verbetes de widget + 58 de módulo, 86.328
// caracteres. Traduzir para os outros três é escrever ~259.000
// caracteres de prosa técnica.
//
// Não é trabalho de tradução automática. O comentário abaixo é explícito
// sobre o que este conteúdo é — "descreve o comportamento REAL daquele
// parâmetro naquele módulo", com coisas como a assimetria de vactrol do
// LPG e o alcance de captura do PLL. Passar isso por máquina produziria
// texto que parece explicação e não é.
//
// Decisão registrada em `PUBLICACAO.md` como limitação declarada. Se for
// traduzido por etapas, a ordem que rende mais por caractere é: os 58
// verbetes de MÓDULO primeiro (13.125 car., ~39.000 nos três idiomas) —
// eles respondem "para que serve este módulo", que é a pergunta de quem
// abre o rack pela primeira vez.
//
// A relação `Cable` (RingMod/Fold/Difference) TEM verbete desde 25 set.
// 2026 (`learnCable()`), depois que a sessão de escuta mostrou que a ideia
// central do instrumento estava escondida. O texto abaixo diz que ela
// "fica de fora" e ficou desatualizado nessa parte:
//
// A relação `Cable` (RingMod/Fold/Difference, `PESQUISA_MODULOS.md`
// item 3 da tabela de módulos) fica de fora — é propriedade do CABO, não
// um tipo do catálogo com knobs/jacks próprios; não há widget de painel
// pra ela hospedar hover-learn.

namespace rasgo::panel {

struct LearnEntry {
    std::string quick;
    std::string understand;
    std::string explore;
};

// O CABO, e não um módulo.
//
// Existe por um achado da sessão de escuta de 23 set. 2026: pedido a
// experimentar RING, FOLD, DIFF, AMT e COND, o autor respondeu "não achei
// esses módulos". E não achou porque não SÃO módulos — são propriedades
// do cabo, no inspector que abre ao clicar sobre ele. Nada no instrumento
// dizia isso: nem o tutorial, nem o LEARN, nem as instruções que eu mesmo
// escrevi no protocolo de validação.
//
// É o defeito mais caro da lista, porque o que estava escondido não era
// um recurso lateral: é a ideia que separa este instrumento de um modular
// comum — o cabo como objeto com estado, e não um fio que só liga.
//
// 2 out. 2026: o verbete existia só em PORTUGUÊS — aparecia assim mesmo com
// a interface em inglês, francês ou espanhol, e escapou da tradução do
// LEARN inteiro em 27 set. E dizia que a RELAÇÃO combina o cabo "com o
// sinal que já chegava no destino", com "SOMA" como normal: não é o que o
// motor faz. A relação combina o que atravessa com um SEGUNDO sinal, o
// companion (`Relation` em `SignalGraph.hpp`), e o normal é NONE. Corrigido
// junto com a caixa nova do cabo (GAIN, COND sempre, DESPLUGAR).
inline const LearnEntry& learnCable(const Lang lang = Lang::pt) {
    static const LearnEntry en{
        "a cable is an OBJECT, not a wire. Click it to open its box (bottom "
        "right) and change what it does to the signal passing through.",

        "every cable has GAIN (0 to 2, neutral in the middle) and "
        "conductance, COND — the chance of letting through at each moment, "
        "where the living intermittence comes from. Its RELATION combines "
        "what crosses it with a second signal, the companion: NONE just "
        "lets it through, RING multiplies, FOLD folds the signal driven by "
        "the companion, DIFF subtracts it; AMT says how much. A cable can "
        "also be BROKEN without being removed — the sound decays into a "
        "scar instead of cutting dead, and [space] breaks and restores them "
        "all at once.",

        "set a cable to RING with an oscillator as companion: the same "
        "wiring, another instrument. Then drop COND to half and hear the "
        "connection flicker."};
    static const LearnEntry pt{
        "o cabo é um OBJETO, não um fio. Clique sobre ele para abrir a "
        "caixa dele (no canto inferior direito) e mexer no que ele faz com "
        "o sinal que passa.",

        "todo cabo tem GANHO (0 a 2, neutro no meio) e condutância, COND — "
        "a chance de deixar passar em cada instante, de onde vem a "
        "intermitência viva. A RELAÇÃO combina o que atravessa o cabo com "
        "um segundo sinal, o companion: NONE só deixa passar; RING "
        "multiplica; FOLD dobra o sinal comandado pelo companion; DIFF o "
        "subtrai; AMT diz quanto. Um cabo também pode ser ROMPIDO sem ser "
        "removido — o som decai numa cicatriz em vez de cortar seco, e "
        "[espaço] rompe e reata todos de uma vez.",

        "ponha um cabo em RING com um oscilador como companion: a mesma "
        "fiação, outro instrumento. Depois baixe COND pela metade e ouça a "
        "ligação piscar."};
    static const LearnEntry fr{
        "un câble est un OBJET, pas un fil. Cliquez dessus pour ouvrir sa "
        "fenêtre (en bas à droite) et changer ce qu’il fait au signal qui "
        "le traverse.",

        "tout câble a un GAIN (0 à 2, neutre au milieu) et une conductance, "
        "COND — la chance de laisser passer à chaque instant, d’où vient "
        "l’intermittence vivante. Sa RELATION combine ce qui le traverse "
        "avec un second signal, le compagnon : NONE laisse simplement "
        "passer, RING multiplie, FOLD replie le signal piloté par le "
        "compagnon, DIFF le soustrait ; AMT dit combien. Un câble peut aussi "
        "être ROMPU sans être retiré — le son décroît en cicatrice au lieu "
        "de couper net, et [espace] les rompt et les rétablit tous d’un "
        "coup.",

        "mettez un câble en RING avec un oscillateur comme compagnon : le "
        "même câblage, un autre instrument. Puis baissez COND de moitié et "
        "écoutez la liaison clignoter."};
    static const LearnEntry es{
        "un cable es un OBJETO, no un hilo. Haga clic sobre él para abrir "
        "su caja (abajo a la derecha) y cambiar lo que hace con la señal "
        "que lo atraviesa.",

        "todo cable tiene GANANCIA (0 a 2, neutra en el centro) y "
        "conductancia, COND — la probabilidad de dejar pasar en cada "
        "instante, de donde viene la intermitencia viva. Su RELACIÓN "
        "combina lo que lo atraviesa con una segunda señal, el compañero: "
        "NONE solo deja pasar, RING multiplica, FOLD pliega la señal guiada "
        "por el compañero, DIFF lo resta; AMT dice cuánto. Un cable también "
        "puede ser ROTO sin ser retirado — el sonido decae en una cicatriz "
        "en vez de cortarse seco, y [espacio] los rompe y restablece todos "
        "a la vez.",

        "ponga un cable en RING con un oscilador como compañero: el mismo "
        "cableado, otro instrumento. Luego baje COND a la mitad y escuche la "
        "conexión parpadear."};
    switch (lang) {
    case Lang::en: return en;
    case Lang::fr: return fr;
    case Lang::es: return es;
    default:       return pt;
    }
}

namespace detail {
using LearnTable =
    std::unordered_map<std::string, std::unordered_map<std::string, LearnEntry>>;

inline const LearnTable& learnTable() {
    static const LearnTable table = {
        {"RESONATOR", {
            {"freq", {
                "Fundamental do banco de modos (20–5000 Hz, +CV 1 V/oct). "
                "Os parciais são múltiplos disso (esticados por STRC).",
                "", ""}},
            {"structure", {
                "As razões dos parciais: 0 harmônico exato (1, 2, 3…), "
                "até 1 esticado/inarmônico (k·√(1+B·k²) — rigidez de "
                "barra/sino/corda grossa).",
                "Mesma fórmula de esticamento do ADDITIVE.stretch.", ""}},
            {"partials", {"Quantos modos no banco (1–24). 1 = quase um "
                         "FILTER BP muito ressonante.", "", ""}},
            {"decay", {
                "Tempo de anel (Q dos ressoadores): curto = pluck, longo "
                "= arco/drone. Nunca auto-oscila (pólos < 1).", "", ""}},
            {"damp", {
                "Amortecimento dos AGUDOS no anel — os parciais altos "
                "decaem antes dos graves (corda de verdade: ataque "
                "brilhante, cauda escura).", "", ""}},
            {"tilt", {
                "Inclinação espectral das amplitudes: <0 grave forte (a "
                "saída LOW domina), >0 agudo forte (HIGH domina), 0 "
                "plano. **Varrer TILT cruza LOW↔HIGH** — a relação entre "
                "as saídas é o processo (Three Sisters).", "",
                "RESONATOR.low → MIXER e .high → SWIRL; varra TILT e ouça "
                "a energia migrar entre os destinos."}},
            {"position", {
                "Onde a excitação 'bate' — um pente sobre a entrada que "
                "zera alguns parciais (o ponto de pluck de uma corda; "
                "0,5 = sem harmônicos pares).", "", ""}},
            {"mix", {"Seco (a excitação crua) ↔ ressoado. 0 = bypass "
                    "exato.", "", ""}},
            {"in:in", {
                "A EXCITAÇÃO — bata com o que quiser (DRUM, NOISE, uma "
                "voz). Livre → um ruído interno de −36 dB arqueia o banco "
                "(modo autônomo — canto de taça).", "", ""}},
            {"in:strike", {
                "Exciter embutido: cada borda de subida injeta uma "
                "rajada de ruído de ~3 ms (um maço). TRIGSEQ → STRK = o "
                "banco toca um ritmo.", "", ""}},
            {"in:freq_mod", {"CV 1 V/oct somada a FREQ.", "", ""}},
            {"out:low", {"Soma do 1/3 grave dos parciais (com MIX).", "",
                        ""}},
            {"out:mid", {"O 1/3 do meio.", "", ""}},
            {"out:high", {"O 1/3 agudo — cruza com LOW quando você varre "
                         "TILT.", "", ""}},
        }},
        {"FORMANT", {
            {"vowel", {
                "Posição na sequência de vogais A → E → I → O → U. "
                "Cada vogal é um conjunto de 5 formantes (picos "
                "espectrais); o knob interpola entre as vizinhas.",
                "Teoria fonte-filtro (Fant): a voz é uma fonte × o filtro "
                "do trato vocal; a vogal é onde os 5 formantes estão. "
                "Dados fonéticos de voz de baixo.",
                "Cabeie SEQUENCE → VOW: uma melodia de vogais sobre um "
                "drone."}},
            {"shift", {
                "Escala TODAS as frequências de formante (2^(shift·1,5), "
                "≈ 0,35× a 2,8×) — o comprimento do trato vocal: pra baixo "
                "= voz grande/grave, pra cima = voz pequena/aguda.",
                "É o formant shift — muda o timbre da voz sem mudar a "
                "altura da fonte.", ""}},
            {"res", {
                "Estreita as 5 bandas (largura ÷ (1 + res·8)). 0 = "
                "coloração sutil; 1 = bandas que cantam/apitam.",
                "Q = frequência ÷ largura de banda. Com res alto e ruído "
                "na entrada, cada vogal vira 5 tons senoidais.",
                "ENVELOPE → RES: a voz 'aperta' no ataque e relaxa."}},
            {"vocoder", {
                "Mistura para o modo VOCODER: os GANHOS das 5 bandas "
                "passam a seguir a energia do MODULADOR (jack MOD) em cada "
                "frequência de formante, em vez da tabela de vogal. 0 = "
                "FORMANT clássico; 1 = vocoder de 5 bandas.",
                "Vocoder de Homer Dudley (1938): (modulador → banco de "
                "seguidores de envelope por banda) × (portadora → banco "
                "de filtros). Aqui as 5 bandas de formante são as bandas "
                "do vocoder — grosso mas vocálico. Sem MOD cabeado, não "
                "faz efeito.",
                "Fala no MOD, um OSC/PAD rico no IN, VOCODER=1: o pad "
                "'fala'. VOWEL escolhe QUAIS 5 frequências vocodar."}},
            {"mix", {"Seco ↔ ressoado. 0 = passa-direto.", "", ""}},
            {"drift", {
                "Cada formante ganha um wobble lento e independente "
                "(±drift·3 %) — a voz respira. Determinístico (senos, "
                "sem RNG). 0 = estático.", "", ""}},
            {"in:in", {"A PORTADORA — o que vai ser moldado (um OSC "
                      "serra, um pad, NOISE). No modo vocoder é a voz "
                      "'sintética' que fala.", "", ""}},
            {"in:vowel", {"CV somada em VOWEL (LFO, SEQUENCE, envelope…).",
                         "", ""}},
            {"in:shift", {"CV somada em SHIFT.", "", ""}},
            {"in:mod", {"O MODULADOR do vocoder — o som cuja envoltória "
                       "espectral vai controlar as 5 bandas (uma voz, "
                       "um DRUM, uma gravação). Só faz efeito com VOCODER "
                       "> 0.", "", ""}},
            {"out:out", {
                "Seco + as 5 bandas, misturados por MIX, com limitador "
                "suave.", "", ""}},
        }},
        {"VOCODER", {
            {"bands", {
                "Quantas bandas de frequência (4–20). Poucas = 'robô' "
                "grosso; muitas (16–20) = fala inteligível.",
                "Vocoder de Homer Dudley (1938): a energia do MODULADOR "
                "por banda controla o ganho da mesma banda na PORTADORA. "
                "O `FORMANT` faz o mesmo com 5 bandas de vogal; este é o "
                "vocoder dedicado, banda larga.",
                "Voz no MOD, um pad/serra no CAR, BANDS=16: o pad fala. "
                "BANDS=6 = o vocoder dos anos 70."}},
            {"shift", {
                "Desloca as frequências da SÍNTESE em relação à análise "
                "(2^(shift·1,2)) — formant shift: pra cima = voz pequena, "
                "pra baixo = voz grande, sem mudar a fala.", "",
                "LFO → nada (não é entrada); mexa à mão pra uma voz que "
                "'cresce' na frase."}},
            {"attack", {
                "Ataque dos seguidores de envelope (1–60 ms). Curto = as "
                "consoantes cortam secas; longo = tudo amolece.", "", ""}},
            {"release", {
                "Release (20–600 ms). Curto = staccato/inteligível; longo "
                "= as sílabas borram uma na outra (pad falado).", "", ""}},
            {"sibilance", {
                "Quanto do AGUDO do modulador (> ~3,5 kHz) passa DIRETO — "
                "as fricativas (s, f, ch, x) que a análise de banda não "
                "pega. 0 = ceceio; alto = fala nítida.", "", ""}},
            {"freeze", {
                "Congela as envoltórias das bandas — a portadora fica "
                "'falando a última sílaba' para sempre. Tira o MOD depois "
                "de congelar e o pad segura.", "", ""}},
            {"mix", {"Portadora seca ↔ vocodada. 0 = bypass.", "", ""}},
            {"in:carrier", {
                "A PORTADORA — o que vai 'falar' (serra rica, pad, "
                "acorde, ruído). Livre → uma serra interna afinável por "
                "PIT.", "", ""}},
            {"in:mod", {
                "O MODULADOR — a voz/fala (ou DRUM, ou qualquer som "
                "rítmico) cuja envoltória espectral molda a portadora.",
                "", ""}},
            {"in:pitch", {"CV 1 V/oct pra a serra interna (só quando CAR "
                         "está livre).", "", ""}},
            {"out:out", {"A portadora moldada pelas bandas do modulador + "
                        "sibilância, com softclip.", "", ""}},
        }},
        {"FILTER", {
            {"cutoff", {
                "Frequência de corte — acima dela, o som é atenuado.",
                "Um filtro subtrativo tira energia acima (ou perto) do "
                "corte; abaixo dele o sinal passa quase sem mudança.",
                "Gire devagar e ouça o timbre escurecer. Depois conecte "
                "ENVELOPE.env em FC — o corte abre sozinho a cada nota."}},
            {"resonance", {
                "Realça as frequências perto do corte.",
                "Ressonância alta empurra o filtro pra perto da "
                "auto-oscilação — ele passa a soar quase como um oscilador.",
                "Suba RESO perto do máximo com CUT numa nota grave: o "
                "filtro começa a cantar sozinho, mesmo sem entrada."}},
            {"spread", {
                "Espalha as 3 saídas (low/center/high) em frequências "
                "diferentes.",
                "São 3 filtros \"irmãos\" na mesma base, afastados por "
                "SPRD — dá corpo de multimodo sem precisar de 3 módulos.",
                "Compare a saída ALL com SPRD=0 (um filtro só) e SPRD=1 "
                "(três bem separados)."}},
            {"drive", {
                "Ganho de entrada — empurra o sinal pra saturação suave.",
                "Satura ANTES do filtro: o grave que entra saturado muda "
                "o que sobra pra filtrar.",
                "Suba DRIVE com CUT baixo: o grave engorda antes de ser "
                "cortado."}},
            {"in:in", {"Entrada de áudio a filtrar.", "", ""}},
            {"in:cutoff_mod", {
                "CV (1 V/oct) que SOMA ao knob CUT.",
                "O cabo modula o corte por cima do valor do knob — girar "
                "CUT continua funcionando com o cabo plugado (motor "
                "aditivo).",
                ""}},
            {"in:res_mod", {"CV que soma a RESO.", "", ""}},
            {"in:spread_mod", {"CV que soma a SPRD.", "", ""}},
            {"out:low", {"Saída grave isolada (passa-baixa).", "", ""}},
            {"out:center", {"Saída ressonante central (passa-faixa).", "", ""}},
            {"out:high", {"Saída aguda isolada (passa-alta).", "", ""}},
            {"out:all", {
                "A soma das 3 saídas — a mais usada pra sair do filtro.",
                "", ""}},
        }},
        {"ENVELOPE", {
            {"attack", {
                "Tempo de subida — quanto leva pra chegar ao pico depois "
                "do gate.",
                "", ""}},
            {"decay", {
                "Tempo de queda depois do pico (ou até o SUS, se houver).",
                "", ""}},
            {"sustain", {
                "Nível em que o envelope segura enquanto o gate fica ligado.",
                "SUS=0 tira o platô — vira um AD (envelope de pluck, sobe "
                "e cai sozinho).",
                ""}},
            {"release", {
                "Tempo de queda depois que o gate desliga.", "", ""}},
            {"curve", {
                "Forma da curva — de côncava a convexa.",
                "Curvas convexas soam mais \"batidas\" (ataque percebido "
                "rápido); côncavas, mais suaves.",
                ""}},
            {"mode", {
                "TRIG: alterna ADSR ↔ AD (trigger).",
                "ADSR segura enquanto o gate fica alto; AD/trigger sempre "
                "completa o contorno inteiro, ignorando quanto tempo o "
                "gate fica ligado.",
                ""}},
            {"in:in", {
                "Entrada de áudio — o ENVELOPE embute um VCA: multiplica "
                "o áudio pelo contorno.", "", ""}},
            {"in:gate", {"Gate/trigger que dispara o envelope.", "", ""}},
            {"in:time_mod", {"CV que soma aos tempos (ATK/DEC/REL).", "", ""}},
            {"out:out", {"Áudio × o contorno (o VCA embutido).", "", ""}},
            {"out:env", {
                "O contorno em si, como CV — patcheável pra modular outro "
                "parâmetro (ex.: FILTER.cutoff).", "", ""}},
        }},
        {"WAVETABLE", {
            {"freq", {
                "Frequência base. 1 V/oct pela entrada 1V/O.", "", ""}},
            {"fine", {"Afinação fina em cents (±100).", "", ""}},
            {"pos", {
                "Posição no eixo de FORMA: 0 = serra, ⅓ = quadrada, ⅔ = "
                "formante (vogal), 1 = seno. Soma com a CV de POS e o DRIFT.",
                "As 16 tabelas são geradas por uma receita espectral fixa "
                "no prepare() — sem arquivo de dados; band-limited em 10 "
                "mip-maps pra não aliasar em afinação alta.",
                "Cabeie ENVELOPE.env → POS: a forma abre junto com a nota."}},
            {"warp", {
                "Distorção de fase (Casio CZ / WAVE CUT do WAVE-6): "
                "comprime a primeira metade do ciclo numa janela que "
                "encolhe — de identidade (0) a quase-pulso brilhante (1).",
                "Não muda a tabela, só como a fase percorre ela — cria "
                "harmônicas altas de graça.", ""}},
            {"fm_amount", {"Profundidade da FM linear da entrada FM.", "", ""}},
            {"drift", {
                "Varredura lenta autônoma de POS — a wavetable respira. "
                "0 = determinístico.", "", ""}},
            {"in:pitch", {"CV 1 V/oct.", "", ""}},
            {"in:pos", {"CV somada em POS (LFO, envelope, DRIFT…).", "", ""}},
            {"in:fm", {"Áudio pra FM linear (× FM_AMOUNT).", "", ""}},
            {"in:capture", {
                "Áudio a capturar como tabela — de um AUDIO-IN, um OSC, "
                "qualquer saída de áudio do grafo.",
                "Sem cabo aqui, GRAB não faz nada e POS=1 é o seno "
                "procedural.", ""}},
            {"in:grab", {
                "Trigger: na borda de subida, pega 1024 amostras de "
                "CAPTURE (DC removido + normalizado) — vira o quadro no "
                "topo de POS (crossfade de ~0,9 a 1,0). Sem detecção de "
                "pitch: afine a fonte de ouvido.",
                "Cabeie CLOCK → GRAB: a tabela se renova a cada compasso.",
                ""}},
            {"out:out", {"A forma de onda varrida.", "", ""}},
        }},
        {"ADDITIVE", {
            {"freq", {
                "Frequência base. 1 V/oct pela entrada 1V/O.", "", ""}},
            {"fine", {"Afinação fina em cents (±100).", "", ""}},
            {"tilt", {
                "Brilho — a inclinação do espectro. 0 = escuro (harmônicas "
                "caem rápido, ~k^-2,6); 1 = quase plano (todas as 64 "
                "parciais fortes).",
                "É o oposto de um filtro: em vez de cortar agudo, você "
                "escolhe quanta energia cada parcial recebe na construção.",
                "Cabeie ENVELOPE.env → TILT: o timbre abre o brilho junto "
                "com a nota."}},
            {"odd", {
                "Balanço ímpar/par das parciais. 0 = série cheia; +1 = só "
                "ímpares (quadrada, clarinete, oco brilhante); −1 = só "
                "pares (uma oitava acima, som anasalado).", "", ""}},
            {"stretch", {
                "Inarmonicidade — estica ou comprime as parciais. 0 = "
                "harmônico (razão k); +1 = esticado (sino, metal, piano "
                "agudo); −1 = comprimido (parciais juntas, batimento "
                "denso).",
                "razão_k = k + stretch·0,004·k·(k−1) — o efeito é "
                "quadrático: quase nada nas primeiras parciais, forte nas "
                "de cima.",
                "Module STRCH com um LFO lento: o som metaliza e volta."}},
            {"comb", {
                "Pente espectral: cava vales no espectro. 0 = plano; 1 = "
                "12 dentes, vales fundos — formante grosseiro ou efeito de "
                "filtro em pente, sem filtrar nada.",
                "g_k = (1−comb) + comb·(0,5 + 0,5·cos(2π·dentes·k/64)), "
                "dentes = 1 + 11·comb.", ""}},
            {"fm_amount", {"Profundidade da FM linear da entrada FM.", "", ""}},
            {"drift", {
                "Cintilância — cada parcial ganha um micro-desafino e uma "
                "respiração de amplitude de senóides lentas. 0 = "
                "determinístico.",
                "Sem RNG: é a soma de senóides incomensuráveis, então o "
                "percurso não repete mas o render é reprodutível.", ""}},
            {"in:pitch", {"CV 1 V/oct.", "", ""}},
            {"in:tilt", {"CV somada em TILT.", "", ""}},
            {"in:stretch", {"CV somada em STRCH.", "", ""}},
            {"in:fm", {"Áudio pra FM linear (× FM_AMOUNT).", "", ""}},
            {"out:out", {
                "A soma das 64 parciais, com seguidor de ganho + tanh "
                "pra não estourar.", "", ""}},
        }},
        {"OPERATOR", {
            {"freq", {
                "Frequência do operador A. 1 V/oct pela entrada 1V/O.",
                "", ""}},
            {"fine", {"Afinação fina em cents (±100).", "", ""}},
            {"algo", {
                "Qual operador modula qual — 8 algoritmos, de 0 (A→B→C→D, "
                "o mais 'FM') a 7 (A,B,C,D em paralelo, aditivo/órgão).",
                "A ordem é sempre A→B→C→D e só A tem feedback, então não "
                "há laço entre operadores — o cálculo é uma passada.",
                "Comece no 0 com INDEX médio e gire ALGO devagar: o "
                "espectro se reorganiza a cada passo."}},
            {"ratio_b", {
                "Razão de frequência do operador B em relação a A, "
                "QUANTIZADA: 0,5 / 1 / 1,5 / 2 / 2,5 / 3 / 4 / 5 / 7 / 9.",
                "Inteira = harmônico (timbre musical); quebrada (2,5) = "
                "inarmônico (sino, metal). É o QUANTIZER da altura, mas "
                "pro timbre.", ""}},
            {"ratio_c", {"Razão do operador C (mesma tabela).", "", ""}},
            {"ratio_d", {"Razão do operador D (mesma tabela).", "", ""}},
            {"index", {
                "Profundidade de modulação global. 0 = os 4 operadores "
                "são senóides puras; 1 ≈ 6 ciclos de desvio de fase "
                "(bem brilhante).",
                "Na FM, o índice controla quantas bandas laterais "
                "aparecem e com que força (as funções de Bessel de "
                "Chowning).",
                "Cabeie ENVELOPE.env → IDX: o ataque brilhante que "
                "escurece na cauda — o 'som DX'."}},
            {"feedback", {
                "O operador A modula a própria fase (média das 2 últimas "
                "amostras, à la DX7).",
                "Sozinho já leva a senóide de A a um dente-de-serra — "
                "uma fonte de harmônicos sem precisar de outro operador.",
                ""}},
            {"drift", {
                "Micro-desafino lento e independente por operador. "
                "Determinístico (soma de senos, sem RNG). 0 = sem "
                "desafino.", "", ""}},
            {"in:pitch", {"CV 1 V/oct.", "", ""}},
            {"in:index", {"CV somada em INDEX.", "", ""}},
            {"out:out", {
                "A soma das portadoras do algoritmo, ÷ nº de portadoras "
                "+ softclip.", "", ""}},
        }},
        {"SPECTRA", {
            {"voices", {
                "Quantas senóides o banco de re-síntese usa (2–24). Poucas "
                "= caricatura do espectro; muitas = re-síntese fiel. Cada "
                "voz herda um dos parciais mais fortes que a análise achou.",
                "Resíntese espectral (Panharmonium / phase vocoder): o "
                "módulo OUVE `in`, acha os picos do espectro de curto "
                "prazo e re-oscila. O `ADDITIVE` constrói do zero; aqui o "
                "material é o som que entra.",
                "VOICE baixo + STRETCH = uma sombra inarmônica do som. "
                "OSC/ADDITIVE → IN, VOICE alto = quase um clone."}},
            {"blur", {
                "Quão RÁPIDO cada voz persegue o parcial que herdou: 0 = "
                "trava no som (transientes passam), 1 = arrasta ~0,6 s "
                "(borra, coro fantasma, o som 'derrete').",
                "É a constante de tempo do deslize freq/amp das vozes — "
                "sem zíper em qualquer posição.", ""}},
            {"shift", {
                "Transpõe a RE-SÍNTESE (±2 oitavas, +CV 1 V/oct em PIT) "
                "sem tocar na análise — pitch-shift de espectro, formante "
                "junto.", "", "LFO → PIT: o espectro sobe e desce inteiro."}},
            {"stretch", {
                "Afasta/junta os parciais da re-síntese em torno do meio "
                "→ inarmônico (sino, metal) sem mexer na altura percebida.",
                "", ""}},
            {"tone", {
                "Inclinação espectral da saída: <0 abafa os agudos das "
                "vozes, >0 realça. 0 = fiel à análise.", "", ""}},
            {"jitter", {
                "Wobble lento e SEMEADO na frequência de cada voz "
                "(±3 %·jitter) — a re-síntese 'respira', nunca idêntica. "
                "Determinístico. 0 = estático.", "", ""}},
            {"freeze", {
                "Congela os alvos das vozes: a análise para, o banco "
                "continua oscilando no último espectro. Pad infinito a "
                "partir de qualquer som.",
                "É o *spectral freeze* clássico. Tire a entrada depois de "
                "congelar e a nota fica.", "FRZ ← gate do CLOCK: janelas "
                "de espectro fixo alternando com o som vivo."}},
            {"mix", {"Seco (o que entra em IN) ↔ ressintetizado. 0 = "
                    "bypass. Sem IN, só o molhado importa.", "", ""}},
            {"in:in", {
                "O som a ANALISAR — qualquer voz, um DRUM, uma gravação. "
                "Livre → um ruído interno de −30 dB com dois parciais que "
                "derivam devagar (semeados) alimenta a análise: SPECTRA "
                "sozinho é um drone tonal que evolui.", "", ""}},
            {"in:pitch", {"CV 1 V/oct somada a SHIFT.", "", ""}},
            {"in:freeze", {"Gate: enquanto alto, congela (= o toggle "
                          "FRZ).", "", ""}},
            {"out:out", {"Canal esquerdo do banco re-sintetizado + MIX.",
                        "", ""}},
            {"out:r", {"Canal direito — vozes ímpares/pares "
                      "panoramizadas.", "", ""}},
        }},
        {"PULSAR", {
            {"freq", {
                "Taxa de repetição dos pulsarets = a ALTURA (20–2000 Hz, "
                "+CV 1 V/oct). Cada disparo lança um grão; o silêncio "
                "entre eles é o que define a nota.",
                "Síntese pulsar (Curtis Roads): um trem de pulsarets — "
                "grão curto + silêncio, período total p. freq = 1/p.",
                "SEQUENCE.pitch → PIT: uma melodia. Abaixe até ~30 Hz e "
                "cada pulsaret vira um evento separado (ritmo)."}},
            {"formant", {
                "Frequência interna do pulsaret = o TIMBRE, INDEPENDENTE "
                "da altura (0,1–8×, +CV). Baixo = oco/formântico; alto = "
                "brilhante/nasal. NÃO desafina.",
                "É a segunda frequência da síntese pulsar: formant = 1/d "
                "(d = duração do pulsaret). O pente harmônico fica em "
                "múltiplos de freq; formant só move o ENVELOPE espectral.",
                "LFO → FQM: um 'wah' sem filtro. Em formante≈1 o pulsaret "
                "preenche o período todo (duty 100 %)."}},
            {"shape", {
                "Forma do pulsaret: 0 = 1 ciclo de seno; até 1 = 2–3 "
                "ciclos + um harmônico agudo (pulso mais estreito e "
                "rico).", "", ""}},
            {"window", {
                "Envelope do grão: 0 ≈ retangular (transientes duros, "
                "brilhante), 0,4 = Hann (limpo), 1 = expodec (ataque "
                "rápido + cauda — percussivo).",
                "A janela do grão granular. Retangular vaza banda larga "
                "nas bordas; Hann/expodec suavizam.", ""}},
            {"jitter", {
                "Desvio Rasgo SEMEADO no período e na amplitude de cada "
                "pulsaret — de trem rígido a nuvem irregular. "
                "Determinístico (reproduz com o seed).", "", ""}},
            {"mask", {
                "Probabilidade de PULAR um pulsaret (Roads): 0 = trem "
                "cheio, 1 = quase tudo silêncio. Rareia a textura sem "
                "mudar a altura.",
                "O masking da síntese pulsar — cria padrões rítmicos "
                "burlescos por subtração.", ""}},
            {"spread", {
                "Pulsarets alternados jogados para L/R (± spread) — "
                "largura estéreo por granulação. 0 = mono.", "", ""}},
            {"level", {"Saída, com softclip (tanh) antes.", "", ""}},
            {"in:pitch", {"CV 1 V/oct somada a FREQ.", "", ""}},
            {"in:formant_mod", {
                "CV (em oitavas) somada a FORMANT — move o timbre sem "
                "tocar na altura.", "", ""}},
            {"out:out", {"Canal esquerdo (mono se SPREAD=0).", "", ""}},
            {"out:r", {"Canal direito.", "", ""}},
        }},
        {"OSC", {
            {"freq", {
                "Frequência base do oscilador.",
                "1 V/oct: f = freq · 2^(fine/1200) · 2^(pitch) · 2^(drift).",
                "Conecte SEQUENCE.pitch em 1V/O e ouça a melodia "
                "quantizada tocar o OSC."}},
            {"fine", {
                "Afinação fina em cents (±100).", "",
                "Desafine dois OSC em ±5 cents e some as saídas — "
                "batimento de coro."}},
            {"pw", {
                "Largura de pulso — só afeta a saída PLS.",
                "Em 0,5 o pulso é onda quadrada; nos extremos vira um "
                "trem de picos finos.",
                "Module PWM com um LFO lento e ouça o pulso \"respirar\"."}},
            {"fm_amount", {
                "Quantidade de FM linear through-zero pela entrada FM.",
                "dp pode ficar negativo — a fase anda pra trás, FM de "
                "verdade (não a FM de fase, que \"quebra\" em áudio-rate).",
                "Suba FM com outro OSC na entrada FM: em taxa de áudio "
                "vira timbre metálico."}},
            {"drift", {
                "Passeio lento e correlacionado na afinação (±meio "
                "semitom no máximo).",
                "", "drift=0 é determinístico; suba aos poucos pra um "
                "vibrato natural bem lento."}},
            {"sub_2", {
                "Sub-oscilador uma oitava (desligado) ou duas (ligado) "
                "abaixo.", "", ""}},
            {"sync_enable", {
                "Liga a resposta ao jack SYNC (hard sync).",
                "Borda de subida em SYNC reinicia a fase do oscilador "
                "principal (e do SUB, que acompanha).",
                "Toque um OSC2 lento em SYNC com SYNC_ENABLE ligado: a "
                "afinação do OSC principal \"trava\" na do OSC2."}},
            {"prox", {
                "Mistura com uma versão abafada de si mesmo — "
                "\"profundidade\" sem precisar de outro módulo.",
                "prox=0 é exatamente a saída crua; subindo, mistura um "
                "passa-baixa de 1 polo bem suave por cima.",
                "Suba PROX devagar numa saída SAW rica em harmônicos: o "
                "som escurece sem precisar de um FILTER."}},
            {"in:pitch", {"CV 1 V/oct — soma ao FREQ.", "", ""}},
            {"in:fm", {"Entrada de FM through-zero.", "", ""}},
            {"in:pwm", {"CV que soma ao PW.", "", ""}},
            {"in:sync", {"Trigger de hard sync (precisa de SYNC_ENABLE).",
                        "", ""}},
            {"out:sine", {"Saída senoidal.", "", ""}},
            {"out:tri", {"Saída triangular.", "", ""}},
            {"out:saw", {"Saída dente-de-serra (antialias PolyBLEP).", "", ""}},
            {"out:pulse", {"Saída de pulso (largura = PW).", "", ""}},
            {"out:sub", {"Sub-oscilador (uma ou duas oitavas abaixo).", "", ""}},
        }},
        {"VCA", {
            {"level1", {
                "Ganho manual do canal 1.",
                "ganho = LVL1 + CV1_AMOUNT · cv1 — o knob continua vivo "
                "com CV conectada (soma por PORTA, não por "
                "connectToParameter).",
                ""}},
            {"level2", {"Ganho manual do canal 2 (igual ao canal 1).", "", ""}},
            {"cv1_amount", {
                "Atenuversor da entrada CV1 (−1..1) — soma ao LVL1.", "", ""}},
            {"cv2_amount", {
                "Atenuversor da entrada CV2 (−1..1) — soma ao LVL2.", "", ""}},
            {"response1", {
                "0 = linear (bom pra somar CV), 1 = exponencial "
                "(\"dB-linear\", bom pra volume de áudio).",
                "ganho_final = ganho^(1+3·resp).", ""}},
            {"response2", {"Igual ao RSP1, pro canal 2.", "", ""}},
            {"drift", {
                "Oscilação lenta e correlacionada nos dois ganhos "
                "(±~3% no máximo) — \"o VCA respira\".", "", ""}},
            {"in:in1", {"Entrada de áudio ou CV do canal 1.", "", ""}},
            {"in:cv1", {"CV de controle do ganho do canal 1.", "", ""}},
            {"in:in2", {"Entrada de áudio ou CV do canal 2.", "", ""}},
            {"in:cv2", {"CV de controle do ganho do canal 2.", "", ""}},
            {"out:out1", {"Saída do canal 1.", "", ""}},
            {"out:out2", {"Saída do canal 2.", "", ""}},
            {"out:sum", {"out1 + out2, com teto suave — mini-mixer de 2 "
                        "canais de brinde.", "", ""}},
        }},
        {"VCA4", {
            {"level1", {
                "Ganho manual do canal 1 (o knob que a CV1 soma — "
                "modulação por PORTA, o knob fica vivo).",
                "Banco de 4 VCAs + um mixer — Mutable Veils / Intellijel "
                "Quad VCA. O `VCA` (#20) é duplo; este é o banco pra "
                "patch grande.", ""}},
            {"level2", {"Ganho manual do canal 2.", "", ""}},
            {"level3", {"Ganho manual do canal 3.", "", ""}},
            {"level4", {"Ganho manual do canal 4.", "", ""}},
            {"cv1_amt", {"Atenuversor da CV1 (−1..1) — soma ao LVL1. "
                        "Negativo INVERTE a ação da CV.", "", ""}},
            {"cv2_amt", {"Atenuversor da CV2.", "", ""}},
            {"cv3_amt", {"Atenuversor da CV3.", "", ""}},
            {"cv4_amt", {"Atenuversor da CV4.", "", ""}},
            {"curve", {
                "COMPARTILHADO: 0 = linear (bom pra somar CV), 1 = "
                "exponencial (dB-linear, bom pra volume percebido de "
                "áudio). `ganho^(1+3·curve)`.", "", ""}},
            {"mix_gain", {
                "Ganho da saída MIX (0–2×) — a soma dos 4 canais com teto "
                "suave. Um mixer de 4 canais de brinde.", "", ""}},
            {"drift", {
                "Oscilação lenta e SEMEADA nos 4 ganhos (±~3 %) — o banco "
                "respira. Determinístico. 0 = estático.", "", ""}},
            {"in:in1", {"Áudio (ou CV) do canal 1.", "", ""}},
            {"in:in2", {"Áudio do canal 2.", "", ""}},
            {"in:in3", {"Áudio do canal 3.", "", ""}},
            {"in:in4", {"Áudio do canal 4.", "", ""}},
            {"in:cv1", {"CV do ganho do canal 1.", "", ""}},
            {"in:cv2", {"CV do ganho do canal 2.", "", ""}},
            {"in:cv3", {"CV do ganho do canal 3.", "", ""}},
            {"in:cv4", {"CV do ganho do canal 4.", "", ""}},
            {"out:out1", {"Saída do canal 1.", "", ""}},
            {"out:out2", {"Saída do canal 2.", "", ""}},
            {"out:out3", {"Saída do canal 3.", "", ""}},
            {"out:out4", {"Saída do canal 4.", "", ""}},
            {"out:mix", {"Soma dos 4 canais × MIXG, com teto suave.",
                        "", ""}},
        }},
        {"CLOCK", {
            {"bpm", {
                "Andamento em batidas por minuto.", "", ""}},
            {"mult", {
                "Multiplicador de passos por tempo (0,25–8) — quantos "
                "passos de CLOCK cabem numa batida.", "", ""}},
            {"length", {
                "Nº de passos da grade euclidiana (pra EUC/ACC).", "", ""}},
            {"fill", {
                "Quantos dos LENGTH passos disparam em EUC.",
                "Distribuídos o mais uniformemente possível (Bresenham, "
                "equivalente cíclico ao Bjorklund).",
                "FILL baixo (ex. 3 de 16) soa esparso; perto de LENGTH "
                "soa quase contínuo."}},
            {"rotate", {
                "Gira o padrão euclidiano — mesma densidade, fase "
                "diferente.", "", ""}},
            {"swing", {
                "Atrasa os passos ÍMPARES dentro da própria fatia de "
                "tempo.", "", ""}},
            {"drift", {
                "Passeio lento no andamento efetivo (±~12%).",
                "drift=0 → determinístico.", ""}},
            {"gate_len", {
                "Fração de cada passo em que CLOCK fica em nível alto.",
                "", ""}},
            {"accent_a", {
                "1º divisor do acento — ACC dispara quando o passo é "
                "múltiplo de ACC-A (e/ou de ACC-B, ver AND).", "", ""}},
            {"accent_b", {"2º divisor do acento.", "", ""}},
            {"accent_mode", {
                "AND: acento exige múltiplo de A E de B. Desligado: "
                "múltiplo de A OU de B.",
                "", "Compare AND ligado/desligado com ACC-A=3, ACC-B=4 — "
                "polirritmia \"de graça\"."}},
            {"feel", {
                "Agrupamento do passo: reto/tercina/quintina/septina/"
                "nonina/undecina/glitch.",
                "Multiplica MULT por 1/3/5/7/9/11 (quantizado e nomeado, "
                "não precisa acertar o número de ouvido); glitch sorteia "
                "um fator de tempo por passo (~0,7–1,4×) em vez de "
                "multiplicar por um número fixo — o clock deixa de ser "
                "regular.",
                "Compare FEEL=reto e FEEL=tercina no mesmo MULT: a "
                "tercina soa 3x mais rápida, agrupada de 3 em 3."}},
            {"in:ext_clock", {
                "Clock externo — se conectado, CLOCK segue as bordas de "
                "subida em vez do BPM interno.", "", ""}},
            {"in:reset", {"Zera a posição do passo euclidiano.", "", ""}},
            {"in:bpm_mod", {"CV que soma ao BPM.", "", ""}},
            {"out:clock", {"Pulso de passo estável em bpm·mult.", "", ""}},
            {"out:euclid", {"Gate euclidiano (FILL de LENGTH passos).", "", ""}},
            {"out:accent", {"Acento — AND/OR de ACC-A e ACC-B.", "", ""}},
        }},
        {"NOISE", {
            {"rate", {
                "Ritmo do relógio interno do S&H/smooth (ignorado se "
                "TRIG estiver conectado).", "", ""}},
            {"slew", {
                "Tempo de glide da saída SMTH — 0 é instantâneo (degrau, "
                "vira quase um S&H), 1 é ~2 s de deslize.", "", ""}},
            {"spread", {
                "Puxa o sorteio de uniforme pra sino (média de 4 "
                "uniformes) — afeta SH e SMTH.",
                "spread=0 é uniforme (cai igual em qualquer valor); "
                "spread=1 concentra perto de 0.", ""}},
            {"poisson", {
                "Troca o relógio interno periódico por um processo de "
                "Poisson LIVRE — SH/SMTH atualizam em tempos aleatórios, "
                "não presos a um clock. RATE vira a taxa média.",
                "Intervalo = mistura entre o período regular e um sorteio "
                "exponencial (t = −ln(U)/λ); poisson=1 é Poisson puro. "
                "Ignorado se TRIG estiver conectado.",
                "É o \"contador Geiger\" — par da saída GEIG do BOXCAR "
                "(#51). Semeado: dois renders idênticos."}},
            {"in:trigger", {
                "Trigger externo pro S&H/SMTH — presente, substitui o "
                "relógio interno (RATE).", "", ""}},
            {"in:in", {
                "Fonte pro S&H amostrar — sem cabo, o S&H usa o sorteio "
                "interno em vez da entrada.", "", ""}},
            {"out:white", {
                "Ruído branco — energia igual em toda frequência.", "", ""}},
            {"out:pink", {
                "Ruído rosa — cai ~3 dB/oitava (filtro Kellet de 7 "
                "polos). Mais natural pra vento/mar que o branco.", "", ""}},
            {"out:brown", {
                "Ruído marrom — cai ~6 dB/oitava (passeio aleatório "
                "filtrado). Grave, surdo.", "", ""}},
            {"out:sh", {
                "Sample & hold — segura um valor novo a cada pulso "
                "(RATE ou TRIG), degraus.", "", ""}},
            {"out:smooth", {
                "Tensão que desliza até um alvo novo a cada pulso, em "
                "vez de saltar — Buchla 266 \"smooth random\".", "",
                "Compare SH e SMTH com o mesmo RATE: SH pula, SMTH "
                "desliza."}},
            {"out:blue", {
                "Ruído azul — sobe ~3 dB/oitava (branco diferenciado). "
                "Mais agudo/áspero que o branco.", "", ""}},
            {"out:violet", {
                "Ruído violeta — sobe ~6 dB/oitava (diferenciado 2×). "
                "Ainda mais agudo que o azul — quase só chiado.", "", ""}},
            {"out:bit", {
                "1 bit bipolar (+1/−1) direto do gerador, trocando a "
                "cada amostra — textura digital dura, diferente do "
                "degrau do S&H (que só troca no pulso).", "", ""}},
        }},
        {"DRIFT", {
            {"rate", {
                "Ritmo do passeio do campo compartilhado — escala de "
                "MINUTOS, não de áudio.", "", ""}},
            {"depth", {
                "Alcance do passeio — quanto o campo pode se afastar do "
                "repouso.", "", ""}},
            {"momentum", {
                "Quanto a velocidade do passeio tende a continuar em vez "
                "de reagir na hora.",
                "vel = vel·(0,2+0,78·momentum) + impulso·(1,1−0,75·"
                "momentum) — momentum alto = trajetória mais suave, "
                "menos nervosa.", ""}},
            {"stride", {
                "0 = as 4 saídas (A-D) andam quase juntas; 1 = cada uma "
                "pro seu lado.",
                "Cada saída lê o mesmo campo com PESOS diferentes — "
                "STRIDE controla o quanto esses pesos divergem.", ""}},
            {"bias", {
                "Empurra o repouso do campo pra um lado (não fica "
                "centrado em 0).", "", ""}},
            {"anchor", {
                "Memória de topologia — a cada 8 tiques grava o estado "
                "como marco; com probabilidade ∝ anchor², volta pro "
                "marco em vez de dar um passo novo.",
                "anchor=0 é passeio livre (nunca volta); anchor alto faz "
                "a deriva orbitar paisagens já visitadas em vez de "
                "vagar pra sempre.", ""}},
            {"in:advance", {
                "Trigger — força um passo (cadência por compasso/frase "
                "em vez do RATE interno).", "", ""}},
            {"in:rate_mod", {"CV que soma ao RATE.", "", ""}},
            {"out:a", {"Saída correlacionada A — lê o campo com um peso "
                       "próprio.", "", ""}},
            {"out:b", {"Saída correlacionada B — peso diferente de A.", "", ""}},
            {"out:c", {"Saída correlacionada C.", "", ""}},
            {"out:d", {"Saída correlacionada D.", "", ""}},
            {"out:field", {"O passeio cru compartilhado, sem os pesos "
                           "por saída.", "", ""}},
            {"out:event", {"Pulso a cada tique do passeio.", "", ""}},
        }},
        {"MIXER", {
            {"gain1", {"Ganho do canal 1, em dB.", "", ""}},
            {"gain2", {"Ganho do canal 2, em dB.", "", ""}},
            {"gain3", {"Ganho do canal 3, em dB.", "", ""}},
            {"gain4", {"Ganho do canal 4, em dB.", "", ""}},
            {"pan1", {"Posição estéreo do canal 1 (lei de potência "
                      "constante).", "", ""}},
            {"pan2", {"Posição estéreo do canal 2.", "", ""}},
            {"pan3", {"Posição estéreo do canal 3.", "", ""}},
            {"pan4", {"Posição estéreo do canal 4.", "", ""}},
            {"mute1", {"Silencia o canal 1 sem mexer no GAIN.", "", ""}},
            {"mute2", {"Silencia o canal 2.", "", ""}},
            {"mute3", {"Silencia o canal 3.", "", ""}},
            {"mute4", {"Silencia o canal 4.", "", ""}},
            {"out_gain", {"Ganho do barramento de saída — depois da "
                          "soma dos 4 canais.", "", ""}},
            {"in:ch1", {"Entrada de áudio do canal 1.", "", ""}},
            {"in:ch2", {"Entrada de áudio do canal 2.", "", ""}},
            {"in:ch3", {"Entrada de áudio do canal 3.", "", ""}},
            {"in:ch4", {"Entrada de áudio do canal 4.", "", ""}},
            {"out:out", {"Soma estéreo dos 4 canais — normalmente vai "
                        "pro MASTER.", "", ""}},
        }},
        {"MASTER", {
            {"gain", {
                "Ganho final — depois dele só a proteção de saída "
                "(bloqueio de DC + limitador) e as caixas.",
                "O medidor VU mostra o pico recente relativo ao teto de "
                "−1 dBFS; o quadrado à direita acende quando o "
                "limitador teve mesmo que segurar algo.", ""}},
            {"width", {
                "Largura estéreo (mid/side) — 0 é mono, 1 é normal, 2 é "
                "o dobro de lado.",
                // Achado da escuta de 23 set. 2026: o autor girou o WIDTH
                // e não ouviu diferença. Medido: em fonte MONO a variação
                // é 0,00 dB em todo o curso, e em estéreo é 232 dB. O
                // código está certo — mid/side não tem o que escalar
                // quando os canais são iguais. O que faltava era DIZER
                // isso, em vez de deixar o músico concluir que o knob
                // está quebrado.
                "Não faz nada quando a fonte é MONO, e isso não é "
                "defeito: largura mexe no LADO (a diferença entre os "
                "canais), e num sinal em que L = R o lado é zero. A "
                "maioria dos módulos é mono — para ouvir o WIDTH, "
                "alimente o MASTER com duas fontes diferentes, uma por "
                "canal, ou com um módulo de saída estéreo como o HALL.",
                ""}},
            {"mono", {"Força L=R=(L+R)/2 — checagem de compatibilidade "
                      "mono.", "", ""}},
            {"dc_block", {
                "Bloqueio de DC (passa-alta ~5 Hz) — tira offset que "
                "não é som, só aqueceria o limitador à toa.", "", ""}},
            {"body_guard", {
                "BODY — governador de corpo. Só age em som ALTO, "
                "SUSTENTADO e CONCENTRADO na faixa de 2,5 a 8 kHz, que é "
                "onde o ouvido cansa e dói.",
                "Ataque lento (~250 ms) e limiar alto, de propósito: "
                "transiente, ritmo e rajada de ruído passam intocados — "
                "só o que fica PARADO, alto e concentrado ali dispara "
                "(filtro auto-oscilando, quadrada segurada, folding "
                "travado). Quando dispara, aplica poucos dB de "
                "high-shelf, acompanhando. Na esmagadora maioria do "
                "material ele NÃO age, e é por isso que girar o knob "
                "costuma não mudar nada. A marca laranja na ponta "
                "ESQUERDA do VU acende quando ele está agindo de fato — "
                "sem ela, um controle que trabalha em silêncio é "
                "indistinguível de um controle quebrado.",
                "ponha um FILTER em auto-oscilação na faixa de 3–5 kHz e "
                "deixe soando: é aí que o BODY mostra a que veio. Compare "
                "com ele em 0 (bypass exato) e em 1."}},
            {"limit", {
                "Limitador com look-ahead — segura o pico ANTES dele "
                "chegar, sem distorcer o transiente.",
                "O detector olha o MAIOR entre o pico de amostra e uma "
                "estimativa de pico ENTRE amostras — um pico que a "
                "amostra sozinha não veria também é pego.", ""}},
            {"in:in", {"Entrada estéreo — normalmente vem do MIXER.", "", ""}},
            {"out:out", {"Saída final, estéreo.", "", ""}},
            {"out:level", {
                "Pico de saída com decaimento (~300 ms), 0..1 — CV pra "
                "um módulo SEGUIR o próprio volume do master.", "", ""}},
        }},
        {"WASP", {
            {"cutoff", {"Frequência de corte.", "", ""}},
            {"resonance", {
                "Ressonância — perto de 1 o filtro auto-oscila, mais "
                "\"palhetado\" que o FILTER comum.", "", ""}},
            {"mode", {"LP ↔ BP ↔ HP.", "", ""}},
            {"drive", {
                "Ganho de entrada — empurra o sinal pro ceifador ANTES "
                "do filtro.", "", ""}},
            {"grit", {
                "Joelho do ceifador no laço — quanto mais GRIT, mais "
                "cedo e mais duro ceifa.",
                "Estudado do EDP Wasp (1978): um inversor CMOS 4069 "
                "usado como ganho dentro de um Sallen-Key não-linear.",
                ""}},
            {"bias", {
                "Teto assimétrico — soma harmônicos pares (mais "
                "\"buzz\") e bloqueia DC.", "", ""}},
            {"drift", {
                "Passeio lento e correlacionado no corte/ressonância.", "", ""}},
            {"in:in", {"Entrada de áudio.", "", ""}},
            {"in:cutoff_mod", {"CV que soma ao CUT.", "", ""}},
            {"in:res_mod", {"CV que soma a RESO.", "", ""}},
            {"out:out", {"Saída filtrada/ceifada.", "", ""}},
        }},
        {"BOXCAR", {
            {"delay", {
                "Onde a janela de amostragem abre, como fração do período "
                "medido pelo TRIG. Gira uma \"cabeça de leitura\" por "
                "dentro da forma de onda capturada.",
                "Inspirado no boxcar averager (integrador de porta) — "
                "equipamento de teste nuclear. O DELAY é o \"aperture "
                "delay\" do instrumento.", ""}},
            {"aperture", {
                "Largura da janela, fração do período. →0 = amostra "
                "pontual (vira um S&H de fase fixa); larga = a média "
                "borra um arco da onda.", "", ""}},
            {"average", {
                "Profundidade N da média: quantas capturas se empilham "
                "num mesmo ponto de fase. N alto = o ruído descorrelato "
                "some (~1/√N), o sinal coerente fica.",
                "Média corrente com piso 1/min(N, capturas) — as "
                "primeiras capturas contam mais, depois estabiliza.", ""}},
            {"scan", {
                "Velocidade e direção com que o DELAY varre o período "
                "sozinho. 0 = estático (um ponto só). ≠0 = a janela "
                "passeia e reconstrói a onda TODA (scanning boxcar).",
                "Os modos RECONSTRUCT e OSC precisam de SCAN≠0 pra o "
                "buffer de fase encher.", ""}},
            {"mode", {
                "0 FOLLOWER — a saída segura a média da última janela "
                "(um S&H de janela). 1 RECONSTRUCT — toca a onda "
                "reconstruída em sincronia com o TRIG. 2 OSC — relê o "
                "buffer reconstruído a RATE, livre do TRIG.",
                "No modo 2 a \"wavetable\" é o que o módulo ouviu, "
                "mediado — a onda vem da própria história (desvio "
                "Warps/Rings: a relação é o processo).", ""}},
            {"rate", {
                "Relógio interno (se TRIG estiver livre) e frequência de "
                "releitura no modo OSC.", "", ""}},
            {"thresh", {
                "Limiar do auto-trigger: sem cabo no TRIG, um cruzamento "
                "ascendente de IN por este nível é a referência de "
                "repetição (edge trigger de osciloscópio). O jack THR "
                "soma CV.", "", ""}},
            {"geiger", {
                "Densidade de um trem de gates de POISSON livre na saída "
                "GEIG — não preso a nenhum clock. 0 = saída muda.",
                "t = −ln(U)/λ, λ ≈ 0,5 + geiger²·40 ev/s. Semeado "
                "(xorshift) — dois renders idênticos. É o \"contador "
                "Geiger\"; o NOISE tem o mesmo modo (POIS).", ""}},
            {"blend", {
                "IN ↔ resultado. 0 = passa o seco (bypass); 1 = só o "
                "processado.", "", ""}},
            {"in:in", {
                "Sinal a analisar (áudio ou CV). Desconectado, o módulo "
                "usa um piso de ruído interno de −34 dB pra ter o que "
                "reconstruir (soa ao carregar).", "", ""}},
            {"in:trig", {
                "Referência de repetição — o evento a que as capturas se "
                "travam. Sem cabo: cruzamentos do limiar (THRSH) ou o "
                "relógio interno (RATE).", "", ""}},
            {"in:sweep", {
                "CV somada ao DELAY — varre a posição da janela pela "
                "onda (uma frase de posições vinda de um SEQUENCE).", "",
                ""}},
            {"in:thr", {"CV somada ao limiar THRSH do auto-trigger.", "",
                       ""}},
            {"out:out", {
                "O sinal seguido / reconstruído, misturado com o seco "
                "por BLEND. Clamp de segurança a ±8.", "", ""}},
            {"out:geiger", {
                "Trem de gates de Poisson livre (ver GEI). Gate de "
                "~5 ms.", "", ""}},
        }},
        {"ABACUS", {
            {"op", {
                "Operação entre A e B: 0 soma, 1 subtrai, 2 multiplica, 3 "
                "resto (mod RANGE), 4-7 bit a bit (AND/OR/XOR/NAND).",
                "0-3 é aritmética contínua; 4-7 trata A/B como inteiros de "
                "5 bits (janela ±RANGE → 0..31) e opera nos bits — ideia "
                "Lunetta (Numeric Repetitor).",
                "Compare OP=soma com OP=XOR na mesma entrada — a versão "
                "bit a bit quebra o sinal em degraus imprevisíveis."}},
            {"modulus", {
                "Módulo do contador binário — ele reinicia a cada MOD "
                "tiques de CLOCK.", "", ""}},
            {"steps", {"Nº de degraus da quantização em QNT.", "", ""}},
            {"range", {
                "Janela de tensão (±RNG) usada por MATH/QNT/RECT e pela "
                "rampa interna do contador (quando A não está conectado).",
                "", ""}},
            {"rect_mode", {
                "Modo do retificador: meia-onda positiva, meia-onda "
                "negativa, onda completa, ou sinal (±RANGE/0).", "", ""}},
            {"count_step", {
                "Quanto o contador soma (ou subtrai, se negativo) a cada "
                "CLOCK.", "", ""}},
            {"pattern", {
                "Escolhe qual bit do contador vira P1 (e o par P1/P2).",
                "PAT desloca a janela de 2 bits lidos do contador — bits "
                "baixos trocam rápido (divisor fino), bits altos trocam "
                "devagar (divisor grosso).", ""}},
            {"slew", {"Suaviza MATH e QNT — 0 é degrau instantâneo.", "", ""}},
            {"rate", {"Relógio interno do contador, usado só se CLK estiver "
                      "livre.", "", ""}},
            {"in:a", {
                "Entrada A — sem cabo, a fonte é a própria rampa do "
                "contador (o ABACUS toca melodia e ritmo sozinho).", "", ""}},
            {"in:b", {"Entrada B — usada em soma/subtração/multiplicação e "
                      "nas operações bit a bit.", "", ""}},
            {"in:clock", {"Avança o contador na borda de subida — se livre, "
                          "usa RATE.", "", ""}},
            {"in:reset", {"Zera o contador e a fase interna.", "", ""}},
            {"out:math", {"A ⊕ B por OP.", "", ""}},
            {"out:quant", {"A quantizado em STEPS degraus dentro de RANGE.",
                          "", ""}},
            {"out:rect", {"A retificado conforme RECT.", "", ""}},
            {"out:p1", {"Bit do contador escolhido por PAT — pulso limpo, "
                       "divisor.", "", ""}},
            {"out:p2", {"P1 XOR o bit seguinte — sincopado.", "", ""}},
            {"out:carry", {
                "Pulso curto sempre que o contador cruza um múltiplo de "
                "MOD — a \"virada\" do ritmo.", "", ""}},
        }},
        {"SIGNAL-IN", {
            {"gain", {"Ganho aplicado à entrada de áudio ao vivo.", "", ""}},
            {"bend", {
                "Alcance do pitch-bend do MIDI, em semitons (0–24). "
                "Afeta a saída 1V/O.", "", ""}},
            {"cc_num", {
                "Qual Control Change do MIDI a saída CC segue (1 = mod "
                "wheel).", "", ""}},
            {"out:out", {
                "Áudio da entrada do sistema, canal esquerdo — sem nada "
                "capturando, silêncio (nunca trava, nunca lê lixo).",
                "Lado receptor de um anel SPSC alimentado por uma thread "
                "de captura externa (`AlsaSource`); o núcleo "
                "`rasgo_modular_core` não sabe o que é ALSA.",
                "Rode outro instrumento e ligue L num FILTER — o Rasgo "
                "passa a processar áudio de fora."}},
            {"out:r", {"Áudio da entrada, canal direito.", "", ""}},
            {"out:pitch", {
                "Altura da nota MIDI tocada, como CV 1 V/oct (nota 60 = "
                "0 V), com o pitch-bend somado (× BEND).",
                "Voz MONOFÔNICA com pilha de notas (last-note priority): "
                "solte a nota de cima e a de baixo volta a soar. É a "
                "contraparte de entrada do NOTE-OUT.", ""}},
            {"out:gate", {
                "Alto enquanto alguma tecla está pressionada (rampa de "
                "1 ms anti-clique).", "", ""}},
            {"out:vel", {"Velocity da última nota (0–1), segurada.",
                        "", ""}},
            {"out:cc", {"O valor do CC nº CC# (0–1), segurado.", "", ""}},
        }},
        {"CHAOS", {
            {"rate", {
                "Velocidade da dinâmica — de CV bem lenta (0,02 Hz) a "
                "textura de áudio (400 Hz).", "", ""}},
            {"drive", {
                "Força da não-linearidade — quanto mais alto, mais forte o "
                "sistema é puxado entre os dois poços estáveis.", "", ""}},
            {"damping", {
                "Amortecimento — alto assenta num poço só; baixo deixa o "
                "sistema oscilar/\"caçar\" entre os dois.", "", ""}},
            {"freeze", {"Congela a dinâmica no valor atual.", "", ""}},
            {"in:reseed", {
                "Trigger — reposiciona x/y num ponto novo aleatório, sem "
                "esperar o sistema escapar sozinho.", "", ""}},
            {"in:rate_mod", {"CV que soma ao RATE.", "", ""}},
            {"out:out", {
                "A posição do sistema, sempre em [−1,1] — o sinal caótico "
                "de poço duplo.",
                "Sem o \"chute\" periódico (ligado ao RATE), o sistema "
                "nunca cruza de um poço pro outro sozinho — é uma EDO "
                "determinística sem forçamento.",
                "Suba DRIVE perto do máximo com DAMP baixo: o sistema "
                "\"caça\" entre os dois poços de forma imprevisível."}},
        }},
        {"CHORD", {
            {"freq", {"Frequência base — a fundamental do acorde.", "", ""}},
            {"chord", {
                "Escolhe o formato do acorde entre 10 tabelas (uníssono, "
                "oitavas, quinta, maior, menor, sus4, maj7, min7, dim, "
                "add9).", "", ""}},
            {"voices", {
                "Quantas vozes soam (2-4) — menos vozes é um acorde mais "
                "simples mesmo com CHORD alto.", "", ""}},
            {"inversion", {
                "Sobe as vozes mais graves uma oitava — inversão do "
                "acorde.", "", ""}},
            {"voicing", {
                "Liga a condução de vozes: na troca de acorde, cada voz "
                "desliza pro tom mais PRÓXIMO do que já tocava, em vez de "
                "pular direto.",
                "Sem VLEAD toda troca é paralela (todas as vozes saltam "
                "juntas); com VLEAD o movimento fica mínimo — atribuição "
                "gulosa por distância, com glide.",
                "Compare CHORD variando com VLEAD ligado/desligado: "
                "desligado soa \"em bloco\", ligado soa como um coral "
                "conduzindo vozes."}},
            {"detune", {"Desafina levemente as vozes entre si — coro.", "", ""}},
            {"wave", {
                "Morfa a forma de cada voz: serra → pulso → triângulo.", "", ""}},
            {"drift", {"Passeio lento e independente por voz.", "", ""}},
            {"in:pitch", {"CV 1 V/oct — soma à base FREQ.", "", ""}},
            {"in:chord_cv", {
                "CV que soma ao knob CHORD — module com HARMONY.root pra "
                "progressões que trocam sozinhas.", "", ""}},
            {"in:fm", {"FM linear simples sobre todas as vozes.", "", ""}},
            {"out:out", {
                "Soma normalizada (1/√vozes) das vozes ativas.", "", ""}},
        }},
        {"CONTROL", {
            {"scale1", {"Atenuversor do canal 1 — ganho de −2 a 2 (negativo "
                       "inverte).", "", ""}},
            {"scale2", {"Atenuversor do canal 2.", "", ""}},
            {"offset1", {
                "Constante somada depois do SCALE — com SCALE=0 vira fonte "
                "de tensão manual.", "", ""}},
            {"offset2", {"Igual ao OFFSET1, canal 2.", "", ""}},
            {"rectify1", {
                "Retificação contínua: 0 passa, 0,5 meia-onda (= max(x,0) "
                "exato), 1 onda completa.", "", ""}},
            {"rectify2", {"Igual ao RECT1, canal 2.", "", ""}},
            {"slew1", {"Tempo de deslize do canal 1 (0-2 s).", "", ""}},
            {"slew2", {"Tempo de deslize do canal 2.", "", ""}},
            {"curve1", {
                "Forma do slew: 0 linear (inclinação constante — "
                "portamento), 1 exponencial (RC — seguidor de envelope).",
                "RECT + SLEW = seguidor de envelope clássico (retifica, "
                "depois passa-baixa).", ""}},
            {"curve2", {"Igual ao CRV1, canal 2.", "", ""}},
            {"sum_mode", {"SUM = soma (com teto) ou média dos dois canais.",
                         "", ""}},
            {"drift", {
                "Passeio lento opt-in nos dois offsets — \"humaniza\" uma "
                "tensão parada.", "", ""}},
            {"in:in1", {"Entrada do canal 1.", "", ""}},
            {"in:in2", {"Entrada do canal 2.", "", ""}},
            {"out:out1", {"Saída processada do canal 1.", "", ""}},
            {"out:out2", {"Saída processada do canal 2.", "", ""}},
            {"out:sum", {"out1+out2, soma ou média conforme SUM.", "", ""}},
        }},
        {"DECISION", {
            {"rate", {"Relógio interno, usado só se TRIG estiver livre.",
                     "", ""}},
            {"bias", {
                "Probabilidade do GATE disparar a cada passo (0 nunca, 1 "
                "sempre).", "", ""}},
            {"spread", {
                "Alcance de X/Y — 0 trava perto de 0, 1 usa a faixa toda "
                "±1.", "", ""}},
            {"shape", {
                "Forma da distribuição de X/Y: 0 uniforme, 1 em sino (soma "
                "de 4 uniformes — central limit, sem log/cos).", "", ""}},
            {"steps", {"Quantiza X/Y em STEPS degraus — 1 é contínuo.", "", ""}},
            {"slew", {"Suaviza a transição entre valores sorteados.", "", ""}},
            {"dejavu", {
                "Probabilidade de RELER a memória em vez de sortear de "
                "novo — o \"déjà-vu\".",
                "Com DEJA alto, o mesmo trecho de LOOP passos tende a se "
                "repetir; com DEJA=0 é sempre sorteio novo.",
                "Suba DEJA aos poucos com LOOP curto: o patch começa a "
                "repetir frases, como um loop que trava sozinho."}},
            {"loop_length", {
                "Tamanho da memória circular relida pelo déjà-vu.", "", ""}},
            {"in:trigger", {
                "Avança um passo na borda de subida — presente, substitui "
                "RATE.", "", ""}},
            {"in:bias_mod", {"CV que soma a BIAS.", "", ""}},
            {"in:spread_mod", {"CV que soma a SPRD.", "", ""}},
            {"out:x", {"CV sorteada (ou relida do déjà-vu), canal X.", "", ""}},
            {"out:y", {"CV sorteada (ou relida), canal Y — independente de "
                      "X.", "", ""}},
            {"out:gate", {
                "Gate de Bernoulli — dispara com probabilidade BIAS a cada "
                "passo.", "", ""}},
        }},
        {"STAGES", {
            {"segments", {
                "Quantos degraus/rampas na volta (2–8). Mais segmentos = "
                "mais resolução na forma.", "", ""}},
            {"rate", {
                "Velocidade da volta no modo LOOP (0,02–20 Hz, +CV). Cada "
                "segmento leva (1/rate)/segments, ajustado por TILT.", "",
                ""}},
            {"contour", {
                "A FORMA dos níveis-alvo: 0 escada subindo (0→1), 0,5 "
                "arco (0→1→0), 1 escada descendo — blend contínuo. É o "
                "'desenho' da função.",
                "Gerador, não editor de breakpoints (identidade RASGO): "
                "você esculpe a forma com macros.", ""}},
            {"curve", {
                "Curva de transição de cada segmento: <0 exponencial "
                "(rápido→lento), 0 linear, >0 logarítmica (devagar→"
                "rápido). Não faz nada com HOLD=1 (não há transição).",
                "", ""}},
            {"hold", {
                "0 = cada segmento DESLIZA suave até o próximo nível "
                "(rampa — envelope/LFO); 1 = SALTA e segura (degrau — "
                "S&H/sequência); entre, rampa parte e segura o resto.",
                "É o knob que faz o STAGES virar envelope OU sequenciador "
                "sem trocar de módulo.", ""}},
            {"tilt", {
                "Distorção das durações: <0 os segmentos do começo mais "
                "longos (attack lento), >0 os do fim (release lento).",
                "", ""}},
            {"jitter", {
                "Passeio lento SEMEADO nos níveis e durações — a forma "
                "'respira' sem deixar de ser reprodutível (jitter=0 → "
                "sem termo, byte-idêntico).", "", ""}},
            {"loop", {
                "Corre livre (LFO complexo de N segmentos) ↔ um disparo "
                "(envelope — precisa do GATE; congela no último nível até "
                "o próximo gate).", "", ""}},
            {"in:gate", {
                "Modo one-shot (LOOP off): a borda de subida inicia uma "
                "passagem. Ignorado no modo loop.", "", ""}},
            {"in:reset", {"Volta ao segmento 0.", "", ""}},
            {"in:rate_mod", {"CV somada a RATE.", "", ""}},
            {"out:out", {"A função — o contorno dos N segmentos.", "", ""}},
            {"out:eoc", {"Pulso no fim da volta (end-of-cycle) — encadeia "
                        "com o RESET de outra instância.", "", ""}},
            {"out:step", {"Pulso em cada fronteira de segmento — STEP → "
                         "ENVELOPE.gate encadeia um AD por segmento.", "",
                         ""}},
        }},
        {"FUNCTION", {
            {"rate", {
                "Velocidade da rampa — de LFO (0,01 Hz) a oscilador de "
                "áudio (12 kHz).",
                "Mesma matemática em qualquer taxa: envelope, LFO e "
                "oscilador são a mesma função, só muda a velocidade.",
                "Suba RATE aos poucos num patch cabeado como envelope — em "
                "algum ponto a rampa vira tom audível."}},
            {"slope", {
                "Forma da rampa: 0 dente-de-serra descendente, 0,5 "
                "triângulo, 1 dente-de-serra ascendente.", "", ""}},
            {"drift", {
                "Passeio lento e correlacionado na taxa efetiva — o tempo "
                "\"respira\" sem nenhuma entrada.",
                "drift=0 é determinístico.", ""}},
            {"sync_enable", {"Liga a resposta ao jack SYNC (reinicia a "
                            "fase).", "", ""}},
            {"in:rate_mod", {"CV 1 V/oct que multiplica RATE.", "", ""}},
            {"in:slope_mod", {"CV que soma a SLOPE.", "", ""}},
            {"in:sync", {"Trigger de hard sync (precisa de SYNC ligado).",
                        "", ""}},
            {"out:uni", {
                "Saída unipolar (0..1) — boa pra envelope/LFO de "
                "amplitude.", "", ""}},
            {"out:bi", {
                "Saída bipolar (−1..1) — boa pra CV de pitch/FM.", "", ""}},
        }},
        {"HARMONY", {
            {"movement", {
                "Técnica de movimento harmônico: Coltrane / substituição "
                "tritônica+ii-V / mediante cromática / intercâmbio modal / "
                "jazz modal / backdoor ii-V.",
                "Cada técnica tem seu próprio padrão de intervalo de raiz "
                "— Coltrane sempre +4 st (ciclo de 3), backdoor sempre +2 "
                "st, jazz modal quase sempre parado.",
                "Compare MOVE=Coltrane (anda rápido, cíclico) com "
                "MOVE=jazz modal (fica parado, dá um passo raro)."}},
            {"rate", {"Relógio interno, usado só se ADV estiver livre.",
                     "", ""}},
            {"root_start", {
                "Nota fundamental no reset/início (0=C ... 11=B).", "", ""}},
            {"scale_lo", {"Menor índice de escala sorteável nas trocas.",
                         "", ""}},
            {"scale_hi", {"Maior índice de escala sorteável nas trocas.",
                         "", ""}},
            {"hold", {
                "Probabilidade de PULAR uma troca agendada, mantendo o "
                "centro tonal atual.", "", ""}},
            {"in:advance", {
                "Trigger — avança pro próximo centro tonal na borda de "
                "subida; presente, substitui RATE.", "", ""}},
            {"in:reset", {"Volta pro ROOT/escala inicial.", "", ""}},
            {"out:root", {
                "Nova fundamental, como CV (semitom/12) — ligue em "
                "QUANTIZER.root.", "", ""}},
            {"out:scale", {
                "Novo índice de escala, como CV (índice/11) — ligue em "
                "QUANTIZER.scale.", "", ""}},
            {"out:change", {
                "Pulso curto (~20 ms) toda vez que ROOT ou SCALE mudam de "
                "verdade.", "", ""}},
        }},
        {"LOGIC", {
            {"rate", {
                "Relógio interno, usado só quando CLK está livre — LOGIC "
                "sozinho vira gerador de ritmo.", "", ""}},
            {"divide", {
                "Contador módulo-N sobre as bordas de CLK — DIV pulsa 1 "
                "vez a cada N.", "", ""}},
            {"multiply", {
                "Mede o período entre bordas de CLK e agenda sub-tiques "
                "dentro dele — multiplica a densidade sem precisar de "
                "clock mais rápido de fora.",
                "Extrapola o período anterior um pouco à frente (estilo "
                "Pamela's) — o primeiro ciclo depois de ligar ainda não "
                "tem período medido.", ""}},
            {"gate_len", {"Duty do pulso DIV.", "", ""}},
            {"delay", {"Atraso do pulso DIV (0-200 ms), num anel de "
                      "amostras.", "", ""}},
            {"in:clock", {"Clock externo — se conectado, substitui o "
                         "relógio interno.", "", ""}},
            {"in:a", {"Entrada A da lógica AND/OR/XOR e do flip-flop.",
                     "", ""}},
            {"in:b", {"Entrada B da lógica AND/OR/XOR.", "", ""}},
            {"in:reset", {"Zera o contador de divisão e o flip-flop.",
                         "", ""}},
            {"out:div", {"Pulso do divisor/multiplicador, com DELAY e GATE "
                        "aplicados.", "", ""}},
            {"out:and", {"A AND B, nível a nível.", "", ""}},
            {"out:or", {"A OR B.", "", ""}},
            {"out:xor", {"A XOR B.", "", ""}},
            {"out:flip", {
                "Flip-flop tipo T — alterna estado a cada borda de subida "
                "de A.", "", ""}},
        }},
        {"LPG", {
            {"mode", {
                "Crossfade filtro↔VCA — 0 só filtro, 1 só VCA, 0,5 os dois "
                "totalmente ativos (o LPG clássico, fecha de vez quando o "
                "envelope cai a 0).", "", ""}},
            {"response", {
                "Tempo da cauda pós-golpe (~30 ms a 2,5 s) — a \"memória\" "
                "do LDR simulado.",
                "Sobe rápido (~2 ms) sempre; desce devagar e freia perto "
                "de 0 — resposta assimétrica de vactrol, não um envelope "
                "simétrico comum.", ""}},
            {"offset", {
                "Abertura de repouso — quanto o LPG fica aberto mesmo sem "
                "golpe.", "", ""}},
            {"resonance", {"Ressonância do filtro de 2 polos.", "", ""}},
            {"bounce", {
                "Overshoot pós-golpe — uma senoide amortecida soma ao "
                "envelope logo depois do STRIKE, simulando o \"repique\" "
                "do vactrol.", "", ""}},
            {"drift", {"Passeio lento e correlacionado na cauda de "
                      "resposta.", "", ""}},
            {"in:in", {"Entrada de áudio a golpear.", "", ""}},
            {"in:strike", {
                "Trigger — abre o envelope na borda de subida, o \"golpe\".",
                "", ""}},
            {"in:cv", {"CV que soma ao STRIKE — abre o LPG por fora, sem "
                      "trigger.", "", ""}},
            {"out:out", {
                "Áudio × envelope de vactrol — o timbre plucky da costa "
                "oeste.", "", ""}},
        }},
        {"MATRIX", {
            {"g11", {"Ganho de IN1 → OUT1 (diagonal — padrão 1, "
                    "passa-direto).",
                    "Cada célula é um atenuversor (−1..1); negativo "
                    "inverte a fase. Duas células não-zero na mesma "
                    "COLUNA (mesmo destino) somam — ou, com RING alto, "
                    "multiplicam (ring-mod de 4 quadrantes).", ""}},
            {"g12", {"Ganho de IN1 → OUT2.", "", ""}},
            {"g13", {"Ganho de IN1 → OUT3.", "", ""}},
            {"g14", {"Ganho de IN1 → OUT4.", "", ""}},
            {"g21", {"Ganho de IN2 → OUT1.", "", ""}},
            {"g22", {"Ganho de IN2 → OUT2 (diagonal — padrão 1).", "", ""}},
            {"g23", {"Ganho de IN2 → OUT3.", "", ""}},
            {"g24", {"Ganho de IN2 → OUT4.", "", ""}},
            {"g31", {"Ganho de IN3 → OUT1.", "", ""}},
            {"g32", {"Ganho de IN3 → OUT2.", "", ""}},
            {"g33", {"Ganho de IN3 → OUT3 (diagonal — padrão 1).", "", ""}},
            {"g34", {"Ganho de IN3 → OUT4.", "", ""}},
            {"g41", {"Ganho de IN4 → OUT1.", "", ""}},
            {"g42", {"Ganho de IN4 → OUT2.", "", ""}},
            {"g43", {"Ganho de IN4 → OUT3.", "", ""}},
            {"g44", {"Ganho de IN4 → OUT4 (diagonal — padrão 1).", "", ""}},
            {"level", {"Ganho geral de saída, antes do SAT.", "", ""}},
            {"norm", {
                "Normalização por coluna — mantém o nível de saída "
                "constante mesmo com vários ganhos altos somados no mesmo "
                "destino.", "", ""}},
            {"ring", {
                "Cruza cada coluna entre soma linear (0) e ring-mod de 4 "
                "quadrantes (1) — duas entradas não-zero no mesmo destino "
                "passam a multiplicar em vez de somar.", "", ""}},
            {"sat", {"Satura suavemente a matriz — segura ela mesmo em "
                    "laço de feedback.", "", ""}},
            {"drift", {
                "Os 16 ganhos respiram lentamente (soma de senos, sem "
                "RNG) — a matriz nunca fica 100% estática.", "", ""}},
            {"in:in1", {"Entrada 1 da matriz.", "", ""}},
            {"in:in2", {"Entrada 2 da matriz.", "", ""}},
            {"in:in3", {"Entrada 3 da matriz.", "", ""}},
            {"in:in4", {"Entrada 4 da matriz.", "", ""}},
            {"out:out1", {"Soma ponderada da coluna 1 (todas as entradas × "
                         "seus ganhos g_1).", "", ""}},
            {"out:out2", {"Soma ponderada da coluna 2.", "", ""}},
            {"out:out3", {"Soma ponderada da coluna 3.", "", ""}},
            {"out:out4", {"Soma ponderada da coluna 4.", "", ""}},
        }},
        {"PLANAR", {
            {"x", {
                "Posição horizontal do ponto no quadrado (0 = lado A/C, "
                "1 = lado B/D). Soma com a CV de X.",
                "Com um gesto tocando, o knob vira nudge bipolar: 0,5 = "
                "sem desvio, <0,5 empurra pra esquerda.", ""}},
            {"y", {
                "Posição vertical (0 = lado A/B de cima, 1 = lado C/D de "
                "baixo). Soma com a CV de Y.", "", ""}},
            {"curve", {
                "Linear (0) ↔ potência constante (1). Linear: os pesos "
                "somam 1 — o morph honesto pra CV. Potência constante: "
                "escala os pesos por 1/√Σw² pra a saída de ÁUDIO não "
                "afundar ~6 dB no centro do quadrado.", "", ""}},
            {"smooth", {
                "Glide de 1 polo no ponto — τ de ~1 ms a ~0,5 s. "
                "De resposta imediata a slew que arrasta a trajetória.",
                "", ""}},
            {"rate", {
                "Velocidade do loop do gesto E da deriva. 0,5 = 1×; "
                "2^((rate−0,5)·4), então 1/16× a 16×.", "", ""}},
            {"drift", {
                "Passeio 2D autônomo do ponto quando não há gesto — "
                "Lissajous de três senos lentos incomensuráveis. "
                "Determinístico, sem RNG. 0 = ponto parado.", "", ""}},
            {"in:a", {"Fonte do canto superior-esquerdo.", "", ""}},
            {"in:b", {"Fonte do canto superior-direito.", "", ""}},
            {"in:c", {"Fonte do canto inferior-esquerdo.", "", ""}},
            {"in:d", {"Fonte do canto inferior-direito.", "", ""}},
            {"in:x", {"CV somada em X (LFO, envelope, outro x_out…).", "", ""}},
            {"in:y", {"CV somada em Y.", "", ""}},
            {"in:gesture", {
                "Gate: enquanto alto, GRAVA a trajetória do ponto "
                "(decimada 32×, até ~4 s).",
                "Na descida, se gravou o bastante, o gesto passa a tocar "
                "em loop. Toque curto (<~2 ms) = limpa, volta ao ao vivo.",
                ""}},
            {"out:out", {
                "A mistura bilinear das 4 fontes, com softclip de "
                "segurança.", "", ""}},
            {"out:x_out", {
                "A posição X efetiva (já suavizada) como CV — cabeie no "
                "cutoff de um FILTER, no pos de um WAVETABLE… o gesto "
                "dirige o patch.", "", ""}},
            {"out:y_out", {"A posição Y efetiva como CV.", "", ""}},
        }},
        {"DRUM", {
            {"tone", {
                "Altura do corpo (20–1000 Hz). Grave = bumbo, médio = "
                "tom/caixa, agudo = clave. 1 V/oct pela entrada PIT.",
                "O corpo é uma senóide com envelope de altura — no golpe "
                "a frequência salta e cai de volta a TONE (o 'pow' do "
                "808).", ""}},
            {"bend", {
                "Profundidade do envelope de altura. 0 = tonal (a "
                "frequência fica em TONE); 1 = varredura de bumbo (salta "
                "6× e desce).",
                "É o pitch-sweep clássico da voz de bumbo — o que dá o "
                "transiente com peso.", ""}},
            {"decay", {
                "Tempo de decaimento geral (~20 ms a ~2 s). Curto = "
                "click/laser; longo = sub que sustenta.", "", ""}},
            {"snap", {
                "Dose da rajada de ruído de ataque — o 'estalo' da caixa, "
                "o chiado do chimbal, o click do bumbo.",
                "Ruído branco por um passa-alta cujo corte sobe com MAP "
                "(808 surdo → acústico brilhante), com envelope próprio "
                "bem curto.", ""}},
            {"map", {
                "Caráter 808 → 909 → acústico. Sobe: o corpo ganha clique "
                "(tanh), o ruído fica mais agudo, entra um pouco de "
                "drive.", "", ""}},
            {"drive", {"Saturação de saída (tanh + makeup) — o crunch do "
                      "909.", "", ""}},
            {"roll", {
                "Auto-disparo interno. 0 = só o gate externo; acima disso "
                "a voz se retriga a ~2–40 Hz — rufo, buzz, e o modo que "
                "toca sozinho.", "", ""}},
            {"drift", {
                "Humanização: cada golpe varia levemente altura/decay/"
                "nível. De um xorshift semeado avançado NO disparo — "
                "mesmos gates → mesmo áudio. 0 = golpes idênticos.",
                "", ""}},
            {"in:gate", {"Trigger — um pulso, um golpe.", "", ""}},
            {"in:accent", {
                "CV de acento: escala o nível (e o brilho) do golpe. "
                "Cabeie TRIGSEQ.accent aqui.", "", ""}},
            {"in:tone", {"CV 1 V/oct somada sobre o knob TONE (toms "
                        "afinados de um SEQUENCE).", "", ""}},
            {"out:out", {"A voz — corpo + estalo, saturada por DRIVE.",
                        "", ""}},
        }},
        {"MATTER", {
            {"freq", {"Frequência fundamental do banco de modos.", "", ""}},
            {"structure", {
                "Corda (0, modos harmônicos) → sino/metal (1, modos "
                "esticados/inarmônicos).", "", ""}},
            {"brightness", {
                "Quantos modos agudos soam — rolloff da amplitude por "
                "modo.", "", ""}},
            {"damping", {
                "Quanto tempo os modos continuam soando depois de "
                "excitados.", "", ""}},
            {"position", {
                "Onde o objeto é excitado — decide QUAIS modos recebem "
                "energia (tocar no nó de um harmônico não o excita).", "",
                "Varra POS devagar com um golpe constante: alguns modos "
                "somem, outros aparecem — é a física real de tocar num "
                "ponto diferente."}},
            {"exciter", {
                "Quanto ruído entra na rajada do golpe interno.", "", ""}},
            {"mix", {"Seco (excitação crua) ↔ molhado (ressonância dos "
                    "modos).", "", ""}},
            {"in:in", {"Entrada de áudio pra excitar os modos (clique, "
                      "ruído, voz).", "", ""}},
            {"in:strike", {"Trigger — dispara um golpe interno (rajada de "
                          "ruído curta).", "", ""}},
            {"in:freq_mod", {"CV 1 V/oct que multiplica FREQ.", "", ""}},
            {"in:struct_mod", {"CV que soma a STRC.", "", ""}},
            {"out:out", {
                "Mistura entre a excitação crua e a ressonância dos 24 "
                "modos.", "", ""}},
        }},
        {"MEMORY", {
            {"grain", {"Duração de cada grão.", "", ""}},
            {"density", {"Quantos grãos novos nascem por segundo.", "", ""}},
            {"position", {
                "Onde no buffer (0=mais recente, até 3 s atrás; 1=mais "
                "antigo) os grãos começam a ler.", "", ""}},
            {"spray", {
                "Espalha a posição de início de cada grão — de leitura "
                "exata a nuvem borrada no tempo.", "", ""}},
            {"pitch", {
                "Transposição da leitura dos grãos, em semitons (±24).",
                "", ""}},
            {"feedback", {
                "Realimenta a saída de volta pro buffer antes de gravar — "
                "o material se acumula/degenera.", "", ""}},
            {"blend", {"Seco (entrada crua) ↔ molhado (nuvem de grãos).",
                     "", ""}},
            {"freeze", {
                "Para a gravação — o buffer vira uma textura fixa, só os "
                "grãos continuam relendo.", "",
                "Ligue FREEZE no meio de uma frase: o material capturado "
                "vira um loop granular estático, sem precisar segurar "
                "gate nenhum."}},
            {"in:in", {
                "Entrada de áudio, gravada continuamente no buffer "
                "(exceto com FREEZE).", "", ""}},
            {"in:position_mod", {"CV que soma a POS.", "", ""}},
            {"in:pitch_mod", {"CV 1 V/oct que soma a PITCH (em oitavas).",
                             "", ""}},
            {"in:freeze_gate", {
                "Gate — nível alto congela o buffer, igual ao toggle "
                "HOLD.", "", ""}},
            {"out:out", {"Mistura entre entrada crua e a nuvem de grãos.",
                       "", ""}},
        }},
        {"MULT", {
            {"dual", {
                "Modo 1→4 (desligado) ou dois múltiplos de 1→2 "
                "independentes (ligado, A-180-2): out1/2 seguem IN, "
                "out3/4 seguem IN2.", "", ""}},
            {"scale1", {"Atenuversor da saída 1 (negativo inverte).", "", ""}},
            {"scale2", {"Atenuversor da saída 2.", "", ""}},
            {"scale3", {"Atenuversor da saída 3 (em DUAL, segue IN2).", "", ""}},
            {"scale4", {"Atenuversor da saída 4 (em DUAL, segue IN2).", "", ""}},
            {"offset1", {
                "Constante somada na saída 1 — sem IN conectado, vira "
                "fonte de tensão manual.", "", ""}},
            {"offset2", {"Constante somada na saída 2.", "", ""}},
            {"offset3", {"Constante somada na saída 3.", "", ""}},
            {"offset4", {"Constante somada na saída 4.", "", ""}},
            {"slew", {"Suaviza as 4 saídas (compartilhado).", "", ""}},
            {"in:in", {
                "Entrada — alimenta as 4 saídas (ou só out1/2 em modo "
                "DUAL).", "", ""}},
            {"in:in2", {
                "2ª entrada — só usada em modo DUAL, alimenta out3/4.",
                "", ""}},
            {"out:out1", {"Saída 1: SCALE1·entrada + OFFSET1.", "", ""}},
            {"out:out2", {"Saída 2: SCALE2·entrada + OFFSET2.", "", ""}},
            {"out:out3", {"Saída 3: SCALE3·entrada + OFFSET3.", "", ""}},
            {"out:out4", {"Saída 4: SCALE4·entrada + OFFSET4.", "", ""}},
        }},
        {"GLIDE", {
            {"time", {
                "Tempo do escorregão na SUBIDA — para uma mudança de 1,0 "
                "(uma oitava). 0 = salto seco.",
                "É a base; a descida sai daqui multiplicada por FALL.", ""}},
            {"fall", {
                "Assimetria: tempo de descida = TIME·6^FALL. −1 = descida "
                "6× mais rápida que a subida; +1 = 6× mais lenta; 0 = "
                "simétrico.", "", ""}},
            {"curve", {
                "Formato do escorregão: 0 = linear (velocidade constante, "
                "chega no tempo exato — MS-20/Minimoog); 1 = exponencial "
                "(RC, arrasta na chegada).",
                "Mesma inclinação no começo; a exponencial desacelera e "
                "só encosta no alvo depois de ~3 TIME.", ""}},
            {"mode", {
                "0 = sempre desliza (portamento clássico); 1 = só enquanto "
                "SLIDE está alto (o slide do TB-303, cada passo decide); "
                "2 = legato (desliza só se GATE segue alto na troca; um "
                "gate novo salta).", "", ""}},
            {"in:pitch", {
                "CV de nota a conduzir (1 V/oct ou qualquer). Sem cabo, a "
                "saída congela no último valor.", "", ""}},
            {"in:slide", {
                "Gate que habilita o escorregão no MODE 1 — venha de uma "
                "linha do TRIGSEQ/TURING pra ter slide generativo.", "", ""}},
            {"in:gate", {
                "Gate da nota, pro MODE 2 (legato): borda de subida = "
                "ataque destacado (salta); sustentado = desliza.", "", ""}},
            {"out:out", {"A CV de nota, agora conduzida.", "", ""}},
            {"out:moving", {"Alto enquanto está escorregando.", "", ""}},
            {"out:done", {
                "Pulso de ~2 ms quando chega ao alvo — pra um acento, "
                "ratchet ou mudança de timbre reagir a \"chegou\".", "", ""}},
        }},
        {"NOTE-OUT", {
            {"in:gate", {
                "Gate/trigger da voz a capturar — a borda de descida "
                "fecha a nota completa.", "", ""}},
            {"in:pitch", {
                "CV 1 V/oct da voz — capturado no instante em que o gate "
                "liga.", "", ""}},
            {"in:velocity", {
                "Velocidade (0-1) — capturada no instante em que o gate "
                "liga; sem cabo, 1,0.", "", ""}},
            {"in:accent", {
                "Acento — capturado no instante em que o gate liga.", "", ""}},
            {"out:gate_thru", {
                "Cópia exata de GATE — precisa encadear aqui pro NOTE-OUT "
                "ficar no caminho de verdade do patch.",
                "O motor só processa módulos que chegam ao sink ativo "
                "(órfãos não custam DSP) — sem esse encadeamento, o "
                "NOTE-OUT nunca roda.",
                "Cabeie ALGO.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → "
                "ENVELOPE.gate: a nota continua soando igual, e agora "
                "fica registrada na MUSICAL SCORE."}},
            {"out:pitch_thru", {
                "Cópia exata de PITCH — mesmo motivo do GTHR.", "", ""}},
        }},
        {"PARAMETRIC", {
            {"type1", {
                "Tipo do estágio 1: desligado / corte grave / prateleira "
                "grave / realce (peak) / prateleira aguda / corte agudo.",
                "", ""}},
            {"type2", {"Tipo do estágio 2 (mesmas opções do TYPE1).", "", ""}},
            {"type3", {"Tipo do estágio 3.", "", ""}},
            {"type4", {"Tipo do estágio 4.", "", ""}},
            {"freq1", {"Frequência central do estágio 1.", "", ""}},
            {"freq2", {"Frequência central do estágio 2.", "", ""}},
            {"freq3", {"Frequência central do estágio 3.", "", ""}},
            {"freq4", {"Frequência central do estágio 4.", "", ""}},
            {"gain1", {
                "Ganho do estágio 1 — só afeta prateleiras e realce, "
                "cortes não têm ganho.", "", ""}},
            {"gain2", {"Ganho do estágio 2.", "", ""}},
            {"gain3", {"Ganho do estágio 3.", "", ""}},
            {"gain4", {"Ganho do estágio 4.", "", ""}},
            {"q1", {
                "Q do estágio 1 — controla a largura no realce, "
                "reinterpretado como \"slope S\" nas prateleiras (fórmula "
                "RBJ).", "", ""}},
            {"q2", {"Q do estágio 2.", "", ""}},
            {"q3", {"Q do estágio 3.", "", ""}},
            {"q4", {"Q do estágio 4.", "", ""}},
            {"slope1", {
                "Inclinação do corte do estágio 1: 12/24/48 dB/oitava "
                "(1/2/4 biquads em cascata; só afeta os tipos de corte).",
                "", ""}},
            {"slope2", {"Inclinação do corte do estágio 2.", "", ""}},
            {"slope3", {"Inclinação do corte do estágio 3.", "", ""}},
            {"slope4", {"Inclinação do corte do estágio 4.", "", ""}},
            {"output", {"Ganho de saída, depois dos 4 estágios.", "", ""}},
            {"drive", {
                "Satura suavemente a saída, antes do soft-clip de "
                "segurança.", "", ""}},
            {"mix", {"Seco ↔ processado.", "", ""}},
            {"in:in", {"Entrada de áudio.", "", ""}},
            {"in:sweep", {
                "CV 1 V/oct que desloca os 4 estágios JUNTOS, como grupo.",
                "A relação entre as bandas como processo (não um estágio "
                "isolado) — module SWEEP com um LFO lento e o EQ inteiro "
                "\"varre\" o espectro mantendo a forma.", ""}},
            {"in:amount", {"CV que escala os 4 ganhos ao mesmo tempo.",
                          "", ""}},
            {"out:out", {
                "Saída dos 4 estágios em série, com DRIVE/MIX/OUTPUT "
                "aplicados.", "", ""}},
        }},
        {"PLL", {
            {"freq", {
                "Frequência livre — usada quando REF não está conectado, "
                "e como ponto de partida quando está.", "", ""}},
            {"fine", {"Afinação fina em cents.", "", ""}},
            {"shape", {
                "Morph contínuo: seno → triângulo → serra → quadrada.",
                "", ""}},
            {"ratio", {
                "Multiplica a fase da referência antes de comparar — o "
                "PLL trava em sub/super-harmônicos de REF, não só na "
                "mesma nota.",
                "RATIO baixo trava num sub-múltiplo bem lento da "
                "referência — quase um divisor de clock via malha de "
                "fase.",
                "Ligue REF num OSC.saw grave e varra RATIO: o PLL salta "
                "entre travar na oitava, na quinta, em divisores bem "
                "fundos."}},
            {"lock_gain", {
                "Quão forte o PLL persegue a referência — baixo \"caça\" "
                "devagar, alto trava rápido (mas mais instável se a razão "
                "não bate exatamente).", "", ""}},
            {"fm_amount", {"Quantidade de FM linear pela entrada FM.", "", ""}},
            {"feedback_amount", {
                "Quanto da rede de feedback (FTYP) entra na fase lida.",
                "A rede modula a FASE, não a frequência, e só na LEITURA "
                "da forma — nunca escreve de volta no acumulador de fase "
                "(por isso não desafina o PLL, só cria textura).", ""}},
            {"feedback_type", {
                "Tipo de feedback: direto / retificado / capacitivo "
                "(memória lenta) / pulso / \"transistor\" (tanh "
                "assimétrico) / refluxo.", "", ""}},
            {"in:pitch", {"CV 1 V/oct — multiplica FREQ.", "", ""}},
            {"in:fm", {"Entrada de FM linear.", "", ""}},
            {"in:ref", {
                "Referência externa (serra bipolar, ex.: OSC.saw) — "
                "presente, o PLL persegue a fase dela em vez de tocar "
                "livre.", "", ""}},
            {"out:out", {
                "A forma de onda (SHAPE), na frequência corrigida pela "
                "perseguição de fase.", "", ""}},
            {"out:ring", {
                "out × ref — o mesmo heterodino clássico entre dois "
                "osciladores.", "", ""}},
            {"out:lock", {
                "0..1 — quão perto o detector de fase está de zero (1 = "
                "travado).",
                "",
                "Ligue LOCK num VCA: o volume sobe sozinho quando o PLL "
                "trava na referência, e cai quando está \"caçando\"."}},
        }},
        {"QUANTIZER", {
            {"scale", {
                "Escala musical (12 tabelas curadas — cromática, maior, "
                "modos, pentatônicas, tons inteiros, oitava).", "", ""}},
            {"root", {"Tônica da escala (0=C ... 11=B).", "", ""}},
            {"range", {
                "Quantas oitavas a CV de entrada cobre antes de "
                "quantizar.", "", ""}},
            {"glide", {
                "Tempo de portamento entre uma nota quantizada e a "
                "próxima.", "", ""}},
            {"hysteresis", {
                "Zona-morta antes de trocar de grau — evita tremular "
                "entre dois graus vizinhos com CV instável.", "", ""}},
            {"in:cv", {"CV contínua a quantizar.", "", ""}},
            {"in:transpose", {
                "CV 1 V/oct que soma antes de quantizar — transpõe a "
                "escala inteira.", "", ""}},
            {"in:trigger", {
                "Sample & hold — presente, a nota só atualiza no pulso "
                "(assim TURING/DECISION viram melodia).", "", ""}},
            {"out:pitch", {
                "CV quantizada, em 1 V/oct — pronta pra alimentar "
                "OSC.pitch.", "", ""}},
            {"out:gate", {
                "Pulso curto (~5 ms) toda vez que a nota realmente muda.",
                "", ""}},
            {"out:semitone", {
                "A mesma nota, normalizada pra ±1 em vez de oitavas — "
                "útil como CV de controle.", "", ""}},
        }},
        {"SH", {
            {"rate", {"Relógio interno, usado só se TRIG1/2 estiver livre.",
                     "", ""}},
            {"slew1", {
                "Tempo de deslize do canal 1 até o valor amostrado — 0 é "
                "degrau instantâneo.", "", ""}},
            {"slew2", {"Igual ao SLW1, canal 2.", "", ""}},
            {"slope", {
                "Assimetria do deslize: >0 desliza pra baixo devagar e "
                "sobe rápido (portamento de pluck), <0 o oposto, 0 "
                "simétrico.", "", ""}},
            {"track1", {
                "TRK1 ligado: enquanto o trigger fica alto, segue IN1 ao "
                "vivo (track & hold) em vez de amostrar só na borda.",
                "", ""}},
            {"track2", {"Igual ao TRK1, canal 2.", "", ""}},
            {"spread", {
                "Puxa o sorteio interno de uniforme pra sino — só afeta "
                "os canais sem IN conectado.", "", ""}},
            {"correlation", {
                "Correlação entre os dois acasos internos: −1 espelhados, "
                "0 independentes, 1 gêmeos.",
                "Só afeta os canais usando o sorteio interno (sem IN "
                "conectado) — com IN1/IN2 cabeados, CORR não tem efeito.",
                "Suba CORR pra 1 com os dois canais no interno: os dois "
                "passam a sortear exatamente o mesmo valor a cada pulso."}},
            {"in:in1", {"Fonte do canal 1 — sem cabo, usa o sorteio "
                       "interno.", "", ""}},
            {"in:trig1", {"Trigger do canal 1 — presente, substitui RATE.",
                         "", ""}},
            {"in:in2", {
                "Fonte do canal 2 — sem cabo, usa o sorteio interno "
                "(correlacionável com o canal 1 via CORR).", "", ""}},
            {"in:trig2", {"Trigger do canal 2.", "", ""}},
            {"out:out1", {
                "Valor amostrado do canal 1, com SLW1/SLOPE aplicados.",
                "", ""}},
            {"out:out2", {"Valor amostrado do canal 2.", "", ""}},
        }},
        {"SCOPE", {
            {"trigger", {"Nível de disparo do comparador.", "", ""}},
            {"edge", {
                "Dispara na borda de descida (ligado) ou subida "
                "(desligado).", "", ""}},
            {"reject", {
                "Zona de histerese ao redor do nível de disparo — evita "
                "disparo múltiplo por ruído perto do limiar.", "", ""}},
            {"response", {
                "Velocidade dos seguidores de LVL/BRT/PIT — ataque sempre "
                "bem mais rápido que o release.", "", ""}},
            {"hold", {"Congela LVL/BRT/PIT no valor atual.", "", ""}},
            {"sens", {
                "Sensibilidade do detector de ONSET (ataque): alto = "
                "dispara em qualquer subida de amplitude; baixo = só nos "
                "ataques fortes. Envelope rápido vs lento.", "", ""}},
            {"in:in", {"Sinal a analisar/exibir.", "", ""}},
            {"in:ext", {"Fonte externa de disparo — sem cabo, TRIG usa o "
                       "próprio IN.", "", ""}},
            {"out:thru", {
                "IN copiado sem alteração — é o que o Display do painel "
                "desenha.", "", ""}},
            {"out:trig", {"Pulso quando IN cruza TRIGGER na direção de "
                         "EDGE.", "", ""}},
            {"out:level", {
                "Pico do sinal, como CV — a medição do scope volta pro "
                "patch.", "", ""}},
            {"out:bright", {
                "Centroide espectral (agudo↔grave), como CV — quanto mais "
                "alto, mais brilhante o sinal.",
                "Calculado por um diferenciador simples (energia da "
                "derivada / energia do sinal, Parseval) — sem FFT, "
                "RT-seguro.", ""}},
            {"out:pitch", {
                "Altura detectada em 1 V/oct — autocorrelação (YIN) num "
                "sinal decimado, robusta a harmônicos (serra, acorde). "
                "Faixa ~53–1000 Hz; ruído → 0.",
                "O ZCR anterior reportava a oitava errada em som rico; o "
                "YIN acha o período pela diferença acumulada normalizada.",
                "SCOPE.pitch → OSC.pitch: um segundo oscilador afina pela "
                "altura da entrada."}},
            {"out:onset", {
                "Pulso a cada ATAQUE/transiente do sinal (envelope rápido "
                "> lento × SENS). Um gate de ~2 ms; não dispara em tom "
                "estável.",
                "É o detector de transiente clássico (Maths ch2/3 pra "
                "áudio). Ruído estável não dispara.",
                "SCOPE.onset ← uma linha de bateria → dispara ENVELOPE/LPG "
                "no compasso do áudio; ou re-dispara o SAMPLER."}},
        }},
        {"CRUSH", {
            {"rate", {
                "Taxa do sample-and-hold interno (100 Hz–24 kHz, +CV). "
                "Baixo = escada grossa + aliasing forte (a 'voz de robô' "
                "dos 8-bit) — NÃO há filtro anti-alias, o aliasing É o som.",
                "", "LFO → RATE varre a taxa: o 'engoli-fita ao "
                "contrário'."}},
            {"bits", {
                "Quantiza a amplitude a 2^bits níveis "
                "(round(x·L)/L). bits=1 ≈ onda quadrada; bits=16 ≈ "
                "transparente.", "",
                "ENVELOPE → BITS: a resolução cai na cauda da nota."}},
            {"drive", {
                "Ganho ANTES da quantização — empurra o sinal pro "
                "transbordo (ver WRAP).", "", ""}},
            {"wrap", {
                "Como o sinal que passa de ±1 se comporta: 0 CLIPA "
                "(limita), 1 ENROLA (mod(x+1,2)−1 — o estouro de inteiro, "
                "dente de serra brutal), entre os dois um blend.",
                "Com DRIVE alto + WRAP=1 o sinal enrola várias vezes → "
                "harmônicos até Nyquist, aliasa (aceito — é o efeito).",
                ""}},
            {"glitch", {
                "Probabilidade, por amostra-de-hold, de uma FALHA: "
                "amostra travada (fica no valor anterior), dropout "
                "(silêncio) ou repique (o anterior amplificado). Simula "
                "uma conexão digital ruim / CD arranhado.",
                "Semeado (xorshift) — o mesmo patch soa igual duas vezes. "
                "Dano REPRODUTÍVEL, ao contrário de um bug.", ""}},
            {"jitter", {
                "Instabilidade da taxa do S&H — o clock treme → "
                "instabilidade de afinação (o 'wow' digital). Semeado.",
                "", ""}},
            {"tone", {
                "Filtro de 1 polo na saída (o filtro de reconstrução, ou "
                "a falta dele): <0 passa-baixa (dócil), >0 passa-alta "
                "(só o lixo agudo), 0 neutro.", "", ""}},
            {"mix", {"Seco ↔ destruído. 0 = passa-direto (bypass exato).",
                     "", ""}},
            {"in:in", {
                "Áudio a destruir. Desconectado + MIX>0 → o S&H amostra o "
                "próprio ruído branco → uma fonte de ruído lo-fi/glitch.",
                "", ""}},
            {"in:rate_mod", {"CV somada a RATE.", "", ""}},
            {"in:mix_mod", {"CV somada a MIX (o dano entra por dose, via "
                           "um ENVELOPE).", "", ""}},
            {"out:out", {"O sinal destruído, misturado com o seco por "
                        "MIX. Clamp de segurança a ±4.", "", ""}},
        }},
        {"SHIFTER", {
            {"shift", {
                "Move o espectro INTEIRO por tantos HERTZ (−2000..2000, "
                "+CV em SFT: 1 unidade = 1000 Hz). 0 = passa-direto. "
                "Como o deslocamento é em Hz e não em razão, os parciais "
                "deixam de ser harmônicos → metálico, sineiro.",
                "Deslocador de frequência (SSB — single sideband): "
                "diferente do ring-mod do SHAPE, que dá as DUAS bandas "
                "(soma e diferença) simétricas. Aqui `up` = espectro + "
                "shift, `down` = espectro − shift, cada um em sua saída.",
                "Δf pequeno (5–20 Hz) = um 'fase' que nunca fecha (as duas "
                "cópias batem devagar). Δf grande = clangor inarmônico."}},
            {"feedback", {
                "Parte da saída `up` volta pra entrada → o espectro sobe "
                "de novo, e de novo: glissando infinito (o 'barber pole' / "
                "shimmer de Shepard). `feedback` negativo puxa de `down` "
                "(desce sem parar). `tanh` no laço — nunca explode.", "",
                "SHIFT ~8 Hz + FEEDBACK ~0,8 = a corda de Risset que sobe "
                "eternamente."}},
            {"tone", {
                "Inclina o molhado (1 polo): <0 abafa o agudo do sinal "
                "deslocado, >0 realça. 0 = neutro.", "", ""}},
            {"drift", {
                "Wobble lento e SEMEADO no Δf (±drift·50 %) — o "
                "deslocamento 'respira', nunca fixo. Determinístico. "
                "0 = estático.", "", ""}},
            {"mix", {"Seco (IN) ↔ deslocado, nas DUAS saídas. 0 = bypass.",
                    "", ""}},
            {"in:in", {"O som a deslocar — uma voz, um acorde, um pad.",
                      "", ""}},
            {"in:shift_mod", {"CV somada em SHIFT, em Hz (1 unidade = "
                             "1000 Hz). LFO → um vibrato de espectro.",
                             "", ""}},
            {"out:up", {"Espectro + SHIFT (a banda lateral superior).",
                       "", ""}},
            {"out:down", {"Espectro − SHIFT (a banda inferior). Cabeie as "
                         "duas em destinos diferentes — a relação entre "
                         "elas é o processo.", "", ""}},
        }},
        {"SHAPE", {
            {"ring", {"Mistura IN com IN×MOD (ring-mod de 4 quadrantes).",
                     "", ""}},
            {"fold", {
                "Quantidade de dobra — waveshaper que reflete o sinal em "
                "vez de cortar (wavefolder).", "", ""}},
            {"symmetry", {
                "Desloca o centro da dobra — soma harmônicos pares "
                "(\"buzz\") em vez de só ímpares.", "", ""}},
            {"wrap", {
                "Mistura a dobra triangular (reflete) com wrap-around "
                "seco (corta e reentra do outro lado).", "", ""}},
            {"sat", {"Saturação (tanh) depois da dobra.", "", ""}},
            {"level", {"Ganho de saída (VCA embutido).", "", ""}},
            {"drift", {"Passeio lento e correlacionado na quantidade de "
                      "dobra.", "", ""}},
            {"in:in", {"Entrada de áudio a moldar.", "", ""}},
            {"in:mod", {"Sinal pro ring-mod (multiplicado por IN, "
                       "misturado via RING).", "", ""}},
            {"in:fold_mod", {"CV que soma a FOLD.", "", ""}},
            {"out:out", {
                "Áudio processado pela cadeia ring→fold→wrap→sat→level.",
                "", ""}},
        }},
        {"SPACE", {
            {"time", {"Tempo base do atraso.", "", ""}},
            {"taps", {
                "Quantas tomadas somam na saída molhada — mais tomadas é "
                "mais denso.", "", ""}},
            {"spread", {
                "Espalha os tempos das tomadas entre si — 0 todas juntas "
                "(eco simples), 1 bem distribuídas (textura rítmica).",
                "", ""}},
            {"feedback", {
                "Quanto da saída volta pra entrada do atraso — mais "
                "repetições, cauda mais longa.", "", ""}},
            {"diffusion", {
                "Mistura a saída molhada crua com a mesma saída passada "
                "por uma cadeia de all-pass — de eco discreto a cauda de "
                "reverb espalhada.", "", ""}},
            {"tone", {
                "Brilho da realimentação — baixo escurece a cauda a cada "
                "repetição (mais natural), alto mantém o brilho.", "", ""}},
            {"mod", {
                "Profundidade de um LFO lento no tempo de leitura — "
                "chorus/ensemble nos ecos.", "", ""}},
            {"mix", {"Seco ↔ molhado.", "", ""}},
            {"in:in", {"Entrada de áudio.", "", ""}},
            {"in:time_mod", {"CV que soma a TIME.", "", ""}},
            {"in:feedback_mod", {"CV que soma a FBK.", "", ""}},
            {"out:out", {"Seco + molhado, misturados por MIX.", "", ""}},
            {"out:wet", {
                "Só a parte molhada (tomadas + difusão), sem o seco — "
                "útil pra rotear o espaço separado do sinal direto.",
                "", ""}},
        }},
        {"HALL", {
            {"size", {
                "Tamanho do espaço — escala os 8 comprimentos de linha "
                "de ~8 ms (sala) a ~88 ms (hall).",
                "É uma rede de atraso realimentada (FDN): 8 linhas + uma "
                "matriz de Householder (sem perda) as recombina a cada "
                "volta. O SPACE é comb+allpass; isto é a rede.",
                "Cabeie um LFO lento em SIZE: a sala 'respira' e desafina "
                "a cauda (efeito de fita)."}},
            {"decay", {
                "Tempo de cauda (RT60): 0,2 s a 15 s. Perto do máximo a "
                "cauda quase não decai.",
                "O ganho de cada linha sai de RT60: g = 10^(−3·tempo/"
                "RT60). Householder é ortogonal, então g < 1 sempre "
                "decai, nunca explode.", ""}},
            {"damp", {
                "Passa-baixa dentro do laço de cada linha — o agudo decai "
                "antes do grave, como numa sala real. 0 = brilhante, "
                "1 = escuro.", "", ""}},
            {"mod", {
                "Profundidade da modulação do ponto de leitura de cada "
                "linha (LFOs lentos, fases distintas) — chorus na cauda, "
                "quebra o 'apito' metálico. Determinístico, sem RNG.",
                "", ""}},
            {"pre", {
                "Pré-atraso: 0 a ~120 ms antes da rede. Separa o som "
                "direto da reverberação — o que define o tamanho "
                "percebido.", "", ""}},
            {"mix", {"Seco ↔ molhado. 0 = passa-direto.", "", ""}},
            {"in:in", {"Áudio a reverberar (somado mono).", "", ""}},
            {"in:size", {"CV somada em SIZE.", "", ""}},
            {"in:decay", {"CV somada em DECAY.", "", ""}},
            {"in:freeze", {
                "Gate: enquanto alto, a cauda vira infinita (g = 1, a "
                "rede preserva energia) e a entrada nova para de entrar — "
                "como o HOLD do LOOPER, mas pro espaço.", "", ""}},
            {"out:l", {"Canal esquerdo (combinação das linhas pares).",
                      "", ""}},
            {"out:r", {"Canal direito (linhas ímpares) — descorrelacionado "
                      "do L pra a imagem ficar larga.", "", ""}},
        }},
        {"SAMPLER", {
            {"start", {
                "Ponto de partida da reprodução, dentro da fatia "
                "selecionada (0 = começo, 1 = fim).", "", ""}},
            {"speed", {
                "Varispeed bipolar: sinal·2^(|speed|·2) → ±0,25× a ±4×. "
                "Negativo = tocar de trás pra frente.",
                "Sem REPIT, a velocidade e a altura andam juntas (como um "
                "toca-fitas). 2× mais rápido = uma oitava acima.",
                "Cabeie SEQUENCE → PIT e toque a mesma fatia em várias "
                "alturas."}},
            {"slices", {
                "Divide o buffer gravado em N fatias iguais (1–16). O CV "
                "de POS escolhe qual disparar.",
                "É o 'chop' do MPC — grava um compasso, corta em 8, "
                "dispara as fatias fora de ordem por um TRIGSEQ.", ""}},
            {"repitch", {
                "0 = a transposição (PIT) muda a velocidade (varispeed). "
                "1 = a velocidade fica solta e a altura vem de um "
                "pitch-shifter (a duração da fatia é preservada).",
                "O pitch-shifter é o do G09.pitchshift.pd do Pure Data, "
                "portado do Navalha 2 (crédito: Glerm Soares / Lúcio "
                "Araújo).", ""}},
            {"wear", {
                "Desgaste por disparo: jitter no ponto de início, redução "
                "de taxa de amostragem e bit-crush que crescem com o "
                "knob. Cada disparo soa um pouco diferente.",
                "Determinístico — um xorshift semeado NO disparo, então a "
                "mesma sequência de triggers dá o mesmo áudio.", ""}},
            {"loop", {
                "One-shot (desligado) ↔ loop da fatia (ligado). O ponto "
                "de loop não tem crossfade ainda — pode estalar em fatia "
                "muito curta.", "", ""}},
            {"in:trig", {"Dispara a fatia atual.", "", ""}},
            {"in:in", {"Áudio a gravar (enquanto REC estiver alto).",
                      "", ""}},
            {"in:rec", {
                "Gate: enquanto alto, grava IN no buffer (até ~8 s). Na "
                "descida, congela o comprimento gravado.",
                "Sem gravar e sem arquivo carregado pelo painel, o "
                "SAMPLER fica em silêncio.", ""}},
            {"in:pos", {"CV (0–1) que escolhe a fatia.", "", ""}},
            {"in:pitch", {"CV 1 V/oct — transposição (via velocidade ou "
                         "pitch-shifter, conforme REPIT).", "", ""}},
            {"out:out", {"A fatia tocada, com de-click adaptativo nas "
                        "bordas.", "", ""}},
        }},
        {"LOOPER", {
            {"time", {
                "Comprimento do atraso — de eco curto a laço de 2 s.",
                "É a distância entre a cabeça de escrita e a de leitura no "
                "buffer circular; muda suavizada pra não estalar.",
                "Com FBK alto, gire TIME devagar: o laço 'estica' e "
                "'encolhe' como fita puxada à mão."}},
            {"feedback", {
                "Quanto da saída volta pro laço — acima de 1 auto-oscila.",
                "O sinal realimentado passa pelo filtro de AGE e por um "
                "tanh que segura o nível antes de somar de volta.",
                "Ponha em ~1.05 sem entrada: o laço se sustenta e o AGE "
                "vira o timbre da cauda."}},
            {"age", {
                "Caráter de fita/BBD: perda de agudo no laço, wow & "
                "flutter, saturação e chiado.",
                "Um passa-baixa dentro da realimentação escurece a cada "
                "volta; dois LFOs lentos (~0,9 e ~6,5 Hz) modulam a "
                "leitura; ruído semeado soma no laço.",
                "Segure com HOLD e suba o AGE: cada repetição fica um "
                "pouco mais escura e trêmula, como cópia de cópia."}},
            {"mix", {"Seco ↔ molhado na saída OUT.", "", ""}},
            {"hold", {
                "Congela o laço: para de escrever e repete a janela atual "
                "pra sempre.",
                "Na borda de subida ancora a janela na posição de escrita "
                "atual; a leitura passa a circular só nesse trecho, sem "
                "realimentação nova.",
                "Toque uma frase, ligue HOLD no fim dela e improvise por "
                "cima — o trecho vira base."}},
            {"reverse", {
                "Lê o laço de trás pra frente, sem clique.",
                "Duas leituras em janela de Hann defasadas meia volta se "
                "cruzam (crossfade), então a virada no fim do buffer não "
                "estala.",
                "Combine REV + HOLD: um trecho fixo tocando ao contrário "
                "em loop."}},
            {"heads", {
                "1 cabeça = eco simples. 2–4 = eco de fita MULTI-CABEÇA "
                "(Space Echo): as cabeças extras leem frações do TIME "
                "(0,75 / 0,5 / 0,25×) e somam — eco denso e rítmico.",
                "Como as cabeças de reprodução do Roland RE-201 antes da "
                "cabeça de apagar; a realimentação regenera todas.",
                "FBK perto de 1 + TIME longo + várias cabeças = "
                "Frippertronics denso."}},
            {"in:in", {"Entrada de áudio.", "", ""}},
            {"in:time", {"CV que soma a TIME (em segundos).", "", ""}},
            {"in:freeze", {"Gate: enquanto alto, equivale a HOLD ligado.",
                          "", ""}},
            {"in:rev", {"Gate: enquanto alto, equivale a REV ligado.",
                       "", ""}},
            {"out:out", {"Seco + molhado, misturados por MIX.", "", ""}},
            {"out:wet", {"Só o laço, sem o seco.", "", ""}},
        }},
        {"TURNTABLE", {
            {"speed", {
                "Velocidade alvo do prato: sinal·2^|speed| → ±0,5× a ±2× "
                "(33⅓ ↔ 45 ↔ lento). Negativo = disco girando pra trás.",
                "É só o ALVO — o prato tem massa e leva tempo pra chegar "
                "lá (ver TORQ).",
                "Negativo + BRK: o disco desacelera de −2× até parar, "
                "'subindo' de afinação no caminho."}},
            {"torque", {
                "Força do motor: quão rápido o prato ATINGE a velocidade. "
                "Baixo = 'wow' longo de partida (~1 s); alto = quase "
                "instantâneo.",
                "Constante de tempo de um sistema de 1ª ordem: "
                "motorCoef = 0,00002 + torque²·0,0022.",
                "TORQ no mínimo: o motor mal puxa — o prato só anda se a "
                "mão (SCR) empurrar. Modo 'vinil sem motor'."}},
            {"friction", {
                "Atrito: quão rápido o prato para no BRK (coasting curto "
                "ou longo) e quanto ele 'volta' sozinho depois de um "
                "scratch.", "", ""}},
            {"grab", {
                "Firmeza da mão: quanto a CV SCR joga o prato. SCR "
                "pequeno = pitch-bend (empurrãozinho pra beatmatch); SCR "
                "grande e oscilando = scratch de verdade.",
                "grip = clamp01(|scratch|·5)·grab; a mão puxa a "
                "velocidade pra scratch·4 com acoplamento ·0,3.",
                "A mão age mesmo com o motor parado — LFO → SCR faz o "
                "disco scratchear no compasso sem nenhum trig."}},
            {"start", {
                "Onde a agulha cai (posição no disco) na borda de "
                "subida do TRIG.", "", ""}},
            {"wear", {
                "Desgaste do vinil: estalos/clicks que crescem + leve "
                "instabilidade de rotação + queda mínima de nível.",
                "Determinístico — xorshift semeado; wear=0 zera o termo. "
                "Dois renders com os mesmos parâmetros são "
                "byte-idênticos.", ""}},
            {"loop", {
                "One-shot (o disco 'acaba' e trava na última amostra) ↔ "
                "groove travado (a leitura dá a volta no buffer).", "", ""}},
            {"in:trig", {
                "Põe a agulha: readPos = START·comprimento, rampa de "
                "~5 ms (sem clique) e LIGA o motor. Depois do 1º trig o "
                "motor fica ligado.", "", ""}},
            {"in:in", {"Áudio a gravar (enquanto REC estiver alto).",
                      "", ""}},
            {"in:rec", {
                "Gate: enquanto alto, grava IN no buffer (até ~8 s). Na "
                "descida, congela o comprimento — não dá pra tocar e "
                "gravar ao mesmo tempo.",
                "Sem gravar e sem arquivo carregado pelo painel, o "
                "TURNTABLE fica em silêncio.", ""}},
            {"in:scratch", {
                "A mão, como CV bipolar. Um LFO e uma mão de DJ são "
                "intercambiáveis aqui.",
                "handTgt = scratch·4 (±4× a velocidade); a firmeza vem "
                "de GRAB.",
                "SEQUENCE → SCR: uma frase de scratch rítmica. "
                "ENVELOPE → BRK: um tape-stop na virada."}},
            {"in:brake", {
                "Gate: enquanto alto, o motor solta e o prato para "
                "(coasting governado por FRIC). Solto o gate, o motor "
                "puxa de volta.", "", ""}},
            {"out:out", {
                "O material gravado lido pelo prato, com acoplamento AC "
                "(sem degrau de DC quando o prato para).", "", ""}},
        }},
        {"SWIRL", {
            {"type", {
                "0 CHORUS (2 vozes, engorda) · 1 FLANGER (1 atraso curto "
                "+ realimentação, jato/pente) · 2 ENSEMBLE (3 vozes, Juno/"
                "Solina) · 3 PHASER (cascata de all-pass, notch móvel).",
                "Os três primeiros são atrasos CURTOS modulados; o phaser "
                "troca a linha de atraso por 6 all-pass de 1ª ordem — "
                "timbre mais oco, menos metálico.",
                "Compare FLANGER e PHASER no mesmo RATE/DEPTH: o pente "
                "varre metálico, o notch varre oco."}},
            {"rate", {
                "Velocidade do LFO (triangular) que varre a modulação. "
                "0,02 Hz = quase parado; 8 Hz = vibrato.", "", ""}},
            {"depth", {
                "Profundidade da varredura — quanto o atraso (ou o corte "
                "dos all-pass) se move. 0 = coloração estática.", "", ""}},
            {"feedback", {
                "Realimentação (−1..1). No FLANGER/PHASER cria a "
                "ressonância (positivo = pente/notch agudo, negativo = "
                "invertido). No CHORUS/ENSEMBLE um fio dá vibrato.",
                "Um tanh no laço segura o nível; perto de ±1 auto-oscila "
                "(o modo autônomo, a partir do piso de ruído do AGE).", ""}},
            {"spread", {
                "Largura estéreo: o LFO do canal R defasa de spread·¼ de "
                "ciclo do L; nas vozes múltiplas também espalha o "
                "desafino. 0 = mono.", "", ""}},
            {"tone", {
                "Filtro de 1 polo NO molhado: <0 passa-baixa (o "
                "'aveludado' do BBD), >0 passa-alta (afina, tira o "
                "grave), 0 neutro.", "", ""}},
            {"age", {
                "Caráter BBD (bucket-brigade): companding (comprime antes "
                "da linha, expande depois), um fio de ruído semeado e uma "
                "pontinha de aliasing. 0 = limpo.",
                "Determinístico — dois renders com o mesmo AGE são "
                "byte-idênticos.", ""}},
            {"mix", {"Seco ↔ molhado. 0 = passa-direto (bypass exato).",
                     "", ""}},
            {"in:in", {"Áudio a processar.", "", ""}},
            {"in:rate_mod", {"CV somada a RATE (varredura fora do "
                            "compasso).", "", ""}},
            {"in:mix_mod", {"CV somada a MIX (o efeito entra no ataque/"
                           "cauda, via um ENVELOPE).", "", ""}},
            {"out:l", {"Canal esquerdo (seco + molhado por MIX).", "", ""}},
            {"out:r", {"Canal direito — o LFO defasado por SPREAD faz a "
                      "imagem abrir.", "", ""}},
        }},
        {"SEQUENCE", {
            {"length", {"Quantos dos 8 passos entram no padrão.", "", ""}},
            {"mode", {
                "Direção de leitura: pra frente / pra trás / ping-pong / "
                "aleatório / browniano.", "", ""}},
            {"rate", {"Relógio interno, usado só se CLK estiver livre.",
                     "", ""}},
            {"gate_len", {
                "Duty do gate de cada passo (clock interno) ou janela do "
                "gate (clock externo).", "", ""}},
            {"glide", {"Portamento entre a altura de um passo e o "
                      "próximo.", "", ""}},
            {"range", {"Quantas oitavas os valores P1-8 cobrem.", "", ""}},
            {"p1", {"Altura do passo 1 (−1..1, escalado por RANGE).", "", ""}},
            {"p2", {"Altura do passo 2.", "", ""}},
            {"p3", {"Altura do passo 3.", "", ""}},
            {"p4", {"Altura do passo 4.", "", ""}},
            {"p5", {"Altura do passo 5.", "", ""}},
            {"p6", {"Altura do passo 6.", "", ""}},
            {"p7", {"Altura do passo 7.", "", ""}},
            {"p8", {"Altura do passo 8.", "", ""}},
            {"g1", {
                "Liga/desliga o gate do passo 1 — desligado, o passo toca "
                "a altura mas fica mudo.", "", ""}},
            {"g2", {"Liga/desliga o gate do passo 2.", "", ""}},
            {"g3", {"Liga/desliga o gate do passo 3.", "", ""}},
            {"g4", {"Liga/desliga o gate do passo 4.", "", ""}},
            {"g5", {"Liga/desliga o gate do passo 5.", "", ""}},
            {"g6", {"Liga/desliga o gate do passo 6.", "", ""}},
            {"g7", {"Liga/desliga o gate do passo 7.", "", ""}},
            {"g8", {"Liga/desliga o gate do passo 8.", "", ""}},
            {"in:clock", {
                "Avança um passo na borda de subida — presente, "
                "substitui RATE.", "", ""}},
            {"in:reset", {
                "Volta pro passo 0 e reinicia a direção (ping-pong/etc).",
                "", ""}},
            {"out:pitch", {"Altura do passo atual, com GLIDE aplicado.",
                          "", ""}},
            {"out:gate", {"Gate do passo atual (se o G_N dele estiver "
                         "ligado).", "", ""}},
            {"out:eos", {
                "Pulso curto toda vez que a sequência volta pro passo 0 "
                "(fim de ciclo).", "", ""}},
        }},
        {"STRING", {
            {"freq", {"Frequência da corda — o comprimento do laço de "
                     "atraso.", "", ""}},
            {"decay", {"Sustain — quanto tempo a corda continua soando.",
                     "", ""}},
            {"damping", {
                "Brilho — alto deixa a corda mais escura, apagando "
                "harmônicos mais rápido.", "", ""}},
            {"position", {
                "Onde a corda é pinçada — um filtro pente na excitação, "
                "cujo nó em POS não é excitado.", "",
                "Varra POS: em certos pontos alguns harmônicos somem — é "
                "a física real de pinçar uma corda em lugares diferentes."}},
            {"exciter", {"Quanto ruído entra no golpe (PLK).", "", ""}},
            {"drive", {"Satura o laço da corda — mais grão, menos limpo.",
                     "", ""}},
            {"mix", {"Seco ↔ ressonância da corda.", "", ""}},
            {"in:in", {
                "Entrada contínua — excita a corda como um arco, em vez "
                "de golpe único.",
                "O tanh no laço garante estabilidade — o arco leva a um "
                "ciclo-limite, não à divergência (mesmo princípio do "
                "FILTER auto-oscilante).", ""}},
            {"in:pluck", {
                "Trigger — pinça a corda (rajada de ruído filtrada pela "
                "posição).", "", ""}},
            {"in:freq_mod", {"CV 1 V/oct que multiplica FREQ.", "", ""}},
            {"in:damp_mod", {"CV que soma a DAMP.", "", ""}},
            {"out:out", {"Seco + ressonância da corda, misturados por "
                       "MIX.", "", ""}},
        }},
        {"SWITCH", {
            {"steps", {"Quantas posições a chave usa (2-4).", "", ""}},
            {"mode", {
                "Ordem de avanço: pra frente / ping-pong / aleatório (sem "
                "repetir a mesma duas vezes) / só por ADR (ignora CLK).",
                "", ""}},
            {"dir", {
                "DEMUX ligado: 1 entrada (A) distribuída pra N saídas "
                "conforme o passo. Desligado (MUX): N entradas pra 1 "
                "saída.", "", ""}},
            {"glide", {"Crossfade no ponto de troca — só no modo MUX.",
                     "", ""}},
            {"slew", {
                "Suaviza sempre o degrau da troca (~1 ms a mais), "
                "anti-clique em qualquer modo.", "", ""}},
            {"in:a", {"Entrada A (MUX) ou a única entrada distribuída "
                     "(DEMUX).", "", ""}},
            {"in:b", {"Entrada B (só MUX).", "", ""}},
            {"in:c", {"Entrada C (só MUX).", "", ""}},
            {"in:d", {"Entrada D (só MUX).", "", ""}},
            {"in:clock", {
                "Avança um passo na borda de subida (ignorado se ADR "
                "estiver conectada).", "", ""}},
            {"in:reset", {"Volta pro passo 0.", "", ""}},
            {"in:addr", {
                "CV que escolhe o passo diretamente (0..1 mapeado pros "
                "STEPS) — presente, manda, ignora CLK/MODE.", "", ""}},
            {"out:out", {
                "Saída MUX (A/B/C/D selecionada) — ou, em DEMUX, a fatia "
                "do passo 0.", "", ""}},
            {"out:step", {"Posição atual normalizada (0..1).", "", ""}},
            {"out:out_b", {
                "Em DEMUX, a fatia do passo 1 (senão, sempre 0).", "", ""}},
            {"out:out_c", {"Em DEMUX, a fatia do passo 2.", "", ""}},
            {"out:out_d", {"Em DEMUX, a fatia do passo 3.", "", ""}},
        }},
        {"TRIGSEQ", {
            {"length", {"Quantos dos 16 passos entram no ciclo.", "", ""}},
            {"rate", {"Relógio interno, usado só se CLK estiver livre.",
                     "", ""}},
            {"map", {
                "Morfa entre 4 caracteres rítmicos: reto (rock/house) / "
                "quebrado (breakbeat) / suingado (hip-hop) / esparso "
                "(minimal/dub).", "",
                "Varra MAP devagar com CLK correndo: o groove muda de "
                "personalidade em tempo real, sem trocar de padrão \"na "
                "marra\"."}},
            {"density1", {
                "Limiar de disparo da linha 1 (bumbo) — mais alto, mais "
                "passos disparam.", "", ""}},
            {"density2", {"Limiar da linha 2 (caixa).", "", ""}},
            {"density3", {"Limiar da linha 3 (chimbal).", "", ""}},
            {"density4", {"Limiar da linha 4 (perc).", "", ""}},
            {"swing", {"Atrasa os passos ímpares.", "", ""}},
            {"chaos", {
                "Probabilidade de notas-fantasma aparecerem ou disparos "
                "previstos falharem — humaniza, não é flip cru.", "", ""}},
            {"ratchet", {
                "Probabilidade de um disparo virar uma rajada de "
                "repetições rápidas.", "", ""}},
            {"fill_amt", {
                "Quanto FILL empurra as densidades pra cima, "
                "temporariamente.", "", ""}},
            {"drift", {
                "Passeio lento e limitado no MAP e nas densidades — o "
                "groove evolui sozinho aos poucos.", "", ""}},
            {"in:clock", {
                "Avança um passo na borda de subida — presente, "
                "substitui RATE.", "", ""}},
            {"in:reset", {"Volta pro passo 0.", "", ""}},
            {"in:fill", {
                "Gate — nível alto empurra as densidades pra cima "
                "(FILL_AMT), pra uma virada.", "", ""}},
            {"in:map_cv", {"CV que soma a MAP.", "", ""}},
            {"out:t1", {"Gate da linha 1 (bumbo).", "", ""}},
            {"out:t2", {"Gate da linha 2 (caixa).", "", ""}},
            {"out:t3", {"Gate da linha 3 (chimbal).", "", ""}},
            {"out:t4", {"Gate da linha 4 (perc).", "", ""}},
            {"out:accent", {
                "Dispara quando 2 ou mais linhas coincidem no mesmo "
                "passo.", "", ""}},
            {"out:any", {"OR das 4 linhas — dispara se qualquer uma "
                       "tocar.", "", ""}},
        }},
        {"TURING", {
            {"rate", {"Relógio interno, usado só se CLK estiver livre.",
                     "", ""}},
            {"length", {
                "Nº de estágios do registrador de deslocamento — o "
                "tamanho do laço que pode ficar travado.", "", ""}},
            {"lock", {
                "Probabilidade do laço se PRESERVAR em vez de mudar a "
                "cada passo — 0 acaso puro, 1 laço travado (repete pra "
                "sempre).", "", ""}},
            {"mutate", {
                "Quando o laço muda, o quanto: perto de 0 é mutação "
                "pequena (ruído sobre o valor antigo), 1 é sorteio "
                "totalmente novo.", "", ""}},
            {"range", {"Alcance da saída CV, em torno do OFST.", "", ""}},
            {"steps", {"Quantiza CV em STEPS degraus — 1 é contínuo.",
                     "", ""}},
            {"offset", {"Centro da saída CV.", "", ""}},
            {"in:clock", {
                "Avança um passo do registrador na borda de subida — "
                "presente, substitui RATE.", "", ""}},
            {"in:lock_mod", {"CV que soma a LOCK.", "", ""}},
            {"out:cv", {
                "A frente do registrador, bipolar, escalada por "
                "RANGE/OFST e quantizada por STEPS.", "", ""}},
            {"out:cv2", {
                "Soma ponderada de 4 estágios do registrador — uma "
                "segunda CV correlacionada com CV, mas diferente "
                "(\"expansor Volts\" do Turing Machine original).", "", ""}},
            {"out:pulse", {
                "O estágio da frente como bit (0 ou 1) — um gate derivado "
                "do mesmo acaso/memória (\"expansor Pulses\").", "", ""}},
        }},
    };
    return table;
}
}  // namespace detail

// nullptr se não há conteúdo pra esse `moduleType`/`bind` — chamador
// trata como "sem nota ainda" (silencioso, não é erro).
namespace detail {

// ---- verbetes de WIDGET traduzidos, por família -----------------------
//
// Preenchidos por LOTES, uma família de módulos por vez, começada em
// 27 set. 2026. Traduzir 803 verbetes de uma vez seria uma mudança
// impossível de revisar e fácil de abandonar pela metade; por família,
// cada lote é verificável e o medidor de cobertura
// (`tool_learn_coverage`) mostra o que falta.
//
// REGRA DO LOTE: quando um verbete é traduzido, TODOS os seus campos
// preenchidos no português são traduzidos juntos. A queda é por VERBETE,
// não por campo — traduzir só o `quick` e deixar o `understand` cair no
// português daria uma caixa com duas línguas dentro, que é pior que uma
// caixa numa língua só.
inline LearnTable& learnTableEnMutable() { static LearnTable t; return t; }
inline LearnTable& learnTableFrMutable() { static LearnTable t; return t; }
inline LearnTable& learnTableEsMutable() { static LearnTable t; return t; }

// Registro dos lotes. Cada família chama isto uma vez, na primeira
// consulta — `std::call_once` seria exagero aqui porque as tabelas são
// preenchidas por funções `inline` chamadas de um inicializador estático
// local, que o C++ já garante rodar uma vez só.
void registrarLotesTraduzidos();

inline const LearnTable& learnTableEn() {
    static const bool feito = (registrarLotesTraduzidos(), true);
    (void)feito;
    return learnTableEnMutable();
}
inline const LearnTable& learnTableFr() {
    static const bool feito = (registrarLotesTraduzidos(), true);
    (void)feito;
    return learnTableFrMutable();
}
inline const LearnTable& learnTableEs() {
    static const bool feito = (registrarLotesTraduzidos(), true);
    (void)feito;
    return learnTableEsMutable();
}

}  // namespace detail

// Verbete de WIDGET no idioma pedido, caindo no português quando faltar.
//
// A tradução dos 803 verbetes de widget é feita por LOTES (uma família de
// módulos por vez), então a queda não é temporária por acidente: é o
// estado normal enquanto o trabalho avança, e tem de mostrar o texto
// CERTO em português em vez de uma caixa vazia.
inline const LearnEntry* lookupLearn(const std::string& moduleType,
                                     const std::string& bind,
                                     const Lang lang = Lang::pt) {
    if (lang != Lang::pt) {
        const auto& t = (lang == Lang::en) ? detail::learnTableEn()
                      : (lang == Lang::fr) ? detail::learnTableFr()
                                           : detail::learnTableEs();
        const auto m = t.find(moduleType);
        if (m != t.end()) {
            const auto b = m->second.find(bind);
            if (b != m->second.end() && !b->second.quick.empty())
                return &b->second;
        }
    }
    const auto& table = detail::learnTable();
    const auto mi = table.find(moduleType);
    if (mi == table.end()) return nullptr;
    const auto pi = mi->second.find(bind);
    if (pi == mi->second.end()) return nullptr;
    return &pi->second;
}

namespace detail {

// Definição de cada MÓDULO — o LEARN mostra isto quando o mouse está
// sobre o corpo/título de um módulo do rack (não sobre um knob).
// `quick` = o que é; `understand` = o lugar dele / como difere dos
// vizinhos; `explore` = uma cadeia pra experimentar.
inline const std::unordered_map<std::string, LearnEntry>& moduleLearnTable() {
    static const std::unordered_map<std::string, LearnEntry> table = {
        {"OSC", {
            "Oscilador subtrativo — a voz de partida. Dente/quadrada/"
            "triângulo/seno com PW e drift orgânico; toca em 1 V/oct.",
            "É a fonte 'clássica'; o WAVETABLE varre tabelas, o ADDITIVE "
            "soma parciais, o OPERATOR faz FM.",
            "OSC → FILTER → VCA com um ENVELOPE no gate: a cadeia mínima "
            "de um sintetizador."}},
        {"WAVETABLE", {
            "Oscilador de tabela — um POS varre um banco de formas de "
            "onda, de suave a espectral, com anti-aliasing.", "",
            "LFO lento → POS: o timbre 'respira' entre as ondas."}},
        {"ADDITIVE", {
            "Oscilador aditivo — soma até 64 parciais; TILT/ODD-EVEN/"
            "STRETCH esculpem o espectro direto.",
            "Constrói o timbre somando senos, o oposto do FILTER (que "
            "tira de um som rico).", ""}},
        {"OPERATOR", {
            "FM de 4 operadores, 8 algoritmos, feedback estilo DX7 — "
            "metais, sinos, baixos que o subtrativo não faz.", "",
            "RATIO inteiro = harmônico; quebrado = inarmônico (sino)."}},
        {"SPECTRA", {
            "Resíntese espectral — OUVE um som, acha os parciais mais "
            "fortes e re-oscila como um banco de senóides que o segue. A "
            "ponte análise → síntese (Panharmonium / phase vocoder).",
            "O ADDITIVE constrói o espectro do zero; o MEMORY grão no "
            "tempo; aqui o material é o espectro de curto prazo da "
            "entrada. FREEZE = pad infinito de qualquer som. Sem IN, um "
            "ruído semeado alimenta a análise → drone que evolui sozinho.",
            "DRUM → IN, VOICE baixo, STRETCH = uma sombra sineira do "
            "ritmo; FRZ ← gate = espectro congelado alternando com o "
            "vivo."}},
        {"PULSAR", {
            "Síntese pulsar (Curtis Roads) — trem de pulsarets (grão + "
            "silêncio) com DUAS frequências independentes: FREQ = a "
            "altura, FORMANT = o timbre. Entre a granular e a de formante.",
            "O OPERATOR/ADDITIVE fixam o espectro na altura; aqui FORMANT "
            "desliza o envelope espectral SEM desafinar. MASK/JITTER "
            "rareiam e desalinham o trem (padrões burlescos, semeados).",
            "LFO → FQM: 'wah' sem filtro. MASK ~0,7 + JITTER ~0,4 = uma "
            "nuvem rítmica irregular sobre a mesma nota."}},
        {"PLL", {
            "Phase-locked loop — trava a fase num sinal de entrada e "
            "gera divisões/multiplicações dele; solto, auto-oscila.",
            "É oscilador E extrator de clock — segue o que ouve dentro "
            "de uma janela de captura.", ""}},
        {"CHORD", {
            "Voz de acorde — 3–4 osciladores afinados por um intervalo/"
            "inversão; uma nota entra, um acorde sai.", "", ""}},
        {"NOISE", {
            "Ruído e acaso contínuo — branco/rosa/brown/azul/violeta/bit "
            "+ S&H + tensão que passeia, 8 saídas ao mesmo tempo.",
            "O DECISION decide eventos; o NOISE dá o piso e as CVs "
            "aleatórias. POIS troca o clock interno por Poisson livre.",
            ""}},
        {"MATTER", {
            "Voz de modelagem física — matéria excitada (sopro/atrito/"
            "impacto) ressoando; rigidez/tensão/desgaste como knobs.",
            "", ""}},
        {"STRING", {
            "Corda Karplus-Strong estendida — pluck/bow/hammer numa "
            "linha de atraso com amortecimento e material.", "", ""}},
        {"DRUM", {
            "Voz de percussão — corpo com pitch-sweep + estalo de ataque; "
            "um mapa 808 ↔ 909 ↔ acústico e ROLL.", "", ""}},
        {"SIGNAL-IN", {
            "Entrada do mundo — áudio ao vivo (ALSA) + MIDI num adaptador "
            "só; voz monofônica last-note com saídas pitch/gate/vel/cc.",
            "É a contraparte de ENTRADA do NOTE-OUT. Sem nada ligado, "
            "silêncio determinístico.", ""}},
        {"FILTER", {
            "Filtro multimodo ressonante — LP/BP/HP, com a ressonância "
            "chegando à auto-oscilação (vira senoide).",
            "Tira energia de um som rico (subtrativo); o WASP distorce "
            "no caminho, o LPG fecha com vactrol.",
            "ENVELOPE → cutoff_mod: o 'wah' clássico. RES perto do talo "
            "+ cutoff baixo = uma voz de seno afinável."}},
        {"FORMANT", {
            "Banco de 5 passa-faixas paralelos — as ressonâncias de "
            "vogais; VOWEL faz o morph A→E→I→O→U. VOCODER>0 + jack MOD = "
            "vocoder de 5 bandas (as bandas seguem a envoltória do "
            "modulador em vez da tabela de vogal).",
            "Fonte-filtro de Fant; no modo vocoder, o canal de Dudley "
            "(1938).",
            "Num ruído/pulso: fala sintética. Num acorde: coro. Voz no "
            "MOD + serra no IN + VOCODER=1: o sinte 'fala'. Pra fala "
            "inteligível de banda larga, o VOCODER dedicado."}},
        {"VOCODER", {
            "Vocoder de N bandas (4–20) — a energia do MODULADOR por "
            "banda controla o ganho da mesma banda na PORTADORA: a "
            "portadora 'fala' o modulador. Homer Dudley, 1938.",
            "O FORMANT tem um mode vocoder de 5 bandas (vocálico); este "
            "é o dedicado, banda larga (fala inteligível). SIBIL passa "
            "as fricativas; FREEZE = pad falado infinito; SHIFT = "
            "formant shift.",
            "Voz no MOD + serra/pad no CAR + BANDS=16 = o clássico. Sem "
            "CAR, a serra interna afina por PIT."}},
        {"RESONATOR", {
            "Banco de ≤ 24 modos AFINADOS a uma série, excitado por sinal "
            "EXTERNO (você bate com o que quiser). Rings/Elements no modo "
            "ressoador.",
            "O MATTER é voz fechada (exciter próprio); aqui você traz a "
            "excitação. As 3 saídas LOW/MID/HIGH se CRUZAM quando você "
            "varre TILT — a relação entre elas é o processo (Three Sisters).",
            "DRUM → IN (uma marimba tocada pelo bumbo); TRIGSEQ → STRK; "
            "RESONATOR.low e .high em destinos diferentes."}},
        {"WASP", {
            "Filtro-distorção — o caráter sujo/gritado do EDP Wasp; a "
            "não-linearidade fica NO caminho do filtro.", "", ""}},
        {"LPG", {
            "Low-pass gate — filtro + VCA num controle só, com a "
            "assimetria de vactrol (ataque rápido, cauda lenta). O 'bongô' "
            "Buchla.", "",
            "Ping com um trigger curto: o LPG dá o corpo e o decaimento "
            "de uma vez."}},
        {"VCA", {
            "Amplificador controlado por tensão — o volume/nível como CV; "
            "resposta linear ou exponencial.", "", ""}},
        {"VCA4", {
            "Banco de 4 VCAs + um mixer somado (MIX). O `VCA` (#20) é "
            "duplo; este é o banco pra patch grande — Mutable Veils / "
            "Intellijel Quad VCA.",
            "CURVE linear/exp é COMPARTILHADA. Cada canal: LVL + CV "
            "atenuvertida (soma por porta). MIXG dá o ganho da soma.",
            "As 3 saídas low/mid/high do RESONATOR/FILTER em 3 canais + "
            "um envelope em cada CV = um espectro que se move; a MIX "
            "junta tudo."}},
        {"CONTROL", {
            "Utilidades de CV contínua — atenuversor, offset, retificação "
            "como lerp, slew. O canivete do sinal de controle.", "", ""}},
        {"GLIDE", {
            "Portamento/slew — desliza entre valores em vez de saltar; "
            "tempo separado pra subida e descida, modo constante ou "
            "proporcional.", "", ""}},
        {"SHAPE", {
            "Waveshaper — dobra/satura/retifica a forma de onda pra criar "
            "harmônicos; de calor sutil a destruição.", "", ""}},
        {"SHIFTER", {
            "Deslocador de frequência — move o espectro INTEIRO por tantos "
            "Hz (não por razão). Os parciais deixam de ser harmônicos → "
            "metálico, sineiro. SSB: saídas `up` (+Δf) e `down` (−Δf).",
            "O ring-mod do SHAPE dá as DUAS bandas simétricas; o SHIFTER "
            "dá uma só por saída. FEEDBACK = o 'barber pole' de Risset "
            "(glissando infinito). Bode/Moog frequency shifter.",
            "SHIFT ~8 Hz = fase que nunca fecha; SHIFT grande + FEEDBACK "
            "= a espiral de Shepard; `up`/`down` em destinos diferentes."}},
        {"CRUSH", {
            "Destruidor lo-fi / decimador — o lixo DIGITAL: redução de "
            "taxa (aliasing), de bits, transbordo que enrola, glitch "
            "(dropout/travada/repique) e jitter de clock.",
            "É onde mora o verbo DAMAGE. O SHAPE/WASP distorcem de forma "
            "ANALÓGICA (fold, tanh); aqui é quantização, overflow de "
            "inteiro, conexão ruim. Tudo semeado → dano reprodutível.",
            "ENVELOPE → BITS (resolução cai na cauda); LFO → RATE; sem "
            "IN + MIX=1 = uma fonte de ruído lo-fi."}},
        {"PARAMETRIC", {
            "EQ paramétrico — bandas de sino/shelf pra esculpir o "
            "espectro cirurgicamente, sem ressonância de filtro.", "", ""}},
        {"ENVELOPE", {
            "Gerador de envelope — AD/ADSR/AR e laços; a forma da "
            "amplitude (ou de qualquer CV) no tempo, disparada por gate.",
            "", "ENVELOPE → VCA + ENVELOPE → cutoff: o contorno do som."}},
        {"FUNCTION", {
            "Gerador de função tipo Maths — rampa/envelope/LFO deformável "
            "com saídas derivadas; soma, integra, dispara.",
            "É envelope E LFO E slew, tudo na curva. O DRIFT é passeio "
            "lento; o CHAOS é caótico de verdade.", ""}},
        {"STAGES", {
            "Gerador de N segmentos configuráveis (2–8) cuja FUNÇÃO "
            "EMERGE de como se encadeiam: rampas → envelope/LFO, degraus "
            "→ sequência de CV. Mutable Stages / Rossum Control Forge.",
            "O FUNCTION é UMA rampa; o STAGES é a forma COMPOSTA, "
            "esculpida por CONTOUR/TILT/HOLD (gerador, não editor). "
            "HOLD=0 desliza (env), HOLD=1 salta (seq); LOOP corre ↔ dispara.",
            "STAGES → FILTER.cutoff; CLOCK → GATE (loop off) = envelope; "
            "STAGES.step → ENVELOPE.gate encadeia."}},
        {"DRIFT", {
            "Campo de deriva orgânica — várias CVs correlacionadas que "
            "passeiam devagar (escala de minutos), com momentum e "
            "âncora a marcos gravados.",
            "Dá VIDA sem direção — nada fica estático. Determinístico "
            "(semeado).", ""}},
        {"CHAOS", {
            "Campo caótico de poço duplo — EDO determinística que assenta "
            "num poço, oscila entre dois, ou 'caça' imprevisível.",
            "Genuinamente caótico, não pseudo-aleatório (DECISION/TURING) "
            "nem passeio filtrado (DRIFT).", ""}},
        {"SH", {
            "Sample & hold duplo — segura um valor no pulso; 2 canais "
            "com CORRELATION (gêmeos ↔ espelho ↔ independentes) e slew.",
            "", "CLOCK → SH · out1 → pitch · out2 → cutoff: melodia e "
            "timbre que 'combinam'."}},
        {"CLOCK", {
            "Relógio mestre euclidiano — pulso com divisões, fill "
            "euclidiano, swing e drift; a batida de todo o patch.",
            "", "CLOCK → tudo que tem gate/trig. FILL espalha os pulsos "
            "pelo compasso (padrões de Toussaint)."}},
        {"LOGIC", {
            "Lógica de clock — divide/multiplica, AND/OR/XOR de gates, "
            "atraso; recombina o TEMPO.",
            "O par do ABACUS (que faz aritmética de CV). Aqui é só "
            "gate/trigger.", ""}},
        {"TURING", {
            "Registrador de deslocamento aleatório (Turing Machine) — um "
            "laço de bits que LOCK trava e MUT reembaralha; melodia/"
            "ritmo que se repete com variação.", "", ""}},
        {"SEQUENCE", {
            "Sequenciador de 8 passos — sliders de altura + direção "
            "(frente/trás/ping-pong/aleatório/browniano), gate por passo.",
            "", "Os sliders são a partitura — a Motion Engine mal os "
            "toca. Use MUDA/EVOLUI pra reembaralhar de propósito."}},
        {"TRIGSEQ", {
            "Sequenciador de TRIGGERS — 4 faixas de densidade euclidiana "
            "com chaos/ratchet/fill/swing; a percussão generativa.", "",
            ""}},
        {"QUANTIZER", {
            "Quantizador de escala — encaixa uma CV nos graus de uma "
            "escala/tom; slew e histerese pra não tremular na borda.", "",
            "SH/TURING/DRIFT → QUANTIZER → pitch: passeio aleatório vira "
            "melodia na escala."}},
        {"HARMONY", {
            "Harmonizador — de uma nota gera intervalos/acordes coerentes "
            "com uma tonalidade e uma condução de voz.", "", ""}},
        {"ABACUS", {
            "Aritmética de CV — soma/subtrai/multiplica/resto e AND/OR/"
            "XOR bit a bit entre dois sinais + um contador binário cujos "
            "bits viram ritmo (Numeric Repetitor).",
            "O par do LOGIC (que recombina tempo). Sozinho, o contador já "
            "toca uma sequência + um ritmo.", ""}},
        {"DECISION", {
            "Fonte de acaso ESTRUTURADO tipo Marbles — distribuição "
            "(bias/spread/shape), déjà-vu (repete o padrão), steps e "
            "slew; gera CV e gate com 'intenção'.", "", ""}},
        {"BOXCAR", {
            "Averager de porta (boxcar averager) — uma janela amostra um "
            "ponto do período, a média de N capturas revela o sinal no "
            "ruído; SCAN reconstrói a onda; GEIG = gates de Poisson.",
            "É o SCOPE ao contrário: mede pra CONSTRUIR uma onda. mode 0 "
            "= S&H de janela, 1 = reconstrói, 2 = relê o buffer próprio "
            "como oscilador.",
            "Ruído + um tom fraco sincronizado ao TRIG, mode 1, SCAN alto: "
            "o tom 'se desenha' ao longo de segundos."}},
        {"SWITCH", {
            "Chave/roteador — N entradas → 1 saída (ou 1 → N), por passo "
            "de clock, CV ou aleatório; sequencia FONTES em vez de notas.",
            "", ""}},
        {"MATRIX", {
            "Matriz de roteamento 4×4 — qualquer entrada pra qualquer "
            "saída com ganho por célula; um mixer de CV/áudio programável.",
            "", ""}},
        {"MULT", {
            "Múltiplo com buffer — copia um sinal pra vários destinos sem "
            "queda de nível; modo dual e slew opcional.", "", ""}},
        {"PLANAR", {
            "Morph vetorial XY — desliza entre 4 fontes num plano; o "
            "gesto (o caminho no plano) é gravável e reproduzível.", "",
            ""}},
        {"SPACE", {
            "Reverberação — rede de Schroeder/Dattorro multitap, de "
            "ambiente curto a cauda longa; pre-delay e amortecimento.",
            "O SPACE é o reverb 'clássico'; o HALL é FDN de 8 linhas "
            "(mais denso, estéreo); o MEMORY é granular.", ""}},
        {"SWIRL", {
            "Efeitos de MODULAÇÃO — chorus, flanger, ensemble e phaser "
            "num módulo (TYPE). Atrasos CURTOS modulados por LFO (+ o "
            "phaser, cascata de all-pass), com caráter BBD no AGE.",
            "O RASGO não tinha nenhum. Distinto do LOOPER (delay de "
            "linha, ecos audíveis) e do reverb — aqui você ouve o "
            "MOVIMENTO, não o eco.",
            "voz → SWIRL → MASTER engorda; LFO → RATE deixa a varredura "
            "respirar fora do compasso; FBK perto de ±1 + AGE = auto-oscila."}},
        {"HALL", {
            "Reverb FDN de 8 linhas + matriz de Householder — cauda densa "
            "e estéreo, com FREEZE (congela infinito) e pre-delay.", "",
            ""}},
        {"LOOPER", {
            "Delay de linha com HOLD (congela e repete), REVERSE (lê pra "
            "trás sem clique), AGE (caráter de fita/BBD) e HEADS (eco "
            "multi-cabeça tipo Space Echo).",
            "O 'TAPE' virou este HEADS. FBK perto de 1 + TIME longo + "
            "várias cabeças = Frippertronics.", ""}},
        {"MEMORY", {
            "Granular — buffer picado em grãos (grão/nuvem/spray), com "
            "posição, tamanho, densidade e FREEZE; texturas e time-stretch.",
            "", ""}},
        {"SAMPLER", {
            "Toca-fatias (MPC/Akai) — TRIG dispara um trecho gravado ao "
            "vivo (gate REC) ou de arquivo; varispeed, slices + POS, "
            "repitch, wear.",
            "O SAMPLER salta a posição de leitura (fatia); o TURNTABLE "
            "lê contínuo com inércia; o MEMORY é granular.", ""}},
        {"TURNTABLE", {
            "Toca-discos de prato com massa — o buffer do SAMPLER lido por "
            "uma velocidade angular com INÉRCIA: o motor puxa devagar "
            "(TORQ), a mão empurra (SCR), o freio faz coasting (BRK).",
            "SEM quantização de BPM — beatmatch é gesto, como no vinil.",
            "LFO → SCR: o disco scratcheia no compasso. ENVELOPE → BRK: "
            "um tape-stop na virada."}},
        {"MIXER", {
            "Mixer de 4 canais — nível (slider), pan e mute por canal + "
            "saída geral. Onde o músico faz o balanço.",
            "Isento da Motion Engine — os pans e níveis são teus.", ""}},
        {"MASTER", {
            "Estágio de saída estéreo — largura, guarda de corpo e "
            "limitador true-peak; o teto do patch.",
            "Isento da Motion Engine. O botão ESPERA do cabeçalho é o "
            "mute deste módulo.", ""}},
        {"SCOPE", {
            "Osciloscópio + medidor que DEVOLVE como CV — pitch (YIN), "
            "envelope, centroide espectral, ONSET (pulso no ataque); o "
            "patch se ouve.",
            "É o SCOPE que mede pra reduzir (→ escalar); o BOXCAR mede "
            "pra reconstruir (→ onda).",
            "SCOPE.onset ← bateria → dispara ENVELOPE no compasso do "
            "áudio; SCOPE.pitch → OSC.pitch afina pelo que entra."}},
        {"NOTE-OUT", {
            "Saída MIDI — converte gate + 1 V/oct em note on/off, com "
            "canal, velocidade e um registro de partitura.",
            "A contraparte de SAÍDA do SIGNAL-IN.", ""}},
    };
    return table;
}

// Definição do módulo (hover sobre o corpo/título). nullptr se não há.
// ---- os 58 verbetes de MÓDULO em EN / FR / ES --------------------------
//
// Traduzidos em 27 set. 2026, a pedido do autor, depois que a revisão de
// idiomas mostrou que o LEARN era monolíngue enquanto interface e tutorial
// já estavam em quatro línguas.
//
// Só o nível de MÓDULO ("para que serve isto") — é a pergunta de quem abre
// o rack pela primeira vez, e rende mais por caractere: 13 mil caracteres
// em vez dos 86 mil do catálogo inteiro. Os verbetes de WIDGET seguem em
// português, e isso está declarado em `PUBLICACAO.md` como limitação.
//
// Traduzido À MÃO, não por máquina. O conteúdo descreve comportamento real
// de cada módulo — a assimetria de vactrol do LPG, o alcance de captura do
// PLL, o cruzamento das saídas do RESONATOR ao varrer TILT — e tradução
// automática produziria texto que PARECE explicação sem ser. Num
// instrumento didático isso é pior que não ter texto.
//
// Nomes de módulo, siglas e unidades ficam como estão (OSC, VCA, 1 V/oct,
// S&H): são o vocabulário do modular em qualquer língua, e traduzi-los
// esconderia o que o painel mostra.

inline const std::unordered_map<std::string, LearnEntry>& moduleLearnTableEn() {
    static const std::unordered_map<std::string, LearnEntry> table = {
        {"OSC", {
            "Subtractive oscillator — the starting voice. Saw/square/triangle/sine with PW and organic drift; tracks 1 V/oct.",
            "This is the 'classic' source; WAVETABLE sweeps tables, ADDITIVE sums partials, OPERATOR does FM.",
            "OSC → FILTER → VCA with an ENVELOPE on the gate: the minimum chain of a synthesiser."}},
        {"WAVETABLE", {
            "Wavetable oscillator — a POS knob sweeps a bank of waveforms, from smooth to spectral, anti-aliased.",
            "",
            "Slow LFO → POS: the timbre 'breathes' between the waves."}},
        {"ADDITIVE", {
            "Additive oscillator — sums up to 64 partials; TILT/ODD-EVEN/STRETCH sculpt the spectrum directly.",
            "Builds timbre by adding sines, the opposite of FILTER (which takes away from a rich sound).",
            ""}},
        {"OPERATOR", {
            "4-operator FM, 8 algorithms, DX7-style feedback — brass, bells and basses the subtractive path will not make.",
            "",
            "Whole-number RATIO = harmonic; fractional = inharmonic (bell)."}},
        {"PULSAR", {
            "Pulsar synthesis (Curtis Roads) — a train of pulsarets (grain + silence) with TWO independent frequencies: FREQ is the pitch, FORMANT the timbre. Between granular and formant synthesis.",
            "OPERATOR/ADDITIVE pin the spectrum to the pitch; here FORMANT slides the spectral envelope WITHOUT detuning. MASK/JITTER thin out and misalign the train (burlesque patterns, seeded).",
            "LFO → FQM: 'wah' with no filter. MASK ~0.7 + JITTER ~0.4 = an irregular rhythmic cloud over the same note."}},
        {"SPECTRA", {
            "Spectral resynthesis — it LISTENS to a sound, finds the strongest partials and re-oscillates as a bank of sines that follows it. The analysis → synthesis bridge (Panharmonium / phase vocoder).",
            "ADDITIVE builds the spectrum from nothing; MEMORY grains in time; here the material is the short-term spectrum of the input. FREEZE = an endless pad from any sound. With no IN, seeded noise feeds the analysis → a drone that evolves on its own.",
            "DRUM → IN, low VOICE, STRETCH = a bell-like shadow of the rhythm; FRZ ← gate = frozen spectrum alternating with the live one."}},
        {"PLL", {
            "Phase-locked loop — locks phase to an input and generates divisions/multiplications of it; left free, it self-oscillates.",
            "It is oscillator AND clock extractor — it follows what it hears, within a capture window.",
            ""}},
        {"CHORD", {
            "Chord voice — 3–4 oscillators tuned by an interval/inversion; one note in, a chord out.",
            "",
            ""}},
        {"NOISE", {
            "Noise and continuous chance — white/pink/brown/blue/violet/bit + S&H + a wandering voltage, 8 outputs at once.",
            "DECISION decides events; NOISE gives the floor and the random CVs. POIS swaps the internal clock for free Poisson.",
            ""}},
        {"MATTER", {
            "Physical-modelling voice — matter excited (blown/rubbed/struck) and ringing; stiffness/tension/wear as knobs.",
            "",
            ""}},
        {"STRING", {
            "Extended Karplus-Strong string — pluck/bow/hammer into a delay line with damping and material.",
            "",
            ""}},
        {"DRUM", {
            "Percussion voice — body with pitch sweep + attack click; an 808 ↔ 909 ↔ acoustic map, and ROLL.",
            "",
            ""}},
        {"SIGNAL-IN", {
            "The way in from the world — live audio (ALSA) + MIDI in a single adapter; monophonic last-note voice with pitch/gate/vel/cc outputs.",
            "It is the INPUT counterpart of NOTE-OUT. With nothing connected, deterministic silence.",
            ""}},
        {"FILTER", {
            "Resonant multimode filter — LP/BP/HP, with resonance reaching self-oscillation (it becomes a sine).",
            "Takes energy away from a rich sound (subtractive); WASP distorts on the way, LPG closes with a vactrol.",
            "ENVELOPE → cutoff_mod: the classic 'wah'. RES near the top + low cutoff = a tunable sine voice."}},
        {"FORMANT", {
            "Bank of 5 parallel bandpasses — the resonances of vowels; VOWEL morphs A→E→I→O→U. VOCODER>0 + the MOD jack = a 5-band vocoder (the bands follow the modulator's envelope instead of the vowel table).",
            "Fant's source-filter model; in vocoder mode, Dudley's channel (1938).",
            "On noise or pulses: synthetic speech. On a chord: a choir. Voice into MOD + saw into IN + VOCODER=1: the synth 'speaks'. For intelligible wideband speech, use the dedicated VOCODER."}},
        {"VOCODER", {
            "N-band vocoder (4–20) — the MODULATOR's energy per band controls the gain of that same band in the CARRIER: the carrier 'speaks' the modulator. Homer Dudley, 1938.",
            "FORMANT has a 5-band (vowel) vocoder mode; this is the dedicated, wideband one (intelligible speech). SIBIL passes the fricatives; FREEZE = an endless spoken pad; SHIFT = formant shift.",
            "Voice into MOD + saw or pad into CAR + BANDS=16 = the classic. With no CAR, the internal saw is tuned by PIT."}},
        {"RESONATOR", {
            "Bank of up to 24 modes TUNED to a series, excited by an EXTERNAL signal (you strike it with whatever you like). Rings/Elements in resonator mode.",
            "MATTER is a closed voice (its own exciter); here you bring the excitation. The 3 outputs LOW/MID/HIGH CROSS OVER as you sweep TILT — the relation between them is the process (Three Sisters).",
            "DRUM → IN (a marimba played by the kick); TRIGSEQ → STRK; RESONATOR.low and .high into different destinations."}},
        {"WASP", {
            "Filter-distortion — the dirty, shouted character of the EDP Wasp; the non-linearity sits IN the filter path.",
            "",
            ""}},
        {"LPG", {
            "Low-pass gate — filter + VCA in one control, with vactrol asymmetry (fast attack, slow tail). The Buchla 'bongo'.",
            "",
            "Ping it with a short trigger: the LPG gives body and decay in one gesture."}},
        {"VCA", {
            "Voltage-controlled amplifier — volume/level as CV; linear or exponential response.",
            "",
            ""}},
        {"VCA4", {
            "Bank of 4 VCAs plus a summed mixer (MIX). `VCA` (#20) is dual; this is the bank for a large patch — Mutable Veils / Intellijel Quad VCA.",
            "The linear/exp CURVE is SHARED. Each channel: LVL + attenuverted CV (summed at the port). MIXG sets the gain of the sum.",
            "The 3 low/mid/high outputs of RESONATOR/FILTER into 3 channels + an envelope on each CV = a spectrum that moves; MIX brings it together."}},
        {"SHAPE", {
            "Waveshaper — folds/saturates/rectifies the waveform to create harmonics; from subtle warmth to destruction.",
            "",
            ""}},
        {"SHIFTER", {
            "Frequency shifter — moves the WHOLE spectrum by so many Hz (not by a ratio). The partials stop being harmonic → metallic, bell-like. SSB: `up` (+Δf) and `down` (−Δf) outputs.",
            "SHAPE's ring-mod gives BOTH symmetric sidebands; SHIFTER gives one per output. FEEDBACK = Risset's 'barber pole' (endless glissando). Bode/Moog frequency shifter.",
            "SHIFT ~8 Hz = a phase that never closes; large SHIFT + FEEDBACK = the Shepard spiral; `up`/`down` into different destinations."}},
        {"CRUSH", {
            "Lo-fi destroyer / decimator — DIGITAL rubbish: rate reduction (aliasing), bit reduction, wrapping overflow, glitch (dropout/stuck/stutter) and clock jitter.",
            "This is where the verb DAMAGE lives. SHAPE/WASP distort in an ANALOGUE way (fold, tanh); here it is quantisation, integer overflow, a bad connection. All seeded → reproducible damage.",
            "ENVELOPE → BITS (resolution falls away in the tail); LFO → RATE; no IN + MIX=1 = a lo-fi noise source."}},
        {"PARAMETRIC", {
            "Parametric EQ — bell and shelf bands to sculpt the spectrum surgically, without filter resonance.",
            "",
            ""}},
        {"GLIDE", {
            "Portamento/slew — slides between values instead of jumping; separate time for rise and fall, constant or proportional mode.",
            "",
            ""}},
        {"CONTROL", {
            "Continuous CV utilities — attenuverter, offset, rectification as a lerp, slew. The pocket knife of the control signal.",
            "",
            ""}},
        {"ENVELOPE", {
            "Envelope generator — AD/ADSR/AR and loops; the shape of amplitude (or of any CV) over time, fired by a gate.",
            "",
            "ENVELOPE → VCA + ENVELOPE → cutoff: the contour of the sound."}},
        {"FUNCTION", {
            "Maths-style function generator — a deformable ramp/envelope/LFO with derived outputs; it sums, integrates, fires.",
            "It is envelope AND LFO AND slew, all in the curve. DRIFT is a slow walk; CHAOS is genuinely chaotic.",
            ""}},
        {"STAGES", {
            "Generator of N configurable segments (2–8) whose FUNCTION EMERGES from how they chain: ramps → envelope/LFO, steps → a CV sequence. Mutable Stages / Rossum Control Forge.",
            "FUNCTION is ONE ramp; STAGES is the COMPOUND shape, sculpted by CONTOUR/TILT/HOLD (a generator, not an editor). HOLD=0 slides (env), HOLD=1 jumps (seq); LOOP runs ↔ fires.",
            "STAGES → FILTER.cutoff; CLOCK → GATE (loop off) = an envelope; STAGES.step → ENVELOPE.gate chains them."}},
        {"DRIFT", {
            "Field of organic drift — several correlated CVs wandering slowly (on the scale of minutes), with momentum and an anchor to recorded landmarks.",
            "It gives LIFE without direction — nothing stays still. Deterministic (seeded).",
            ""}},
        {"CHAOS", {
            "Double-well chaotic field — a deterministic ODE that settles in one well, oscillates between two, or 'hunts' unpredictably.",
            "Genuinely chaotic, not pseudo-random (DECISION/TURING) nor a filtered walk (DRIFT).",
            ""}},
        {"SH", {
            "Dual sample & hold — holds a value on a pulse; 2 channels with CORRELATION (twins ↔ mirror ↔ independent) and slew.",
            "",
            "CLOCK → SH · out1 → pitch · out2 → cutoff: a melody and a timbre that 'go together'."}},
        {"CLOCK", {
            "Euclidean master clock — a pulse with divisions, euclidean fill, swing and drift; the beat of the whole patch.",
            "",
            "CLOCK → everything with a gate or trigger. FILL spreads the pulses across the bar (Toussaint's patterns)."}},
        {"LOGIC", {
            "Clock logic — divide/multiply, AND/OR/XOR of gates, delay; it recombines TIME.",
            "The partner of ABACUS (which does CV arithmetic). Here it is gates and triggers only.",
            ""}},
        {"TURING", {
            "Random shift register (Turing Machine) — a loop of bits that LOCK holds and MUT reshuffles; melody/rhythm that repeats with variation.",
            "",
            ""}},
        {"SEQUENCE", {
            "8-step sequencer — pitch sliders + direction (forward/back/ping-pong/random/brownian), with a gate per step.",
            "",
            "The sliders are the score — the Motion Engine barely touches them. Use CHANGE/EVOLVE to reshuffle on purpose."}},
        {"TRIGSEQ", {
            "TRIGGER sequencer — 4 lanes of euclidean density with chaos/ratchet/fill/swing; the generative percussion.",
            "",
            ""}},
        {"QUANTIZER", {
            "Scale quantiser — snaps a CV to the degrees of a scale and key; slew and hysteresis so it will not flutter at the edge.",
            "",
            "SH/TURING/DRIFT → QUANTIZER → pitch: a random walk becomes a melody in the scale."}},
        {"HARMONY", {
            "Harmoniser — from one note it generates intervals and chords coherent with a key and with a voice leading.",
            "",
            ""}},
        {"ABACUS", {
            "CV arithmetic — add/subtract/multiply/modulo and bitwise AND/OR/XOR between two signals, plus a binary counter whose bits become rhythm (Numeric Repetitor).",
            "The partner of LOGIC (which recombines time). On its own, the counter already plays a sequence and a rhythm.",
            ""}},
        {"DECISION", {
            "Source of STRUCTURED chance, Marbles-style — distribution (bias/spread/shape), déjà-vu (repeats the pattern), steps and slew; it makes CV and gates with 'intent'.",
            "",
            ""}},
        {"BOXCAR", {
            "Boxcar averager — a window samples one point of the period, and the average of N captures reveals the signal inside the noise; SCAN reconstructs the wave; GEIG = Poisson gates.",
            "It is the SCOPE in reverse: it measures in order to BUILD a wave. mode 0 = windowed S&H, 1 = reconstructs, 2 = reads its own buffer back as an oscillator.",
            "Noise plus a weak tone synchronised to TRIG, mode 1, high SCAN: the tone 'draws itself' over seconds."}},
        {"SWITCH", {
            "Switch/router — N inputs → 1 output (or 1 → N), by clock step, CV or at random; it sequences SOURCES instead of notes.",
            "",
            ""}},
        {"MATRIX", {
            "4×4 routing matrix — any input to any output with a gain per cell; a programmable CV/audio mixer.",
            "",
            ""}},
        {"MULT", {
            "Buffered multiple — copies a signal to several destinations with no level drop; dual mode and optional slew.",
            "",
            ""}},
        {"PLANAR", {
            "XY vector morph — slides between 4 sources on a plane; the gesture (the path across the plane) can be recorded and replayed.",
            "",
            ""}},
        {"SPACE", {
            "Reverberation — a multitap Schroeder/Dattorro network, from short ambience to a long tail; pre-delay and damping.",
            "SPACE is the 'classic' reverb; HALL is an 8-line FDN (denser, stereo); MEMORY is granular.",
            ""}},
        {"HALL", {
            "8-line FDN reverb with a Householder matrix — a dense stereo tail, with FREEZE (endless hold) and pre-delay.",
            "",
            ""}},
        {"LOOPER", {
            "Delay line with HOLD (freezes and repeats), REVERSE (plays backwards without a click), AGE (tape/BBD character) and HEADS (multi-head echo, Space Echo style).",
            "The old 'TAPE' became this HEADS. FBK near 1 + a long TIME + several heads = Frippertronics.",
            ""}},
        {"SWIRL", {
            "MODULATION effects — chorus, flanger, ensemble and phaser in one module (TYPE). SHORT delays modulated by an LFO (plus the phaser, a cascade of all-passes), with BBD character in AGE.",
            "RASGO had none of these. Distinct from LOOPER (a delay line, audible echoes) and from reverb — here you hear the MOVEMENT, not the echo.",
            "voice → SWIRL → MASTER thickens it; LFO → RATE lets the sweep breathe off the bar; FBK near ±1 + AGE = it self-oscillates."}},
        {"MEMORY", {
            "Granular — a buffer chopped into grains (grain/cloud/spray), with position, size, density and FREEZE; textures and time-stretch.",
            "",
            ""}},
        {"SAMPLER", {
            "Slice player (MPC/Akai) — TRIG fires a passage recorded live (REC gate) or from a file; varispeed, slices + POS, repitch, wear.",
            "SAMPLER jumps the read position (a slice); TURNTABLE reads continuously with inertia; MEMORY is granular.",
            ""}},
        {"TURNTABLE", {
            "Turntable with a platter that has mass — the SAMPLER's buffer read by an angular velocity with INERTIA: the motor pulls slowly (TORQ), the hand pushes (SCR), the brake lets it coast (BRK).",
            "NO BPM quantisation — beatmatching is a gesture, as on vinyl.",
            "LFO → SCR: the record scratches in time. ENVELOPE → BRK: a tape-stop on the turnaround."}},
        {"MIXER", {
            "4-channel mixer — level (slider), pan and mute per channel plus a master out. Where the musician does the balance.",
            "Exempt from the Motion Engine — the pans and levels are yours.",
            ""}},
        {"MASTER", {
            "Stereo output stage — width, body guard and a true-peak limiter; the ceiling of the patch.",
            "Exempt from the Motion Engine. The header's STANDBY button is this module's mute.",
            ""}},
        {"SCOPE", {
            "Oscilloscope and meter that GIVES BACK as CV — pitch (YIN), envelope, spectral centroid, ONSET (a pulse on the attack); the patch hears itself.",
            "SCOPE measures in order to reduce (→ a scalar); BOXCAR measures in order to reconstruct (→ a wave).",
            "SCOPE.onset ← drums → fires an ENVELOPE in time with the audio; SCOPE.pitch → OSC.pitch tunes to whatever comes in."}},
        {"NOTE-OUT", {
            "MIDI out — converts gate + 1 V/oct into note on/off, with channel, velocity and a score log.",
            "The OUTPUT counterpart of SIGNAL-IN.",
            ""}},
    };
    return table;
}

inline const std::unordered_map<std::string, LearnEntry>& moduleLearnTableFr() {
    static const std::unordered_map<std::string, LearnEntry> table = {
        {"OSC", {
            "Oscillateur soustractif — la voix de départ. Dent de scie/carré/triangle/sinus avec PW et dérive organique ; suit le 1 V/oct.",
            "C’est la source « classique » ; WAVETABLE balaie des tables, ADDITIVE somme des partiels, OPERATOR fait de la FM.",
            "OSC → FILTER → VCA avec une ENVELOPPE sur le gate : la chaîne minimale d’un synthétiseur."}},
        {"WAVETABLE", {
            "Oscillateur à table d’ondes — un POS balaie une banque de formes d’onde, du doux au spectral, anti-aliasé.",
            "",
            "LFO lent → POS : le timbre « respire » entre les ondes."}},
        {"ADDITIVE", {
            "Oscillateur additif — somme jusqu’à 64 partiels ; TILT/ODD-EVEN/STRETCH sculptent le spectre directement.",
            "Construit le timbre en ajoutant des sinus, l’inverse du FILTER (qui retire d’un son riche).",
            ""}},
        {"OPERATOR", {
            "FM à 4 opérateurs, 8 algorithmes, feedback à la DX7 — cuivres, cloches et basses que le soustractif ne fait pas.",
            "",
            "RATIO entier = harmonique ; fractionnaire = inharmonique (cloche)."}},
        {"PULSAR", {
            "Synthèse pulsar (Curtis Roads) — un train de pulsarets (grain + silence) avec DEUX fréquences indépendantes : FREQ est la hauteur, FORMANT le timbre. Entre le granulaire et la synthèse par formants.",
            "OPERATOR/ADDITIVE fixent le spectre sur la hauteur ; ici FORMANT fait glisser l’enveloppe spectrale SANS désaccorder. MASK/JITTER raréfient et désalignent le train (motifs burlesques, semés).",
            "LFO → FQM : un « wah » sans filtre. MASK ~0,7 + JITTER ~0,4 = un nuage rythmique irrégulier sur la même note."}},
        {"SPECTRA", {
            "Resynthèse spectrale — elle ÉCOUTE un son, trouve les partiels les plus forts et ré-oscille en banc de sinus qui le suit. Le pont analyse → synthèse (Panharmonium / vocodeur de phase).",
            "ADDITIVE construit le spectre de rien ; MEMORY granule dans le temps ; ici la matière est le spectre à court terme de l’entrée. FREEZE = un pad infini à partir de n’importe quel son. Sans IN, un bruit semé alimente l’analyse → un drone qui évolue seul.",
            "DRUM → IN, VOICE bas, STRETCH = une ombre de cloches du rythme ; FRZ ← gate = spectre gelé alternant avec le vivant."}},
        {"PLL", {
            "Boucle à verrouillage de phase — se verrouille sur un signal d’entrée et en génère divisions et multiplications ; laissée libre, elle auto-oscille.",
            "C’est un oscillateur ET un extracteur d’horloge — elle suit ce qu’elle entend, dans une fenêtre de capture.",
            ""}},
        {"CHORD", {
            "Voix d’accord — 3 à 4 oscillateurs accordés par un intervalle/renversement ; une note entre, un accord sort.",
            "",
            ""}},
        {"NOISE", {
            "Bruit et hasard continu — blanc/rose/brown/bleu/violet/bit + S&H + une tension qui erre, 8 sorties à la fois.",
            "DECISION décide des événements ; NOISE donne le plancher et les CV aléatoires. POIS remplace l’horloge interne par un Poisson libre.",
            ""}},
        {"MATTER", {
            "Voix de modélisation physique — matière excitée (soufflée/frottée/frappée) qui résonne ; raideur/tension/usure comme potentiomètres.",
            "",
            ""}},
        {"STRING", {
            "Corde Karplus-Strong étendue — pincée/archet/marteau dans une ligne à retard avec amortissement et matière.",
            "",
            ""}},
        {"DRUM", {
            "Voix de percussion — corps avec balayage de hauteur + clic d’attaque ; une carte 808 ↔ 909 ↔ acoustique, et ROLL.",
            "",
            ""}},
        {"SIGNAL-IN", {
            "L’entrée du monde — audio en direct (ALSA) + MIDI dans un seul adaptateur ; voix monophonique last-note avec sorties pitch/gate/vel/cc.",
            "C’est la contrepartie d’ENTRÉE de NOTE-OUT. Sans rien de branché, silence déterministe.",
            ""}},
        {"FILTER", {
            "Filtre multimode résonant — LP/BP/HP, la résonance allant jusqu’à l’auto-oscillation (il devient un sinus).",
            "Retire de l’énergie à un son riche (soustractif) ; WASP distord en chemin, LPG referme avec un vactrol.",
            "ENVELOPPE → cutoff_mod : le « wah » classique. RES près du maximum + cutoff bas = une voix de sinus accordable."}},
        {"FORMANT", {
            "Banc de 5 passe-bande parallèles — les résonances des voyelles ; VOWEL fait le morphing A→E→I→O→U. VOCODER>0 + le jack MOD = un vocodeur à 5 bandes (les bandes suivent l’enveloppe du modulateur au lieu de la table de voyelles).",
            "Le modèle source-filtre de Fant ; en mode vocodeur, le canal de Dudley (1938).",
            "Sur du bruit ou des impulsions : parole synthétique. Sur un accord : un chœur. Voix dans MOD + scie dans IN + VOCODER=1 : le synthé « parle ». Pour une parole intelligible à large bande, le VOCODER dédié."}},
        {"VOCODER", {
            "Vocodeur à N bandes (4–20) — l’énergie du MODULATEUR par bande commande le gain de cette même bande dans la PORTEUSE : la porteuse « parle » le modulateur. Homer Dudley, 1938.",
            "FORMANT a un mode vocodeur à 5 bandes (vocalique) ; celui-ci est le dédié, à large bande (parole intelligible). SIBIL laisse passer les fricatives ; FREEZE = un pad parlé infini ; SHIFT = décalage de formants.",
            "Voix dans MOD + scie ou pad dans CAR + BANDS=16 = le classique. Sans CAR, la scie interne s’accorde par PIT."}},
        {"RESONATOR", {
            "Banc de 24 modes au plus ACCORDÉS sur une série, excité par un signal EXTERNE (vous frappez avec ce que vous voulez). Rings/Elements en mode résonateur.",
            "MATTER est une voix fermée (son propre exciteur) ; ici vous apportez l’excitation. Les 3 sorties LOW/MID/HIGH se CROISENT quand vous balayez TILT — la relation entre elles est le processus (Three Sisters).",
            "DRUM → IN (un marimba joué par la grosse caisse) ; TRIGSEQ → STRK ; RESONATOR.low et .high vers des destinations différentes."}},
        {"WASP", {
            "Filtre-distorsion — le caractère sale et criard de l’EDP Wasp ; la non-linéarité est DANS le chemin du filtre.",
            "",
            ""}},
        {"LPG", {
            "Low-pass gate — filtre + VCA en une seule commande, avec l’asymétrie du vactrol (attaque rapide, queue lente). Le « bongo » Buchla.",
            "",
            "Un ping avec un trigger court : le LPG donne le corps et la décroissance d’un seul geste."}},
        {"VCA", {
            "Amplificateur commandé en tension — le volume/niveau comme CV ; réponse linéaire ou exponentielle.",
            "",
            ""}},
        {"VCA4", {
            "Banc de 4 VCA plus un mélangeur sommé (MIX). `VCA` (#20) est double ; celui-ci est le banc pour un grand patch — Mutable Veils / Intellijel Quad VCA.",
            "La CURVE linéaire/exp est PARTAGÉE. Chaque canal : LVL + CV atténuversée (sommée au port). MIXG donne le gain de la somme.",
            "Les 3 sorties low/mid/high du RESONATOR/FILTER dans 3 canaux + une enveloppe sur chaque CV = un spectre qui bouge ; MIX rassemble le tout."}},
        {"SHAPE", {
            "Waveshaper — replie/sature/redresse la forme d’onde pour créer des harmoniques ; de la chaleur subtile à la destruction.",
            "",
            ""}},
        {"SHIFTER", {
            "Décaleur de fréquence — déplace le spectre ENTIER de tant de Hz (pas d’un rapport). Les partiels cessent d’être harmoniques → métallique, de cloche. BLU : sorties `up` (+Δf) et `down` (−Δf).",
            "Le ring-mod de SHAPE donne LES DEUX bandes symétriques ; le SHIFTER n’en donne qu’une par sortie. FEEDBACK = le « barber pole » de Risset (glissando infini). Frequency shifter Bode/Moog.",
            "SHIFT ~8 Hz = une phase qui ne se referme jamais ; grand SHIFT + FEEDBACK = la spirale de Shepard ; `up`/`down` vers des destinations différentes."}},
        {"CRUSH", {
            "Destructeur lo-fi / décimateur — la saleté NUMÉRIQUE : réduction de fréquence d’échantillonnage (aliasing), de bits, débordement qui s’enroule, glitch (dropout/bloqué/bégaiement) et gigue d’horloge.",
            "C’est ici qu’habite le verbe DAMAGE. SHAPE/WASP distordent de façon ANALOGIQUE (fold, tanh) ; ici c’est de la quantification, du débordement d’entier, une mauvaise connexion. Tout est semé → dommage reproductible.",
            "ENVELOPPE → BITS (la résolution tombe dans la queue) ; LFO → RATE ; sans IN + MIX=1 = une source de bruit lo-fi."}},
        {"PARAMETRIC", {
            "Égaliseur paramétrique — bandes en cloche et en plateau pour sculpter le spectre chirurgicalement, sans résonance de filtre.",
            "",
            ""}},
        {"GLIDE", {
            "Portamento/slew — glisse entre les valeurs au lieu de sauter ; temps séparé pour la montée et la descente, mode constant ou proportionnel.",
            "",
            ""}},
        {"CONTROL", {
            "Utilitaires de CV continue — atténuverseur, décalage, redressement en interpolation, slew. Le couteau de poche du signal de commande.",
            "",
            ""}},
        {"ENVELOPE", {
            "Générateur d’enveloppe — AD/ADSR/AR et boucles ; la forme de l’amplitude (ou de n’importe quelle CV) dans le temps, déclenchée par un gate.",
            "",
            "ENVELOPPE → VCA + ENVELOPPE → cutoff : le contour du son."}},
        {"FUNCTION", {
            "Générateur de fonction à la Maths — rampe/enveloppe/LFO déformable avec sorties dérivées ; il somme, intègre, déclenche.",
            "C’est une enveloppe ET un LFO ET un slew, tout dans la courbe. DRIFT est une marche lente ; CHAOS est vraiment chaotique.",
            ""}},
        {"STAGES", {
            "Générateur de N segments configurables (2–8) dont la FONCTION ÉMERGE de la façon dont ils s’enchaînent : rampes → enveloppe/LFO, marches → une séquence de CV. Mutable Stages / Rossum Control Forge.",
            "FUNCTION est UNE rampe ; STAGES est la forme COMPOSÉE, sculptée par CONTOUR/TILT/HOLD (un générateur, pas un éditeur). HOLD=0 glisse (env), HOLD=1 saute (séq) ; LOOP tourne ↔ déclenche.",
            "STAGES → FILTER.cutoff ; CLOCK → GATE (loop off) = une enveloppe ; STAGES.step → ENVELOPE.gate les enchaîne."}},
        {"DRIFT", {
            "Champ de dérive organique — plusieurs CV corrélées qui errent lentement (à l’échelle des minutes), avec inertie et ancrage à des repères enregistrés.",
            "Il donne de la VIE sans direction — rien ne reste immobile. Déterministe (semé).",
            ""}},
        {"CHAOS", {
            "Champ chaotique à double puits — une EDO déterministe qui se pose dans un puits, oscille entre deux, ou « chasse » de façon imprévisible.",
            "Vraiment chaotique, ni pseudo-aléatoire (DECISION/TURING) ni marche filtrée (DRIFT).",
            ""}},
        {"SH", {
            "Sample & hold double — retient une valeur sur une impulsion ; 2 canaux avec CORRELATION (jumeaux ↔ miroir ↔ indépendants) et slew.",
            "",
            "CLOCK → SH · out1 → pitch · out2 → cutoff : une mélodie et un timbre qui « vont ensemble »."}},
        {"CLOCK", {
            "Horloge maîtresse euclidienne — une impulsion avec divisions, remplissage euclidien, swing et dérive ; le battement de tout le patch.",
            "",
            "CLOCK → tout ce qui a un gate ou un trigger. FILL répartit les impulsions dans la mesure (les motifs de Toussaint)."}},
        {"LOGIC", {
            "Logique d’horloge — divise/multiplie, AND/OR/XOR de gates, retard ; elle recombine le TEMPS.",
            "Le pendant d’ABACUS (qui fait l’arithmétique de CV). Ici, uniquement gates et triggers.",
            ""}},
        {"TURING", {
            "Registre à décalage aléatoire (Turing Machine) — une boucle de bits que LOCK retient et que MUT rebat ; mélodie/rythme qui se répète avec variation.",
            "",
            ""}},
        {"SEQUENCE", {
            "Séquenceur à 8 pas — curseurs de hauteur + direction (avant/arrière/ping-pong/aléatoire/brownien), avec un gate par pas.",
            "",
            "Les curseurs sont la partition — la Motion Engine y touche à peine. Utilisez CHANGER/ÉVOLUE pour rebattre exprès."}},
        {"TRIGSEQ", {
            "Séquenceur de TRIGGERS — 4 pistes de densité euclidienne avec chaos/ratchet/fill/swing ; la percussion générative.",
            "",
            ""}},
        {"QUANTIZER", {
            "Quantificateur de gamme — cale une CV sur les degrés d’une gamme et d’une tonique ; slew et hystérésis pour qu’elle ne tremble pas à la frontière.",
            "",
            "SH/TURING/DRIFT → QUANTIZER → pitch : une marche aléatoire devient une mélodie dans la gamme."}},
        {"HARMONY", {
            "Harmoniseur — d’une seule note il engendre des intervalles et des accords cohérents avec une tonalité et une conduite des voix.",
            "",
            ""}},
        {"ABACUS", {
            "Arithmétique de CV — addition/soustraction/multiplication/modulo et AND/OR/XOR bit à bit entre deux signaux, plus un compteur binaire dont les bits deviennent du rythme (Numeric Repetitor).",
            "Le pendant de LOGIC (qui recombine le temps). Seul, le compteur joue déjà une séquence et un rythme.",
            ""}},
        {"DECISION", {
            "Source de hasard STRUCTURÉ, à la Marbles — distribution (bias/spread/shape), déjà-vu (répète le motif), steps et slew ; elle fait des CV et des gates avec une « intention ».",
            "",
            ""}},
        {"BOXCAR", {
            "Moyenneur à porte (boxcar averager) — une fenêtre échantillonne un point de la période, et la moyenne de N captures révèle le signal dans le bruit ; SCAN reconstruit l’onde ; GEIG = gates de Poisson.",
            "C’est le SCOPE à l’envers : il mesure pour CONSTRUIRE une onde. mode 0 = S&H fenêtré, 1 = reconstruit, 2 = relit son propre tampon comme oscillateur.",
            "Du bruit plus un ton faible synchronisé au TRIG, mode 1, SCAN élevé : le ton « se dessine » au fil des secondes."}},
        {"SWITCH", {
            "Commutateur/routeur — N entrées → 1 sortie (ou 1 → N), par pas d’horloge, CV ou au hasard ; il séquence des SOURCES au lieu de notes.",
            "",
            ""}},
        {"MATRIX", {
            "Matrice de routage 4×4 — n’importe quelle entrée vers n’importe quelle sortie avec un gain par cellule ; un mélangeur CV/audio programmable.",
            "",
            ""}},
        {"MULT", {
            "Multiple bufferisé — copie un signal vers plusieurs destinations sans perte de niveau ; mode double et slew optionnel.",
            "",
            ""}},
        {"PLANAR", {
            "Morphing vectoriel XY — glisse entre 4 sources sur un plan ; le geste (le chemin sur le plan) s’enregistre et se rejoue.",
            "",
            ""}},
        {"SPACE", {
            "Réverbération — un réseau Schroeder/Dattorro multitap, de l’ambiance courte à la longue queue ; pre-delay et amortissement.",
            "SPACE est la réverbe « classique » ; HALL est un FDN à 8 lignes (plus dense, stéréo) ; MEMORY est granulaire.",
            ""}},
        {"HALL", {
            "Réverbe FDN à 8 lignes avec matrice de Householder — une queue dense et stéréo, avec FREEZE (maintien infini) et pre-delay.",
            "",
            ""}},
        {"LOOPER", {
            "Ligne à retard avec HOLD (gèle et répète), REVERSE (lit à l’envers sans clic), AGE (caractère bande/BBD) et HEADS (écho multi-têtes, façon Space Echo).",
            "L’ancien « TAPE » est devenu ce HEADS. FBK près de 1 + un TIME long + plusieurs têtes = Frippertronics.",
            ""}},
        {"SWIRL", {
            "Effets de MODULATION — chorus, flanger, ensemble et phaser dans un seul module (TYPE). Des retards COURTS modulés par LFO (plus le phaser, une cascade de passe-tout), avec un caractère BBD dans AGE.",
            "RASGO n’en avait aucun. Distinct du LOOPER (ligne à retard, échos audibles) et de la réverbe — ici on entend le MOUVEMENT, pas l’écho.",
            "voix → SWIRL → MASTER épaissit ; LFO → RATE laisse le balayage respirer hors mesure ; FBK près de ±1 + AGE = il auto-oscille."}},
        {"MEMORY", {
            "Granulaire — un tampon découpé en grains (grain/nuage/spray), avec position, taille, densité et FREEZE ; textures et time-stretch.",
            "",
            ""}},
        {"SAMPLER", {
            "Lecteur de tranches (MPC/Akai) — TRIG déclenche un passage enregistré en direct (gate REC) ou depuis un fichier ; varispeed, tranches + POS, repitch, usure.",
            "Le SAMPLER saute la position de lecture (une tranche) ; le TURNTABLE lit en continu avec inertie ; MEMORY est granulaire.",
            ""}},
        {"TURNTABLE", {
            "Tourne-disque à plateau qui a une masse — le tampon du SAMPLER lu par une vitesse angulaire avec INERTIE : le moteur tire lentement (TORQ), la main pousse (SCR), le frein laisse filer (BRK).",
            "AUCUNE quantification de BPM — le beatmatch est un geste, comme sur vinyle.",
            "LFO → SCR : le disque scratche dans la mesure. ENVELOPPE → BRK : un tape-stop au retournement."}},
        {"MIXER", {
            "Mélangeur 4 canaux — niveau (curseur), panoramique et mute par canal, plus une sortie générale. Là où le musicien fait l’équilibre.",
            "Exempt de la Motion Engine — les panoramiques et les niveaux sont à vous.",
            ""}},
        {"MASTER", {
            "Étage de sortie stéréo — largeur, garde de corps et limiteur true-peak ; le plafond du patch.",
            "Exempt de la Motion Engine. Le bouton VEILLE de l’en-tête est le mute de ce module.",
            ""}},
        {"SCOPE", {
            "Oscilloscope et mesureur qui REND en CV — hauteur (YIN), enveloppe, centroïde spectral, ONSET (une impulsion à l’attaque) ; le patch s’entend lui-même.",
            "Le SCOPE mesure pour réduire (→ un scalaire) ; le BOXCAR mesure pour reconstruire (→ une onde).",
            "SCOPE.onset ← batterie → déclenche une ENVELOPPE au rythme de l’audio ; SCOPE.pitch → OSC.pitch accorde sur ce qui entre."}},
        {"NOTE-OUT", {
            "Sortie MIDI — convertit gate + 1 V/oct en note on/off, avec canal, vélocité et un journal de partition.",
            "La contrepartie de SORTIE de SIGNAL-IN.",
            ""}},
    };
    return table;
}

inline const std::unordered_map<std::string, LearnEntry>& moduleLearnTableEs() {
    static const std::unordered_map<std::string, LearnEntry> table = {
        {"OSC", {
            "Oscilador sustractivo — la voz de partida. Diente/cuadrada/triángulo/seno con PW y deriva orgánica; sigue 1 V/oct.",
            "Es la fuente «clásica»; WAVETABLE barre tablas, ADDITIVE suma parciales, OPERATOR hace FM.",
            "OSC → FILTER → VCA con una ENVOLVENTE en el gate: la cadena mínima de un sintetizador."}},
        {"WAVETABLE", {
            "Oscilador de tabla de ondas — un POS barre un banco de formas de onda, de suave a espectral, con antialias.",
            "",
            "LFO lento → POS: el timbre «respira» entre las ondas."}},
        {"ADDITIVE", {
            "Oscilador aditivo — suma hasta 64 parciales; TILT/ODD-EVEN/STRETCH esculpen el espectro directamente.",
            "Construye el timbre sumando senos, lo contrario del FILTER (que quita de un sonido rico).",
            ""}},
        {"OPERATOR", {
            "FM de 4 operadores, 8 algoritmos, feedback estilo DX7 — metales, campanas y bajos que el sustractivo no hace.",
            "",
            "RATIO entero = armónico; fraccionario = inarmónico (campana)."}},
        {"PULSAR", {
            "Síntesis pulsar (Curtis Roads) — un tren de pulsarets (grano + silencio) con DOS frecuencias independientes: FREQ es la altura, FORMANT el timbre. Entre lo granular y la síntesis por formantes.",
            "OPERATOR/ADDITIVE fijan el espectro a la altura; aquí FORMANT desliza la envolvente espectral SIN desafinar. MASK/JITTER ralean y desalinean el tren (patrones burlescos, sembrados).",
            "LFO → FQM: un «wah» sin filtro. MASK ~0,7 + JITTER ~0,4 = una nube rítmica irregular sobre la misma nota."}},
        {"SPECTRA", {
            "Resíntesis espectral — ESCUCHA un sonido, halla los parciales más fuertes y vuelve a oscilar como un banco de senos que lo sigue. El puente análisis → síntesis (Panharmonium / vocoder de fase).",
            "ADDITIVE construye el espectro desde cero; MEMORY granula en el tiempo; aquí la materia es el espectro de corto plazo de la entrada. FREEZE = un pad infinito de cualquier sonido. Sin IN, un ruido sembrado alimenta el análisis → un drone que evoluciona solo.",
            "DRUM → IN, VOICE bajo, STRETCH = una sombra de campanas del ritmo; FRZ ← gate = espectro congelado alternando con el vivo."}},
        {"PLL", {
            "Lazo de enganche de fase — se engancha a la fase de una entrada y genera divisiones y multiplicaciones de ella; libre, auto-oscila.",
            "Es oscilador Y extractor de reloj — sigue lo que oye, dentro de una ventana de captura.",
            ""}},
        {"CHORD", {
            "Voz de acorde — 3 o 4 osciladores afinados por un intervalo/inversión; entra una nota, sale un acorde.",
            "",
            ""}},
        {"NOISE", {
            "Ruido y azar continuo — blanco/rosa/brown/azul/violeta/bit + S&H + una tensión que pasea, 8 salidas a la vez.",
            "DECISION decide eventos; NOISE da el suelo y las CV aleatorias. POIS cambia el reloj interno por un Poisson libre.",
            ""}},
        {"MATTER", {
            "Voz de modelado físico — materia excitada (soplada/frotada/golpeada) resonando; rigidez/tensión/desgaste como mandos.",
            "",
            ""}},
        {"STRING", {
            "Cuerda Karplus-Strong extendida — pulsada/con arco/con martillo en una línea de retardo con amortiguación y material.",
            "",
            ""}},
        {"DRUM", {
            "Voz de percusión — cuerpo con barrido de altura + chasquido de ataque; un mapa 808 ↔ 909 ↔ acústico, y ROLL.",
            "",
            ""}},
        {"SIGNAL-IN", {
            "La entrada del mundo — audio en vivo (ALSA) + MIDI en un solo adaptador; voz monofónica last-note con salidas pitch/gate/vel/cc.",
            "Es la contraparte de ENTRADA de NOTE-OUT. Sin nada conectado, silencio determinista.",
            ""}},
        {"FILTER", {
            "Filtro multimodo resonante — LP/BP/HP, con la resonancia llegando a la auto-oscilación (se vuelve un seno).",
            "Quita energía a un sonido rico (sustractivo); WASP distorsiona en el camino, LPG cierra con un vactrol.",
            "ENVOLVENTE → cutoff_mod: el «wah» clásico. RES cerca del tope + cutoff bajo = una voz de seno afinable."}},
        {"FORMANT", {
            "Banco de 5 pasabanda en paralelo — las resonancias de las vocales; VOWEL hace el morfismo A→E→I→O→U. VOCODER>0 + el jack MOD = un vocoder de 5 bandas (las bandas siguen la envolvente del modulador en vez de la tabla de vocales).",
            "El modelo fuente-filtro de Fant; en modo vocoder, el canal de Dudley (1938).",
            "Sobre ruido o pulsos: habla sintética. Sobre un acorde: un coro. Voz en MOD + diente en IN + VOCODER=1: el sinte «habla». Para habla inteligible de banda ancha, el VOCODER dedicado."}},
        {"VOCODER", {
            "Vocoder de N bandas (4–20) — la energía del MODULADOR por banda controla la ganancia de esa misma banda en la PORTADORA: la portadora «habla» el modulador. Homer Dudley, 1938.",
            "FORMANT tiene un modo vocoder de 5 bandas (vocálico); éste es el dedicado, de banda ancha (habla inteligible). SIBIL pasa las fricativas; FREEZE = un pad hablado infinito; SHIFT = desplazamiento de formantes.",
            "Voz en MOD + diente o pad en CAR + BANDS=16 = el clásico. Sin CAR, el diente interno se afina con PIT."}},
        {"RESONATOR", {
            "Banco de hasta 24 modos AFINADOS a una serie, excitado por una señal EXTERNA (usted golpea con lo que quiera). Rings/Elements en modo resonador.",
            "MATTER es una voz cerrada (con su propio excitador); aquí usted trae la excitación. Las 3 salidas LOW/MID/HIGH se CRUZAN al barrer TILT — la relación entre ellas es el proceso (Three Sisters).",
            "DRUM → IN (una marimba tocada por el bombo); TRIGSEQ → STRK; RESONATOR.low y .high a destinos distintos."}},
        {"WASP", {
            "Filtro-distorsión — el carácter sucio y chillón del EDP Wasp; la no linealidad está EN el camino del filtro.",
            "",
            ""}},
        {"LPG", {
            "Low-pass gate — filtro + VCA en un solo mando, con la asimetría del vactrol (ataque rápido, cola lenta). El «bongó» Buchla.",
            "",
            "Un ping con un trigger corto: el LPG da el cuerpo y la caída de una vez."}},
        {"VCA", {
            "Amplificador controlado por tensión — el volumen/nivel como CV; respuesta lineal o exponencial.",
            "",
            ""}},
        {"VCA4", {
            "Banco de 4 VCA más un mezclador sumado (MIX). `VCA` (#20) es doble; éste es el banco para un patch grande — Mutable Veils / Intellijel Quad VCA.",
            "La CURVE lineal/exp es COMPARTIDA. Cada canal: LVL + CV atenuvertida (sumada en el puerto). MIXG da la ganancia de la suma.",
            "Las 3 salidas low/mid/high del RESONATOR/FILTER en 3 canales + una envolvente en cada CV = un espectro que se mueve; MIX lo junta todo."}},
        {"SHAPE", {
            "Waveshaper — pliega/satura/rectifica la forma de onda para crear armónicos; de un calor sutil a la destrucción.",
            "",
            ""}},
        {"SHIFTER", {
            "Desplazador de frecuencia — mueve el espectro ENTERO tantos Hz (no por una razón). Los parciales dejan de ser armónicos → metálico, de campana. BLU: salidas `up` (+Δf) y `down` (−Δf).",
            "El ring-mod de SHAPE da LAS DOS bandas simétricas; el SHIFTER da una por salida. FEEDBACK = el «barber pole» de Risset (glissando infinito). Frequency shifter Bode/Moog.",
            "SHIFT ~8 Hz = una fase que nunca cierra; SHIFT grande + FEEDBACK = la espiral de Shepard; `up`/`down` a destinos distintos."}},
        {"CRUSH", {
            "Destructor lo-fi / diezmador — la basura DIGITAL: reducción de frecuencia de muestreo (aliasing), de bits, desborde que se envuelve, glitch (dropout/trabado/tartamudeo) y jitter de reloj.",
            "Aquí vive el verbo DAMAGE. SHAPE/WASP distorsionan de forma ANALÓGICA (fold, tanh); aquí es cuantización, desborde de entero, una conexión mala. Todo sembrado → daño reproducible.",
            "ENVOLVENTE → BITS (la resolución cae en la cola); LFO → RATE; sin IN + MIX=1 = una fuente de ruido lo-fi."}},
        {"PARAMETRIC", {
            "Ecualizador paramétrico — bandas de campana y de estante para esculpir el espectro con precisión, sin resonancia de filtro.",
            "",
            ""}},
        {"GLIDE", {
            "Portamento/slew — desliza entre valores en vez de saltar; tiempo separado para la subida y la bajada, modo constante o proporcional.",
            "",
            ""}},
        {"CONTROL", {
            "Utilidades de CV continua — atenuversor, desplazamiento, rectificación como interpolación, slew. La navaja del señal de control.",
            "",
            ""}},
        {"ENVELOPE", {
            "Generador de envolvente — AD/ADSR/AR y bucles; la forma de la amplitud (o de cualquier CV) en el tiempo, disparada por un gate.",
            "",
            "ENVOLVENTE → VCA + ENVOLVENTE → cutoff: el contorno del sonido."}},
        {"FUNCTION", {
            "Generador de función estilo Maths — rampa/envolvente/LFO deformable con salidas derivadas; suma, integra, dispara.",
            "Es envolvente Y LFO Y slew, todo en la curva. DRIFT es un paseo lento; CHAOS es de verdad caótico.",
            ""}},
        {"STAGES", {
            "Generador de N segmentos configurables (2–8) cuya FUNCIÓN EMERGE de cómo se encadenan: rampas → envolvente/LFO, escalones → una secuencia de CV. Mutable Stages / Rossum Control Forge.",
            "FUNCTION es UNA rampa; STAGES es la forma COMPUESTA, esculpida por CONTOUR/TILT/HOLD (un generador, no un editor). HOLD=0 desliza (env), HOLD=1 salta (sec); LOOP corre ↔ dispara.",
            "STAGES → FILTER.cutoff; CLOCK → GATE (loop off) = una envolvente; STAGES.step → ENVELOPE.gate los encadena."}},
        {"DRIFT", {
            "Campo de deriva orgánica — varias CV correlacionadas que pasean despacio (en escala de minutos), con inercia y anclaje a marcas grabadas.",
            "Da VIDA sin dirección — nada se queda quieto. Determinista (sembrado).",
            ""}},
        {"CHAOS", {
            "Campo caótico de doble pozo — una EDO determinista que se asienta en un pozo, oscila entre dos, o «caza» de forma imprevisible.",
            "Genuinamente caótico, ni pseudoaleatorio (DECISION/TURING) ni paseo filtrado (DRIFT).",
            ""}},
        {"SH", {
            "Sample & hold doble — retiene un valor en el pulso; 2 canales con CORRELATION (gemelos ↔ espejo ↔ independientes) y slew.",
            "",
            "CLOCK → SH · out1 → pitch · out2 → cutoff: una melodía y un timbre que «combinan»."}},
        {"CLOCK", {
            "Reloj maestro euclidiano — un pulso con divisiones, relleno euclidiano, swing y deriva; el latido de todo el patch.",
            "",
            "CLOCK → todo lo que tenga gate o trigger. FILL reparte los pulsos por el compás (los patrones de Toussaint)."}},
        {"LOGIC", {
            "Lógica de reloj — divide/multiplica, AND/OR/XOR de gates, retardo; recombina el TIEMPO.",
            "La pareja de ABACUS (que hace aritmética de CV). Aquí sólo gates y triggers.",
            ""}},
        {"TURING", {
            "Registro de desplazamiento aleatorio (Turing Machine) — un bucle de bits que LOCK retiene y MUT vuelve a mezclar; melodía/ritmo que se repite con variación.",
            "",
            ""}},
        {"SEQUENCE", {
            "Secuenciador de 8 pasos — deslizadores de altura + dirección (adelante/atrás/ping-pong/aleatorio/browniano), con un gate por paso.",
            "",
            "Los deslizadores son la partitura — la Motion Engine casi no los toca. Use CAMBIA/EVOLUCIONA para remezclar a propósito."}},
        {"TRIGSEQ", {
            "Secuenciador de TRIGGERS — 4 pistas de densidad euclidiana con chaos/ratchet/fill/swing; la percusión generativa.",
            "",
            ""}},
        {"QUANTIZER", {
            "Cuantizador de escala — encaja una CV en los grados de una escala y tónica; slew e histéresis para que no tiemble en el borde.",
            "",
            "SH/TURING/DRIFT → QUANTIZER → pitch: un paseo aleatorio se vuelve melodía en la escala."}},
        {"HARMONY", {
            "Armonizador — de una nota genera intervalos y acordes coherentes con una tonalidad y una conducción de voces.",
            "",
            ""}},
        {"ABACUS", {
            "Aritmética de CV — suma/resta/multiplicación/módulo y AND/OR/XOR bit a bit entre dos señales, más un contador binario cuyos bits se vuelven ritmo (Numeric Repetitor).",
            "La pareja de LOGIC (que recombina el tiempo). Por sí solo, el contador ya toca una secuencia y un ritmo.",
            ""}},
        {"DECISION", {
            "Fuente de azar ESTRUCTURADO, estilo Marbles — distribución (bias/spread/shape), déjà-vu (repite el patrón), steps y slew; hace CV y gates con «intención».",
            "",
            ""}},
        {"BOXCAR", {
            "Promediador de puerta (boxcar averager) — una ventana muestrea un punto del período, y el promedio de N capturas revela la señal dentro del ruido; SCAN reconstruye la onda; GEIG = gates de Poisson.",
            "Es el SCOPE al revés: mide para CONSTRUIR una onda. mode 0 = S&H de ventana, 1 = reconstruye, 2 = relee su propio búfer como oscilador.",
            "Ruido más un tono débil sincronizado al TRIG, mode 1, SCAN alto: el tono «se dibuja» a lo largo de segundos."}},
        {"SWITCH", {
            "Conmutador/enrutador — N entradas → 1 salida (o 1 → N), por paso de reloj, CV o al azar; secuencia FUENTES en vez de notas.",
            "",
            ""}},
        {"MATRIX", {
            "Matriz de enrutamiento 4×4 — cualquier entrada a cualquier salida con una ganancia por celda; un mezclador de CV/audio programable.",
            "",
            ""}},
        {"MULT", {
            "Múltiple con búfer — copia una señal a varios destinos sin caída de nivel; modo doble y slew opcional.",
            "",
            ""}},
        {"PLANAR", {
            "Morfismo vectorial XY — desliza entre 4 fuentes en un plano; el gesto (el camino en el plano) se graba y se reproduce.",
            "",
            ""}},
        {"SPACE", {
            "Reverberación — una red Schroeder/Dattorro multitap, de una ambiencia corta a una cola larga; pre-delay y amortiguación.",
            "SPACE es la reverb «clásica»; HALL es un FDN de 8 líneas (más densa, estéreo); MEMORY es granular.",
            ""}},
        {"HALL", {
            "Reverb FDN de 8 líneas con matriz de Householder — una cola densa y estéreo, con FREEZE (retención infinita) y pre-delay.",
            "",
            ""}},
        {"LOOPER", {
            "Línea de retardo con HOLD (congela y repite), REVERSE (lee al revés sin clic), AGE (carácter de cinta/BBD) y HEADS (eco multicabeza, estilo Space Echo).",
            "El viejo «TAPE» se volvió este HEADS. FBK cerca de 1 + un TIME largo + varias cabezas = Frippertronics.",
            ""}},
        {"SWIRL", {
            "Efectos de MODULACIÓN — chorus, flanger, ensemble y phaser en un solo módulo (TYPE). Retardos CORTOS modulados por LFO (más el phaser, una cascada de pasa-todo), con carácter BBD en AGE.",
            "RASGO no tenía ninguno. Distinto del LOOPER (línea de retardo, ecos audibles) y de la reverb — aquí se oye el MOVIMIENTO, no el eco.",
            "voz → SWIRL → MASTER engorda; LFO → RATE deja que el barrido respire fuera del compás; FBK cerca de ±1 + AGE = auto-oscila."}},
        {"MEMORY", {
            "Granular — un búfer picado en granos (grano/nube/spray), con posición, tamaño, densidad y FREEZE; texturas y time-stretch.",
            "",
            ""}},
        {"SAMPLER", {
            "Reproductor de rodajas (MPC/Akai) — TRIG dispara un pasaje grabado en vivo (gate REC) o de archivo; varispeed, rodajas + POS, repitch, desgaste.",
            "El SAMPLER salta la posición de lectura (una rodaja); el TURNTABLE lee continuo con inercia; MEMORY es granular.",
            ""}},
        {"TURNTABLE", {
            "Tocadiscos con un plato que tiene masa — el búfer del SAMPLER leído por una velocidad angular con INERCIA: el motor tira despacio (TORQ), la mano empuja (SCR), el freno lo deja rodar (BRK).",
            "SIN cuantización de BPM — el beatmatch es un gesto, como en el vinilo.",
            "LFO → SCR: el disco rasca en compás. ENVOLVENTE → BRK: un tape-stop en la vuelta."}},
        {"MIXER", {
            "Mezclador de 4 canales — nivel (deslizador), paneo y mute por canal, más una salida general. Donde el músico hace el balance.",
            "Exento de la Motion Engine — los paneos y los niveles son suyos.",
            ""}},
        {"MASTER", {
            "Etapa de salida estéreo — anchura, guarda de cuerpo y limitador true-peak; el techo del patch.",
            "Exento de la Motion Engine. El botón ESPERA del encabezado es el mute de este módulo.",
            ""}},
        {"SCOPE", {
            "Osciloscopio y medidor que DEVUELVE como CV — altura (YIN), envolvente, centroide espectral, ONSET (un pulso en el ataque); el patch se oye a sí mismo.",
            "El SCOPE mide para reducir (→ un escalar); el BOXCAR mide para reconstruir (→ una onda).",
            "SCOPE.onset ← batería → dispara una ENVOLVENTE al compás del audio; SCOPE.pitch → OSC.pitch afina por lo que entra."}},
        {"NOTE-OUT", {
            "Salida MIDI — convierte gate + 1 V/oct en note on/off, con canal, velocidad y un registro de partitura.",
            "La contraparte de SALIDA de SIGNAL-IN.",
            ""}},
    };
    return table;
}

}  // namespace detail

// Verbete de MÓDULO no idioma pedido, caindo no português quando faltar.
//
// A queda é deliberada e não é preguiça: o português é a língua em que o
// catálogo foi escrito e revisado, e um verbete faltando numa tradução
// deve mostrar o texto CERTO em outra língua, não uma caixa vazia. Só o
// nível de módulo está traduzido; os verbetes de widget continuam em
// português — declarado em `PUBLICACAO.md`.
inline const LearnEntry* lookupLearnModule(const std::string& moduleType,
                                           const Lang lang = Lang::pt) {
    const std::string key =
        (moduleType == "AUDIO-IN") ? std::string("SIGNAL-IN") : moduleType;

    if (lang != Lang::pt) {
        const auto& t = (lang == Lang::en) ? detail::moduleLearnTableEn()
                      : (lang == Lang::fr) ? detail::moduleLearnTableFr()
                                           : detail::moduleLearnTableEs();
        const auto i = t.find(key);
        if (i != t.end() && !i->second.quick.empty()) return &i->second;
    }
    const auto& table = detail::moduleLearnTable();
    const auto it = table.find(key);
    return it == table.end() ? nullptr : &it->second;
}

}  // namespace rasgo::panel

// Os lotes de tradução dos verbetes de widget vivem em arquivo próprio —
// eles crescem muito, e misturá-los aqui tornaria este catálogo ilegível.
// A inclusão vai no FIM de propósito: o arquivo de lotes precisa de
// `LearnEntry`, de `LearnTable` e das tabelas mutáveis já declaradas.
#include "panel/LearnWidgetI18n.hpp"
