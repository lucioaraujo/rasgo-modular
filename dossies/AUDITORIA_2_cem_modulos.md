# Auditoria 2 — os 100 módulos: o que falta, o que se funde, o que a síntese ainda não tem, o Maestruc e o plugin

Data: 20/11/2026. Pedidos do autor, nesta ordem: "faça uma auditoria para
verificar quais módulos ainda não estão com suas finalidades ou recursos
completos, verifique também se existem módulos que poderiam se fusionar por
falta de propósito ou por melhoria musical - técnica - de síntese"; "pode se
pautar nas pesquisas do atlas ou mesmo do cardinal"; "também o que ainda
falta em termos de ideias de síntese ou pontos fracos do rasgo modular"; "e
agora que temos o maestruc se há algo a implementar para melhorar a
interação com o maestruc"; "também verifique o que já haviamos cogitado de
transformar o rasgo modular também em plugin, vst3, etc".

Estado: **diagnóstico e proposta; nada implementado**. Cada item espera a
decisão do autor. Continua a auditoria de 10/11
(`AUDITORIA_modulos_e_cardinal.md`, 69 módulos).

## 0. Método

- **Levantamento mecânico** dos 100 tipos de `ModuleCatalog.hpp`: arquivo,
  tamanho, testes que citam a classe, marcas técnicas (sobreamostragem,
  BLEP, ADAA, suavização, guarda de finitude), visor próprio no app
  (`paintDisplay` em `RasgoModularApp.cpp`), papel nomeado na seed.
- **Dossiês e TAREFAS**: as pendências escritas em cada dossiê, as escutas
  ainda não feitas e as caixas abertas.
- **Auditoria de 10/11**: o que ela pediu foi conferido no código (§1).
- **Cardinal 26.02**: os `plugin.json` dos 92 plugins
  (a extração local do Cardinal 26.02) e os estudos já feitos
  (`ESTUDO_cardinal_catalogo_e_licencas.md`,
  `PESQUISA_cardinal_baixo_e_osciladores.md`).
- **Atlas** (`RASGO_DOCUMENTATION/ATLAS_DE_REFERENCIA_RASGO(4).md`): as
  seções de síntese, IRCAM, KTH, NESS, Aalto e Mutable.
- **Maestruc**: `apps/juce/MaestrucLink.h` (cópia igual ao receptor,
  SHA-256 `7cc9164840db…`) contra o que o app liga de fato.
- O que está marcado **(a conferir)** é leitura de código, não medição.

## 1. O que a auditoria de 10/11 pediu e está feito (conferido no código)

| Pedido | Estado |
|---|---|
| MATTER finito com FM de áudio | feito |
| Estalos nas trocas (LOOPER, SWIRL, MEMORY, MIXER mute) | feito, `test_declick` |
| Medir aliasing dos 16 não-lineares | feito: só o OPERATOR pedia (4×) |
| Instâncias (passos 1 e 2) | feito; **passo 3 (a seed abrir cópias) em aberto** |
| OSC: phase distortion, unísono, sub-harmônico | feito (`PD`, `UNI`, `SUBD`) |
| ENVELOPE LOOP; VCA com saturação | feito (`LOOP`; `SAT` com ADAA) |
| QUANTIZER peneiras + afinações | feito (`M1/R1/M2/R2`, `AFIN`) |
| HARMONY neo-Riemann | feito (`CICLO`) |
| SWITCH fantasma; SEQUENCE cursor físico e fadiga | feito (`FANT`; `INERC/FORCA/ALVO/FADIG`) |
| MATRIX rota com história; WAVETABLE deformação; SHAPE rasgo | feito (`HIST`; `DEF/MODO`; `TEAR`) |
| DRIFT resíduo e elasticidade; CHAOS mais atratores | feito (`RESID/ELAST`; `MODO`) |
| CLOCK phasing e acento métrico | feito (saídas `FASE`, `MET`, `ACC`) |
| Módulos novos aprovados (COERÊNCIA, TOM, BLEFE, APÓCRIFO, TAP, FRASE, LADDER, VIA, RELATE…) | feitos |

## 2. Módulos com finalidade ou recursos incompletos

### 2.1 A escuta: o maior ponto em aberto

Pelo menos **33 módulos nunca foram escutados pelo autor**. São eles:

