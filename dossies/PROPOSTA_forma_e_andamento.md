# Proposta — FORM (a dinâmica da forma) e TEMPO (o andamento que respira)

Data: 20/11/2026. Estado: **proposta, nada implementado**. Pedido do autor:
"existe um módulo que trabalha com dinâmicas tipo mixer e outro para
andamentos etc?" → "faça isso" (a proposta de um ou dois módulos, com prior
art e rascunho de painel, para decidir antes do código). O desenho do painel
é rascunho de função; a imagem do painel vem para aprovação antes do código.

## 1. O que existe hoje (conferido no código, 20/11)

- **Nível**: MIXER (4 canais, nível/pan/mute, PALCO; jack `FRASE`), VCA,
  VCA4 (4 VCAs + soma), DYNAMICS (compressor/gate, ENV e GR como sinal),
  MASTER (limitador), LPG e ENVELOPE (a dinâmica de cada nota).
- **Tempo**: CLOCK (`EuclidClock.hpp`: BPM, swing, drift, euclidiano,
  fase; o jack `BPM` **soma BPM** ao knob: `bpm + bpm_mod`), TAP, LOGIC,
  SEASON (andamento por estação).
- **O que falta**: nada no Modular faz um **arco da peça** — crescendo e
  diminuendo, accelerando e ritardando como forma. Hoje se improvisa com
  DRIFT/FUNCTION/ENVELOPE lentos num VCA ou no `BPM` do CLOCK: uma curva,
  não uma forma. O Form Engine (`ESTUDO_seed_composicao_generativa.md` §3.4
  e §3.7) foi desenhado em setembro e nunca construído.
- **No Cardinal 26.02** (92 plugins, nome + descrição): há arranjadores de
  seção (CVfunk *Arrange*, JW *Arrange*), sequenciadores de padrões
  (Impromptu *PhraseSeq*) e um humanizador de fase (HetrickCV *Phasor
  Humanizer*); **nenhum condutor de arco, de crescendo ou de ritardando**.
  A lacuna é real e é da família RASGO, não de catálogo.

## 2. O que a família já sabe

| Fonte | O que ensina |
|---|---|
| **rasgo-synth / Studio** (`_catalogo_mecanismos.md`, Doc. de Referência §4.2; lido na cópia de preservação) | caminhada de Markov sobre `intro / subida / clímax / queda / coda`, **coda absorvente** (a peça só termina nela; a duração é consequência); gramáticas **Freytag** e **Kishōtenketsu**, misturáveis; andamento em **salto-e-espera** nas transições; **accelerando** exponencial real rumo ao 1º clímax e **ritardando** exponencial na coda; cortes secos em algumas transições |
| **Studio, `docs/COMPOSICAO.md`** (estudo da família §4.x) | **"uma forma só se ouve se todas as camadas a seguem"**: na 1ª escuta três gramáticas soaram iguais porque os gestos constantes mascaravam a curva; com tudo seguindo, a Freytag ganhou **11 dB entre vale e clímax**. Clímax na seção áurea (Howat); **silêncio composto** logo antes do clímax; os enfeites rareiam nos vales, **o essencial nunca** |
| **Trioio** (estudo §4.4) | arco de tensão escolhido pela semente (subir e cair; ondas; crescimento lento; calmaria entre agitações), período de 4–8 min; a dinâmica **nunca acima do nível de base**; a melodia sobe uma oitava no clímax; respiração depois da cadência. "O contraste é sutil de propósito; a escuta decide se pede mais" |
| **Asterismus** (estudo §4.1) | o **tempo maleável**: mola amortecida de 2ª ordem no andamento (ω 1,1, ζ 0,62; a tensão puxa ritardando, a cadência desfaz); **rubato em série de Fourier** (4 harmônicos, 1/k²); **arco de frase de Todd** (1992); a **equação do tempo** como desvio lento. Já listado no estudo como proposta **TEMPO** (não aprovada ainda) |
| **Antitotem** (`PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md`) | a distinção entre **PROCESS** (crescendo, diminuendo, accelerando, ritardando, morendo) e **GESTURE** pontual (sforzando, fortepiano, respiração); a **atitude temporal** (rubato, estrito, atrás, à frente, elástico) |
| **Maestruc** (`PROTOCOLO_MAESTRUC_LINK.md`) | o regente já prevê `/rasgo/1/section` e `macro:intensidade`; a frente 4 (forma/grade) e a 5 (dinâmica) estão abertas. FORM e TEMPO seriam o lugar natural para o Modular **receber** isso, mais tarde |
| **Pesquisa** (Atlas §108, KTH) | o sistema de regras de interpretação do KTH (Friberg, Sundberg): o **arco de frase** (mais rápido e mais forte no meio, como em Todd) e o **ritardando final** modelado na desaceleração de um corredor que para (Friberg & Sundberg 1999): v(x) = [1 + (wᵠ − 1)·x]^(1/q), q ≈ 2–3, w = andamento final relativo |

