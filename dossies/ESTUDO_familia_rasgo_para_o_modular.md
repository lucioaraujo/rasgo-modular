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

## 3. Limites

Leitura por documentos e cabeçalhos; o conteúdo de cada mecanismo do Studio
não foi auditado linha a linha. Nada foi copiado. O Batruc foi só lido.
