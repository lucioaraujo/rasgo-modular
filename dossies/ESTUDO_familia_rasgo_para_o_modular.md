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

### 4.0 O método comum — `RASGO_SYNTH_STUDIO/docs/pt/Metodo_RASGO.md`

Lido (≈ ⅔; faltam "cartografias e descartógrafos" e o fim). É a régua de
todas as outras leituras.

- **"Genealogia comum, personalidades próprias"**: os instrumentos "não são
  skins do mesmo motor nem etapas destinadas a convergir".
- **"O rasgo soa."** "Se a implementação declara um rasgo que não transforma o
  que se ouve, o rasgo ainda não aconteceu." **Há gesto em rasgar** (força,
  direção, duração, resistência, uma matéria; preparação, contato, pressão,
  abertura, propagação, interrupção, vestígio) e **há movimento** (passagem,
  não ponto entre antes e depois); **como um cismo**, separa enquanto revela
  e produz **instabilidade** — que "não equivale a aleatoriedade contínua":
  se tudo é igualmente instável, a instabilidade vira padrão. "Um módulo que
  produz som sem trajetória, resistência ou consequência pode ter timbre, mas
  ainda não necessariamente rasga."
- **Módulos como arquivos abertos**: cada módulo deve poder **expor** com
  segurança o que tem e o que está fazendo (matéria, gesto, estado, memória,
  regra, trajetória, energia, resistência, eventos, vestígios) para outros
  lerem, responderem, contradizerem; "reler não é repetir a saída".
- **Ativação ≠ presença ≠ agência** — uma flag prova ativação; RMS prova
  presença; nenhum prova consequência musical. **Oito estados de vida**:
  disponível, selecionado, manifestado, audível, saliente, influente,
  afetado, transformador.
- **Orquestra relacional: tocar junto não é tocar em uníssono.** Cada corpo
  tem **sensibilidade própria** (a quê é sensível, limiar, saturação, fadiga,
  curiosidade, resistência, tempo de reação). **"Influenciar não equivale a
  controlar"**: em vez de "A escreve no parâmetro de B", **A publica → a
  passagem traduz → B interpreta/resiste → algo muda e soa → fica memória →
  pode haver resposta**. Agressão, penetração, variação nascida da história
  das relações.
