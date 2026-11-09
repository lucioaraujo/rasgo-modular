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

## 3. Limites

Leitura por documentos e cabeçalhos; o conteúdo de cada mecanismo do Studio
não foi auditado linha a linha. Nada foi copiado. O Batruc foi só lido.
