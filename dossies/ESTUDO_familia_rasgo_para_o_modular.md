# Estudo: a família RASGO como fonte para o Modular (2026-11-09)

Pedido do autor (9/11): "veja também nossos instrumentos da família rasgo, o
que temos já codado, os conceitos implícitos, o que ainda não foi utilizado no
modular, o que poderia realmente acrescentar"; com indicações do Batruc
(física), do Asterismus (física, astros, constelações), do Aquorbium (bioma,
fundo do mar) e do Trioio.

Leitura só de documentos e cabeçalhos (sem copiar código). O Batruc está
sendo editado em paralelo pelo autor: só leitura. Regra da família para
reaproveitar: copiar de um commit conhecido, registrar proveniência e licença
e **adaptar** (RASGO/AGENTS.md §4; `GOVERNANCA_E_TRANSVERSALIDADE.md §7.0`).
O Rasgo Synth Studio é GPLv3 e de leitura (o Modular é AGPLv3: combinação
compatível, mas a escolha segue o padrão de reimplementar o conceito).

Fontes consultadas: `RASGO_DOCUMENTATION/architecture/INVENTARIO_GLOBAL_MODULOS.md`;
`BATRUC/MOVIMENTO_GRAFICO_E_SOM.md`, `OPERACOES_GENERATIVAS.md`,
`RITMOS_E_CONTAGENS.md`; `RASGO_SYNTH_ASTERISMUS/include/rasgo/performance/PerformanceSkyDirector.hpp`
e `PerformanceMotionFamily`; `AQUORBIUM/ECOLOGICAL_MUSIC_DESIGN.md`,
`ECOSYSTEM_DESIGN.md`, `GEOMETRIC_SEQUENCER_DESIGN.md`,
`AUTONOMOUS_ENERGY_DESIGN.md`, `AQUARIUM_SPACE_DESIGN.md`;
`TRIOIO/docs/` (MOSAICO, LINHAS, JOGO, ESCALAS) e o `KNOW-ESCALAS` do
inventário; `RASGO_SYNTH_STUDIO/docs/pt/_catalogo_mecanismos.md` (142
mecanismos); `ANTITOTEM/docs/`; READMEs de Maravi, Maestro e Risco.

## 1. O que cada instrumento tem e o Modular ainda não

| Instrumento | Conceito já codado ou documentado | No Modular hoje |
|---|---|---|
| **Asterismus** | gravitação de N corpos (Newton, leapfrog simplético); famílias de movimento da constelação (bilhar, espuma, pêndulo, magnético, gravidade; girar/transladar); **eclipses** pelo mês sinódico × dracônico (dois ciclos que só às vezes se alinham); "tempo-luz" (a resposta atrasa com a distância); atos entre estrelas (resposta, contágio, sombra em aumentação, hoquetus, apoio); progressões neo-riemannianas | nada disso como módulo (o ORBIT aprovado é o lugar natural da gravitação) |
| **Batruc** | **epiciclos** (soma de Fourier desenhada: círculos girando dentro de círculos); pêndulo triplo acoplado por molas; **peneiras (sieves) de Xenakis**, agrupamentos aditivos/aksak, sequências de durações; escolha por **fator de Boltzmann** (temperatura composicional); leis matemáticas de intensidade misturáveis | PENDULUM (duplo, sem molas); TEMP do MARKOV é parente do Boltzmann |
| **Aquorbium** | ecossistema: **predador/presa** (cânone que fecha em stretto; fuga por diminuição e síncope — Lotka–Volterra), **cardume** coeso/disperso (homorritmia/heterofonia), migrante que modula, séssil = pedal, nicho acústico; **dia/noite/estação** escolhem modo e andamento; meio **ar/água**; escuta subaquática em camadas; memória formal Markov de regiões | nada do ecossistema; MARKOV é genérico |
| **Trioio** | **banco de 2576 escalas** com fonte citada (gêneros gregos, temperamentos, maqam, microtons — `KNOW-ESCALAS`); leitura por bandas (já virou o LISTEN); **mosaico** (memória pesquisável por timbre — síntese concatenativa); máquina de harmonia; o jogo de Vitoriamario (regras como objetos) | LISTEN; QUANTIZER com 12 escalas |
| **Rasgo Synth Studio** | 142 mecanismos: **GENDYN** (síntese estocástica de Xenakis), **Rungler** (Hordijk), oscilador **Walsh**, **VOSIM**, **AY-3-8910**, **Atari Punk Console**, **SieveScale** e polifonia de peneiras, **Bloom** (melodia fractal), **Agon/Atropelo** (vozes que disputam e interrompem), **Heisenberg** (ritmo ↔ afinação inversos), morte térmica, sinal de blefe, saturadores de fita e válvula, corte de montagem, parênteses, arquivo apócrifo | WASP, TURING, CHAOS, DRUM; nenhum dos citados |
| **Antitotem** | CMOS série 4000 / Lunetta (divisores, multiplexadores, osciladores Schmitt), rotas e memória de topologia, breadboard virtual | LOGIC, PLL |
| **Navalha 2** | sampler/fatias/fita/toca-discos | já portado (SAMPLER, TURNTABLE) |
| **Maravi / Maestro / Risco** | transcrição em partitura; sessão entre instrumentos; (Risco: sem conceito sonoro documentado) | NOTE-OUT |

## 2. O que realmente acrescentaria (proposta ao autor)

Critério: ideia que **nenhum** módulo do Modular faz, que tem identidade RASGO
(vem de um instrumento da família) e que soa ou move o patch sozinha.

1. **SIEVE** — peneiras de Xenakis para ritmo e altura (Batruc e Studio têm
   o conceito; Formalized Music). Uniões e interseções de módulos geram
   ritmos não periódicos e escalas fora do comum.
2. **PREDATOR** (ecossistema) — duas ou três populações em Lotka–Volterra;
   as populações viram CV, e os encontros viram eventos (caça, fuga,
   nascimento); a saída "cânone" repete a presa com atraso que encolhe até o
   stretto. A assinatura do Aquorbium.
3. **EPICYCLE** — círculos dentro de círculos (Fourier) do Batruc: X/Y e
   ângulos como CV; o desenho é o mostrador. Com poucos círculos, ritmo; com
   muitos, timbre.
4. **ECLIPSE** — dois ciclos de períodos diferentes (sinódico × dracônico do
   Asterismus): eventos só quando se alinham — polirritmo astronômico, raro e
   previsível.
5. **Escalas do Trioio no QUANTIZER** — a pendência microtonal antiga, agora
   com um banco de escalas citado e testado na família.
6. **SWARM** (cardume) — agentes que seguem um líder (coeso) ou se dispersam:
   homorritmia ↔ heterofonia como um só controle.