Princípio comum: **a forma é intenção, não preset**. Os dois módulos não
comandam os outros; dão curvas e acontecimentos que o patch escolhe ouvir.

## 3. FORM — a dinâmica da forma (família MODULATE, #103 provisório)

Um **mixer que conduz**: quatro camadas entram, passam pelo arco da peça e
saem com crescendo, diminuendo, respiração e morendo. Responde ao "tipo
mixer" do pedido e à regra decisiva do Studio: quem passa pelo FORM segue a
forma. Também dá a curva como sinal, para o resto do patch seguir.

**O arco** — uma caminhada de Markov sobre os papéis `INTRO · SUBIDA ·
CLÍMAX · QUEDA · CODA`, com a coda absorvente:

- **GRAM** escolhe a gramática: ARCO (Freytag), ONDAS, CRESCIMENTO,
  CALMARIA (entre duas agitações), KISHŌ (a virada *ten* é um corte seco e
  troca o material, não só o nível). As probabilidades de transição de cada
  gramática vêm do Studio/Trioio e ficam no dossiê.
- **LEN** é a escala de tempo: 1–20 min (ou em compassos, se houver CLK);
  a duração real sai da caminhada, LEN só a centra.
- Cada papel tem um **alvo de tensão** (0–1) e a tensão anda até ele por
  uma curva suave (exponencial em dB, sem degrau audível: ≥ 50 ms).

**As camadas (CH1–CH4)** — cada uma com **SEG** (o quanto segue o arco,
0–1): 0 é constante, 1 segue tudo. Ganho em dB: `g = −DEPTH·(1 − t)·SEG`,
sempre ≤ 0 dB (**nunca acima do nível de base**, nunca estoura o MASTER).
A camada 1 é o **essencial**: nos vales desce menos (o essencial nunca
some); a 4 é o **enfeite**: nos vales rareia mais. Isso é a nuance do
Studio, sem pedir um controle a mais.

**Os gestos** (opcionais, **GEST** 0–1 dá a chance):

- **respiração** antes do clímax — um silêncio composto de ½–2 tempos em
  todas as camadas (o gesto "simples e forte" do Studio);
- **subito piano** na virada do KISHŌ e nos cortes secos;
- **morendo** na coda — o diminuendo final até o silêncio, e o fim (`END`).

**Controles** (rascunho): GRAM, LEN, DEPTH (contraste, 3–24 dB, padrão
11 dB — o número medido do Studio), GEST, SEG1–SEG4, MODE (IMPOR o arco /
SEGUIR a energia que entra em `ENV` / CONTRAPOR, a curva invertida — as
estratégias do Studio), RESTART.

**Jacks**: entradas `IN1–IN4` (áudio), `CLK` (opcional: conta compassos),
`NEXT` (força a próxima seção), `RST`, `ENV` (a energia a seguir, ex.:
LISTEN ou o ENV do DYNAMICS); saídas `OUT1–OUT4`, `MIX` (soma), `TENS`
(a tensão 0–1 para filtro, densidade, VARIA), `SECT` (trigger na troca de
seção), `ROLE` (o papel como degrau), `CLMX` (trigger ao chegar ao clímax),
`BRTH` (gate da respiração), `END` (gate ao terminar a coda).

**Visor** (a propor em imagem): o arco desenhado no tempo, o ponto que
caminha, o nome do papel grande (como o tom do KEY).

## 4. TEMPO — o andamento que respira (família TIME, #104 provisório)

O **regente do relógio**: dá ao CLOCK um andamento que se move — mapa de
forma, frase e respiração. Sozinho já soa (rubato e arco de frase); com o
FORM ligado, segue a mesma forma.

**Camadas de movimento** (somadas em razão, não em BPM):

1. **forma** — `SECT` do FORM faz **salto-e-espera**; o caminho até o
   1º clímax arma o **accelerando** exponencial; a coda faz o
   **ritardando final** pela curva do corredor (KTH): q = **SHAPE**,
   w = **FINAL** (o andamento de chegada, ex. 0,6);
