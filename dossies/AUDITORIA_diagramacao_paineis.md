# Auditoria de diagramação dos painéis

**Pedido do autor (25–26/11/2026):** "precisa auditar os módulos para
verificar o melhor jeito de posicionar os objetos, tanto no modo normal como
no modo compacto"; "essa auditoria módulo a módulo também precisa levar em
consideração o human polish". Motivo imediato: o NOISE "bem mal diagramado"
nos dois modos; contexto: a recusa da VCV Library por falta de "human
polish" (TAREFAS.md, 25/11).

**Como ler este documento.** Ele não muda nenhum painel. Mede, aponta e
propõe; cada desenho novo passa pelo autor antes do commit, módulo a módulo
— essa é a parte humana. O que for aprovado muda o painel no próprio módulo
(`Signal::panel()`), e por isso vale ao mesmo tempo para o app, o plugin de
DAW, o navegador e (na próxima regeneração) o plugin VCV — a regra de
front-ends do `DESENVOLVIMENTO.md`.

## 1. Método

**Medidas** (`tools/auditar_paineis.mjs`, a partir dos painéis do próprio
motor, nos dois modos):

| medida | o que é |
|---|---|
| colunas | objetos de uma fileira sem nenhum objeto de outra fileira na mesma coluna (±1 mm), e fora do centro |
| passo | o maior desvio dos vãos de uma fileira em relação à média dela |
| centro | o meio do conteúdo contra o meio do painel |
| vão vertical | a maior faixa de altura sem nenhum objeto, abaixo do visor |
| aperto | vizinhos (objeto ou rótulo) a menos de 1,5 mm |
| visor | quanto o visor é mais estreito que o painel |
| uso | quanto da largura o conteúdo ocupa |

A nota soma os problemas com pesos (0 = nada a apontar). Ela ordena; não
decide. **Leitura humana** (os 25 piores, nas folhas de imagem): hierarquia
(o controle principal se destaca?), fluxo (entradas → controles → saídas, na
mesma ordem em todos), agrupamento por função, rótulos legíveis e
coerentes, coerência de família, a saída em lugar previsível, o visor,
simetria e respiro.

Folhas: `.local-build/web/capturas/auditoria/` (`piores-normal.png`,
`piores-compacto.png`, `melhores-normal.png`, `noise-antes-depois.png`).

## 2. O quadro geral

| | normal | compacto |
|---|---|---|
| nota somada (105 módulos) | 1414 | 1160 |
| objetos fora de coluna | 329 | 256 |
| módulos com vão vertical > 20 mm | 48 | 48 |
| módulos descentrados (> 1,5 mm) | 40 | 3 |
| apertos (< 1,5 mm) | 14 | 39 |
| visores mais estreitos que o painel | 4 | 0 |

Em 26/11 (o autor: "o noise ainda está com o display menor") os visores do
NOISE, SCOPE e PHRASE passaram à largura do painel menos 5 mm, como os
outros. O do MIXER fica como está: é o PALCO, embaixo, entre os canais e as
saídas, de propósito.

O COMPACTO acerta o centro e o visor, mas não resolve colunas nem o vão do
meio — e cria apertos (ele só aperta o que existe). **A diagramação ruim não
é um defeito do modo: está no desenho de cada módulo.**

## 3. Os padrões que se repetem

1. **O vão do meio.** Os jacks ficam presos na base (y ≈ 88–120 mm), seja
   qual for o número de controles; sobram 25–35 mm vazios entre os knobs e os
   jacks em 48 módulos (SEASON, TAP, KEY, FUNCTION, LENS, SAMPLER…).
2. **A saída sozinha no canto.** O OUT isolado à esquerda na última fileira,
   sem coluna nem centro (MATTER, STRING, SAMPLER, TURNTABLE, MEMORY, LOOPER;
   o L R do HALL e do LENS).
3. **Fileiras de 4 e de 5 em grades diferentes.** Duas fileiras de jacks com
   contagens diferentes, cada uma com o seu passo: nada se alinha (SEASON,
   SWARM, ORBIT, PREDATOR, KEY, DRIFT).