- **Contrato mínimo de uma relação** (10 pontos): participantes e direção;
  o que cada um percebe e publica; o verbo (escutar, seguir, responder,
  contrariar, imitar, penetrar, agredir, proteger, negociar, recusar,
  interromper, contaminar); intensidade, alcance, atraso, duração;
  **permeabilidade e resistência** de quem recebe; tradução entre domínios;
  memória/fadiga/cicatriz; condição de transformação ou fim; consequência
  observável; proveniência. A matriz de relações deve ser **esparsa, mutável
  e assimétrica** ("se todos leem todos o tempo inteiro, surge outra
  padronização").
- **Direito de emergir não é obrigação de aparecer**; silêncio, recusa,
  latência e continuidade são resultados válidos; "o ganho concede margem;
  não cria gesto".
- **Topologia**: não uma cadeia fixa fonte → efeito → soma → master, mas
  regiões, bordas, passagens, atalhos; **o rasgo transforma a topologia**;
  dentro e fora se misturam; **rotas têm história** (alargam pelo uso,
  acumulam resíduos, ficam resistentes, somem, bifurcam); o feedback volta
  "como outra força"; **dividir é transformação de estado com proveniência —
  nunca duplicação silenciosa ou pan de duas cópias idênticas**.

**O que isso pede ao Modular, em concreto** (o Modular é o instrumento em
que a topologia é literalmente o patch):

- **Cabos que interpretam** — hoje um cabo de CV soma ao parâmetro (A escreve
  em B). Um **RELATE** (proposto) pode ser a "passagem que traduz": verbo,
  permeabilidade, resistência, atraso, fadiga e memória entre duas portas —
  o contrato de 10 pontos como módulo que o músico põe no caminho.
- **Módulos que se expõem** — avaliar, módulo a módulo, se o estado
  interessante sai como saída (o ENV/GR do DYNAMICS, o DENS do CELL, o SURP do
  MARKOV, o ACC do BOUNCE e o FLIP do PENDULUM já são isso; vários módulos
  antigos não expõem nada além do áudio).
- **Rotas com história** — o MATRIX/SWITCH como rotas que se alargam com o uso,
  acumulam resíduo e resistem (um modo, não um módulo novo).
- **Instabilidade com dinâmica** — o VARIA (a "mão caótica") deve variar a
  própria intensidade de instabilidade, ganhar e perder chão; instabilidade
  constante vira padrão.
- **Estados de vida** — uma auditoria futura dos patches de seed pelo crivo
  "audível / saliente / influente" (não só "presente").

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

Lidos inteiros os documentos de conceito e de projeto (lista nas seções
abaixo); fora do método, como combinado: `aquorbium-tarefas.md`,
`docs/DESENVOLVIMENTO.md`, `CHANGELOG.md` (diários) e `INSTALL.md`.
Lidos só em parte: `OUTPUT_STAGE_DESIGN.md` (seções de true peak, EBU,
denormais, aliasing, master criativo), `REALTIME_CONTRACT.md` (diagnóstico),
`aquorbium-arquitetura.md` (§1, §9–§12). Não lidos: `ORGANISM_VOICE_…`,
`SPECTRAL_ENVIRONMENT_…`, `PROBABILISTIC_MEMORY_…`, `STRUCTURAL_CONTROL_…`,
`CPU_BASELINE.md` (os conceitos deles aparecem resumidos no banco integrado
e no `BRAIN_DECISION_DESIGN.md`).

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

**Composição** (`COMPOSITIONAL_VARIATION_DESIGN.md`,
`SEQUENCER_MEMORY_AND_HARMONY_DESIGN.md`, lidos inteiros):

- **A forma A → A′ → B → suspensão**, com **um protagonista por segmento**
  (A: corpos melódicos; A′: o pulso; B: textura e movimento, leitura
  retrógrada; suspensão: o espaço). A primeira versão "desenhava quase a mesma
  curva" (segmentos a ~3 dB de diferença) e o ciclo inteiro durava ~20 s —
  corrigido para durações de 6/4/6/3 fronteiras. **Lição**: medir o contraste
  entre seções no render, não só no código.
- **A gênese da sessão** — o autor: "essa sequência de notas aleatórias é
  chata; sempre se repete cada vez que eu abro". Causa medida: tônica, modo,
  registros, órbita, contorno… eram constantes. A semente passou a escolher um
  **mundo** (tônica e modo, caminho de acordes na tonalidade, registros,
  órbita de 5–16 passos, contorno, andamento de vida, quem abre, cor de cada
  corpo, hora e estação em que acorda). **É o conceito "seed como hipótese"
  do próprio Modular** (`dossies/ESTUDO_seed_composicao_generativa.md` §1.1),
  reescrito lá antes de existir aqui em código.
- **Linguagens harmônicas** (de `TRIOIO/docs/MAQUINA_DE_HARMONIA.md`):
  funcional (T…S…D, meia cadência e autêntica, engano, plagal, dominantes
  secundárias, sensível e terça picarda no menor), modal (vamps i–♭VII, i–IV,
  i–♭II, I–♭VII–IV sobre pedal), **brasileira** (I6/9, diminuto de passagem,
  VI7(♭9), IIm7, V7(9), IV7M, a plagal da bossa ivm6), **impressionista**
  (*planing* de nonas, tons inteiros, nunca V–I), **cromática** (P, L, R de
  Cohn); **episódios politonais** (Milhaud) em ~⅓ das sessões.
- **Condução de vozes por papel**: o baixo toca o baixo do acorde; as vozes do
  meio, a nota do acorde que menos se move; a melodia canta o modo e pousa no
  acorde nos tempos fortes; a nota cromática do acorde substitui o grau
  vizinho (Russell). **A harmonia evolui** (progressão nova, modulação
  vizinha ou outra linguagem) e **volta para casa** a cada três ciclos.
- **A emoção da forma**: um **arco de tensão** escolhido pela semente (subir
  ao clímax e cair; ondas; crescimento lento; calmaria entre duas agitações),
  período de 4–8 min; tensão = 0,6·arco + 0,4·tensão do acorde. A tensão move
  densidade de eventos, dinâmica (nunca acima do nível de base), **registro
  (a melodia sobe uma oitava no clímax)**, **respiração** (cala meio acorde
  depois de cada cadência) e **retardos 4–3** que resolvem. "O contraste é
  sutil de propósito; a escuta decide se pede mais."
- **Motivo, pergunta e resposta** (Schoenberg): ao fechar uma cadência a frase
  vira motivo; o respondente devolve imitação, inversão, retrógrado ou
  retrógrado da inversão; a cada duas viradas a melodia reapresenta o motivo.
  (Defeito instrutivo: com uma só fila, a resposta sobrescrevia o retorno.)
- **Memória de passos com 4 valores por passo** (altura, gate, viés,
  desejo) e **leitura direta por CV que não move o cursor sequencial**.

**Para o Modular**: este é o material do **Form Engine** que o próprio
Modular já desenhou (`ESTUDO_seed_composicao_generativa.md` §3.4) e não
implementou: o arco de tensão, a respiração depois da cadência, o registro
no clímax e o motivo que volta — como camada acima do VARIA, decidida com o
autor. O **HARMONY** do Modular pode ganhar as linguagens (funcional,
modal, brasileira, impressionista, cromática), a politonalidade e o retorno
para casa; conferir o que o HARMONY já faz antes de propor. E a seed do
Modular deve ser auditada contra a mesma queixa ("sempre igual ao abrir") —
o estudo do próprio Modular (§2.2) já registra essa limitação.

**Corpos e espaço** (`GRANULAR_STRIKER_DESIGN.md`, `MULTITAP_SPACE_DESIGN.md`,
`PHYSICAL_BODIES_COMPARISON.md`, lidos inteiros) — nuances de engenharia que
o Modular ainda não pratica:

- **Escuta cega A/B/C**: takes com a mesma seed e os mesmos eventos,
  **igualados em RMS** (o alvo escolhido pelo maior fator de crista, para
  ninguém tocar o teto — "loudness não vira atalho para preferência"), a
  chave revelada só depois do relato do autor (`--reveal`). A primeira
  escuta achou um defeito real (o contato estocástico da corda antes da
  memória) e o corpo **não entrou** até a segunda versão. **Proposta para o
  Modular**: o mesmo protocolo para a escuta dos módulos novos (ex.: SPRING ×
  HALL × SPACE; MOUSSE × ENVELOPE; PRISM × LOOPER) — sai da opinião sobre
  um módulo isolado e vai para a função que ele acrescenta.
- **Presença performática medida**: um controle podia estar "numericamente
  ativo porém quase silencioso" (−61 dB entre os extremos de SPACE MIX); lá
  entrou um **critério mínimo de diferença audível por controle** nos testes
  (−25/−35 dB relativos à mistura). **O Modular não tem esse teste** — e a
  sessão anterior do Asterismus já perguntou "controles que não fazem nada".
  Candidato a teste transversal: cada knob de cada módulo, extremo contra
  extremo, acima de um limiar.
- **Atraso fracionário**: a interpolação linear é Lagrange de ordem 1 e
  colore (passa-baixas que depende da fração); o espaço usa **Lagrange
  cúbico** e limita a velocidade de mudança do atraso (0,2 amostra por
  amostra) — **o SPRING e o PRISM que escrevi nesta sessão usam interpolação
  linear**: aperfeiçoamento direto, antes da escuta deles.
- **Limite do laço por conta, não por limitador escondido**: a soma dos
  módulos dos coeficientes de Lagrange (1,25) entra no cálculo do ganho
  máximo de realimentação; "não há limiter ou saturador escondido no corpo".
  Vale para LOOPER, SPACE, HALL, SPRING, PRISM.
- **Partículas que congelam os parâmetros ao nascer** (mexer no controle não
  re-afina uma cauda; o Q é calculado na criação, sem trigonometria por
  amostra) e **pool fixo com substituição da partícula de menor energia,
  contada na telemetria** ("sobrecarga nunca fica invisível").
- **Memória como decisão de arquitetura**: um corpo de 131 kB não vira membro
  de 25 despachantes; um banco dedicado de 5 instâncias usa 80 % menos.

**Coortes do master, do refinamento e da voz** (`CREATIVE_MASTER_…`,
`CORE_REFINEMENT_…`, `ADVANCED_VOICE_ECOLOGY_…`, `SPECIALIZED_INTEGRATION_…`,
`PERCUSSION_AND_HARMONIC_MORPH_…`, lidos):

- **Master criativo separado da proteção técnica**: crossover
  Linkwitz–Riley de 4ª ordem em 3 bandas, compressão por banda, EQ derivado
  da mesma separação (sem filtro duplicado), **largura M/S só acima do grave**
  (a fundação fica no centro), compressão óptica lenta de "cola" antes do
  limitador; o limitador técnico nunca entra no laço. A **redução de ganho
  real volta ao cérebro** com ataque, release e teto (0,32) — fecha o laço
  "a dinâmica final realimenta as decisões".
- **Detalhes de precisão** que valem como padrão: a **máscara estocástica de
  pulsos decide só na borda de subida e congela até a descida** (sem alternar
  dentro de um pulso); o **morph harmônico interpola os coeficientes**, não
  mistura duas saídas prontas; o **soft take-over** de um polo em toda CV
  vinda de decisão (eventos e áudio não são suavizados); wavefolder 4× leve
  com teste espectral honesto ("não demonstra ausência universal de alias").
- Componentes de voz: `BiomaBassVoice` (FM thru-zero, sub intervalar pelo
  mesmo quantizador da melodia, wavefold pré-filtro, 2–4 polos,
  autorregulação por energia), `SchoolWaveVoice` (seis membros de uma fase
  comum), `RecentMixWavetable`, `FrequencyShifter` por Hilbert,
  `SpotlightMixer`, `HungerProcessor`, `WaterInertia`.

**Para o Modular**: conferir no Modular a decisão de gates probabilísticos
(TRIGSEQ, DECISION, ARP SKIP, CELL MUT) contra a regra "decide na borda e
congela"; o MASTER do Modular já protege — um **MASTER criativo** (bandas,
largura acima do grave, cola) seria camada à parte, se o autor quiser.

**Saída técnica** (`OUTPUT_STAGE_DESIGN.md`: limitador em true peak,
EBU Tech 3341, denormais, aliasing, o achado do master criativo) — **três
auditorias concretas para o Modular**, verificadas no código em 9/11:

1. **Denormais — o Modular não tem proteção nenhuma** (nenhum FTZ/DAZ,
   nenhum `ScopedNoDenormals` em `src/` nem em `apps/`). No Aquorbium, filtros
   e ressonadores decaindo no silêncio entravam na faixa denormal (até ~100×
   mais lenta): o soak de 30 min dava **pior bloco a 122 %** do orçamento;
   com flush-to-zero ligado pelo próprio motor durante o render
   (`core/FlushDenormals.h`: MXCSR no x86-64, FPCR no AArch64), caiu para
   52 %, sem mudar os renders dourados. **No Modular, a memória do projeto
   registra que os cliques do painel com o VARIA são xruns de reprodução, não
   o DSP — denormais são uma causa candidata direta desses xruns** (filtros,
   HALL, SPACE, MATTER, RESONATOR, STRING decaindo). Primeira coisa a medir.