- **Dos dossiês**: RUNGLER, SEASON, GENDYN, ARP, SPRING, PRISM, MOUSSE,
  DYNAMICS, PENDULUM, VIA, EPICYCLE, INTERPRET, RELATE, CUT-UP, POLYGON,
  INTERFERENCE, PRESSURE, BUDGET, ELASTIC, SWARM, REFRACT, LENS, TAPE,
  PREDATOR, ECLIPSE.
- **Do TAREFAS**: CELL, MARKOV, BOUNCE, SCOPE (o novo), STRING (afinada),
  FORMANT, MAGNET, ORBIT.

Um terço do catálogo está pronto no código, mas sem confirmação musical. O
que vem nas seções abaixo, sobretudo as fusões, depende dessa escuta. A
proposta é escutar em lotes por família, com as receitas do guia como
roteiro, e anotar em cada um "fica / ajustar / fundir".

### 2.2 Visores: onde o visor é a finalidade

Só **14 tipos têm visor próprio**: ARP, BOUNCE, CELL, COHERENCE, DYNAMICS,
KEY, LISTEN, MARKOV, MASTER, MIXER, MOUSSE, PENDULUM, PHRASE e TRIGSEQ,
além do SCOPE, que é o próprio osciloscópio. O traço genérico basta para
as fontes e os processadores de áudio.

Ele **não basta** nos módulos em que a figura é o que o módulo quer dizer:

- **PLANAR**: o gesto gravado é um caminho no plano XY, e hoje não se vê.
- **TAP**: o andamento medido e a dispersão das batidas.
- **TURING**: os bits do laço.
- **POLYGON**: os polígonos e as coincidências.
- **ORBIT, EPICYCLE, ECLIPSE, SEASON**: o céu.
- **MAGNET**: a grade de Ising.
- **PRESSURE**: a caixa com o pistão.
- **REFRACT**: o raio nas camadas.
- **INTERFERENCE**: as franjas.
- **PREDATOR**: o ciclo das duas populações.
- **SWARM**: as seis fases.
- **BUDGET**: o teto e quem gastou.
- **VIA**: as portas abertas.
- **RELATE**: o ato em curso.

São cerca de 18. Como é estética, entram em lotes com imagem para
aprovação. A ordem sugerida começa por PLANAR, TAP, TURING e POLYGON,
porque neles o uso depende de ver.

### 2.3 Papéis na seed

