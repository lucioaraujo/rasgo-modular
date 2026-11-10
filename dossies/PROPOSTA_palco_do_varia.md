# Proposta — o PALCO do VARIA (gerência inteligente de pan)

Data: 10/11/2026. Estado: **proposta, nada implementado**. Pedido do autor:
"há uma gerência de pan inteligente no varia?" → "sim, prepare a proposta";
e a revisão de uma decisão dele: "podemos rever essa minha decisão de o
varia não mexer no pan do mixer".

## 1. O que existe hoje (conferido no código)

- **MIXER**: 4 canais **mono**, pan de potência constante (sin/cos; no
  centro, 0,707 em cada lado). O canal inteiro vai para o lado, **grave
  incluído**.
- **Seed**: todos os pans nascem em 0 ("o autor abre o palco à mão").
  A largura do MASTER é sorteada entre 0,85 e 1,5.
- **VARIA** (`MotionEngine.hpp`): **não toca MIXER nem MASTER** ("o músico
  mistura, a engine não"). Os `spread` de FILTER/SPACE/PRISM/SWIRL/PULSAR
  andam como qualquer parâmetro livre, sem noção de palco. O VARIA já
  recebe a **energia do som** (0–1) e acelera a mão com ela.

## 2. O que a família já sabe (lido no código, 10/11)

| Instrumento | O que faz | A nuance |
|---|---|---|
| **Asterismus** `PerformanceSkyPan.hpp` | a posição x da estrela na Constelação vira o lugar no estéreo | **grave no centro**: abaixo de ~150 Hz por um polo, agudo = sinal − grave, soma exata; **nunca além de 70 %** de um lado; ESPAÇO 0 = mono exato; lei de potência igual normalizada para o centro manter o ganho anterior; posição alisada em ~30 ms |
| **Navalha 2** `AssistedPerformer.cpp` (AUTO MIX) | por frase, com probabilidade 0,08 + 0,22·intensidade | as duas fontes têm **lados opostos** (casa = ±(0,06 + 0,12·energia)); o pan novo = 0,72·atual + 0,28·casa + deriva — **inércia**, não salto; teto de 0,28 a 0,48 conforme a energia (nunca no extremo); largura 0,85–1,25; só balanço, pan e largura |
| **Aquorbium** `Ecosystem::updateMix` + `AquariumSpatializer` | o lugar de cada organismo **emerge de quem o observa**: quatro observadores fixos no palco (−0,78, +0,78, −0,26, +0,26), cada um pesa pelo quanto vê e escuta | o pan é **consequência da percepção**, não atribuído; o canal mais cheio é normalizado a 0,92; **profundidade** = passa-baixa (o longe é mais escuro), separada do pan |
| **Rasgo Synth Studio** `SpatialExpressionField.hpp` | intenção espacial vira destino (pan, largura, profundidade, deriva) | **consciente do mono**: um fator `monoSafety` encolhe pan e largura; largura máxima 0,85; deriva com velocidade limitada |

## 3. Princípios (o que a proposta respeita)

1. **O pan é relação, não sorteio**: as camadas ocupam lugares
   **complementares**; duas camadas não pesam o mesmo lado.
2. **O grave fica no centro** (Asterismus): mover um canal não pode fazer o
   grave pender nem sumir em mono.
3. **Nunca no extremo** (Navalha 0,48, Asterismus 0,70): o palco abre, não
   rasga.
4. **Inércia**: o lugar muda devagar e carrega o anterior (Navalha 0,72);
   alisado para não ouvir degraus.
5. **Abertura pela energia**: denso e forte → palco mais aberto; rarefeito
   → mais junto (Navalha: teto cresce com a energia).
6. **Mono seguro** (Studio): a soma mono não pode perder nem ganhar
   volume por causa do pan.
7. **O músico manda**: mexeu no PAN à mão, aquele canal sai do VARIA até
   você devolver (mesma regra que o VARIA já usa para parâmetros
   modulados por cabo).

## 4. A proposta

### 4.1 PALCO — uma camada nova do VARIA, ligável

Um botão **PALCO** ao lado do VARIA (ou um modo dele), desligado por
padrão até a sua escuta. Ligado, o VARIA passa a mover **só os quatro PAN
do MIXER** — nunca os ganhos, nunca o MASTER.

- **Casas complementares**: cada canal ativo recebe uma casa no palco,
  espalhadas pelo número de canais que soam (1 → centro; 2 → ±; 3 →
  esquerda/centro/direita; 4 → duas internas e duas externas, como os
  observadores do Aquorbium).
- **Quem vai para onde**: a casa mais central para a **voz principal**
  (canal 1, a espinha da seed); as externas para as camadas.
- **A mão caótica do VARIA** desloca cada canal em torno da sua casa,
  devagar, com inércia; os quatro se movem **em relação** (se um vai à
  esquerda, outro tende a compensar), sem cruzar a casa vizinha.
- **Abertura** = f(energia do som): de ±0,15 em silêncio a ±0,55 no
  máximo (teto 0,70, do Asterismus).
- **Alisamento** de ~50 ms no próprio MIXER. Conferido: hoje o MIXER **não
  alisa** ganho nem pan (o valor vale no bloco inteiro), então o
  alisamento entra junto — é o que evita o clique quando o pan anda.

### 4.2 Grave no centro — no MIXER, para todos

Hoje o canal vai inteiro para o lado. Proposta: o MIXER separa o grave de
cada canal (um polo em ~150 Hz, agudo = sinal − grave, soma exata) e só o
agudo segue o pan; o grave fica no centro. Vale com ou sem PALCO, também
para o pan à mão. **Muda o som de patches com pan aberto** — por isso é
item separado, para você aprovar ou não.

### 4.3 Fora desta proposta (pode vir depois)

- **Profundidade** (Aquorbium): longe = mais escuro e mais baixo. Seria
  um segundo eixo do palco; mexe em timbre e ganho, então fica para outra
  conversa.
- **O pan como consequência da escuta** (Aquorbium): o lugar emergir de
  observadores, não de casas. É mais original e mais caro; vale se o PALCO
  simples convencer.
- Os `spread` dos módulos continuam como estão.

## 4b. Caminho B — o MIXER recebe sinais de pan (sugestão do autor, 10/11)

"Ou aprimorar o módulo mixer para receber sinais que propiciem variação de
pan." Conferido: hoje o MIXER tem só as 4 entradas de áudio; e o app **não
permite ligar cabo num knob** (as modulações de parâmetro só nascem da DRIFT
da seed). Então o pan, hoje, só se move à mão.

Proposta B:

- **Quatro entradas `PAN 1–4` (CV, −1..+1)** no MIXER, somadas ao knob PAN
  de cada canal. O knob vira o **centro** do movimento; a CV o desloca.
  Qualquer módulo move o palco: DRIFT (deriva lenta), PENDULUM (x1/x2 —
  dois pêndulos acoplados, dois canais que se respondem), BOUNCE, CHAOS,
  MOUSSE, LFO… É o músico (ou a seed) quem escolhe a causa, com cabo
  visível — a regra do Antitotem: "uma função só é ativa quando tem rota e
  controle no painel".
- **As salvaguardas ficam no próprio MIXER**, valendo para knob e CV:
  teto de abertura (nunca o extremo), **alisamento** (~50 ms, hoje não há
  nenhum) e, se você aprovar, o **grave no centro** (4.2).
- **A seed pode ligar** de vez em quando uma fonte lenta a uma entrada PAN
  (como já liga a DRIFT em cutoff e ressonância), com profundidade
  contida — o palco passa a variar sem regra nova no VARIA.
- **Painel**: o MIXER tem 14 HP e as tiras estão cheias (fader, PAN, MUTE,
  jack). O jack `PAN` de cada canal entraria entre o MUTE e o jack de
  áudio; se não couber com folga, o MIXER cresce para 16 HP. Layout a
  mostrar para você antes de fechar.

**A e B não se excluem.** B é a base: dá a entrada, as salvaguardas e serve a
qualquer fonte. O PALCO (A) pode vir depois, como **mais uma fonte** — o
VARIA passaria a mandar sinais para essas mesmas entradas, em vez de girar o
knob. Minha recomendação: **começar por B**.

## 4c. Pan que não é centro nem sorteio (pedido do autor, 10/11 noite)

"Precisamos encontrar um meio de trabalhar o pan que não seja no centro,
porém que não seja aleatório, com bom balanço, trajetos interessantes."

### O que o Cardinal tem (lido no código, `cardinal-26.02/plugins`)

| Módulo | Ideia | Nota |
|---|---|---|
| stoermelder **Orbit** (GPL-3) | a cada gatilho, cada voz ganha um lugar novo (normal, normal espelhada, uniforme ou externa) e depois **deriva** linearmente para o centro ou para as bordas; filtro de 5 ms contra clique | o lugar é sorteado; lei de pan linear (cai 6 dB no centro) |
| forsitan **pavo** (GPL-3; o Splay do SuperCollider) | N vozes **espaçadas por igual** entre −spread e +spread, em torno de um CENTER; ganho compensado por √(1/N) | distribuição, não trajeto |
| stocaudio **Spread** (GPL-3) | o mesmo espaçamento por igual, linear | idem |
| GlueTheGiant **MetroCityBus** (GPL-3) | **pan follow**: um único trajeto de pan (CV); a voz *c* repete o trajeto com atraso *c*·d — **cânone no espaço** | o mais musical do lote |
| Bogaudio **FourFO** / HetrickCV **PhasorQuadrature** | quatro fases a 90° | quatro canais em **roda** |
| admiral **Shifts** (MIT) | balanço de Bernoulli: o peso pula entre dois lados por gatilho | é sorteio |
| MindMeld **BassMaster** | grave mono, agudo aberto | já temos (grave no centro) |
| Venom **Pan3D** | X/Y/Z para 8 saídas | fora do nosso estéreo |

Nenhum deles cuida do **balanço** entre as vozes: cada uma anda por si.

### Sugestão: palco com contrapeso

1. **Casas** complementares por papel (voz perto do centro, contravoz e
   eco em lados opostos, textura do outro lado), lado decidido pela seed.
2. **Um trajeto por seed**, escolhido entre poucos e nomeados:
   - **RODA** — os canais giram em torno das casas com fases defasadas
     (90° com 4 vozes, 120° com 3); a soma das posições é zero por
     construção;
   - **CÂNONE** — a voz faz um trajeto lento; a contravoz o repete
     atrasada e espelhada, o eco depois dela (ideia do pan follow);
   - **GESTO** — a cada frase (EOS do ARP, fim do ciclo do CLOCK) o canal
     vai para um lugar novo **escolhido para reequilibrar** (o lado com
     menos peso), com deslize de 1–4 s (a deriva do Orbit, a inércia da
     Navalha);
   - **PÊNDULO** — os dois pêndulos acoplados do PENDULUM movem contravoz
     e eco (precisa das entradas PAN do caminho B).
3. **Contrapeso** (a parte nossa): o MIXER mede o peso de cada canal (RMS
   lento) e o centro de gravidade do palco; se ele passa de ±0,12, as
   casas inteiras deslizam devagar para o lado oposto. Voz forte à
   esquerda puxa as outras para a direita. Balanço sem prender no centro.
4. Salvaguardas: abertura máx. 0,55 (aprovada), grave no centro (feito),
   lei de potência constante, velocidade limitada, nada de salto.

Onde mora: no MIXER (um seletor PALCO: CENTRO / RODA / CÂNONE / GESTO,
mais uma taxa), com as entradas PAN 1–4 do caminho B para fontes
externas. A seed escolhe o trajeto; o VARIA pode mexer só na taxa e na
abertura. Conceito estudado, nenhum código copiado.

## 4d. Decisões de 10/11 noite e o contrapeso elaborado

Autor: "os quatro servem; contrapeso ativo, consegue elaborar de uma forma
original?; no mixer".

### Os mixers do Cardinal (126 módulos com etiqueta Mixer/Panning, lidos)

O que existe lá: soma e VCA (Fundamental, Befaco, Venom, Bogaudio MIX4/8,
mscHack); barramentos e grupos (MindMeld MixMaster — lei de pan
escolhível, largura, DIM, mono; GlueTheGiant); matrizes (Bogaudio
MATRIX, stoermelder INTERMIX); crossfade vetorial (HetrickCV Vector Mix,
Venom Cross Fade 3D); compressão e ducking (CVfunk Pressed Duck);
saturação de console (rackwindows Console); fades automáticos (mscHack
ASAF8); instantâneos (Bidoo ACnE, Frames); grave mono (MindMeld
BassMaster); espalhadores (Orbit, pavo, Spread, MetroCityBus); e o
stoermelder **ARENA**: fontes e pontos de escuta num plano XY, o nível
cai com a distância dentro de um raio, e os pontos andam por trajetos
**desenhados à mão** (16 sequências de 128 pontos) ou por passeio
aleatório.

Procurado no código: nenhum mixer do Cardinal pesa o estéreo pelo **som
que está tocando** — todo pan é posição dada (knob, CV, sorteio, trajeto
gravado). O que o nosso MIXER já tem de diferente: grave no centro por
canal com soma exata (só o BassMaster faz algo parecido, e no barramento
inteiro), joelho de barramento transparente até 0 dBFS, e as quatro
vozes com papéis compostos pela seed.

### O contrapeso como MÓBILE (a proposta original)

Referência: os móbiles de Alexander Calder — peças penduradas em braços
que se equilibram pela **lei da alavanca** (peso × distância de um lado =
peso × distância do outro). O vento os move; eles nunca caem para um lado.

No MIXER:

1. **Peso = o som de agora.** Cada canal pesa a sua energia lenta (~1,5 s)
   — só a parte aguda, a que de fato vai para o lado (o grave já mora no
   centro e não pesa). Canal calado não pesa e não puxa nada.
2. **Alcance pela alavanca.** Quanto mais pesado, mais curto o braço: o
   alcance de cada canal é 0,55 × √(peso médio / peso dele), entre 40 % e
   100 %. A voz forte fica perto do centro sem regra de "voz no centro";
   o eco e a textura, leves, vão longe. Se a voz cala, o braço dela cresce.
3. **Equilíbrio.** O centro de gravidade do palco (Σ peso·posição / Σ
   peso) fica dentro de ±0,10. Passou disso, o móbile se corrige — e quem
   anda é o **leve**: a correção de cada canal é proporcional a 1/peso
   (mínimo esforço para uma inércia que cresce com o quadrado do peso).
   A voz quase não se mexe; as camadas atravessam.
4. **Assentamento.** A correção tem inércia (~2 s) e um leve
   amortecimento: depois de um golpe forte de um lado, o palco **balança e
   assenta**, como o móbile depois de tocado — não se corrige seco.
5. **O vento são os trajetos** (RODA, CÂNONE, GESTO, PÊNDULO): eles
   propõem; o móbile dispõe. A energia do som (já medida pelo VARIA) pode
   ser o vento mais forte ou mais fraco.

Resultado: nem centro, nem sorteio — o lugar de cada voz é a consequência
do peso dela e do peso das outras. Um acento do DRUM à direita empurra a
contravoz para a esquerda por um instante; uma nota longa da voz
desloca o conjunto devagar.

### Proposta de painel (para o autor ver antes)

O MIXER passa de 14 para 18 HP; as quatro tiras ficam como estão e entra
uma coluna PALCO à direita:

```
 CH1  CH2  CH3  CH4 │ PALCO
 ▮    ▮    ▮    ▮   │ (◯) PALCO   MÃO·RODA·CÂNONE·GESTO·PÊNDULO
                    │ (◯) VELOC
 (◯)  (◯)  (◯)  (◯) │ (◯) ABERT
 PAN  PAN  PAN  PAN │ [visor: 4 pontos no palco + o centro de gravidade]
 [M]  [M]  [M]  [M] │ ◎ FRASE  (gatilho do GESTO)
 ◎p1  ◎p2  ◎p3  ◎p4 │
 ◎1   ◎2   ◎3   ◎4  │ (◯)OUT          ◎ L+R
```

PAN de cada canal vira a **casa** quando o PALCO não está em MÃO; em MÃO
tudo funciona como hoje (o contrapeso desliga: o músico manda). As
entradas p1–p4 somam ao lugar em qualquer modo; o PÊNDULO é o modo em que
só elas movem os canais (a seed liga o PENDULUM nelas).

## 5. Decisões para o autor

0. **Caminho B primeiro** (entradas PAN no MIXER), deixando o PALCO para
   depois — ou os dois juntos?

1. **PALCO como botão próprio ou como parte do VARIA** (ligado sempre que o
   VARIA estiver ligado)?
2. **Grave no centro no MIXER (4.2)**: sim, não, ou só quando o PALCO
   estiver ligado?
3. **Abertura máxima**: 0,55 (proposta), 0,48 (Navalha) ou 0,70
   (Asterismus)?
4. **A seed** continua abrindo os pans no centro, ou o PALCO já define as
   casas ao abrir?

## 6. Como validar (antes de pedir sua escuta)

- teste: a soma mono do MIXER com PALCO ligado tem o mesmo RMS que com
  todos os pans no centro (±0,5 dB);
- teste: nenhum pan passa do teto; nenhuma casa é cruzada;
- teste: mexer num PAN à mão tira aquele canal do VARIA;
- teste: grave (< 150 Hz) com o mesmo nível nos dois lados para qualquer
  pan (se 4.2 for aprovado);
- escuta: A/B do mesmo patch com PALCO ligado e desligado, igualado em
  RMS.

## Proveniência

Conceitos lidos no código do autor: Asterismus (`PerformanceSkyPan.hpp`),
Navalha 2 (`AssistedPerformer.cpp`, AUTO MIX — Glerm Soares / Lúcio
Araújo), Aquorbium (`Ecosystem.cpp`, `AquariumSpatializer.h`), Rasgo Synth
Studio (`SpatialExpressionField.hpp`). Lei de potência constante e
crossover complementar de um polo são técnica pública. Nada copiado; a
implementação, se aprovada, cita cada origem no cabeçalho.