2. **Loudness**: o `Loudness.hpp` do Modular guarda energia em **fatias de
   100 ms** — o mesmo desenho em que o Aquorbium achou erro de 0,45 LU no
   momentâneo (casos #13–14 da EBU Tech 3341); lá as fatias passaram a 10 ms
   (o integrado continua normativo). Os testes da EBU podem ser sintetizados
   da descrição publicada, sem arquivo de terceiros.
3. **Limitador**: o Modular já usa o detector 4× do Navalha
   (`TruePeak.hpp`) e um limitador por envoltória com look-ahead + teto suave
   — desenho diferente do que falhou no Aquorbium, mas **sem o teste de
   estresse** que achou os defeitos lá (seis sinais quentes: true peak
   exatamente no teto; seno que volta ao teto depois de um pico). Vale rodar
   o mesmo teste.

E um método reaproveitável: **medida de aliasing por grade prima** (sinal
periódico cuja fundamental cai num número primo de bins; a energia fora da
grade é o aliasing; a parte abaixo de 5 kHz soa como tom inarmônico) — para
OSC, WAVETABLE, SHAPE, WASP, CRUSH, FOLD do Modular.

**Auditoria, movimento e layout** (`ECOSYSTEM_AUDIT.md`,
`AQUARIUM_SPACE_DESIGN.md`, `SYNTHETIC_TAIL_DESIGN.md`,
`SCHOOL_MOTION_DESIGN.md`, `GESTURE_MOTION_DESIGN.md`,
`PHILOSOPHY_MIX_DESIGN.md`, `WINDOW_LAYOUT_DESIGN.md`, lidos):

- **A auditoria do primeiro protótipo** recusou promovê-lo ("pseudocódigo
  conceitual executável") e listou o que nunca pode acontecer: RNG global,
  **resultado que depende do tamanho do bloco**, alocação no callback,
  consulta que sorteia de novo a cada chamada (a mesma entidade em dois lugares
  no mesmo instante), quebra "rara" que matematicamente é inevitável, presa
  com conhecimento total, soma sem orçamento de ganho e pan linear,
  autorreferência só declarada num comentário ("um comentário não conta como
  integração"). E **nomes de artistas em algoritmos** ("Devine", "Autechre",
  "Barbieri", "Cortini") foram trocados por **vocabulário autoral que
  descreve o comportamento** — referências ficam nos créditos.
- **Critério de excelência do ecossistema**: "não estará pronto porque
  contém muitas metáforas em classes; estará pronto quando poucas relações
  forem audíveis, reproduzíveis, seguras e musicalmente profundas" — e cada
  mecanismo responde o que muda na escuta, quem percebe, qual estado causa e
  como continua controlável sem virar automação rígida.
- `HarmonicPhaseBank`: várias fases como múltiplos racionais de **um único
  relógio** (nenhum acumulador deriva do outro); o relógio longo é reduzido a
  cada 720.720 ciclos (divisível por 1…16, a redução não quebra nenhuma
  razão). `FluidMotion`: um estado lido de quatro maneiras (formas,
  amplitudes, tempos, frequências) — o princípio do Tides. Cauda FDN com
  matriz Hadamard e uma segunda matriz assinada ("nightmare") cuja
  interpolação continua contrativa; o FDN segue avançando em bypass (ativar
  não reinicia a memória).
- **Layout** (metodologia obrigatória desde 18/9): knob ≥ 52 px, área
  interativa ≥ 80×92 px, rótulo e valor em áreas próprias (≥ 11 px), **nunca
  diminuir os mínimos para caber** — reorganizar ou rolar; **controles
  dependentes mostram a dependência** (ficam inativos quando a influência é
  zero); posições que são dado musical não se movem para acomodar legendas.

**Para o Modular**: um **teste transversal de invariância ao bloco**
(renderizar cada módulo com blocos de 64 e de 512 e exigir o mesmo
resultado) — o mesmo espírito do teste de presença por controle; revisar se
algum nome de módulo, modo ou rótulo do Modular carrega nome de artista ou
marca em vez do comportamento; e, na revisão de layout dos painéis pedida
pelo autor (com as referências do Cardinal), adotar os mínimos e a regra de
dependência visível do Aquorbium como ponto de partida.

**Corpos com par no Modular** (`MODAL_RESONATOR_DESIGN.md`,
`PHYSICAL_STRING_DESIGN.md`, `GRANULAR_CAPTURE_DESIGN.md`, lidos):

- **Modal**: quatro modos complexos que giram e encolhem; o decaimento é
  dado pelo tempo até 1/e (independe da taxa de amostragem); **`excite()`
  soma energia sem reiniciar a fase nem apagar a cauda** — "repetir eventos
  constrói uma história em vez de repetir uma amostra"; teto de energia
  antes de overflow; corte explícito de caudas subnormais. O eixo MATERIAL
  vai de harmônico (1, 2, 3, 4) a **membrana** (1; 1,593; 2,136; 2,295 — zeros
  de Bessel).
- **Corda**: laço = atraso inteiro N + all-pass fracionário d + FIR de perdas
  (1−b) + b·z⁻¹, com **N + d + b ≈ fs/f0** (o atraso do filtro de perdas é
  descontado da afinação); ganho de volta pelo alvo de −60 dB no tempo de
  decaimento (`g = exp(ln 0,001 / (T·f0))`); a rajada de excitação é gerada
  **uma amostra por vez** (o custo do ataque não salta no callback);
  posição de pinça por cancelamento de dois pontos.
- **Captura granular**: cada grão **congela** duração, velocidade, direção,
  pan e ganho ao nascer; o pool cheio **recusa e conta** (sem roubo
  silencioso); a leitura começa no passado com margem para 2× de velocidade
  (nunca lê onde está sendo escrito); onset pela razão de envoltórias de 3 e
  50 ms com piso e período refratário.

**Achado verificado no Modular (9/11)**: o **STRING** (`StringVoice.hpp`)
calcula o atraso como inteiro + all-pass fracionário, mas **não desconta o
atraso de fase do filtro de perda** (um polo, coeficiente
`0,05 + (1 − damping)·0,9`) — a corda tende a sair **baixa**, mais quanto mais
fechado o DAMP; e **o teste aceita 6 % de erro** (~100 cents) na altura.
A cópia desse núcleo no Aquorbium mediu 4,4 cents baixa e foi corrigida
compensando o atraso. Correção técnica direta (afinação), com teste em cents.
**MATTER/RESONATOR**: conferir se `excite` soma ou reinicia, e o ponto
"membrana" no eixo STRUCTURE.

**Mais para o Modular**: **OBSERVER** (uma mixagem por perspectiva: N
entradas posicionadas, um ponto de escuta móvel com foco que dá detalhe e
tira definição da periferia) — candidato forte, nada parecido no Modular;
**TRACE** reaproveitando o `MotionGestureRecorder` (copiar com proveniência);
**HARMONIC PHASES** (um relógio, várias fases em razões racionais) como
aperfeiçoamento do CLOCK; um leito de ruído ecológico no NOISE (estação,
turbidez, trinca).

### 4.3 Batruc (só leitura — o autor edita em paralelo)

Lidos inteiros: `MOVIMENTO_GRAFICO_E_SOM.md`, `OPERACOES_GENERATIVAS.md`,
`RITMOS_E_CONTAGENS.md`. (Faltam `README.md`, `BIBLIOGRAFIA_DE_ESTUDO.md` e as
auditorias; `TAREFAS.md` fica fora como diário.)

**A nuance central do Batruc é a honestidade do nome**: "isso é uma analogia
composicional de onda viajante; não representa automaticamente uma corda";
"rebote amortecido é um perfil radial com essa aparência; não simula uma
colisão"; "reflexão geométrica, reflexão óptica e projeção anamórfica são
fenômenos distintos"; "a deriva atual é periódica e reproduzível, não é caos
comprovado"; "a transformação não deve receber o nome de 'inversão' se apenas
alterar ganho ou sortear parâmetros"; "τ é temperatura composicional
adimensional; não é temperatura física nem mede emoção"; "um 'preset
Xenakis' sem módulos e resíduos não explica o processo"; "usar palavras como
superposição ou incerteza não constitui modelo". Parâmetros físicos (massa,
tensão, velocidade) **só quando participam das equações com unidades
coerentes**. — **Para o Modular**: revisar os módulos desta sessão por esse
crivo (ex.: o PRISM é dispersão de verdade, por atraso por banda — honesto;
o BOUNCE é física de livro-texto — honesto; o "WIND" é acaso semeado com nome
de vento — dizer isso no LEARN).

**Outras nuances**

- **Uma operação tem fonte, tipo, escopo (célula, compasso, frase, seção,
  peça), parâmetros, seed, invariantes, resultado e versão anterior**, e só
  entra na fronteira do seu escopo. Variar uma função são **quatro ações
  diferentes**: trocar a família matemática, mudar os parâmetros, transformar
  o resultado ou trocar o destino musical — sempre **fonte → transformação →
  destino**, com travas, "intensidade zero congela a dimensão", desfazer e A/B.
- **Três representações consultáveis**: o material calculado, a decisão
  composicional e os eventos efetivamente tocados. "Expressividade não deve ser
  reduzida a sorteios independentes nem a um único controle genérico chamado
  humanize."
- **Gramani** (*Rítmica*, 1988; *Rítmica viva*, 1996; leituras de Fiaminghi e
  Ribeiro): associação, dissociação, **inversão de papéis entre vozes**
  (o ostinato passa a uma voz e a série à outra, numa fronteira de compasso),
  o ostinato como **tempo moldado**, não metrônomo neutro; padrões culturais
  com fonte e contexto, nunca presets universais.
- **Geometria rítmica**: polígonos inscritos (N vértices = N divisões; N e M
  sobrepostos mostram N:M; o retorno conjunto pelo mmc, **demonstrado pela
  aritmética, não pela simetria do desenho**); o estudo **Orbit 3:4:5:7** com
  rastro acumulado; casos de estudo (Tool, Meshuggah) tratados como hipóteses
  a conferir na gravação.
- **Eventos de geometria**: cruzamentos de duas curvas com sentido declarado
  e **teto de eventos por seção**; leis de intensidade pela distância
  (encontro × afastamento — o mesmo movimento, dois sentidos musicais); afinidade
  gradual na borda (smoothstep), não liga/desliga.
- **Espaço classificado antes de compor** (topológico, diferenciável,
  métrico, espectral, simplético, fibrado, quântico), Chern/Berry com as
  hipóteses certas, Gordon–Webb–Wolpert (formas diferentes, mesmo espectro).
- **O Batruc leu o Modular**: VARIA/MotionEngine, MUDA/EVOLUI/CRUZA,
  PHRASE, DECISION, FUNCTION, EUCLID, SWITCH — como "perguntas de design".

**Para o Modular**

- **SIEVE** (já proposto): a expressão (união, interseção, complemento de
  `j mod m ∈ R`) **visível e editável** no painel e no LEARN, com o período
  resultante — nunca "modo Xenakis".
- **POLYGON** (novo, didático e generativo): camadas N:M:… com fases, gates
  por camada, gate de coincidência e EOS no realinhamento; o mostrador
  desenha os polígonos inscritos e o rastro (Orbit) — a geometria rítmica do
  Batruc como módulo, ligada à fase didática do Modular.
- **SWAP de papéis** entre duas sequências numa fronteira (Gramani) — talvez
  um modo do SWITCH; conferir.
- **EPICYCLE** (já proposto): com a distinção Fibonacci (pesos 1/Fₙ ou
  velocidades Fₙω₀ — escolhas diferentes, nomeadas) e espiral áurea
  logarítmica × aproximação por quadrados.
- **PENDULUM**: o próximo passo honesto é a comparação k = 0 / fraco / forte
  de pêndulos acoplados por molas (modos normais), como o Batruc propõe.

### 4.4 Trioio

Lidos inteiros: `docs/TRIOIO_CONCEITO.md`, `docs/TRIOIO_CEREBRO.md`, `docs/ESCALAS.md`, `docs/RITMO.md`. (Continua.)

**Nuances**

- **O Terceiro Olho**: perceber ao mesmo tempo o evento, o sistema que o
  organiza e a possibilidade de transformar esse sistema ("olho 1 participa;
  olho 2 observa; o Terceiro Olho percebe a estrutura e pode transformá-la").
  A trapaça é só um dos modos.
- **Uma nota não é unidade autossuficiente**: seu sentido depende da
  trajetória, do campo e da consequência. Perguntas que o instrumento deve
  responder: o que está tentando acontecer; que expectativa foi criada; qual
  gesto a alterou; "quando uma repetição é estrutura e quando vira
  automatismo; quando o silêncio é ausência e quando é ação".
- **Estratégias** (hipóteses temporárias, não presets): seguir; acompanhar
  com atraso; condensar; expandir; trocar o material; **preservar a energia e
  mudar o timbre**; **preservar o gesto e negar a tonalidade**; repetir com
  diferença; ocultar uma transformação; interromper e abrir espaço; mudar a
  regra do jogo.
- **Barramento de energia semântico**: a decisão não comanda amostras; publica
  alvos suavizados (energia, densidade, brilho, harmonicidade, rugosidade,
  dureza, duração, memória, mutação, espacialidade, intenção de ataque,
  **grau de silêncio**) e **cada espécie interpreta o mesmo campo do seu
  jeito** — "o mesmo aumento de tensão pode abrir um banco modal, aumentar
  partículas granulares, alterar a excitação de uma corda".
- **Intenção declarada × áudio observado**: a distância entre o que se pediu
  e o que soou é material ("um pedido de rarefação pode ter produzido muitos
  ataques"); proveniência em camadas que nunca se confundem (áudio recebido,
  metadados, prompt original, análise inferida — "nunca afirmar ter recuperado
  um prompt da forma de onda").
- **Aprender é ajustar tendências, com esquecimento** para não cristalizar
  um perfil de gosto; tudo removível e contestável pelo performer.
- Três atitudes de escuta: RESPONDER AGORA, LER ANTES, HÍBRIDO.

**`docs/ESCALAS.md`** (lido inteiro) — o material do QUANTIZER microtonal:

- **Banco de 2576 escalas** num **arquivo de texto próprio**, legível e
  editável (nome, família, **período** em cents — não oitavantes funcionam —,
  graus em cents ou razões, **fonte citada**, nota se é modelo ou
  aproximação): melakartas (Venkatamakhin, 1660), EDOs de 5 a 100, gêneros
  gregos nas 7 espécies, maqamat (Cairo 1932) e makam (53 comas de Holder),
  thāts e 22 shrutis, temperamentos históricos (Werckmeister, Kirnberger,
  Vallotti, mesotônicos de 1/4 a 1/11 de coma), justa e séries harmônicas
  (Partch, Johnston), Bohlen-Pierce (período 3/1), gamelão, **modos
  nordestinos** (Siqueira 1981), 1753 MOS, CPS (hexany/dekany/eikosany),
  Euler-Fokker, formas primas de Forte, modos de Messiaen. Escalas do autor e
  arquivos **Scala (.scl)** entram junto. "Escalas não têm dono; cada uma cita
  a fonte." Recurso transversal declarado pelo autor (`KNOW-ESCALAS`): outro
  instrumento copia o arquivo, o gerador e o leitor, com proveniência.
- **A máquina de escalas** (operações em cents, com período): **rotação**
  (modo), **alteração** (grau ± semitom, quarto de tom ou coma), **espelho**
  (o dual: o frígio do jônio), **interpolação** (cada grau anda uma fração até
  a escala de destino — estados intermediários entre duas escalas), **pivô**
  (outra tônica com o máximo de notas comuns), **acréscimo/supressão**,
  **alargamento** do período (1–3 %). Encadeamentos por estratégia (seguir,
  transfigurar, contrapor, interromper); destinos sorteados **primeiro por
  família** (senão as 1753 MOS tomam tudo).
- **Harmonia dos graus**: com escala microtonal ou não oitavante, os acordes
  são tríades de graus da própria escala — "qualquer escala tem harmonia".
  **Variante por acorde** (Russell): a nota do acorde que falta na escala
  substitui o grau vizinho enquanto o acorde dura.
- **Mapa afetivo** (brilho, terça, tensão, estabilidade, ambiguidade,
  valência), conferido contra Temperley & Tan (2013) — "tendências
  declaradas, não verdades"; **intenção** (luminosa, sombria, tensa, serena,
  suspensa, ambígua, arco) guiando a máquina.
- **A escala como comportamento** (como a linguagem a usa): subida e descida
  diferentes, notas de apoio, percurso (sayr/ālāp), frases-assinatura
  (pakaḍ), ornamentos (meend, andolan, gamaka, blue), cadência, textura
  (bordão, heterofonia, acordes), ciclo rítmico usual. Prática modal com a
  nota característica, cadência e vamp por modo.
- **Misturas** (colagem, híbrido por tetracordes, camadas/polimodalidade,
  transição por interpolação) e **costuras** entre seções (corte, pivô por
  nota comum sustentada, ponte pela escala intermediária, sobreposição).

**Para o Modular**: o **QUANTIZER** (e o HARMONY) ganham, por cópia com
proveniência do arquivo e do leitor do Trioio: banco de escalas com
**período**, Scala, e — o que é próprio de um módulo — a **máquina de
escalas por CV** (rotação, espelho, pivô e sobretudo a **interpolação entre
duas escalas** como um knob/CV contínuo: um quantizador que *transita* de
uma escala a outra). Comportamento (subida ≠ descida, apoio nos tempos
fortes) é a camada seguinte. Respeitar a pendência antiga do TAREFAS:
preservar as 12 escalas atuais, seus índices e o significado de `scale_cv`
para o HARMONY e os patches salvos; seleção fora do callback.

**`docs/RITMO.md`** (lido inteiro):

- **Ritmo como campo** (o comportamento do Grids, de Émilie Gillet, MIT — sem
  código nem mapa): cada compasso da referência vira um ponto num mapa
  densidade × síncope com o seu padrão; o caminho a cada compasso é
  transformado pela estratégia (seguir; transfigurar = síncope espelhada;
  contrapor = densidade complementar; interromper = um compasso vazio em
  quatro); mistura dos 4 padrões mais próximos; e **golpes deslocados** do
  tempo ao contratempo quando a posição pede mais síncope do que os padrões
  têm — "sem isso, mover a posição não mudava o som". **No Grids o mapa é
  fixo e treinado; no Trioio o mapa é a própria referência.**
- **Arquivo de 26 ciclos rítmicos com fonte** (iqa' árabe; tāla
  hindustani e carnático; usul turco e aksak; claves e linhas-guia; **baião**;
  colotomia do gamelão; compás flamenco de 12; shuffle), com notação própria
  (golpes, divisões, acentos — o sam —, vazios — o khālī), e a **melodia que
  chega no ciclo** (a nota de apoio no sam, no gong, no dum) e o **tihai**
  (a frase três vezes, a última no sam; Clayton 2000).
- **Groove medido** com dois vieses achados e corrigidos que valem como
  aviso para qualquer medidor: **"a janela larga inventava swing"** (ataques
  soltos davam 24–34 % de swing falso — o swing só existe quando os
  contratempos se *concentram* na região ternária) e **"pulso fraco não tem
  grade"**. Rubato pelo arco de Todd: w(u) = (u + β((u−½)³+⅛))/(1+β/4).

**Para o Modular**: um **CYCLE** (ciclos das culturas com fonte, acentos,
sam/gong como saída própria, tihai como evento) seria o lugar certo para
esse arquivo — o TRIGSEQ é genérico e não sabe o que é um sam; e um **campo
rítmico alimentado pelo LISTEN** (o mapa é o que o Modular ouve, não um mapa
treinado) é um caminho original para o "ritmo como campo".

**`docs/MAQUINA_DE_TIMBRES.md`** (lidos: princípio, peças, §5.0 "o rasgo
modular", §6.6 medidas que descartaram caminhos, levas a fazer, portão
conceitual):

- **Receitas, não 200 instrumentos**: excitador × vibrador × corpo ×
  articulação × sala; "o estilo é um conjunto, não um timbre" (o choro são
  papéis: flauta no motivo, bandolim no contracanto, 7 cordas na baixaria…).
  Nenhuma amostra, nenhum *soundfont*.
- **O autor, depois de ouvir os timbres realistas**: "ainda muito midi — se
  possível trazer mais da **síntese modular criativa** para dentro dos
  timbres, **sons inauditos, não protocolares**". O rumo deixou de ser o
  realismo: o corpo acústico é ponto de partida e é **atravessado por
  processos modulares** (genealogia: SHAPE e LPG do Rasgo Modular, Rungler
  do Studio): dobra movida pelo envelope do próprio som; anel com razão
  inarmônica tirada do espectro da referência; deslocamento de frequência
  (Bode, Hilbert); **LPG re-disparado por um Rungler** (o sustentado vira
  pulsado irregular); gagueira granular; **ressonância simpática** num pente
  afinado fora da série ("uma corda que não existe ressoa"). E as
  **quimeras**: o excitador de um instrumento no corpo de outro (prato
  friccionado, corda cantada, pele soprada).
- "Está muito amador… sonoridade mais sofisticada, profissional, que gere
  emoção" → **a produção** (mistura por voz, palco, salas, EQ — Izhaki,
  Owsinski): grande parte do som profissional vem da cadeia e da mistura.
- Medir antes de implementar: a sobreamostragem foi **descartada** porque a
  medida mostrou que o que aparecia fora dos harmônicos era a inarmonicidade
  natural das cordas, não aliasing.

**Para o Modular**: é a confirmação, vinda de outro instrumento, de que **a
identidade do Modular é o "inaudito, não protocolar"** — o Trioio foi buscar
nele justamente isso. Implicações: (1) as **quimeras** já são possíveis no
Modular por patch (excitador de um módulo no corpo de outro — NOISE/DRUM/
STRING → RESONATOR, MATTER, SPRING); merecem **receitas no guia** (fase
didática) mais do que módulos novos; (2) o **"anel com razão inarmônica do
espectro"** e o **"LPG re-disparado por um registrador caótico"** são
patches que o Modular pode tornar explícitos como receitas ou como modo de
módulo (o LPG + TURING já existem; conferir o Rungler no TURING).

**`docs/MAQUINA_DE_HARMONIA.md`** (lidos: a harmonia própria do Trioio, as
operações, o arquivo de cadências, o Brasil, o som da aura, Partch, a
mistura conceitual de cadências):

- **Harmonia que só um instrumento que escuta pode fazer**: acordes vindos
  dos parciais medidos (harmonia espectral do que se ouve); cadências do jogo
  ("quando rarefeito, cadência de engano"), cadências apócrifas; **cadência
  por timbre, densidade ou registro** — a chegada marcada pela fusão
  espectral ou pelo rarefazer, não pela função.
- **Operações sobre progressões** (como as da máquina de escalas):
  substituir (trítono, relativa, mediante, empréstimo), interpolar
  (dominante secundária, ii–V, diminuto de passagem), rearmonizar, **inverter
  a função** (T ↔ D, o contrapor), retrogradar, comprimir/expandir o ritmo
  harmônico, modular (pivô, nota comum, enarmônica, direta), misturar
  linguagens. **Arquivo de 51 cadências** com graus, condução e fonte.
- **O Brasil** (pedido do autor): o **sistema trimodal nordestino** de José
  Siqueira (lídio, mixolídio e lídio-mixolídio — a "escala nordestina", a
  acústica de Bartók); o baião de Luiz Gonzaga (I7 como tônica, IV7, ♭VII,
  Vm7); repente e aboio (bordão, sus4 sobre pedal); **Guerra-Peixe** e o
  Armorial (modal dissonante, áspero); o **Clube da Esquina** (♭II7M(♯11)
  como dominante, Vm7, sus4, pedais; "Novena" em Ré dórico); **Hermeto
  Pascoal** (cores 7M(9)(♯11), 7(♭9)(♯11), chegada cromática ao acorde meio
  tom acima, *Calendário do Som*).
- **O som da aura** (Hermeto): a fala ouvida como melodia — cada sílaba vira
  nota, dobrada em uníssono, e **cada sílaba ganha um acorde em que a nota
  da fala é uma cor** (9ª, ♯11, 13ª, 7M), nunca a fundamental (Costa-Lima
  Neto, IASPM Journal).
- **Partch**: otonalidade (1/1, 5/4, 3/2, 7/4, 9/8, 11/8) e utonalidade (o
  espelho), unidades do diamante de tonalidades, o pivô O ↔ U; "uma harmonia
  de movimento entre identidades, não de cadência".
- **Mistura conceitual de cadências** (Zacharakis, Kaliakatsos-Papakostas,
  Cambouropoulos, ISMIR 2015; Fauconnier e Turner): misturando a perfeita
  (V7→I) com a frígia do séc. XV (♭vii→I), **o sistema reencontra sozinho a
  substituição de trítono** (♭II7→I) — e a backdoor como mistura mais fraca.

**Para o Modular**: (1) **AURA** (nome a decidir) — SIGNAL-IN → altura (o
PIT do SCOPE) → um acorde por nota em que a nota ouvida é **cor**, nunca
fundamental, com o baixo andando pelo caminho mais curto: o "som da aura"
como módulo, a partir de uma voz ao vivo; (2) o **CHORD/HARMONY** ganham
linguagens brasileiras (nordestina trimodal, mineira, Hermeto) e os acordes
de **Partch** (O/U em entonação justa — combina com o QUANTIZER microtonal);
(3) as **operações sobre progressões** por CV (inverter função, retrogradar,
substituir) no HARMONY; conferir o que o HARMONY já tem.

**`docs/JOGO.md`** (lidos: as respostas do autor, os princípios de
Vitoriamario, as regras como objetos, o ciclo de vida, como inventar
regras, o estado do jogo, a troca de lugar no diálogo):

- **O autor**: "não se trata de seguir as normas de um jogo de Xenakis, ou
  matrizes de Caillois — essas teorias são interessantes, porém o Trioio
  deve ser capaz de **criar suas próprias regras**". Teorias são
  referências, não normas.
- **Vitoriamario** ("um ser autônomo — libertário — que se reinventa — se não
  existia passou a existir"), da fonte apócrifa *Apodrece e vira adubo*:
  "**confunde-te a ti mesmo**" (jogar contra os próprios hábitos: contradizer
  a própria regra, escondê-la de si, inverter uma tendência já consolidada —
  a defesa contra a cristalização) e "**no fim tudo apodrece e vira
  adubo**" (a regra que sai do jogo se decompõe nos seus campos, que
  alimentam as próximas).
- **A regra como objeto**: `quando <condição> · faça <ação> · por <alcance>
  · custo <recursos> · oculta? · fica?`; ciclo proposta → em jogo → ouvida →
  julgada → fica | volta | apodrece; nasce do material, de outra regra, do
  jogo, do autor, por apropriação desviada, do adubo, ou contra si mesma.
- **Critérios de "musicalmente interessante"**: audível (não só um número);
  coerente com o material (a quebra é gesto, não acidente); novidade que se
  aprende (surpreende sem virar ruído); **tem custo**; sobrevive à escuta do
  autor. Os quatro primeiros, medidos compondo com e sem a regra.
- **O estado do jogo** (energia, densidade, memória, coerência, surpresa,
  informação oculta, risco de colapso, liberdade de desvio) **nunca aparece
  como placar: "o jogo se ouve na música"**.
- A **troca de lugar** no diálogo (a resposta fala quando a referência fala)
  e o **"atropelo"** (as duas falam juntas), do catálogo do Studio.

**Para o Modular**: qualquer "módulo de jogo" (o GAME cogitado no
Asterismus) deve ser **regras próprias com custo e ciclo de vida**, não uma
matriz de Caillois ou de Xenakis embutida; o Aquorbium já tem uma gramática
de regras (`OpenRuleProgram`) e o Trioio tem o objeto-regra — juntos, são o
desenho de partida, a decidir com o autor. E o princípio "o jogo se ouve na
música, nunca como placar" vale para os mostradores do Modular.

**`docs/COMPOSICAO.md`** (lidos: dramaturgia; gramáticas narrativas):

- **Curva de tensão por estratégia** (Lerdahl & Krumhansl 2007; Farbood 2012;
  clímax na seção áurea, Howat 1983): seguir = a energia da referência;
  transfigurar = um arco imposto ao ponto áureo; contrapor = a curva
  invertida; interromper = cortes. **O clímax só existe quando é claro**; um
  **silêncio composto** (uma respiração) logo antes dele; o motivo sobe uma
  oitava no clímax e desce nos vales; os enfeites rareiam nos vales, o
  essencial nunca.
- **Gramáticas narrativas** (Freytag; **Kishōtenketsu** — a virada *ten*
  muda o material, não só o nível; ondas; platô; arco) e misturas de duas.
- **A lição medida**: na primeira escuta, três gramáticas soaram iguais. Os
  stems mostraram por quê: as notas seguiam a curva, mas **os gestos do
  Terceiro Olho soavam constantes e mascaravam a forma**. Depois que *tudo*
  passou a seguir a curva, a Freytag ganhou 11 dB entre vale e clímax.

**Para o Modular (Form Engine)**: a regra decisiva — **uma forma só se ouve se
todas as camadas a seguem**; um único módulo constante (um drone, um HALL
alto, um LOOPER) mascara a curva. Um Form Engine no Modular precisaria de um
caminho para alcançar todos os níveis (talvez pelo VARIA, que já toca todas
as fibras). E a respiração antes do clímax é um gesto simples e forte.

**`docs/OUTROS_INSTRUMENTOS.md`, `docs/MUSICALIDADE.md`,
`docs/REAPROPRIACAO.md`** (lidos em parte) — o **método comum** e a
**personalidade de cada um**, a nuance que organiza todo este estudo:

- Do `Metodo_RASGO.md` do Studio, citado pelo Trioio: **"mesma genealogia
  não significa mesma personalidade; diferença de instrumento não significa
  ausência de método comum"**. Cada instrumento decide por si a unidade
  musical, o tempo, **se compõe, reage, habita, corta, escuta, negocia ou
  performa**, a agência e os corpos. "Memória e regras podem ser comuns; no
  Aquorbium aparecem como hábitos de um ecossistema, no Trioio como
  estratégia situada e no Synth como política de uma composição por seed."
  E o Trioio, no seu portão: "**o Rasgo Modular dá módulos para quem toca**".
- **Gradação da reapropriação**: abstração (linguagem, escala, ciclo, forma)
  → gesto (contorno, groove, diálogo) → ressíntese de descritores → ressíntese
  fina → citação (o som de outra obra dentro da nova — "uma mudança de
  natureza, não de grau"); *détournement*, *découpe*, colagem, *cut-up*,
  música concreta, plunderphonics, pós-produção, **antropofagia** (Oswald de
  Andrade, 1928).

**Consequência para todas as propostas deste estudo**: trazer um conceito
de um irmão para o Modular é **traduzi-lo para a personalidade do Modular** —
um módulo que **quem toca** patcheia, com entradas, saídas e controles
legíveis, autônomo quando solto mas sem agência escondida que decida pelo
músico. O ecossistema do Aquorbium, a estratégia do Trioio e a política de
seed do Studio não entram como "cérebro" do Modular: entram como **módulos**
(PREDATOR, RESPONDER, RELATE, GAME…) que o músico pode ligar ou não.

**`docs/MEMORIA.md`, `docs/MOSAICO.md`, `docs/LINHAS.md`** (lidos):

- **Oráculo de fatores** (Allauzen, Crochemore e Raffinot, SOFSEM 1999;
  Assayag e Dubnov 2004 — o algoritmo público do OMax, reescrito sem consultar
  o código do IRCAM): aprende a sequência de alturas, liga cada ponto ao
  ponto anterior de **mesmo contexto** (link de sufixo) e navega seguindo
  adiante ou saltando pelo link — "cada par de notas seguidas continua sendo
  um par que existe na referência, mas a sequência não é mais a dela:
  **reconhecível e imprevisível**". Continuidade = 0,85 − 0,5 ×
  imprevisibilidade. Defeito instrutivo: saltar para um ponto aleatório quando
  o contexto nunca se repetiu derrubava a fidelidade de 96 % para 83 %.
- **Mosaico** (síntese concatenativa por corpus; Schwarz, CataRT 2006/2007):
  cada ataque vira uma unidade com descritores (energia, brilho, ruído,
  altura, duração); o alvo é o momento presente transformado pela estratégia;
  a escolha soma **custo do alvo** e **custo da emenda**; e **a unidade traz o
  timbre do instante de origem** ("o trecho do minuto 1 que volta no minuto 3
  traz o timbre do minuto 1").
- **Linhas por região** (divisão de fase zero offline; "a linha é a escuta de
  uma região do espectro, não de um instrumento") — o LISTEN do Modular já é
  esse conceito em tempo real.

**Para o Modular**: **ORACLE** (novo) — o oráculo de fatores como módulo:
aprende alturas (e gates) de uma entrada e improvisa navegando pelos links,
com um knob de **continuidade**; complementa o MARKOV (que só guarda pares)
com o reconhecível-e-imprevisível de frases inteiras. **MOSAIC** (novo, mais
pesado) — concatenação por descritores sobre um buffer vivo (SIGNAL-IN ou
LOOPER), escolhendo pelo som; parente do MEMORY/SAMPLER.

**`docs/SINTESE.md`, `DESCRITORES.md`, `ESCUTA.md`, `INFLUENCIAS.md`**
(lidos em parte):

- O Trioio toca **33 famílias de síntese**, das quais **nove são módulos do
  Modular** (WAVETABLE, PULSAR, MATTER, STRING, DRUM, OSC→FORMANT, OSC→SHAPE,
  RESONATOR soprado pelo ruído do Antitotem, OPERATOR, OSC→LPG) — o Modular
  já é fornecedor de corpos da família.
- **A escuta do autor como memória do instrumento**: aceitar +1, transformar
  +0,3 ("gostei da matéria, não do resultado"), rejeitar −1, com nota livre;
  o placar pesa as famílias, as escolhas e a sala da resposta julgada;
  esquecimento gradual; arquivo de texto do autor (apagar uma entrada desfaz
  o efeito). — **Para o Modular**: a seed e o PatchGenetics (MUDA/EVOLUI/CRUZA)
  poderiam aprender assim com o BANCO de seeds do autor (as seeds guardadas
  como "aceitar", as descartadas como "rejeitar"), alterando tendências, não
  fixando um gosto. Decisão do autor.
- **A árvore de influências, não um catálogo de estilos** — o autor:
  "todos os estilos possuem variações… nuances, intenções, que vêm da poética
  de cada artista"; "a ideia é criar algo que não se repita ou se limite em
  algo engessado"; "nos nutrimos com informações para criarmos algo novo";
  "**os instrumentos RASGO precisam dos conceitos**"; "**a repetição pode
  existir se o conceito precisar da repetição**". Nós com conceito, contexto
  e **traços como tendências** (chances, não regras), herança, polinização
  entre galhos distantes, mutação.
- Descritores: medir e não decidir; ataques pelo fluxo espectral com mediana
  local; HPSS para o pulso; croma do grave para acordes; saliência harmônica
  para a melodia na mistura.

Não lidos no Trioio: `LEITURA_E_GABARITO.md`, `REFERENCIAS_POR_MODULO.md`,
`PROMPTS_E_INTENCOES.md`, `REGRAS.md`, `design.md` e os de biblioteca e
formatos (operacionais).

**Ainda não lidos no Trioio** (registro antigo, substituído acima): `DESCRITORES.md`, `ESCUTA.md`, `LEITURA_E_GABARITO.md`, `SINTESE.md`,
`INFLUENCIAS.md`, `REFERENCIAS_POR_MODULO.md`, `MEMORIA.md`, `REGRAS.md`,
`design.md`, `PROMPTS_E_INTENCOES.md`.

**Para o Modular**: um módulo **RESPONDER** (nome a decidir) — escuta
descritores (vindos de LISTEN/SCOPE: nível, brilho, densidade de ataques,
altura) e publica um pequeno **barramento semântico** (energia, densidade,
brilho, silêncio…) segundo uma **estratégia** escolhida ou derivada
(seguir, com atraso, condensar, expandir, contrariar, interromper), com
memória curta — a ponte entre "o Modular ouve" e "o Modular responde"
que hoje se faz à mão com cabos. Conferir antes contra o que LISTEN, SCOPE e
o SIGNAL-IN já entregam.

### 4.5 Antitotem

Lidos inteiros: `docs/MEMORIAL_DE_PROCESSO.md`, `IDEIA_BREADBOARD.md`,
`COMPATIBILIDADE_CONCEITO_IMPLEMENTACAO.md`, `PESQUISA_CMOS_LUNETTA.md`;
lidas as partes de levantamento e de brief de `PESQUISA_DERIVA_GENERATIVA.md`
(CRI-DRF-001), `PESQUISA_RUIDO_GENERATIVO.md` (CRI-NOI-001) e
`PESQUISA_ACENTUACAO_GENERATIVA.md` (CRI-ACC-001). Ainda faltam:
SEQUENCER_GENERATIVO, MELODIA, ORQUESTRA_RELACIONAL, REPRESENTACAO,
COMPASSO, FLUXO_DE_SINAL e DESIGN.

O Modular já trouxe do Antitotem, com proveniência: o PLL (estudo
OSC5/4046 de `CmosVoice.h`), o DRIFT (`deriveFromMemory`: momentum,
âncora, viés), o CHAOS (poço duplo, com o "chute" que o próprio autor
achou necessário) e as cores azul/violeta/bit do NOISE. A leitura funda
mostra que o que falta não é mais um chip: é a **atitude** do instrumento.

**As nuances (o que a leitura de cabeçalhos não mostrava)**

1. **A conexão não é um cabo neutro.** No breadboard original, ligar dois
   pontos era passar por um resistor, um diodo, um capacitor ou um
   alto-falante — algo que atenua, retifica, filtra, acumula. "Feedback é
   uma cadeia de componentes e transformações, não um valor de ganho
   abstrato." No Modular todo cabo é um fio ideal; a matéria do caminho
   não existe. É a maior ausência.
2. **O desconhecido não é defeito.** Atitude de circuit bending: o
   resultado que aparece ao mudar rota e velocidade é material — mas isso
   "não autoriza curto-circuitar saídas" (proteção antes da cor; o 386
   "não deve ser usado como proteção contra conexões erradas").
3. **Cinco camadas sem relógio único** — deriva, ritmo, timbre, topologia,
   decisão —, cada uma no seu tempo, com memória de relações e
   trajetórias, não só de valores.
4. **"Uma função só é ativa quando tem rota e controle no painel."** É a
   mesma régua do Método (ativação ≠ presença): nada escondido em menu; o
   painel faz as rotas aparecerem como gesto (lição do SX-150).
5. **Fidelidade declarada.** O breadboard tem três modos (estudo,
   performance, laboratório) e o projeto distingue sempre: o que o
   componente faz (datasheet), o que a comunidade Lunetta fez, o que o
   Antitotem inventa. Nenhum módulo é anunciado como reconstrução
   histórica; o inventário de chips "não é ficção de completude".
6. **Seleção ≠ memória.** O S&H depois do 4051 separa duas ações que um
   scanner funde: escolher a posição e guardar a tensão dela.
7. **Determinístico que parece acaso.** Os QUATRO RELÓGIOS (40106 × 4
   numa escada R–2R) não sorteiam nada: são estados binários que se
   desencontram no tempo, e por isso mantêm "vínculo audível com seus
   clocks". É diferente do random stepwave (4006 + XOR: memória binária
   que se realimenta). Os dois podem coexistir, nomeados.
8. **O limiar é um acontecimento.** O mixer-comparador converte uma
   relação contínua em decisão: abrir um gate, trocar uma rota, reter um
   clock, devolver drive. O mixer polarizador expõe o **zero**: somar,
   opor, anular, inverter a direção de uma modulação.
9. **Combinação é timbre.** Mistura resistiva preserva relações de
   amplitude; por diodo introduz direção e retificação. "São conexões
   diferentes, não dois nomes para mix."
10. **O 4006 como espaço feito pela lógica.** Atraso de um bit, clockado,
    com taps somados: "não é reverberação transparente" — granulado,
    metálico, pulsante, sensível à velocidade. Preservar a relação clock /
    comprimento / taps / realimentação, "em vez de acrescentar um reverb
    genérico".
11. **Deriva ≠ modulação.** "Modulação transforma o presente; deriva
    transforma aquilo que o instrumento considera seu estado normal."
    Elementos do brief ainda ausentes no DRIFT do Modular: **CAPTURE**
    (fixar o estado atual como nova âncora), **resíduo** (depois de uma
    fase ruidosa não volta exatamente ao limpo — a "cicatriz", ligada pelo
    autor à estética RASGO: "ruptura, rasgo, desgaste, arqueologia
    sonora"), os modos de elasticidade (elástico / livre / limitado /
    atrator) e as **regiões de comportamento** (o mesmo campo, respostas
    não-lineares distintas por destino: "timing responde muito, pitch
    pouco, noise só acima de 0,5").
12. **Ruído como princípio de organização**, não tipo de sinal: matéria,
    modulador e acontecimento estrutural (campo lento → limiar → evento).
    O NOISE do Modular cobre bem a matéria; o acontecimento por limiar só
    existe espalhado (BOXCAR geiger, NOISE poisson).
13. **O acento é multidimensional.** Sete famílias (dinâmico, métrico,
    agógico, tímbrico, articulatório, de altura, estrutural — a taxonomia
    é a do acento fenomenal de Lerdahl & Jackendoff, 1983), hierarquia de
    níveis 0–3, **herança** (um acento forte contamina o seguinte),
    **supressão** (período refratário), causas que se combinam. O
    Aquorbium já tem, sem o nome, o "Accent Field" (`geometricAccent` →
    dureza, densidade de partículas, irregularidade). No Modular o acento
    é, módulo a módulo, um ganho.

**O que isto pede ao Modular** (propostas ao autor, nada implementado)

- **VIA** (nome a decidir) — o *cabo com matéria*: um módulo de passagem
  em que o caminho declara seu componente — resistor (atenua, soma
  preservando relação), diodo (limiar, retificação, assimetria;
  direção), capacitor (acoplamento AC, integração, memória curta: o
  caminho "lembra"), alto-falante (corpo mecânico ressonante que satura).
  Cada retorno nomeia seu elemento e seu tempo ("mistura com matéria").
  É a nuance 1 feita módulo, e cabe na régua do Método (o caminho tem
  presença audível). Conferir antes contra o SHAPE e o FILTER para não
  duplicar.
- **SHIFTREG** (proposta 9) ganha sua forma exata: o 4006 como espaço da
  lógica — 1 bit clockado, comprimento, taps somados, realimentação; e a
  variante "memória que vaza", em que os bits decidem **rotas de
  feedback**, não notas. O TURING já é um registrador de valores; o
  SHIFTREG é registrador de **sinal**.
- **QUATRO RELÓGIOS** (proposta 10, R2R) mantém o que importa: quatro
  taxas expostas, glide, e o vínculo audível com os clocks — nunca um
  sorteio.
- **COMPARADOR** com mixer polarizador: duas entradas com zero exposto
  (somar / opor / anular), saídas MIX, INV e COMP (limiar com histerese
  como acontecimento). Hoje o Modular não tem um comparador de uso geral.
- Experimentos da tabela do autor que cabem como **aperfeiçoamento** de
  módulos existentes, um por vez e com A/B: `quase-encontro` (o PLL já tem
  `lock` contínuo; falta o evento quando as frequências se aproximam ou
  se afastam, sem corrigir a afinação); `scanner fantasma` (o SWITCH lendo
  ao mesmo tempo o endereço presente e um residual atrasado); `one-hot
  ferido` (uma etapa que retém, pula ou devolve o pulso ao clock).
- **DRIFT**, aperfeiçoamento: CAPTURE (gatilho que fixa a âncora),
  resíduo (a volta nunca é idêntica) e modos de elasticidade. Antes, reler
  o código atual para ver o que o `anchor` e o `bias` já fazem.
- **ACCENT** (nome a decidir) — um campo de acento: várias causas
  (métrica, salto de altura, posição na frase, externa) somadas num valor
  contínuo com hierarquia, herança e supressão, distribuído em saídas
  diferentes (ganho, brilho, articulação). Prior art direta: o
  `geometricAccent` do Aquorbium e o acento do `AcidBasslineVoice` do
  Rasgo Synth (um flag, dois destinos).

Fica registrado o cuidado do próprio Antitotem com proveniência: os
esquemas lidos (Ray Wilson/MFOS, Ian Fritz, Richard Brewster, phono,
Kevin B., Phil Baljeu, Sam Hoshuyama) são **referência creditada de
princípio**. Nada de esquema, valores ou PCB entra no Modular.

## 3. Limites

Leitura por documentos e cabeçalhos; o conteúdo de cada mecanismo do Studio
não foi auditado linha a linha. Nada foi copiado. O Batruc foi só lido.