- Os 12 módulos novos (#91–#102) entram pela passada genérica da família.
  O papel dedicado de cada um fica para depois da escuta.
- A seed ainda não abre **cópias** de um mesmo módulo (instâncias, passo 3).
  Por isso não existem patches com dois osciladores desafinados ou dois
  envelopes, que são a gramática básica de um modular.
- A poda do processador sem entrada ainda espera a escuta.

### 2.4 Recursos que faltam, módulo a módulo

Os itens abaixo estão escritos nos estudos ou dossiês e conferidos como
ainda ausentes no código.

| Módulo | O que falta | Origem |
|---|---|---|
| QUANTIZER | escalas do Trioio (aprovadas 9/11); Scala, graus em cents e período configurável; seleção legível | TAREFAS 9/11; estudo de escalas |
| TURING | transformação reversível (o estado-base não é reescrito) + MEMORY de passos protegidos — como modo, para não mudar a identidade | estudo da família (Navalha), aprovado |
| CLOCK | agrupamentos aditivos (aksak, 3+3+2), swing kepleriano; entrada `RATE` multiplicativa (ver a proposta FORM/TEMPO) | estudo §4.1 (Asterismus) |
| TRIGSEQ | peneiras de Xenakis ao lado do euclidiano | estudo §4.1 |
| ORBIT | swing kepleriano como saída de clock e Doppler (velocidade radial → microafinação); hoje as saídas são CONJ, PERI, X/Y, DIST | estudo §4.1 |
| PENDULUM | modo pêndulo elástico 2:1 | estudo §4.1 |
| BOUNCE | paredes de estádio (Bunimovich) e spin (Magnus) | estudo §4.1 |
| PHRASE | as seis transformações de motivo (imitação, inversão, retrógrado…) — **(a conferir)** o que já faz | estudo §4.1 |
| LPG | subida dependente do nível; segunda cabeça (o filtro ressonante que soa sozinho, como o LPG do Rings) | dossiê 25 |
| COHERENCE | ligação do COLAPSO ao reset do sequenciador ou à FRASE do PALCO | dossiê 72 |
| LISTEN / SCOPE | os descritores em fila: densidade e pausas, timbre em movimento (centroide, planura), dinâmica do gesto, novidade local, groove contra o relógio, memória curta | TAREFAS (escuta, etapas 2+) |
| NOTE-OUT | a partitura exportável (MusicXML/MIDI) | dossiê 38 |
| SIGNAL-IN | escolher o dispositivo de entrada; MIDI só monofônico (última nota) | auditoria 10/11 |
| CHORD | acordes por tabela fixa; ligar à harmonia por CV já é possível, falta a receita | auditoria 10/11 |

## 3. Fusões: onde dois módulos fazem a mesma coisa

Critério: fundir só quando um dos dois **não tem propósito próprio** ou
quando a fusão **soa melhor ou simplifica o patch**. Fundir tem custo, porque
os patches salvos com o tipo antigo precisam continuar abrindo (um apelido
de tipo resolve).

### 3.1 Recomendo

1. **MULT → SUM (transformar, não apagar).**
   - **Por que o MULT perdeu o propósito:** no Modular uma saída já alimenta
     vários cabos, e só a **entrada** aceita um cabo (`SignalGraph.hpp`:
     "cada porta de entrada recebe UMA conexão; a soma é feita por um
     módulo `Sum` explícito"). O MULT copia o que o cabo já copia. O que
     sobra dele, escala e slew por cópia, o CONTROL já faz.
   - **O que realmente falta:** a soma. Hoje, para somar duas CVs, é preciso
     ocupar um MIXER, um VCA4, a MATRIX ou o ABACUS.
   - **Proposta:** o MULT vira um **SUM**, com 4 entradas de CV ou áudio,
     atenuversor em cada uma, uma saída de soma e duas cópias invertidas.
     Os patches antigos com MULT abrem nele.
2. **VCA e VCA4 — um núcleo só, dois painéis.** O VCA tem SAT com ADAA e
   sobreamostragem, e o VCA4 não tem nenhum dos dois. Basta o VCA4 usar o
   mesmo canal do VCA. Não é fusão de módulos, é fusão de código: o mesmo
   som nos dois.
3. **O caráter de fita num só lugar.** TAPE, a AGE do LOOPER, a AGE do
   SWIRL (BBD) e o WEAR do SAMPLER e do TURNTABLE envelhecem o som cada um
   a seu modo. A proposta é um núcleo `TapeCharacter` com a saturação do
   TAPE, usado nas AGE: o desgaste soaria igual em toda a família de
   buffers. Os módulos continuam todos.

### 3.2 Decidir depois da escuta

4. **ELASTIC dentro do RESONATOR?**
   - **Semelhança:** os dois são modos excitados por um sinal de fora. O
     ELASTIC é uma série de modos dispersivos (a corrente de massas) e
     soa sozinho com o PLUK.
   - **Se fundir:** se na escuta ele soar como "um RESONATOR com outra
     série", vira a série ELÁSTICA do RESONATOR, que ganha o PLUK.
   - **Se não fundir:** se o STRCH, que entorta o som, tiver caráter
     próprio, fica separado.
5. **SPACE e HALL → REVERB com dois algoritmos?**
   - **Hoje:** o SPACE é Schroeder/Dattorro e o HALL é uma FDN de 8 linhas.
     Com instâncias, um módulo com chave `ALGO` (e crossfade na troca) dá
     os dois.
   - **Contra:** a seed e o LEARN tratam os dois como cores diferentes.
   - **Recomendação:** fundir só se a escuta não achar diferença de uso.
6. **APÓCRIFO e CUT-UP.**
   - **Sobreposição:** o "parêntese que gagueja o agora" do APÓCRIFO é o
     STUT do CUT-UP.
   - **Proposta:** o APÓCRIFO fica com o passado longe (a citação) e o
     CUT-UP com o agora no relógio. Basta afinar os textos do LEARN, sem
     fundir.
7. **INTERFERENCE e POLYGON.**
   - **Semelhança:** os dois tratam a coincidência de fontes periódicas. O
     POLYGON é geometria de polímetro; o INTERFERENCE é soma e
     cancelamento com coerência parcial.
   - **Recomendação:** escutar em sequência e fundir só se o pulso de um
     for o do outro.

### 3.3 Parecem iguais, mas não são (manter)

| Par | Por que mantém |
|---|---|
| FORMANT (modo vocoder) / VOCODER | 5 bandas vocálicas × 4–20 bandas para fala; tirar o modo quebraria patches por pouco ganho |
| FILTER / LADDER / WASP / LPG | quatro caracteres de filtro de uso diferente (o grave pediu a escada) |
| GLIDE / slew do CONTROL e do SH | o GLIDE é portamento musical (subida e descida, proporcional) |
| SAMPLER / TURNTABLE | já dividem o buffer; o gesto é outro (fatia × prato com massa) |
| ORBIT / EPICYCLE / ECLIPSE / SEASON | quatro tempos do céu (frase, timbre, polirritmo longo, forma do dia); sugestão: agrupá-los na paleta como "céu" |
| LISTEN / SCOPE / KEY | faixas × altura e timbre × tom; os descritores novos devem ir para um só lugar (o LISTEN), para não espalhar |
| TURING / RUNGLER | o laço de bits como melodia × como oscilador caótico |
| SUPPLY / BUDGET | a energia do som × o teto de eventos |

## 4. O que ainda falta na síntese (com o Atlas e o Cardinal)

Ideias, não pedidos, para o autor escolher. Cada uma traz a lacuna que
preencheria.

| Ideia | Lacuna | Referência |
|---|---|---|
| **FORM e TEMPO** (proposta já escrita) | a forma da peça: crescendo, accelerando, coda | `PROPOSTA_forma_e_andamento.md` |
| **Polifonia** | o Cardinal tem cabos de 16 canais; aqui um acorde exige o CHORD ou várias vozes à mão. Decisão do autor de 18/10: "fora por enquanto". Com instâncias, uma **voz polifônica** fechada (SIGNAL-IN/MIDI → N vozes) seria o meio-termo | Cardinal; TAREFAS 18/10 |
| **Transposição de altura** (pitch shifter / harmonizador por grãos ou por vocoder de fase) | o SHIFTER move por Hz (inarmônico); não há transposição que preserve a harmonia | Atlas §64 (SuperVP) |
| **Síntese FOF / VOSIM** (a voz cantada do CHANT) | o PULSAR chega perto; um modo FOF daria a voz | CHANT (Rodet, IRCAM, 1984) — fora do Atlas, pesquisa a fazer |
| **Placa e membrana** (modelos 2D) | o reverb de placa e o tambor de pele; MATTER, STRING e RESONATOR são 1D ou modais | Atlas §103 (NESS, Edimburgo), §65 (Modalys) |
| **Síntese concatenativa** (o buffer ao vivo reordenado por descritores) | MEMORY é grão por posição, não por timbre | Atlas §62 (CataRT/MuBu) |
| **Improvisador com memória longa** (oráculo de fatores) | o MARKOV é de 1ª ordem e esquece | Atlas §63 (OMax) |
| **Seguir quem toca no tempo** | TAP mede, não acompanha; o TEMPO poderia seguir um pulso externo | Atlas §60.1 (Antescofo) |
| **Convolução** (resposta de impulso, a sala real) | os reverbs são algorítmicos | fora do Atlas (pesquisa a fazer) |
| **Áudio espacial além do estéreo** (ambisônica) | o Modular é estéreo; o Aquorbium já grava em AmbiX (Atlas, 23/10) — horizonte, não pedido | Atlas §100, §102 (IEM, SPARTA) |
| Fora: síntese neural (RAVE, nn~) | dependências pesadas e modelos com licenças próprias | Atlas §56–57 |

## 5. Pontos fracos do sistema (além dos módulos)

1. **Escuta atrasada** (§2.1): 33 módulos ou mais sem validação musical.
2. **A trava tela↔áudio.** O áudio ainda usa `gmx.try_lock()`. Quando a
   interface segura o lock, o áudio entrega um período de silêncio. É a
   origem conhecida dos estalos com o VARIA, que são xruns e não DSP. É
   também a **etapa 1 do plugin** (§7). Passar para uma fila de comandos
   ou troca de grafo resolve as duas coisas.
3. **A soma de sinais** (§3.1): uma entrada aceita só um cabo, e não há
   módulo de soma de CV.
4. **Visores genéricos** onde a figura é o módulo (§2.2).
5. **A seed sem cópias** (§2.3).
6. **Sem camada de forma** (§4, FORM/TEMPO).
7. **Catálogo de 100.** A paleta precisa de subfamílias legíveis (céu,
   óptica, física, ecologia) e da fase didática (receitas, site, PDF), que
   já está prevista. Sem isso, o músico se perde.

## 6. O Maestruc e o Modular (conferido no código)

**O que o Modular já liga** (`RasgoModularApp.cpp`, em torno da linha 6874):

- `seed`;
- `play` e `stop` (pela ESPERA);
- `state`;
- `mixerList` e `mixerSet`, com `mixerChanged`;
- a ponte de som com **um par**, a mistura.

**O que o protocolo oferece e o Modular ainda não usa**, em ordem de valor
por custo:

| # | Mensagem | No Modular | Custo |
|---|---|---|---|
| 1 | `/rasgo/1/tempo` (BPM comum) | o BPM do CLOCK; hoje o handler está vazio, então o Modular não segue o andamento da orquestra | pequeno |
| 2 | `setAudioLoad` + `budget` | carga e xruns ao Maestruc; o nível econômico desliga o VARIA e reduz a sobreamostragem | pequeno |
| 3 | `pushEvent` | acontecimentos: troca de seed, ONSET do SCOPE/LISTEN, o `CLMX`/`SECT` do FORM; o Maestruc grava e pode reagir | pequeno a médio |
| 4 | ponte de som com **stems** | os 4 canais do MIXER como pares separados; o Maestruc mistura e grava cada voz (hoje só a soma) | médio |
| 5 | `rec/start`, `rec/stop` | os stems internos e a partitura (`ScoreRecorder`) na pasta do take | médio |
| 6 | `/rasgo/1/key` | a tônica e o modo no HARMONY/QUANTIZER (ROOT, SCALE); com o KEY, o Modular também **publica** o tom que ouve | médio; pede decisão do que é "tom comum" no Modular |
| 7 | `section`, `macro` (`intensidade`, `densidade`) | o lugar natural é o FORM (seção, DEPTH) e o VARIA; sem o FORM, `section` poderia ser um MUDA | depende do FORM |
| 8 | `open` | abrir um patch `.rasgo` pelo Maestruc | pequeno |

Do lado do Maestruc, as frentes 1 (relógio), 4 (forma) e 5 (dinâmica)
estão abertas e pedem os itens 1, 3 e 7. Recomendação: **1, 2 e 3 numa
leva** (pequenos, sem estética). Os itens 4 e 5 vêm junto com a gravação de
stems dos outros instrumentos.

## 7. O plugin (VST3, CLAP, AU): o que já foi cogitado

**Registrado** em `DESENVOLVIMENTO.md` e no TAREFAS (4/10), a pedido de um
usuário do Audiofanzine e do Palmieri; opções no estudo do Cardinal (§6).

- **Condições do autor:** sem mudar a licença, sem pagar tarifas e sem
  prejuízo ao instrumento.
- **Viabilidade:**
  - JUCE em AGPL;
  - SDK do VST3 aberto (GPLv3, anunciado também como MIT no fim de 2025;
    conferir ao começar);
  - CLAP em MIT;
  - sem o logotipo "VST" sem o acordo de marca, que é gratuito.
- **Caminho em etapas:**
  1. tirar a trava tela↔áudio;
  2. standalone sobre o wrapper do JUCE;
  3. VST3 experimental em Linux e Windows: estado salvo no projeto do DAW,
     CLOCK seguindo o host, MIDI do host no SIGNAL-IN, poucos macros para
     automação;
  4. `pluginval` na CI e testes em Ardour, Reaper e Infinite;
  5. CLAP;
  6. AU quando houver Mac.
- **Opções do estudo do Cardinal:**
  - JUCE, não DPF;
  - SIGNAL-IN, NOTE-OUT e MASTER como os "módulos do host";
  - duas variantes desde o começo, **Synth** e **FX**;
  - parâmetros do host como fonte de modulação;
  - o transporte do host no CLOCK.
- **Decisão aberta:** "alvo de plugin (JUCE, Synth primeiro)".

**Estado hoje** (conferido):

- **Nada começou.** `apps/juce/CMakeLists.txt` só tem `juce_add_gui_app`.
- A etapa 1 continua aberta (§5.2).
- **Ponto de atenção:** "um plugin que cai derruba o DAW de quem usa". A
  prioridade escrita é o standalone estável nos três sistemas.

**Fora do plugin, também registrado:**

- **Módulos para VCV Rack/Cardinal:** exigiriam oferecê-los também sob
  GPLv3+. As seeds, o VARIA, os cabos com relação e o LEARN ficariam de
  fora.
- **A ponte polifônica de 16 canais** proposta pelo clone45: registrada,
  sem promessa.

**Recomendação:** a etapa 1 (a fila de comandos) vale por si, porque tira
os estalos do VARIA. Ela pode entrar antes de qualquer decisão sobre o
plugin. As outras etapas esperam a decisão do autor.

## 7b. Funcionar com outros softwares (o que já foi cogitado)

Pedido do autor (20/11): "para que funcione com outros softwares e
tecnologias, cardinal, vcv, já haviamos cogitado alguns deles, inclusive
aquele que o Palmieri me apresentou". Levantado nos registros; nada novo
implementado.

| Caminho | Onde está registrado | Estado hoje | O que pede |
|---|---|---|---|
| **Áudio e MIDI pelo sistema** (PipeWire/JACK, BlackHole, VB-Cable; MIDI no SIGNAL-IN, NOTE-OUT para fora) | `DESENVOLVIMENTO.md` (Infinite) | **funciona hoje**, como programas separados | nada; falta uma receita no site |
| **Infinite** (DAW de nós de Naman Soni, MIT; apresentado pelo Palmieri) | `DESENVOLVIMENTO.md` | hoje pelo roteamento acima; com um VST3, o Modular rodaria dentro dele | o plugin (§7) |
| **Plugin VST3 / CLAP / AU / LV2** | `DESENVOLVIMENTO.md`; estudo do Cardinal §6 | não começou | etapas do §7 |
| **Módulos para VCV Rack e Cardinal** | `DESENVOLVIMENTO.md`; TAREFAS (fórum do VCV, 6/10) | não começou | casca `rack::Module` por módulo, painéis SVG das medidas, tensões ±5 V / 0–10 V / 1 V/oct; **oferecer também sob GPLv3+** (o Cardinal exige); seeds, VARIA, cabos com relação e LEARN ficam de fora |
| **Ponte polifônica de 16 canais** ("Rasgo Connector" no VCV ↔ "VCV Connector" no Rasgo; proposta do clone45) | TAREFAS (fórum do VCV) | registrada, sem promessa | um plugin do VCV em GPLv3 |
| **Maestruc** (OSC, `maestruc.link/1`) | `PROTOCOLO_MAESTRUC_LINK.md` | parcial (§6) | os itens do §6 |
| **Ableton Link** (relógio comum em rede: andamento, batida e fase) | Atlas, levantamento de 16/11 (Maestruc) | candidato do Maestruc, não decidido | biblioteca de cabeçalhos (GPLv2+ → usável com AGPL); experimento de deriva em 10 min antes de decidir |
| **MIDI clock de entrada** (seguir DAW ou máquina externa) | — (não achei registro) | **não existe**: o ALSA do Modular não lê 0xF8/start/stop; o CLOCK aceita clock externo só pelo jack `EXT` | ler o MIDI clock no SIGNAL-IN e sair como gate e reset |
| **NSM / sessões** (abrir e salvar a orquestra inteira) | Atlas 16/11 | referência | depois do Maestruc |
| **Partitura** (MusicXML/MIDI do NOTE-OUT) | dossiê 38 | não existe | exportação |
| Pure Data, Faust, Bespoke, ossia score | `RASGO_MODULAR.md` §30; Atlas §4–7 | fontes e referências, não pontes | — |

Ordem sugerida: (1) receita de roteamento com outros programas (custo
zero, só documentação); (2) MIDI clock de entrada; (3) trava tela↔áudio,
que abre o plugin; (4) VST3 Synth experimental, testado no Infinite, Ardour
e Reaper; (5) módulos VCV/Cardinal, se o autor decidir a dupla licença
GPLv3+ dos módulos.

## 8. Ordem sugerida (para o autor escolher)

1. **Técnico, sem estética:**
   - a trava tela↔áudio (§5.2);
   - o MULT → SUM (§3.1);
   - o núcleo do VCA no VCA4 (§3.1);
   - Maestruc 1, 2 e 3 (§6).
2. **Escuta em lotes** (§2.1), com a decisão "fica / ajustar / fundir" de
   cada um (§3.2).
3. **FORM e TEMPO**, se aprovados (a proposta à parte).
4. **Visores** em lotes com imagem (§2.2).
5. **Recursos por módulo** (§2.4): QUANTIZER (Trioio/Scala) e TURING
   (reversível) primeiro, porque já estão aprovados.
6. **Ideias de síntese** (§4), uma por vez.
7. **Plugin** (§7), depois da etapa 1 e com o standalone estável.