7. **GENDYN** e **RUNGLER** — duas fontes clássicas que o Studio já usa e o
   Modular não tem.
8. **SEASON** — relógio lentíssimo de dia/noite/estação (Aquorbium) que
   escolhe modo e puxa andamento: forma longa para o patch.

O **ORBIT** já aprovado deve usar a mesma física do Asterismus (leapfrog
simplético), e o **TAPE**/**CUT-UP** aprovados têm referência direta nos
saturadores e no corte de montagem do Studio.

## 2b. Antitotem e Navalha 2 — leitura mais funda (pergunta do autor, 9/11)

**Antitotem** (`docs/PESQUISA_CMOS_LUNETTA.md`, `IDEIA_BREADBOARD.md`, as
pesquisas de ruído, deriva e acentuação): a matéria dos chips CMOS 4000 —
matriz de 64 posições (4020 + dois 4051, um divisor endereçando
multiplexadores), **4006/4015 como registrador de deslocamento** (atraso e
"reverberação" clockados, memória que se reescreve), **semi-random** (quatro
relógios somados numa **escada R–2R**), random stepwave (4006 + XOR + soma de
taps), 4070 (modulação por diferença lógica), **mixer-comparador** (o limiar
como acontecimento), 4069 como filtro, LM13600 (OTA), o poço duplo (já virou o
CHAOS do Modular), a interface tátil do SX-150. No Modular há LOGIC, SH, PLL e
CHAOS; **faltam** o registrador de deslocamento clockado e a escada R–2R.

**Navalha 2** (`CONCEPT_DECONSTRUCTION.md`, `FRAGMENTATION_v0.12`,
`GESTURE_DECONSTRUCTION_v0.14`, `GESTURE_INSTRUMENT_v0.13`,
`MOTIF_MEMORY_LOCKS_v0.22.0`, `DUAL_MATERIAL_v0.11`, `ASSISTED_PERFORMER_v0.18.0`):
"duas fontes não são dois decks, são dois corpos de matéria gravada para
cortar, fragmentar e recombinar". Vocabulário: **GAP** (silêncio como evento),
**STUTTER** (repetição microscópica), **BURST** (rajada de fragmentos
diferentes), **MICROSLICE**, **MEMORY** (células protegidas), **TRACE/LOOP**
(gravar um gesto de controle, não áudio, e repeti-lo), motivos guardados, um
co-performer limitado. O SAMPLER, o TURNTABLE e o HeritagePitch já vieram; o
**vocabulário de fragmentação ainda não**.

Propostas acrescentadas:

9. **SHIFTREG** (Antitotem) — registrador de deslocamento clockado: o som (ou
   uma CV) passa casa a casa no ritmo do clock — atraso granuloso, taps com
   XOR (o random stepwave) e a memória que se reescreve (4015).
10. **R2R** (Antitotem) — quatro relógios de períodos diferentes somados numa
    escada R–2R: uma escada de tensão semi-aleatória, a melodia Lunetta.
11. **CUT-UP** (já aprovado) passa a ser o lugar do vocabulário do Navalha:
    GAP, STUTTER, BURST e MICROSLICE sobre um buffer gravado ao vivo.
12. **TRACE** (Navalha) — grava o movimento de uma CV (um gesto) e o repete em
    laço, com mutação reversível; o PLANAR só grava o gesto XY dele mesmo.

## 4. Leitura aprofundada, instrumento a instrumento (iniciada em 9/11)

Pedido do autor: "preciso que faça uma leitura aprofundada em todos, há
nuances importantes"; "inclusive podemos aperfeiçoar nossos módulos já
existentes". Volume medido: ~1,27 milhão de palavras nos 11 projetos. Método:
um instrumento por vez, os documentos de conceito e de projeto inteiros (fora
diários, auditorias e notas de versão), registrando aqui antes de seguir.

### 4.1 Asterismus

Lidos inteiros: `docs/TRAMA_DE_RELACOES.md`, `docs/MOVIMENTO_FISICO.md`,
`docs/TEMPO_MALEAVEL.md`, `docs/GRAMATICA_DO_CAMPO_PERFORMATIVO.md`.

**Nuances que importam para o Modular**

- **Nada é decorativo.** "Movimento visual e consequência sonora leem o mesmo
  estado causal"; "um controle criativo sem alvo, verbo e resultado
  reconhecível não fica disponível". Vale para os mostradores do Modular: o
  desenho tem de ser o estado que soa (é o caso de BOUNCE, PENDULUM, CELL).
- **A física é motor de cálculo, não nome dado a sorteio.** Cada lei entra
  como equação integrada, limitada e determinística, com consequência
  audível; "um modelo simplificado e declarado". Critério para todos os
  módulos físicos do Modular.
- **Relação não é ganho.** "Distância não deve ser apenas ganho, e um cabo
  não pode ser apenas uma linha mais grossa": toda conexão tem fonte → verbo
  → matéria → destino, com atraso, permeabilidade, resistência e memória. Os
  atos (resposta com tempo-luz e transformação de motivo; sombra em
  aumentação uma oitava abaixo; hoquetus no vão; contágio de ritmo; apoio/
  pedal; atrito → tensão → reparo/cadência) são o centro composicional.
- **A teia é potencial, o foco é escasso**: poucas relações em primeiro plano
  por vez, com pressão por novidade; mudar de foco já é um acontecimento.
- **Ciclo de vida do brilho**: cefeida (pulsa num período), nova (reacende
  muito acima e decai), nova recorrente (irregular), binária eclipsante (cala
  porque outra passa à frente). Relação massa–luminosidade: estrela massiva
  é forte e breve, leve é discreta e longa.
- **Tempo maleável**: mola amortecida de 2ª ordem no andamento (ω 1,1, ζ 0,62;
  a tensão puxa ritardando, a cadência desfaz); rubato em série de Fourier
  (4 harmônicos, 1/k²); arco de frase de Todd (1992); a **equação do tempo**
  (derivada dE/dB do relógio de sol) como desvio lento; **swing kepleriano**
  (passos = ângulos iguais da órbita, instante pela equação de Kepler; 1ª
  colcheia ≈ 0,5 + 0,63·e); groove por posição; contagens euclidianas por
  estrela (polimetria pelo mmc); **agrupamentos aditivos** (aksak, 8 máscaras)
  como acento métrico; **coro de Kuramoto** (fases acopladas que travam na
  defasagem que a relação pede: uníssono, ¼, ½ passo); **deriva de Adler**
  (phasing de Reich calculado, escorregamento que demora no gargalo);
  **Fourier do ritmo** (autocorrelação por Wiener–Khinchin para achar o
  período dominante: hoquetus a ½ período, atrito a 1,5 = hemíola);
  **peneiras de Xenakis** no contraste; **eclipses e Saros** (sinódico ×
  dracônico, estações de eclipses, a sequência volta em 223 meses);
  recapitulação metônica; sequências de durações (Fibonacci e outras).
- **Movimento físico** (famílias LIVRE, BILHAR, ESPUMA, PÊNDULOS, MAGNÉTICO,
  GRAVIDADE): espuma de Durian (bolhas que respiram em razão áurea), molas por
  relação, epiciclos de Fibonacci como lugar que passeia, **pêndulo elástico
  em ressonância 2:1** (troca sem aviso entre balançar e quicar), cadeia de
  pêndulos por molas, bilhar elástico e **estádio de Bunimovich** (caótico),
  **Lorentz** com campo que se inverte devagar, **Magnus** (spin de tênis de
  mesa), correntes de um atrator de **Clifford**, impulso do som (cada ataque
  empurra o corpo), matriz de distorção do espaço. **A física também toca**:
  choques acima de um limiar viram ataques com a força do impacto; a
  separatriz do pêndulo (passar por cima) ataca. "Rapidez limitada: o céu
  dança, não explode."
- **Hierarquia astronômica de movimentos** com consequência distinta:
  rotação (timbre do próprio corpo), translação/órbita (frase cíclica),
  baricentro (liderança negociada), precessão (deriva macro sem ciclo curto),
  nutação (microvariação, "nunca jitter gratuito"), movimento radial (Doppler
  musical), perturbação. Operadores de par: ATTRACT, REPEL, ORBIT, ELLIPTICAL,
  BARYCENTRIC, COLLIDE, MERGE/CAPTURE, FLYBY, SCATTER, CHASE, ESCAPE, ALIGN,
  CONVERGE/DIVERGE, OSCILLATE, SPIRAL, LOCK, SYNC/RESONATE (1:1, 2:1, 3:2),
  SWAP.
- **Leis como cálculo** (tabela §2d): Kepler (andamentos por razões físicas,
  acelera no periélio), Lagrange (lugares de repouso), maré (travamento de
  fase), Wien/corpo negro (temperatura → brilho tímbrico), Doppler
  (microafinação), **séries de Rydberg/Balmer como escalas não catalogadas**,
  níveis quânticos (saltos em degrau), tunelamento (o ato raro),
  superposição/colapso (a escolha só se decide quando observada),
  emaranhamento (pares que mudam juntos sem depender da distância), Hubble
  (expansão = rarefação; a coda), matéria escura (influência inaudível),
  supernova (ruptura que semeia as novas).

**O que muda nas propostas do Modular**

- **ORBIT** (escrito, não testado): manter Kepler e leapfrog, e acrescentar
  o que o Asterismus ensina — o **swing kepleriano** como saída de clock (os
  passos como ângulos iguais da órbita) e o **Doppler** (velocidade radial do
  planeta como CV de microafinação). O tempo-luz já está.
- **ECLIPSE**: usar os meses reais (29,530589 e 27,212221 tempos), o limite
  de 0,051 e o Saros — o Asterismus já validou (45–46 eclipses por Saros, 98 %
  voltam).
- **PENDULUM** (feito): acrescentar o modo **pêndulo elástico 2:1** e a saída
  da separatriz já existe (FLIP). **BOUNCE** (feito): candidato a paredes de
  estádio e spin (Magnus).
- **Novos, vindos daqui**: **KURAMOTO** (osciladores de fase acoplados que
  travam numa defasagem — uníssono, hoquetus, eco — e a deriva de Adler);
  **BRIGHT** (o ciclo de vida do brilho: cefeida, nova, recorrente,
  eclipsante, como VCA/envelope); **RELATE** (os atos entre duas vozes —
  resposta com tempo-luz e motivo transformado, sombra, hoquetus, contágio,
  atrito/reparo — o que o Modular não tem: hoje um cabo só leva sinal);
  **TEMPO** (o tempo elástico: mola de 2ª ordem + rubato de Fourier + arco de
  Todd + equação do tempo, como clock maleável); escalas de **Rydberg** no
  banco do QUANTIZER.
- **Módulos existentes a aperfeiçoar**: CLOCK (agrupamentos aditivos e swing
  kepleriano), TRIGSEQ/EUCLID (peneiras ao lado do euclidiano), PHRASE
  (as seis transformações de motivo — conferir o que já faz).

**Complemento** — lidos também `docs/ESTRATEGIA_MUSICAL.md`,
`docs/MORCEAU_PERFORMATIVO.md`, `docs/MAPA_CONCEITOS_JOGO.md`,
`docs/MODELO_JOGO_SEED_CAMPO.md`:

- **Família não é papel.** A família material diz *como* um módulo age; o
  papel diz *por que e para quem*: **Chão** (sustenta), **Corpo** (primeiro
  plano), **Atrito** (contrapressão), **Contágio** (transporta comportamento),
  **Rasgo** (ruptura com antes e consequência), **Vestígio** (memória, eco,
  resíduo). O mesmo módulo muda de papel. No Modular a seed escolhe módulos e
  cabos, mas não dá papéis — é um caminho para a seed (e para o Form Engine
  pendente) ganhar orquestração.
- **"Nutrir" não é ganho**: atenção, energia, memória, continuidade, risco,
  atrito, espaço, silêncio, tempo. **A pausa da mão** não congela nem repete
  em loop curto: a narrativa continua, com repetição com diferença.
- **Jogo como política composicional, não placar**: Caillois (alea, agon,
  ilinx, mimicry), estratégias seguir/transformar/contrariar/interromper/
  esconder/negociar, Huizinga, de Certeau; blefe/confiança, informação oculta,
  regras que mudam. **Uma jogada situada** tem origem, alvo identificável,
  influência do estado atual, consequência limitada e estado observável — "pode
  surpreender, mas não pode ser arbitrária".
- **Riqueza musical** avaliada em 12 dimensões (ritmo, melodia, harmonia,
  timbre, acontecimentos, orquestração, composição, atmosfera, nuance,
  espacialidade, enredo, organicidade): "nenhum módulo está pronto só porque
  emite áudio, responde a um knob ou apresenta diferença numérica".
- **A obra completa** (morceau): abertura → desenvolvimento → coda → cauda →
  fim; a coda escolhe um **verbo de encerramento** lido da trajetória
  (reunião, retirada em cadeia, memória relacional, dispersão; e implosão,
  cicatriz, última colisão, corte seco).

**Mais para o Modular**: os papéis e as estratégias de jogo como camada da
seed (o Form Engine anotado na memória do projeto); um módulo **GAME** que
emita a estratégia corrente como sinais (quem lidera, quem responde, quando
interromper) e verbos de coda — a decidir com o autor, porque toca a
mecânica da seed.

### 4.2 Aquorbium

Lido inteiro: `ECOLOGICAL_MUSIC_DESIGN.md` (§§1–6). (Continua.)

**Nuances**

- **A música vem dos conceitos do instrumento** (diagnóstico do autor em
  20/10: "musicalmente está muito podre… monolítico… parece um passa tempo…
  falta tempero"). A correção foi traduzir cada conceito ecológico num
  conceito musical com referência: nicho acústico (Krause) → orquestração por
  nichos; predador → cânone que fecha em stretto, captura em uníssono; presa →
  diminuição, síncope, saltos; migrante → modulação (carrega o centro); séssil
  → keynote/pedal e memória do predador em aumentação; territorial → sinal em
  contratempo; cardume coeso/disperso → homorritmia/heterofonia; acasalamento
  → dueto em terças; rachadura → ruptura e pontilhismo; dia/noite/estação →
  modo e andamento; massa → nuvem estatística (Xenakis).
- **Papéis pela estratégia, não pelo índice**; três controles com sentido:
  **influência** (0 = rota anterior intacta), **reação**, **memória**
  (repetir ou transformar).
- **Os estados líquidos** (gelo, fria, temperada, turbulenta, fervura), com
  **calor latente** (histerese: só muda de estado depois de pago o calor).
  Cada estado define viscosidade (portamento), turbulência (rubato,
  heterofonia, fragmentação), cristalinidade (rede pentatônica, grade par),
  velocidade do som, absorção, bolhas e capacidade do nicho.
- **Propagação**: cada nota é uma frente de onda; quem ouve, ouve quando ela
  chega, atenuada por espalhamento e por **absorção ∝ f²**, com **Doppler**; a
  imitação é do que se OUVIU. **Ecolocalização**: os cliques esperam o eco, e
  o "buzz" terminal emerge do tempo de ida e volta.
- **Teia 5×5** (caça, fuga, rivalidade → hoquetus, aliança → homorritmia/
  heterofonia, corte → cópia para endereçar, guarda → sinais); intensidade
  sobe rápido, decai devagar. **Harmonia e incompatibilidade** (Plomp–Levelt):
  tensão → dominante → cromático → cadência; o migrante só modula em água
  calma.
- **Densidade**: capacidade do nicho com crescimento **logístico** até o K do
  meio, repartida por saliência; **efeito Lombard** (fala-se mais alto no
  ruído); **bolhas de Minnaert** (bolha de 3 mm ≈ 1,09 kHz); **cripse**
  acústica (a presa congela); decomposição (quem parte vira memória).
- **Rede de bolhas** das jubartes (espiral ascendente em terças) e **revolução
  de canto** (Garland 2011: um canto novo é adotado inteiro, não por partes).
- **O timbre ouve o meio**: um passa-baixa no corte de −3 dB da absorção ao
  longo do caminho na água — a fervura soa opaca, o gelo límpido, quem nada
  longe soa mais escuro.

**Para o Modular**

- **PREDATOR** ganha forma concreta: não só Lotka–Volterra em CV, mas os
  **atos** (cânone com atraso = distância que fecha em stretto; captura em
  uníssono; fuga por diminuição e síncope) — perto do **RELATE** proposto no
  Asterismus; os dois podem ser um só módulo de relações entre duas vozes
  (decisão do autor).
- **WATER** (novo): um meio com estados líquidos e calor latente, cuja saída
  é ao mesmo tempo CV (viscosidade → glide, turbulência → rubato) e um
  **filtro de propagação** (absorção ∝ f² pela distância, Doppler, eco de
  ecolocalização) — o "som que se propaga pelos líquidos".
- **BUBBLE** (novo, fonte): bolhas de Minnaert (frequência pelo raio, chiado
  de fervura), o som de água corrente — uma fonte que o Modular não tem.
- **SWARM** proposto ganha a referência certa (Blackwell & Bentley 2002) e o
  modo coeso/disperso → homorritmia/heterofonia.
- **Aperfeiçoar**: DRIFT/SH com crescimento logístico; HARMONY com tensão por
  incompatibilidade (Plomp–Levelt) e modulação só "em água calma"; MARKOV
  (feito) com a **revolução de canto** (adotar um motivo inteiro de uma vez).

**`ECOSYSTEM_DESIGN.md`** (lido inteiro):

- **Invariantes conceituais** que valem como ética de módulo: nenhuma entidade
  tem visão total; só reage ao que observou; **o desejo altera tendências, não
  escreve acontecimentos**; o foco local reduz a definição periférica;
  indivíduos da mesma espécie preservam diferenças; primeiro plano e
  sustentação são papéis explícitos; a integridade só responde a causas
  identificáveis; o bioma influencia o master criativo, nunca remove a
  proteção final.
- **Observadores e modos de escuta**: quatro pontos de vista fora do aquário
  (SparseFocus — um subconjunto nítido contra fundo reduzido; EnergyMemory;
  GeometricCycle — a proeminência circula por regra geométrica;
  CollectiveDrift) e a **Lupa** (foco aumenta o detalhe, a periferia perde
  definição). A mixagem é uma **perspectiva**, não um conjunto de faders.
- **Integridade com histerese**: saudável → trincado → estilhaçado
  (irreversível na sessão), só por estresse explícito ou por perigo e energia
  ambos extremos; calma recupera.
- **Já codado e ainda não usado no Modular**: `HarmonicPhaseBank` (até 8
  fases derivadas por razões racionais de um só relógio), `FluidMotion` (um
  estado lido como forma, amplitude, tempo ou frequência, com autopatch
  limitado), `MotionWaveBank` (8 formas), **`MotionGestureRecorder`** (até
  2048 pontos XY em laço, com mutação zero = laço exato; é o **TRACE**
  proposto, já pronto na família), `SchoolSpectralBank` (8 passa-bandas com
  perfil fixo por cardume e seed), `EcologicalNoiseBed` (texturas por estação
  e hora; cor por turbidez e temperatura; a trinca reduz a resolução por S&H),
  `AquariumTail` (FDN de 4 linhas com matriz "nightmare"), `BiomaMarkov`
  (memória formal avançada por cruzamentos coletivos do campo Relabi), os
  misturadores de "filosofias" (vetor XY, sequenciador, morph, matriz 4×4).
- **Critério de entrada de um módulo** (§9) — oito perguntas: pergunta e
  consequência musical; papel; relação que ainda não existe; modelo
  matemático e limites; seed, estado e reset; orçamento de CPU, memória e
  ganho; teste técnico, A/B e escuta; destino (estável, experimental,
  arquivado). **Proposta: adotar no Modular** como checklist dos módulos
  novos (os dossiês já cobrem quase tudo; faltam a pergunta musical explícita
  e o destino).

**`CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md`** (lido inteiro) — o documento
mais importante do Aquorbium para o Modular:

- **Método de verificação** que o Modular deveria adotar nos módulos de
  conceito: para cada conceito, (1) a fonte diz isso mesmo? (2) onde e como
  foi usado; (3) **fiel, simplificado (dito) ou errado**; (4) o que a fonte
  ainda oferece; (5) o módulo concreto e o critério de teste. Várias
  correções saíram daí (vogais [ɔ]/[ɛ], membrana pelos zeros de Bessel J0–J3,
  King & Janik: o chamado responde com a **própria** assinatura).
- **Já codado no Aquorbium, testado e com proveniência** (C++, mesmo autor —
  reaproveitável no Modular copiando de commit conhecido e adaptando):
  - **`ChorusPhase`** — coro de fases (rãs de Aihara): competidores em
    antifase, aliados em fase, corte a ¼; o hoquetus e a homorritmia
    **emergem**. (O mesmo que o KURAMOTO do Asterismus; a lição: relógios
    próximos, acoplamento contínuo proporcional ao que ainda se ouve — a
    primeira versão com períodos de 8 a 32 passos não travava nada.)
  - **`PopulationCycles`** — Lotka–Volterra integrado **em logaritmos pelo
    Euler simplético** (órbitas fechadas, populações sempre positivas, sem
    deriva em 10 ciclos); período 2–5 min; a captura tira presas e alimenta
    predadores. É o núcleo do **PREDATOR**.
  - **`LifeCycle`** — energia como alimento, fome que poupa a voz, idade que
    escurece o corpo, morte que deixa o canto ao recife, renascimento
    cantando a **recombinação** (cabeça de um pai, cauda do outro, uma
    mutação).
  - **`SongForm`** — unidades → frase → tema (repetido 2–4×) → canto (ordem dos
    temas); o 2º tema responde ao 1º uma terça acima, o 3º contrasta; MEMÓRIA
    = deriva lenta (uma unidade por volta); revolução copia o canto inteiro.
    Medido: motivos já ouvidos sobem de ~6–9 % para ~13 %.
  - **`CodaExchange`** — alfabeto ritmo × andamento × rubato × ornamento; no
    diálogo o **rubato converge** turno a turno, o **ornamento marca a mudança
    de assunto**, clãs se formam.
  - **`SpectralConsonance`** — aspereza de Sethares somada sobre os parciais
    reais; a consonância depende do timbre e do registro; com um conjunto
    inarmônico, **a escala nasce dos corpos** (as 7 alturas mais lisas para o
    espectro). Resultado honesto: no temperamento igual a terça maior bate
    quase como o trítono.
  - **`ThermalCloud`** — Pithoprakta: glissandos cujas velocidades seguem
    **Maxwell–Boltzmann** com σ ∝ √T; a parede do registro reflete; no gelo a
    nuvem congela.
  - **`BubbleField`** — bolhas de Minnaert (3,26/R), amortecimento δ ≈ 0,1,
    altura que sobe (van den Doel), raios por Deane & Stokes (R^−3/2 e
    R^−10/3), **bolhas acopladas** que se dividem em modo em fase (grave,
    forte) e antifase (agudo, fraco) (Feuillade); ~30/s na turbulência,
    ~240/s na fervura.
  - **`SoundChannel`** — perfil de Munk, canal SOFAR: ao longo, o som vai
    longe; através, sombra; **a altura decide quem ouve quem**.
  - Água física: absorção de Ainslie & McColm (relaxações do ácido bórico e
    do MgSO₄, temperatura, salinidade, pH), velocidade de Mackenzie, **MAR /
    ÁGUA DOCE**.
  - **`HabitatHealth`** — ADI e ACI do próprio som (FFT 4096 a cada 85 ms,
    limiar relativo 30 dB abaixo da banda mais forte): um habitat saudável
    comporta mais acontecimentos.
  - **`RoomCoupling`** — o microfone como organismo a mais (Di Scipio), com
    o acoplamento das caixas **aprendido por regressão** para só a sala
    contar; Lombard em dB; as vozes fogem do registro da sala. Nada gravado.
  - Soundmark e hi-fi/lo-fi (Schafer): "**o lo-fi vive na articulação, não na
    cor**" (medido).

**Para o Modular, a partir daqui** (cada um no seu tempo, com o autor):

- **PREDATOR** = copiar e adaptar `PopulationCycles` (+ os atos do diretor).
- **BUBBLE** = copiar e adaptar `BubbleField` (uma fonte nova e física).
- **CLOUD** (novo) = `ThermalCloud`: nuvem de glissandos de Maxwell–Boltzmann
  com TEMP — a massa estatística de Xenakis como módulo.
- **HABITAT** (novo, análise) = `HabitatHealth`: ADI/ACI da mistura como CV —
  o patch pode se autorregular (um habitat lotado pede menos eventos).
- **Aperfeiçoar HARMONY/QUANTIZER** com `SpectralConsonance` (a escala que
  nasce do timbre) — além do banco de escalas do Trioio.
- **Aperfeiçoar PHRASE** com `SongForm` (tema, canto, deriva, revolução) e
  **RELATE** com `CodaExchange` (rubato que converge, ornamento na mudança).
- **SIGNAL-IN**: o modo "sala como organismo" de `RoomCoupling` (o
  acoplamento aprendido é a parte delicada).

**`TIMBRE_BODIES_DESIGN.md`** (lido inteiro):

- Um **segundo corpo por organismo**, misturado ao nativo (MISTURA) com um
  eixo de caráter (MATÉRIA): voz por três formantes (Peterson & Barney; as
  vogais [u]-[ɔ]-[ɑ]-[ɛ]-[i]), matéria modal, sopro (ruído num tubo), corda
  (Karplus–Strong, dedilhada → friccionada), gota (altura que sobe enquanto
  decai), dobra (Buchla 259), **membrana** (6 modos pelos zeros de Bessel
  J0–J3, afinação que cede após o golpe), **vidro** (parciais inarmônicos que
  batem, razões que mudam de copo para copo), coro; e, por física,
  **ARCO** (McIntyre–Schumacher–Woodhouse: atrito que gruda e escorrega, o
  movimento de Helmholtz nasce; diagrama de Schelleng — força baixa demais
  dá o "double slip", a oitava) e **PALHETA** (tubo fechado, só ímpares;
  acima de certa pressão a palheta fecha e cala).
- **Timbre ecológico**: o estado do mundo desloca a MATÉRIA (o gelo endurece,
  o caçador aguça, a presa amacia, o vidro danificado escurece) — tendência,
  não troca.
- **Lições de medição** que valem para o Modular: a interpolação linear do
  atraso fracionário é um passa-baixas que depende da fração (usar passa-tudo
  de 1ª ordem, Jaffe & Smith); medir com janela de Hann; compensar o atraso
  dos filtros no laço.
- **Lição de escuta (decisiva)**: a v2 do arco e da palheta, com corpo de
  violino, vibrato, ruído de breu e de sopro, ficou "pior… tudo rachado" e
  foi revertida. **"Realismo não se ganha somando ruído"; escutar o corpo solo
  antes do conjunto; mudar uma coisa de cada vez e entregar o A/B de cada
  uma.** É o "cada módulo no seu tempo" do autor.

**Para o Modular**: o STRING tem arco só como "entrada contínua" (não é
atrito) — um **BOW** de MSW (com o double slip de Schelleng) e uma **REED**
seriam corpos novos, partindo da **v1** do Aquorbium (a v2 foi rejeitada).
O MATTER/RESONATOR vão de harmônico a "esticado"; **acrescentar as razões
da membrana (Bessel) e do vidro** como pontos do eixo STRUCTURE é um
aperfeiçoamento direto.

**`SYNTH_DEVICE_DESIGN.md`** (lido: origem, contrato, fases 3 e 5, formante,
FM4, mistura, drive, lições, caos e realimentação):

- **A decisão do autor que vale para todo o RASGO**: "os timbres são
  consequência de ferramentas bem desenvolvidas de síntese sonora no
  instrumento e o músico que utilizar o instrumento cria os sons" — "melhor do
  que você ficar criando timbres". O critério de pronto de uma ferramenta não
  é um timbre aprovado: é ser **precisa, estável, documentada, tocável**.
- O Aquorbium **copiou núcleos do Modular** (commit `4ce898f`, com sha256):
  Operator, Wavetable, Additive, Pulsar, StringVoice, Matter, Formant. E,
  ao medi-los, **achou coisas que valem de volta para o Modular**:
  - **FORMANT**: com ganho de pico unitário por banda, um dente de serra saía
    ~30 dB abaixo de um passa-baixas, e mais baixo com RESSON; lá o ganho
    passou a ×√k (energia constante quando as bandas estreitam) e as larguras
    acompanham o trato (Q constante) — no Modular as larguras são fixas;
  - **STRING**: a cópia saía **4,4 cents baixa** até compensar o atraso do
    all-pass — **conferir o StringVoice do Modular**;
  - **FM num ressonador re-afinado a cada amostra explode** (até 1e37) — um
    ressonador de dois polos re-afinado não conserva energia; lá a FM ficou
    só nas fontes osciladoras e uma fonte não finita é zerada — **conferir
    MATTER/RESONATOR/STRING com FM ou pitch por CV de áudio no Modular**;
  - **DRIVE com aliasing** (−18,6 dB a 7 kHz) consertado por **antialiasing
    por antiderivada** de 1ª ordem (Parker, Zavalishin & Le Bivic, DAFx 2016),
    sem latência — **candidato para SHAPE, FILTER drive, WASP, CRUSH do
    Modular**;
  - **ADITIVA** cara: só somar parciais acima de −80 dB; sem `floor` no passo
    de fase; nada a calcular entre notas.
- **PROPOSTA que escuta o que propõe**: cada patch candidato toca uma nota
  de teste (0,6 s a 220 Hz), o quase mudo cede lugar ao próximo sorteio da
  mesma semente (até 8 tentativas) e o escolhido é nivelado a −14 dBFS com
  pico ≤ 1,5; 600 de 600 propostas a menos de 6 dB do alvo. O documento
  registra que **a "sugestão por seed" do Modular está escrita no
  `RASGO_MODULAR.md` mas sem código** — e que o Aquorbium a fez primeiro.
  **Para o Modular**: a seed poderia **escutar o patch que gera** (render
  curto offline) e rejeitar o mudo/nivelar — a mesma ideia, no seedPatch.
- **CAOS como fonte de áudio afinada** (Duffing forçado na frequência da nota,
  RK4 em subpassos; três regimes no mesmo volume por um seguidor lento; a
  entrada no caos é por crise, não por duplicação de período — dito assim) —
  **aperfeiçoamento do CHAOS do Modular** (hoje é campo de controle).
- **REALIMENTAÇÃO** (mesa sem entrada afinada: atraso no período da nota,
  passa-baixa, anti-DC e tanh, com a fase dos filtros descontada do atraso —
  sem isso ficava 14 cents agudo; DESAFINAR soma um tap fora do período e o
  laço salta de modo) — **fonte nova para o Modular** (FEEDBACK).
- **Ensinar a tocar**: "preciso aprender a tocar, são muitos parâmetros" →
  cinco lições "faça / ouça" no tutorial — o mesmo caminho da fase didática
  do Modular.

**`MODULE_DEVELOPMENT_STANDARD.md`** (lido inteiro) — **a regra que o autor
chama de "cada módulo no seu tempo"**, escrita no Aquorbium:

- **Três modos obrigatórios** no mesmo motor: autônomo (soa e decide sozinho),
  performance (controles humanos com transição suave e retorno previsível) e
  híbrido (o gesto influencia probabilidades e foco, sem substituir o estado).
- **Seis estados, nunca confundidos**: pesquisada → protótipo de core →
  integrada ao áudio → operável → **validada tecnicamente** → **aprovada
  musicalmente**. "Teste automatizado não significa aprovação musical."
- **Regra de promoção**: **quando há componentes testados mas não ouvidos, a
  dívida de integração e de escuta tem prioridade sobre criar protótipos
  novos** — uma família por vez atravessa rota, controle, persistência,
  bypass, tempo real, layout e **escuta**; só então a próxima.
- Portões: arquitetura, correção, tempo real, áudio (DC, descontinuidade,
  espectro), generativo (percurso sem eventos externos, sem repetição curta
  acidental), performance, escuta ("diferença não é automaticamente
  melhoria"), orçamento.

**Consequência direta para o Modular agora**: os módulos novos desta sessão
(DYNAMICS, ARP, CELL, MARKOV, BOUNCE, SPRING, MOUSSE, PENDULUM, PRISM e o
SCOPE refinado) estão em **"validado tecnicamente"**; nenhum em "aprovado
musicalmente". Pela regra do próprio autor, o próximo passo é a **escuta, um
por vez**, antes de abrir o lote 2 (MAGNET, ORBIT…, escritos mas parados).

**`INTEGRATED_MODULE_BANK_DESIGN.md`** e as coortes (`EMERGENT_…`,
`OPEN_INTERACTION_…`, `GENERATIVE_INTERACTION_…`, lidos inteiros): o
Aquorbium tem um **laboratório de ~60 componentes C++ testados** em 15
famílias, atrás de um bypass mestre (`LAB MIX`), à espera de escuta. Muitos
cobrem lacunas do Modular ou coincidem com propostas desta sessão:

| Componente do Aquorbium | Relação com o Modular |
|---|---|
| `CellularAutomatonSequencer` (32 células) | **o CELL do Modular foi escrito sem consultar este** — comparar e unificar |
| `ShiftRegisterMemory` | é o **SHIFTREG** proposto (Antitotem) |
| `MotionGestureRecorder` | é o **TRACE** proposto (Navalha) |
| `FixedLSystemGenerator` | **L-system** (reescrita formal → eventos) — não existe no Modular |
| `OpenRuleProgram` | uma **gramática de regras** (`fonte < n : destino = v`) avaliada na fronteira de evento — não existe no Modular |
| `DoubleScrollAttractor`, `LorenzCV`, `TripleSlowDrift`, `SlewRateRandomWalk` | atratores e derivas em várias escalas — o CHAOS/DRIFT do Modular têm um modelo cada |
| `InteractionProcessor` (ring, XOR, riqueza, duck como estados de uma relação) | um **RELATE** de áudio entre duas fontes |
| `EnvelopeContagionBus` | contágio de envelopes entre vizinhos sem uníssono — é o "contágio" do Asterismus em CV |
| `SpectralTransferVocoder`, `LockInFocusExtractor`, `FrequencyIsolator`, `RecentMixWavetable` | transferência espectral, foco síncrono, wavetable do mix recente — análise/resíntese |
| `PolygonalOscillator` (com X/Y literais) | oscilador poligonal — não existe no Modular |
| `ReeseVoice`, `DualPeakFilter`, `HarmonicRippleVoice`, `HarmonicMorphOscillator` | vozes e filtros de caráter |
| `StochasticPulseMask`, `ProbabilityDistribution`, `RandomToLoop`, `PatternRotationMutator`, `NotePermutationMatrix`, `AccumulatingTransposer`, `ThresholdStepSelector`, `BranchingStepRouter`, `ConditionalStageMachine`, `EvolutionLoopMachine`, `BehaviourSceneBank` | a família de decisão/sequência que no Modular está espalhada (TURING, DECISION, ABACUS, SEQUENCE, LOGIC) — conferir um a um o que já existe |
| `NoInputFeedbackBus`, `PowerStarve`, `ContactCrackleExciter`, `LightweightBinauralSpatializer`, `QuadratureStereoRotator` | mesa sem entrada, "fome" de alimentação, estalo por contato, binaural leve, rotação estéreo |

**Cuidado registrado pelo próprio Aquorbium**: esse laboratório está
"validado tecnicamente", **não aprovado musicalmente** — copiar daí traz
código testado, não música aprovada. E a proveniência importa: o
double-scroll foi comparado ao Joranalogue Orbit 3 (sem copiar circuito
nem firmware) — a regra RASGO de origem pública e desvio vale também aqui.

**Autonomia, geometria e decisão** (`AUTONOMOUS_ENERGY_DESIGN.md`,
`GEOMETRIC_SEQUENCER_DESIGN.md`, `BRAIN_DECISION_DESIGN.md`,
`DECISION_AND_PERMUTATION_DESIGN.md`, `CONTROL_EVENTS_AND_CLOCK_DESIGN.md`,
`GEOMETRY_EVOLUTION_DESIGN.md`, `PATTERN_EVOLUTION_DESIGN.md`,
`MULTIRATE_GENERATORS_DESIGN.md`, lidos inteiros):

- **Relabi** (John Berndt, 2009): "uma pluralidade coordenada de eventos que
  parece cíclica mas impede a identificação de um padrão uniforme — pulsada e
  não pulsada ao mesmo tempo". No Aquorbium (`BiomaRelabi`), quatro a cinco
  oscilações lentas de **taxas não comensuráveis** somadas; os
  **cruzamentos coletivos** viram eventos (avançam a forma), sem compasso. A
  referência de produto é o ADDAC 405 (comercial); o conceito é público. **No
  Modular não há nada assim** — e é muito RASGO: uma terceira via entre o
  periódico (CLOCK) e o aleatório (SH/NOISE). Candidato **RELABI**.
- **Decisão em evento, som contínuo**: a decisão acontece num relógio de
  controle ou num cruzamento; energia, altura e ativação **deslizam** por
  amostra — "mudanças ecológicas nunca viram degrau".
- **"O acaso decide *se*; o Cérebro decide *para onde*"**: um registrador de
  8 bits lido por cinco somas ponderadas dá **inclinações correlacionadas**
  entre organismos — tendem a subir ou descer juntos, como cardume, sem
  uníssono. (O Rungler do Studio é parente; a ideia de correlação por pesos
  diferentes sobre os mesmos bits é a nuance.)
- **`influence = 0` reproduz o caminho autônomo bit a bit**: os controles de
  performance somam viés ao estado, não trocam de motor (modo híbrido).
- **`IntervalQuantizer`** com graus negativos, até 16 intervalos e **período
  fracionário** (prepara microtonalidade e escalas não oitavantes) —
  referência direta para o QUANTIZER do Modular.
- **Memória formal Markov de regiões** (deslocamentos de seção
  {−5,−3,−2,−1,0,1,3,5}, INERTIA, MUTATION, CONTEXT), avançada só nos
  cruzamentos coletivos.
- Componentes isolados com contratos limpos: `BranchingStepRouter` (2 sinais
  → 8 categorias × 4 variantes), `NotePermutationMatrix` (pool de 16 notas,
  8 permutações; trocar a linha muda a ordem sem duplicar dados),
  `ControlEventExtractor` (qualquer CV → cruzamentos com histerese e
  subida/queda de envoltória), `FractionalLogicClock` (duas razões
  racionais 1/32–32/1 com AND/OR/XOR), `PatternRotationMutator`,
  `StepMemoryLayout` (4×8 paralelo ↔ 2×16 ↔ 1×32 sem copiar dados),
  `DriftCurveShaper` (separa suavidade × família de curva × curvatura),
  `StepPatternBank`, **`RandomToLoop`** (do acaso renovado ao laço fixo, com o
  laço que envelhece no meio — o Turing Machine dito com nuance),
  `FractalCore` e `VectorGenerator` (a mesma matemática a 200 Hz ou 48 kHz).

**Para o Modular**: RELABI (novo); o QUANTIZER com período fracionário
(junto do banco do Trioio); o TURING/SH conferidos contra `RandomToLoop` e as
inclinações correlacionadas; um EVENT (CV → eventos com histerese) talvez já
exista espalhado (LOGIC, SCOPE onset) — conferir antes de propor.

**`aquorbium-arquitetura.md`** (lidos: §1 conceito e padrão de excelência,
laboratório por módulo, matriz de relações, organismos compostos; §9–§12 a
triagem do ModularGrid Top 100, da EMW, da Soundfreak e da Teia):

- **Padrão de excelência**: pesquisa e hipótese ("uma referência inspira a
  pergunta; nunca define uma cópia"), protótipo isolado comparado a uma base
  simples, critérios técnicos, **critérios de instrumento sob gesto real**
  (faixa útil ampla, transições sem clique, "espaço para surpresa sem perda de
  domínio"), integração e memória. "O objetivo não é maximizar recursos."
- **Laboratório por módulo**: todo módulo pode ter uma versão experimental —
  "o equivalente digital de trocar um resistor por um diodo, realimentar onde
  não havia realimentação ou aceitar a gambiarra que produz um novo gesto" —
  com o deslocamento declarado, limites de segurança não negociáveis e um
  modo de congelar a descoberta para ouvi-la de novo. **Ideia para o
  Modular**: um modo EXP por módulo, explícito, não um módulo novo.
- **Três tipos de rota**: som, modulação e **observação** (análise limitada
  que volta à decisão sem carregar áudio bruto). No Modular, o terceiro tipo
  existe só implícito (SCOPE, LISTEN, ENV do DYNAMICS) — vale nomeá-lo.
- **Legibilidade antes da quantidade**: o Top 100 se reduz a ~25–30 princípios
  distintos; "se cada princípio virar uma espécie audível, o ouvinte não
  separa mais nada"; **poucas vozes profundas** em vez de muitas rasas.
- **Princípios de hardware já triados** (apropriação, não clone): "um gerador,
  várias funções" (Maths, Just Friends, Fractalist, Tides: o mesmo núcleo a
  taxa de controle ou de áudio; as 4 saídas relacionadas do Tides);
  probabilidade com distribuição configurável (SAPÈL); máquina de 4 estados
  aleatório → laço → evolui → trava (RANDOM8); divisores com lógica AND/OR
  (Time Wizard); estágios com **salto condicional** (Control Forge);
  endereçamento duplo sequencial/por CV (Step 8); **transposição
  acumulativa** (Metropolix); matriz de roteamento sequenciada e randomizável
  (WMD SSM); três caos em escalas de 15 s a 40 min combinados (Triple Sloths);
  autopatch de LFO (ochd); filtro de **dois picos com cross-modulação**
  (Belgrad); série↔paralelo em crossfade (Blades); **número de polos por
  CV** (Rossum Evolution); bandas espaçadas por **sétima maior** (Serge
  ResEQ); FREQ/SPAN de três filtros que cantam (Three Sisters); a
  **envoltória de redução de ganho como CV** (Messor — o GR do DYNAMICS já
  faz); morph de 10 atmosferas de ruído (Noise Cinema); 4 CVs por passo e
  paralelo↔serial (Sequential Voltage Source, Buchla 123).
- **Hardware que o autor já tocou** (EMW, §12 — "já provou o valor sonoro na
  mão"): POT ACTION RECORDER (= o TRACE), RESONANT FILTER SEQUENCER, FIXED
  FILTER BANK, NOISE CINEMA, MIX SEQUENCER (ARP 2500), ENVELOPE FOLLOWER, VC
  WAVETABLE LFO (31 formas), MIC INTERFACE, RING MOD (LM1496), GLIDE.
- **Teia Synthesizers DriftLog** (manual lido pelo Aquorbium): dois eixos do
  acaso — **Slope** (degrau → glide) × **Shape** (arco/pico, exponencial,
  Bézier) —, **Prob** (o clock pode passar sem mudança), **loop de uma janela
  do próprio histórico** (Start/Length ao vivo), Lock por canal com clock
  global, Choke (prioridade entre canais), camada Shift.

- **Catálogo SchneidersLaden** (~1500 itens triados em §12; os achados com
  apropriação nova): **Silhouette** (SPOT varre um campo circular de 6 entradas:
  primeiro plano que anda, o resto vira sombra redistribuída — a resposta de
  hardware à legibilidade, e parente do OBSERVER); **Scrooge** (vozes sem
  alimentação própria que **roubam energia dos sinais de controle** — recurso
  compartilhado e escasso); **Eddy** (canal sem entrada normalizado à própria
  realimentação — o som que existe sem fonte); **Cracklebox** de Waisvisz/STEIM
  (som cuja identidade depende da pele de quem toca); **Discrete Map** (passo
  escolhido por **qual limiar um sinal cruza**, não por clock nem por índice);
  **Collide 4** (amplificador **lock-in** e deslocamento de frequência por
  Hilbert, que desloca todas as parciais pelo mesmo Hz); **DCSN3** (árvore de
  decisão em camadas para gates); **Vector Wave** (mistura de 4×4 fontes por um
  ponto 2D); **Z-8000** (**pool compartilhado**: canais horizontais e verticais
  lendo os mesmos 16 valores — mexer em um muda várias sequências); **Sofia**
  (formante FOF travado harmonicamente, o oposto do pulsar livre); **Quasar**
  (binaural 3D); **Harlequin's Context** (morph entre configurações inteiras de
  comportamento, não entre valores); **CVilization** (gravador de CV cujo laço
  **muta**, não repete exato); **Polygogo** (síntese poligonal: a geometria é o
  motor do som e também o desenho).

**Para o Modular, a partir daqui**: aperfeiçoar o **SH/DRIFT** com o par
Slope × Shape, Prob e o laço da janela do histórico (DriftLog) — é o
aperfeiçoamento mais direto e de maior nuance; aperfeiçoar o **SEQUENCE**
com 4 CVs por passo e paralelo↔serial, e a **transposição acumulativa**;
um **FILTER** com polos por CV e dois picos cruzados (conferir o que o
FILTER/WASP já fazem); o **NOISE** com atmosferas em morph; nomear a rota de
**observação**; considerar o **modo EXP** por módulo. Novos que o catálogo sugere e
nada no Modular faz: **SPOT** (o primeiro plano que varre — talvez o próprio
OBSERVER), **THRESHOLD MAP** (Discrete Map: o passo pelo limiar cruzado),
**POOL** (Z-8000: sequências que compartilham valores), **STARVE** (Scrooge:
energia escassa entre vozes). Conferir antes o que MATRIX, SWITCH, PLANAR e
SEQUENCE já cobrem.

**Mais para o Modular**: **OBSERVER** (uma mixagem por perspectiva: N
entradas posicionadas, um ponto de escuta móvel com foco que dá detalhe e
tira definição da periferia) — candidato forte, nada parecido no Modular;
**TRACE** reaproveitando o `MotionGestureRecorder` (copiar com proveniência);
**HARMONIC PHASES** (um relógio, várias fases em razões racionais) como
aperfeiçoamento do CLOCK; um leito de ruído ecológico no NOISE (estação,
turbidez, trinca).

## 3. Limites

Leitura por documentos e cabeçalhos; o conteúdo de cada mecanismo do Studio
não foi auditado linha a linha. Nada foi copiado. O Batruc foi só lido.