4. **Fileiras pendendo para a esquerda.** Knobs ou jacks começando na
   esquerda com a direita vazia (KEY, TAP, FUNCTION, NOISE, PLL embaixo à
   direita).
5. **Cinco jacks em 10 HP.** Passo de ~9,5 mm: os rótulos de 4–5 letras
   colam ("LIGHTDAWN" no SEASON, G1…ALL no SWARM).
6. **Controles soltos.** Um toggle ou knob sem fileira (HEMI no SEASON, GATE
   no SWARM, TRAV no TAP, MIX no LENS, HOLD no MEMORY).
7. **Hierarquia plana.** Quase tudo em 8 mm; o grande só em CUT do FILTER e
   do LADDER. O controle que define o módulo (RATE do NOISE, FREQ do OSC, SIZE
   do HALL, GRAIN do MEMORY) não se distingue.

## 4. A gramática de painel — **aprovada pelo autor em 26/11/2026** ("a gramatica ok")

Uma regra comum, com espaço para o estilo de cada módulo — o mesmo
princípio que o autor deu para o VCV ("cada módulo pode ter seu estilo
pessoal, mas com certa padronização no conjunto").

- **G1. Colunas.** Passo de 12,7 mm (o do Eurorack/VCV para objetos de 8 mm
  com rótulo), simétricas em relação ao centro do painel. 4 colunas = 10 HP;
  5 colunas = 13 HP; 3 colunas = 8 HP.
- **G2. Fileiras nas mesmas colunas.** Uma fileira com menos objetos usa o
  mesmo passo e fica centrada (nas colunas do meio, ou nas meias-colunas se a
  paridade mudar). Nenhuma fileira com passo próprio.
- **G3. Ritmo vertical.** Visor no alto (6–22 mm); depois as fileiras em
  passo regular (≈ 22–24 mm), sem vão grande: a sobra de altura vira
  respiro igual entre os grupos — ou knobs maiores (G5) —, não um buraco.
- **G4. Fluxo e lugar da saída.** Controles → entradas → saídas, de cima para
  baixo; **as saídas sempre na última fileira**, centradas; com uma saída
  só, ela fica no centro.
- **G5. Hierarquia.** O controle que define o módulo em 12,7 mm (grande);
  os que têm espaço em 10 mm (médio, a regra do COMPACTO); os demais em 8 mm.
- **G6. Rótulos.** Até 4 letras quando o passo é 10,16 mm; até 5 com 12,7;
  a mesma abreviação para a mesma coisa em todos os módulos (OUT, IN, CLK,
  RST, GATE, PTCH, 1V/O).

O COMPACTO continua existindo por cima disto: com os painéis bem
diagramados, ele só estreita.

## 5. Os 25 piores, na leitura humana

| Módulo | O que se vê | Direção da proposta |
|---|---|---|
| SEASON | 3 knobs, HEMI solto, vão de 33 mm, fileiras de 4 e 5 jacks com rótulos colando | 10 HP em 4 colunas: knobs + HEMI na mesma fileira; entradas (4) e saídas (5 → duas fileiras ou 13 HP) |
| SWARM | GATE solto embaixo de RATE; 5 jacks G1–ALL apertados | GATE na fileira dos knobs; saídas em 4+1 centradas ou 13 HP |
| SEQUENCE | sliders e G bons; knobs em grades diferentes; jacks só à esquerda | knobs nas colunas dos sliders; jacks espalhados nas colunas |
| TAP | 2 knobs, jacks soltos em 3 fileiras (TRAV sozinho) | 8 HP em 3 colunas; TAP RST / CLK UM BPM / TRAV centrado |
| KEY | MEM e ESTAB à esquerda; vão de 51 mm; visor pequeno para o que mostra | visor mais alto (o tom é a informação principal), knobs centrados |
| ORBIT | knobs bons; jacks 4 e 5 em grades diferentes | 5 colunas (12 HP → 13) ou saídas X1–DIST em duas fileiras |
| MEMORY | HOLD solto; OUT sozinho no canto | HOLD na fileira; OUT no centro da última fileira |
| PREDATOR | CANON longe à direita; fileiras 3/4 | entradas e saídas na mesma grade de 4 |
| NOISE | ver §6 | **feito como exemplo** |
| HALL | L R à esquerda; vão | L R centrados nas colunas do meio |
| LOOPER | toggles HOLD REV no meio dos knobs; OUT WET à esquerda | toggles numa fileira própria; saídas centradas |
| MARKOV | LEARN solto; fileiras 4/3 | LEARN na fileira dos knobs; jacks na grade |
| DRIFT | quatro fileiras de jacks irregulares (3, 4, 2) | entradas numa fileira, saídas em duas na grade de 4 |
| SUPPLY | jacks 2/2/4 espalhados | grade de 4, saídas na última |
| MATTER, STRING | OUT sozinho no canto; 4+3 knobs | knob principal grande (FREQ); OUT no centro |
| LADDER | fileiras de 3 e 2 desalinhadas | grade de 4 centrada |
| TURNTABLE, SAMPLER | OUT sozinho; LOOP solto | OUT no centro; LOOP na fileira de WEAR |
| MULT | colunas SCL/OFF boas, mas jacks fora delas | jacks nas mesmas colunas |
| NOTE-OUT | 6 jacks no meio de 7 HP, alto vazio | jacks em 2 colunas com respiro igual; visor maior |
| PLL | ENC SEP BAT só à direita embaixo | saídas centradas na grade |
| FUNCTION | 2 knobs + DRIFT + SYNC soltos; vão | 8 HP em 3 colunas; ritmo vertical |
| LENS | MIX solto; L R à direita | MIX na fileira; saídas centradas |
| SH | toggles TRK1 TRK2 soltos | toggles abaixo dos seus knobs |

## 6. O exemplo: NOISE

Antes: 14 HP; knobs com vãos de 16, 16 e 14 mm; TRIG e IN encostados à
esquerda; as 8 saídas nos dois terços da esquerda; fileiras em 30, 60, 88 e
110 mm, nenhuma alinhada com outra; visor de 56 mm num painel de 71 mm.

Proposta (G1–G4): **10 HP**, quatro colunas simétricas a 12,7 mm; fileiras
a cada 24 mm (knobs, TRIG e IN nas duas colunas do meio, as duas fileiras de
saídas); o visor na largura do painel. Patch em
`dossies/propostas_paineis/NOISE.patch`; antes e depois (normal e compacto)
em `.local-build/web/capturas/auditoria/noise-antes-depois.png`, feito com
`tools/prova_paineis.sh` (aplica as propostas numa cópia da árvore e compila
um motor de prova à parte; a página do autor não muda). **Aguarda o OK do
autor.** Duas observações da imagem: no compacto, o NOISE novo ainda aperta
um pouco os rótulos dos knobs — com os painéis bem diagramados, talvez o
COMPACTO deva parar no passo de 10,16 mm; e o G5 (hierarquia) não foi
aplicado aqui: o RATE poderia ser o knob grande.

## 7. Próximos passos

1. ~~O autor aprova a gramática G1–G6~~ (aprovada em 26/11) e o NOISE.
2. Os módulos passam por ela **em lotes de 5**, dos piores para os
   melhores, cada lote com antes/depois nos dois modos para o OK.
3. Ao fim de cada lote, a auditoria roda de novo (a nota tem de cair) e o
   teste de layout do app (`rasgo_modular_panel_layout_tests`) tem de passar.

## 8. A tabela dos 105 (do pior para o melhor)

n = normal, c = compacto. Gerada por `tools/auditar_paineis.mjs` em
26/11/2026.

| # | Módulo | Família | HP n/c | Nota n/c | Fora de coluna n/c | Vão vertical | Principal (normal) |
|---|---|---|---|---|---|---|---|
| 1 | SEASON | TIME | 10 / 10 | 37 / 37 | 7 / 7 | 33 mm | colunas fora; aperto (4) |
| 2 | SWARM | TIME | 10 / 10 | 32 / 32 | 5 / 5 | 31 mm | aperto (4); colunas fora |
| 3 | SEQUENCE | TIME | 20 / 17 | 47 / 8 | 19 / 1 | 6 mm | colunas fora; passo irregular 9.6 mm |
| 4 | TAP | TIME | 8 / 7 | 27 / 26 | 4 / 4 | 47 mm | vão vertical 47 mm; colunas fora |
| 5 | KEY | DECISION | 10 / 9 | 26 / 26 | 3 / 3 | 51 mm | vão vertical 51 mm; colunas fora |
| 6 | ORBIT | MODULATE | 12 / 11 | 26 / 19 | 8 / 5 | 29 mm | colunas fora; vão vertical 29 mm |
| 7 | MEMORY | SPACE | 12 / 11 | 23 / 22 | 5 / 4 | 33 mm | vão vertical 33 mm; colunas fora |
| 8 | PREDATOR | MODULATE | 10 / 9 | 19 / 24 | 2 / 2 | 31 mm | vão vertical 31 mm; passo irregular 7 mm |
| 9 | NOISE | SOURCE | 14 / 11 | 21 / 20 | 6 / 5 | 18 mm | colunas fora; vão vertical 18 mm |
| 10 | HALL | SPACE | 14 / 12 | 21 / 20 | 6 / 6 | 27 mm | colunas fora; vão vertical 27 mm |
| 11 | LOOPER | SPACE | 12 / 10 | 18 / 23 | 5 / 5 | 25 mm | colunas fora; vão vertical 25 mm |
| 12 | MARKOV | DECISION | 10 / 10 | 20 / 20 | 5 / 5 | 29 mm | colunas fora; vão vertical 29 mm |
| 13 | DRIFT | MODULATE | 12 / 11 | 22 / 17 | 6 / 4 | 10 mm | colunas fora; aperto (3) |
| 14 | SUPPLY | MODULATE | 8 / 8 | 20 / 19 | 4 / 4 | 15 mm | aperto (3); colunas fora |
| 15 | MATTER | SOURCE | 12 / 11 | 20 / 17 | 4 / 3 | 33 mm | vão vertical 33 mm; colunas fora |
| 16 | STRING | SOURCE | 12 / 11 | 20 / 17 | 4 / 3 | 33 mm | vão vertical 33 mm; colunas fora |
| 17 | LADDER | TRANSFORM | 10 / 10 | 22 / 15 | 7 / 4 | 25 mm | colunas fora; vão vertical 25 mm |
| 18 | TURNTABLE | SPACE | 14 / 12 | 23 / 14 | 7 / 3 | 27 mm | colunas fora; vão vertical 27 mm |
| 19 | MULT | ROUTE | 10 / 10 | 18 / 18 | 8 / 8 | 5 mm | colunas fora; passo irregular 1.5 mm |
| 20 | NOTE-OUT | OUT | 7 / 7 | 18 / 18 | 0 / 0 | 33 mm | vão vertical 33 mm; uso 63% |
| 21 | PLL | SOURCE | 16 / 14 | 19 / 16 | 4 / 4 | 29 mm | vão vertical 29 mm; colunas fora |
| 22 | FUNCTION | MODULATE | 10 / 8 | 23 / 12 | 5 / 1 | 31 mm | colunas fora; vão vertical 31 mm |
| 23 | LENS | SPACE | 10 / 9 | 18 / 17 | 4 / 4 | 31 mm | vão vertical 31 mm; colunas fora |
| 24 | SAMPLER | SPACE | 14 / 12 | 21 / 14 | 6 / 3 | 27 mm | colunas fora; vão vertical 27 mm |
| 25 | SH | MODULATE | 12 / 11 | 20 / 12 | 8 / 5 | 12 mm | colunas fora; descentrado 2.9 mm |
| 26 | LOGIC | TIME | 10 / 10 | 16 / 16 | 7 / 7 | 10 mm | colunas fora; descentrado 1.4 mm |
| 27 | RELATE | ROUTE | 8 / 7 | 16 / 16 | 3 / 3 | 23 mm | colunas fora; vão vertical 23 mm |
| 28 | ELASTIC | TRANSFORM | 10 / 9 | 16 / 15 | 3 / 3 | 31 mm | vão vertical 31 mm; colunas fora |
| 29 | FORMANT | TRANSFORM | 14 / 10 | 16 / 15 | 4 / 4 | 27 mm | colunas fora; vão vertical 27 mm |
| 30 | TAPE | TRANSFORM | 10 / 9 | 16 / 15 | 3 / 3 | 31 mm | vão vertical 31 mm; colunas fora |
| 31 | CHORD | SOURCE | 12 / 11 | 15 / 15 | 4 / 5 | 23 mm | colunas fora; vão vertical 23 mm |
| 32 | CONTROL | TRANSFORM | 12 / 12 | 13 / 17 | 6 / 8 | 8 mm | colunas fora; passo irregular 1 mm |
| 33 | FRONTEIRA | DECISION | 10 / 9 | 16 / 14 | 4 / 2 | 27 mm | colunas fora; vão vertical 27 mm |
| 34 | FILTER | TRANSFORM | 10 / 10 | 16 / 13 | 4 / 3 | 25 mm | colunas fora; vão vertical 25 mm |
| 35 | ECLIPSE | TIME | 10 / 9 | 12 / 17 | 1 / 1 | 31 mm | vão vertical 31 mm; colunas fora |
| 36 | TURING | TIME | 12 / 11 | 16 / 13 | 2 / 1 | 33 mm | vão vertical 33 mm; colunas fora |
| 37 | COHERENCE | DECISION | 10 / 9 | 15 / 14 | 3 / 3 | 29 mm | vão vertical 29 mm; colunas fora |
| 38 | DECISION | DECISION | 12 / 11 | 12 / 17 | 0 / 0 | 33 mm | vão vertical 33 mm; descentrado 1.4 mm |
| 39 | BUDGET | TIME | 10 / 9 | 15 / 13 | 3 / 2 | 27 mm | vão vertical 27 mm; colunas fora |
| 40 | QUANTIZER | DECISION | 14 / 14 | 14 / 14 | 1 / 1 | 6 mm | passo irregular 11.5 mm; descentrado 3.7 mm |
| 41 | VIA | ROUTE | 10 / 9 | 16 / 12 | 3 / 2 | 29 mm | vão vertical 29 mm; colunas fora |
| 42 | SPRING | SPACE | 10 / 8 | 15 / 13 | 2 / 2 | 31 mm | vão vertical 31 mm; colunas fora |
| 43 | MIXER | OUT | 18 / 12 | 25 / 3 | 4 / 0 | 0 mm | visor estreito; colunas fora |
| 44 | INTERFERENCE | TIME | 10 / 9 | 14 / 13 | 2 / 2 | 31 mm | vão vertical 31 mm; colunas fora |
| 45 | DRUM | SOURCE | 14 / 10 | 12 / 14 | 2 / 2 | 27 mm | vão vertical 27 mm; colunas fora |
| 46 | PARAMETRIC | TRANSFORM | 22 / 16 | 14 / 12 | 4 / 6 | 9 mm | colunas fora; descentrado 7.5 mm |
| 47 | VOCODER | TRANSFORM | 14 / 9 | 15 / 11 | 2 / 3 | 16 mm | descentrado 7 mm; colunas fora |
| 48 | ABACUS | DECISION | 14 / 13 | 16 / 9 | 7 / 4 | 14 mm | colunas fora; vão vertical 14 mm |
| 49 | SWITCH | ROUTE | 14 / 12 | 13 / 12 | 3 / 3 | 14 mm | passo irregular 8 mm; colunas fora |
| 50 | SPACE | SPACE | 12 / 11 | 14 / 11 | 1 / 0 | 33 mm | vão vertical 33 mm; colunas fora |
| 51 | SCOPE | OUT | 15 / 13 | 14 / 11 | 1 / 1 | 14 mm | passo irregular 12 mm; colunas fora |
| 52 | BOUNCE | MODULATE | 12 / 11 | 12 / 12 | 2 / 2 | 27 mm | vão vertical 27 mm; colunas fora |
| 53 | MATRIX | ROUTE | 20 / 20 | 12 / 12 | 4 / 4 | 14 mm | colunas fora; passo irregular 3 mm |
| 54 | PRISM | SPACE | 10 / 8 | 13 / 11 | 1 / 1 | 31 mm | vão vertical 31 mm; colunas fora |
| 55 | POLY-IN | SOURCE | 8 / 7 | 5 / 18 | 2 / 3 | 8 mm | colunas fora; descentrado 0.9 mm |
| 56 | RUNGLER | SOURCE | 10 / 9 | 12 / 11 | 1 / 1 | 31 mm | vão vertical 31 mm; colunas fora |
| 57 | LPG | TRANSFORM | 10 / 10 | 12 / 11 | 4 / 4 | 17 mm | colunas fora; vão vertical 17 mm |
| 58 | PRESSURE | MODULATE | 10 / 9 | 12 / 11 | 1 / 1 | 31 mm | vão vertical 31 mm; colunas fora |
| 59 | MASTER | OUT | 8 / 7 | 14 / 9 | 6 / 3 | 11 mm | colunas fora; descentrado 2.7 mm |
| 60 | ADDITIVE | SOURCE | 12 / 12 | 11 / 11 | 5 / 5 | 12 mm | colunas fora; descentrado 0.9 mm |
| 61 | OPERATOR | SOURCE | 14 / 11 | 12 / 10 | 2 / 1 | 27 mm | vão vertical 27 mm; colunas fora |
| 62 | SIGNAL-IN | SOURCE | 8 / 8 | 11 / 11 | 4 / 4 | 8 mm | colunas fora; passo irregular 2 mm |
| 63 | WAVETABLE | SOURCE | 12 / 10 | 13 / 9 | 2 / 1 | 25 mm | vão vertical 25 mm; colunas fora |
| 64 | INTERPRET | DECISION | 12 / 11 | 14 / 8 | 3 / 1 | 25 mm | vão vertical 25 mm; colunas fora |
| 65 | WASP | TRANSFORM | 10 / 10 | 11 / 9 | 5 / 4 | 11 mm | colunas fora; descentrado 1.4 mm |
| 66 | BOXCAR | DECISION | 14 / 11 | 9 / 11 | 4 / 4 | 12 mm | colunas fora; descentrado 0.7 mm |
| 67 | SUM | ROUTE | 8 / 7 | 12 / 8 | 5 / 4 | 7 mm | colunas fora; descentrado 2.3 mm |
| 68 | EPICYCLE | MODULATE | 10 / 9 | 10 / 9 | 2 / 2 | 23 mm | vão vertical 23 mm; colunas fora |
| 69 | MAGNET | MODULATE | 10 / 9 | 10 / 9 | 0 / 0 | 31 mm | vão vertical 31 mm; passo irregular 0.7 mm |
| 70 | PENDULUM | MODULATE | 12 / 10 | 9 / 10 | 3 / 4 | 11 mm | colunas fora; passo irregular 2.7 mm |
| 71 | APOCRIFO | SPACE | 10 / 9 | 10 / 9 | 1 / 1 | 27 mm | vão vertical 27 mm; colunas fora |
| 72 | SHAPE | TRANSFORM | 10 / 10 | 9 / 9 | 4 / 4 | 11 mm | colunas fora; descentrado 1.4 mm |
| 73 | ENVELOPE | MODULATE | 12 / 10 | 17 / 1 | 7 / 0 | 8 mm | colunas fora; descentrado 2.6 mm |
| 74 | FORM | MODULATE | 16 / 15 | 10 / 8 | 1 / 1 | 10 mm | passo irregular 9.6 mm; colunas fora |
| 75 | GENDYN | SOURCE | 10 / 9 | 9 / 8 | 4 / 4 | 11 mm | colunas fora; passo irregular 0.5 mm |
| 76 | SPECTRA | SOURCE | 14 / 9 | 11 / 6 | 1 / 3 | 12 mm | descentrado 7 mm; uso 70% |
| 77 | BLEFE | DECISION | 10 / 9 | 8 / 7 | 1 / 1 | 23 mm | vão vertical 23 mm; colunas fora |
| 78 | GLIDE | TRANSFORM | 10 / 9 | 8 / 6 | 0 / 0 | 23 mm | vão vertical 23 mm; descentrado 1.5 mm |
| 79 | VCA | TRANSFORM | 10 / 10 | 8 / 6 | 3 / 2 | 11 mm | colunas fora; passo irregular 2.7 mm |
| 80 | POLYGON | TIME | 14 / 12 | 7 / 6 | 3 / 3 | 9 mm | colunas fora; descentrado 1.7 mm |
| 81 | CUTUP | SPACE | 10 / 9 | 9 / 4 | 4 / 2 | 13 mm | colunas fora; vão vertical 13 mm |
| 82 | OSC | SOURCE | 12 / 11 | 7 / 5 | 1 / 1 | 10 mm | passo irregular 5 mm; colunas fora |
| 83 | CRUSH | TRANSFORM | 10 / 9 | 5 / 7 | 2 / 2 | 12 mm | colunas fora; descentrado 0.7 mm |
| 84 | SHIFTER | TRANSFORM | 10 / 7 | 10 / 2 | 0 / 0 | 12 mm | uso 62%; descentrado 5.8 mm |
| 85 | MOUSSE | MODULATE | 10 / 8 | 10 / 2 | 4 / 1 | 11 mm | colunas fora; descentrado 2 mm |
| 86 | CLOCK | TIME | 16 / 14 | 7 / 5 | 2 / 2 | 12 mm | colunas fora; descentrado 2.6 mm |
| 87 | PHRASE | DECISION | 20 / 11 | 4 / 8 | 0 / 0 | 17 mm | vão vertical 17 mm; visor estreito |
| 88 | PLANAR | ROUTE | 12 / 10 | 6 / 6 | 2 / 2 | 14 mm | colunas fora; vão vertical 14 mm |
| 89 | HARMONY | DECISION | 10 / 9 | 6 / 5 | 2 / 2 | 12 mm | colunas fora; passo irregular 2 mm |
| 90 | TEMPO | TIME | 16 / 13 | 4 / 6 | 0 / 0 | 10 mm | descentrado 4.6 mm |
| 91 | TRIGSEQ | TIME | 16 / 13 | 7 / 3 | 2 / 1 | 14 mm | colunas fora; descentrado 3.1 mm |
| 92 | SWIRL | SPACE | 12 / 8 | 8 / 2 | 1 / 1 | 12 mm | descentrado 4.4 mm; uso 73% |
| 93 | LISTEN | OUT | 16 / 11 | 7 / 2 | 0 / 1 | 13 mm | descentrado 6.1 mm; uso 76% |
| 94 | PULSAR | SOURCE | 12 / 8 | 6 / 0 | 0 / 0 | 12 mm | descentrado 4.4 mm; uso 73% |
| 95 | RESONATOR | TRANSFORM | 12 / 9 | 6 / 0 | 0 / 0 | 12 mm | descentrado 4.6 mm; uso 73% |
| 96 | STAGES | MODULATE | 12 / 9 | 6 / 0 | 0 / 0 | 12 mm | descentrado 4.6 mm; uso 73% |
| 97 | VERSUS | DECISION | 8 / 7 | 5 / 1 | 1 / 0 | 15 mm | colunas fora; descentrado 2.3 mm |
| 98 | DYNAMICS | TRANSFORM | 10 / 9 | 5 / 0 | 2 / 0 | 8 mm | colunas fora; descentrado 1.8 mm |
| 99 | VCA4 | TRANSFORM | 18 / 12 | 3 / 2 | 1 / 1 | 10 mm | colunas fora; descentrado 1.7 mm |
| 100 | CHAOS | MODULATE | 7 / 7 | 2 / 2 | 1 / 1 | 11 mm | colunas fora; descentrado 0.2 mm |
| 101 | ARP | TIME | 10 / 9 | 2 / 2 | 1 / 1 | 11 mm | colunas fora; descentrado 0.4 mm |
| 102 | CELL | TIME | 10 / 9 | 2 / 2 | 1 / 1 | 11 mm | colunas fora; descentrado 0.4 mm |
| 103 | REFRACT | MODULATE | 10 / 9 | 1 / 0 | 0 / 0 | 11 mm | passo irregular 0.7 mm; descentrado 0.4 mm |
| 104 | VOLTA | TIME | 12 / 11 | 1 / 0 | 0 / 0 | 10 mm | descentrado 1.4 mm |
| 105 | ACCENT | DECISION | 14 / 12 | 1 / 0 | 0 / 0 | 9 mm | descentrado 1.7 mm |