2. **frase** — o **arco de Todd**: dentro de cada frase (comprimento
   **PHR**, em tempos), acelera e cresce no meio, recua no fim; **ARCH** é a
   profundidade (é o único ponto em que o tempo encosta na dinâmica: a saída
   `ARCH` vai para o VCA ou para o `ENV` do FORM);
3. **rubato** — série de Fourier de 4 harmônicos, 1/k², **o tempo emprestado
   é devolvido** (média zero na frase): **RUBATO** 0–1;
4. **respiração** — `FERM` (entrada) faz uma **fermata**: o relógio
   segura ½–2 tempos e volta;
5. **mola** — a mola de 2ª ordem do Asterismus (ω 1,1, ζ 0,62) liga tudo:
   a tensão (`TENS` do FORM) puxa, a cadência desfaz; não há degrau.

**Ligação com o CLOCK** — uma decisão sua (§6.1): o jack `BPM` do CLOCK hoje
**soma BPM**; um accelerando de 20 % precisa saber o BPM de base. Dois
caminhos:

- **(A, recomendado)** acrescentar ao CLOCK um jack **`RATE`** multiplicativo
  (1 V = ×2, 0 V = andamento do knob). TEMPO sai em `RATE`. O CLOCK continua
  o mestre; é uma entrada a mais, sem mudar nada do que já soa;
- **(B)** sem tocar o CLOCK: TEMPO tem o knob **REF** (o BPM do CLOCK) e sai
  em BPM somado. Funciona, mas é um número que o músico copia à mão.

**Controles** (rascunho): FINAL, SHAPE, ACCEL (quanto acelera rumo ao
clímax, 0–30 %), PHR, ARCH, RUBATO, MOLA (rigidez), MODE (ESTRITO / ARCO /
ELÁSTICO — a "atitude temporal" do Antitotem, com ATRÁS e À FRENTE como
deslocamento de fase do `CLK` de saída).

**Jacks**: entradas `TENS`, `SECT`, `CODA` (vindos do FORM; também aceitam
qualquer gate/CV), `FERM`, `CLK` (o relógio que mede a frase); saídas
`RATE` (ou `BPM`), `ARCH` (o arco de frase como CV), `BEAT` (o pulso já
deformado, para quem não passa pelo CLOCK), `END` (o último tempo).

**Visor** (a propor em imagem): o andamento como linha no tempo, a frase
atual e o quanto está emprestado/devolvido.

## 5. Como eles soam juntos (a receita de partida)

`CLOCK → TEMPO(CLK)`; `TEMPO.RATE → CLOCK.RATE`; quatro vozes →
`FORM.IN1–4` → `MIXER`; `FORM.TENS → TEMPO.TENS`, `FORM.SECT → TEMPO.SECT`,
`FORM.END → TEMPO.CODA` (ou o FORM manda o `ROLE`); `FORM.TENS` também no
cutoff de um FILTER e no FILL de um TRIGSEQ. Assim **todas as camadas
seguem a forma** e ela se ouve.

## 6. Decisões do autor

1. **CLOCK**: o jack `RATE` multiplicativo (A) ou o REF em BPM (B)?
2. **Dois módulos ou um**? A proposta separa (cada um soa sozinho e o
   pedido foi "um para dinâmicas, outro para andamentos"); a alternativa é
   um FORM só com uma saída de andamento.
3. **Nomes**: FORM e TEMPO (há a proposta TEMPO do Asterismus no estudo, que
   esta absorve). Alternativas: ARCO / MAESTRO.
4. **Seed e VARIA**: nesta proposta, os dois ficam **fora da seed** e o
   VARIA não os move. Entrar na seed muda o que a seed é (a peça ganharia
   começo, meio e fim) — decisão maior, depois da escuta.
5. **Maestruc**: mais tarde, FORM receber `/rasgo/1/section` e
   `macro:intensidade` e TEMPO receber `/rasgo/1/tempo`.

## 7. Critérios de pronto (se aprovado)

- FORM: com GRAM = ARCO, DEPTH = 11 dB, a diferença vale–clímax medida em
  `MIX` fica em 11 ± 1 dB; o ganho nunca passa de 0 dB; nenhuma transição
  com degrau (maior salto entre amostras como o do VCA parado); a coda
  sempre chega e `END` sobe; determinístico por seed.
- TEMPO: o ritardando final é monotônico e chega a FINAL ± 1 %; o rubato tem
  média zero por frase (a frase dura o mesmo); a fermata devolve a fase;
  RATE sem degrau.
- Os dois: teste próprio, LEARN nas 4 línguas, dossiê, guia, página no site
  e créditos (Studio/Trioio/Asterismus/Antitotem; Todd 1992; Friberg &
  Sundberg 1999).
