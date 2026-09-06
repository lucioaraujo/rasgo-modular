#pragma once

#include <string>
#include <unordered_map>

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

namespace detail {
using LearnTable =
    std::unordered_map<std::string, std::unordered_map<std::string, LearnEntry>>;

inline const LearnTable& learnTable() {
    static const LearnTable table = {
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
            {"mix", {"Seco ↔ ressoado. 0 = passa-direto.", "", ""}},
            {"drift", {
                "Cada formante ganha um wobble lento e independente "
                "(±drift·3 %) — a voz respira. Determinístico (senos, "
                "sem RNG). 0 = estático.", "", ""}},
            {"in:in", {"Áudio a ressoar (um OSC, NOISE, qualquer voz).",
                      "", ""}},
            {"in:vowel", {"CV somada em VOWEL (LFO, SEQUENCE, envelope…).",
                         "", ""}},
            {"in:shift", {"CV somada em SHIFT.", "", ""}},
            {"out:out", {
                "Seco + as 5 bandas, misturados por MIX, com limitador "
                "suave.", "", ""}},
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
                "o dobro de lado.", "", ""}},
            {"mono", {"Força L=R=(L+R)/2 — checagem de compatibilidade "
                      "mono.", "", ""}},
            {"dc_block", {
                "Bloqueio de DC (passa-alta ~5 Hz) — tira offset que "
                "não é som, só aqueceria o limitador à toa.", "", ""}},
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
        {"AUDIO-IN", {
            {"gain", {"Ganho aplicado à entrada de áudio ao vivo.", "", ""}},
            {"out:out", {
                "O que estiver tocando na entrada de áudio do sistema, ao "
                "vivo — sem nada capturando, fica em silêncio (nunca "
                "trava, nunca lê lixo).",
                "Lado receptor de um anel circular SPSC alimentado por uma "
                "thread de captura externa (`AlsaSource`) — o núcleo "
                "`rasgo_modular_core` não sabe o que é ALSA, só lê o anel.",
                "Rode outro instrumento (ou um microfone) e ligue OUT num "
                "FILTER ou SHAPE — o Rasgo passa a processar áudio de "
                "fora, não só ele mesmo."}},
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
                "Altura detectada por período entre cruzamentos de zero, "
                "em 1 V/oct — só trava depois de 3 períodos consistentes "
                "(ruído nunca trava, fica em 0).", "", ""}},
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
            {"in:in", {"Entrada de áudio.", "", ""}},
            {"in:time", {"CV que soma a TIME (em segundos).", "", ""}},
            {"in:freeze", {"Gate: enquanto alto, equivale a HOLD ligado.",
                          "", ""}},
            {"in:rev", {"Gate: enquanto alto, equivale a REV ligado.",
                       "", ""}},
            {"out:out", {"Seco + molhado, misturados por MIX.", "", ""}},
            {"out:wet", {"Só o laço, sem o seco.", "", ""}},
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
inline const LearnEntry* lookupLearn(const std::string& moduleType,
                                     const std::string& bind) {
    const auto& table = detail::learnTable();
    const auto mi = table.find(moduleType);
    if (mi == table.end()) return nullptr;
    const auto pi = mi->second.find(bind);
    if (pi == mi->second.end()) return nullptr;
    return &pi->second;
}

}  // namespace rasgo::panel
