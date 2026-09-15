# A relação de cabo — o cabo que processa

Não é um módulo — não tem número de dossiê de módulo, não aparece na
paleta. É uma **propriedade de qualquer cabo do patch**: além de levar
sinal de A para B, um cabo pode **combinar** esse sinal com um terceiro
(o *companion*), pode **romper e cicatrizar** em vez de simplesmente
cortar, e pode **conduzir com uma certa probabilidade** em vez de
sempre. Três recursos, todos no objeto `Cable`, nenhum deles um nó do
grafo.

**Como usar (2026-09-12):** clique no **corpo** de um cabo (não numa
ponta) — abre um pequeno inspector ancorado ali, com botões pra
escolher a relação, um clique num jack de saída pra escolher o
*companion*, dois sliders (`AMT`/`COND`) e um botão de romper/reconectar
só aquele cabo. Ver §4. Editar o `.rmp` à mão continua funcionando (§6)
— útil pra scripts e pras peças de `examples/`, mas não é mais o único
caminho.

---

## 1. A relação (`RingMod` / `Fold` / `Difference`)

Num modular comum, combinar dois sinais (ring-mod, wavefolder dirigido,
subtração) exige um **módulo** no caminho. Aqui a **conexão em si** pode
processar: ela lê um segundo sinal, o *companion* (sempre do bloco
**anterior**, pra não depender da ordem de cálculo — e isso permite até
um cabo se relacionar consigo mesmo, ver a caixa "e se o companion for o
próprio cabo?" em cada relação abaixo), e mistura seco/molhado por
`amount` (0 a 1).

Sejam `x` o sinal que atravessa o cabo (o que já ia passar por ali de
qualquer jeito) e `y` o companion (o terceiro sinal que entra na
mistura, vindo de **qualquer saída de qualquer módulo do patch** — não
precisa ter relação musical óbvia com o cabo).

Tabela-resumo primeiro, pra consulta rápida; a explicação de cada uma,
devagar, vem logo depois.

| Relação | Fórmula | O que soa, em uma frase |
|---|---|---|
| **`RingMod`** | `x·(1−amount) + (x·y)·amount` | Multiplica os dois sinais — timbre metálico/sino quando `y` é áudio, tremolo quando `y` é lento. |
| **`Fold`** | `f = x·(1 + amount·3·\|y\|)`, depois reflete até 4 vezes pra caber em ±1 | Um distorsor cuja intensidade **muda sozinha**, seguindo o tamanho de `y` a cada instante. |
| **`Difference`** | `x − amount·y` | Subtrai um sinal do outro — cancela o que é igual, deixa sobrar o que é diferente. |

### 1.1 `RingMod` — multiplicar os dois sinais

**O que é, pra quem nunca mexeu com isso:** ring modulation (modulação
em anel) é o efeito de **multiplicar** dois sinais de áudio, amostra por
amostra, em vez de somá-los (que é o que um cabo normal faz quando dois
chegam no mesmo destino). Somar dois sons deixa os dois reconhecíveis,
tocando junto. **Multiplicar** é outra coisa: o resultado **não contém
mais as frequências originais** de nenhum dos dois — em vez disso,
aparecem só as **frequências soma e diferença** de cada par de parciais
dos dois sinais. É por isso que ring-mod é o efeito clássico por trás de
vozes de robô/dalek e sinos/gongos metálicos: ele troca notas "normais"
por um espectro novo, geralmente **inarmônico** (as frequências novas
não caem numa escala musical comum, a não ser que as duas entradas
estejam numa razão de afinação bem simples).

**A fórmula, termo a termo:** `x·(1−amount) + (x·y)·amount` é uma
mistura seco/molhado clássica —

- `x·(1−amount)` é a parte **seca**: o sinal original, do jeito que
  chegaria no destino se o cabo não tivesse relação nenhuma.
- `(x·y)·amount` é a parte **molhada**: o produto `x·y`, a multiplicação
  de verdade — de onde vêm as bandas soma/diferença.
- `amount` decide a proporção: em `0`, é só o sinal seco (a relação está
  "desligada" na prática); em `1`, é **só** o produto — nenhum traço do
  sinal original sobrevive (a não ser que `y` tenha um nível médio
  diferente de zero, porque multiplicar por uma constante deixa uma
  cópia escalada do original passar).

**O papel de `y` — devagar × rápido:**

- **`y` em taxa de áudio** (outro oscilador, uma voz): produz as bandas
  soma/diferença de verdade — o timbre metálico/sino. Se as duas
  frequências estiverem numa razão simples (2:1, 3:2…), o resultado soa
  quase harmônico; numa razão qualquer, soa **inarmônico e denso**
  (sinos, gongos, "clangor").
- **`y` lento** (um `ENVELOPE`, um `FUNCTION` de LFO): como `y` não
  oscila em frequência audível, não há bandas soma/diferença
  perceptíveis — o efeito vira **tremolo**: o volume do sinal sobe e
  desce seguindo `y` (porque multiplicar por um número que varia
  devagar entre −1 e 1 é, na prática, um controle de volume — e se `y`
  ficar negativo, a fase do sinal inverte no meio do caminho, o que soa
  como um tremolo "mais cortado" que o normal).
- **`y` = silêncio (0):** com `amount`=1, a saída **também vai a
  zero** — não sobra nada do sinal original, porque em `amount`=1 não há
  parte seca alguma. Com `amount` menor que 1, a parte seca
  `x·(1−amount)` continua passando.

> **E se o companion for o próprio cabo?** `y` vira o próprio `x`, mas
> do bloco anterior — ou seja, `x` multiplicado por uma cópia de si
> mesmo levemente atrasada. Como um sinal não muda muito de um bloco
> pro outro, isso se aproxima de **elevar o sinal ao quadrado**: uma
> onda ao quadrado dobra a frequência aparente (um seno vira algo
> parecido com uma oitava acima, mais retificado) — um jeito barato de
> gerar harmônicos de oitava sem outro oscilador.

### 1.2 `Fold` — a dobra dirigida por `y`

**O que é um "fold", pra quem nunca mexeu com isso:** um *wavefolder*
(dobrador de onda) é um tipo de distorção que, em vez de **cortar** o
topo do sinal quando ele passaria de +1 ou −1 (como um clipper faz —
achatando o pico, "ceifando"), **reflete** esse excesso de volta pra
dentro da faixa, como uma bola batendo numa parede e voltando. Isso cria
dobras extras na onda a cada reflexo, e cada dobra soma mais harmônicos
— é a distorção clássica da síntese "West Coast" (Buchla): quanto mais
você empurra o sinal além do limite, mais ele dobra sobre si mesmo, e
mais denso/brilhante fica o timbre, sem nunca "estourar" pra fora da
faixa ±1 (diferente de um clipper, que fica cada vez mais achatado e
quadrado).

**A fórmula, termo a termo:** primeiro `f = x·(1 + amount·3·|y|)`,
depois `f` é refletido (até 4 vezes) pra caber em ±1.

- `|y|` é o **valor absoluto** do companion — sempre positivo ou zero,
  não importa se `y` em si é positivo ou negativo. Isso é proposital: o
  papel de `y` aqui não é "empurrar pra cima ou pra baixo", é só dizer
  **o quanto** amplificar antes de dobrar — quanto maior a intensidade
  (o módulo) de `y` naquele instante, mais o sinal é multiplicado antes
  da dobra.
- `(1 + amount·3·|y|)` é um **ganho** que só sobe (nunca é menor que 1):
  em `amount`=0, o ganho é sempre 1 — a fórmula vira `f = x`, e a
  reflexão nunca chega a acontecer porque nada passa de ±1 que já não
  passasse antes. Ou seja: **`amount`=0 é passagem limpa, sem dobra
  nenhuma**, não importa o que `y` esteja fazendo.
- Em `amount`=1 e `y` no máximo (±1), o ganho chega a 4× — o suficiente
  pra empurrar o sinal bem além da faixa e provocar várias reflexões.
- **A reflexão em si:** pense numa régua de ±1. Se `f` ultrapassa 1 (ou
  fica abaixo de −1), ele "quica" de volta — a distância que passou do
  limite é refletida pra dentro, como um espelho na parede. Se depois
  do primeiro quique ele ainda estiver fora da faixa, quica de novo —
  até 4 vezes; depois disso, o que sobrar é simplesmente cortado (um
  limite de segurança, pra nunca sair do controle).

**O que isto quer dizer com "dirigida por `y`":** um wavefolder comum
(como o do `SHAPE`) tem um knob de intensidade fixo — você gira e a
dobra fica mais forte, mas **igual o tempo todo**, até você girar de
novo. Aqui, a intensidade da dobra é **o valor de um outro sinal, a
cada instante** — ela pode:

- **seguir um envelope:** `y` = a saída de um `ENVELOPE` na mesma nota.
  O ataque da nota (onde o envelope está alto) dobra forte — harmônico,
  denso, quase agressivo — e a cauda (onde o envelope já caiu perto de
  0) quase não dobra — limpa. O timbre "abre e fecha" sozinho, nota a
  nota, sem automatizar nada à mão.
- **pulsar com um LFO/oscilador:** `y` = uma onda que sobe e desce. A
  quantidade de dobra sobe e desce junto — um "respirar" tímbrico
  rítmico, de limpo a denso e de volta, no tempo do LFO.
- **ficar parada, se `y` for silêncio:** `|y|`=0 sempre → ganho=1 sempre
  → **nunca dobra**, não importa o `amount`. Sem companion ativo, a
  relação `Fold` não faz nada.
- **ficar quase constante, se `y` for um valor fixo alto** (uma tensão
  manual de um `CONTROL`/`MULT`): a dobra fica sempre igual — vira, na
  prática, um wavefolder comum de intensidade fixa.

**Em resumo, por intensidade de `|y|`:** perto de 0 → praticamente sem
mudança (limpo); um pouco acima de 0 → uma dobra, harmônicos pares
somados, ainda reconhecível como "a mesma nota, mais grossa"; perto de 1
com `amount` alto → várias reflexões, timbre denso e brilhante,
tendendo a ruído-com-altura em picos muito fortes.

> **E se o companion for o próprio cabo?** `y` vira `|x|` do bloco
> anterior — a dobra passa a seguir **o próprio volume recente do
> sinal**: quanto mais forte o sinal já estava tocando, mais ele se
> dobra a seguir. É uma distorção dinâmica orgânica — "toca mais alto,
> fica mais denso" — sem precisar de nenhum segundo módulo fazendo esse
> trabalho de detectar envelope.

### 1.3 `Difference` — subtrair um sinal do outro

**O que é, pra quem nunca mexeu com isso:** subtrair um sinal de outro
faz o que já é igual entre os dois **se cancelar**, e só sobra o que é
**diferente**. No limite, se `x` e `y` forem exatamente o mesmo sinal,
`x − y` é **silêncio perfeito** — é o mesmo princípio por trás de
fones com cancelamento de ruído (que captam o ruído externo e subtraem
uma cópia dele do que você ouve). Quando os dois sinais são parecidos
mas não idênticos — por exemplo, `y` é uma cópia atrasada ou filtrada de
`x` — a subtração não cancela tudo: ela cancela as partes que **não
mudaram** entre os dois, e deixa mais evidente o que **mudou**
(transientes, ataques, bordas). E quando `y` é uma cópia atrasada por um
tempo fixo (não relacionada a `x` por filtragem, só por atraso), a
subtração também produz um **filtro em pente** (comb filter): em certas
frequências, a versão atrasada chega quase em fase oposta à direta e
elas se cancelam (entalhes/*notches*); em outras, se reforçam. É o
mesmo fenômeno por trás do "efeito flanger" e do timbre metálico de um
cano/tubo.

**A fórmula, termo a termo:** `x − amount·y` é bem direta —

- em `amount`=0, nada é subtraído: o sinal passa intacto.
- em `amount`=1, subtrai `y` inteiro de `x`.
- entre os dois, uma subtração parcial — útil pra cancelar só uma
  parte, não o sinal inteiro (por exemplo, atenuar um ruído de fundo sem
  apagar a voz por cima dele).

**O papel de `y`:**

- **`y` = cópia atrasada de `x` (por um `LOOPER`/`SPACE` com tempo
  curto):** o que é estacionário/sustentado em `x` cancela bastante
  contra a própria cópia atrasada; o que muda rápido (um ataque, uma
  transição) não teve tempo de "chegar" na cópia atrasada ainda, então
  não cancela — sobra mais evidente. É por isso que isto funciona como
  **realce de transiente**.
- **`y` = versão filtrada de `x`** (por exemplo, só os graves de `x`):
  subtrair os graves de `x` deixa o resultado mais fino/agudo — um jeito
  de fazer um "corte de grave" sem usar um `FILTER` dedicado no cabo.
- **`y` = sinal sem relação nenhuma com `x`:** a subtração deixa de ser
  "cancelamento" no sentido estrito e vira só uma forma a mais de
  **combinar** dois sinais (o resultado pode somar energia em vez de
  cancelar, se os dois estiverem em fases opostas) — ainda é um efeito
  válido, só não tem o caráter de "realce" descrito acima.

> **E se o companion for o próprio cabo?** `y` vira `x` do bloco
> anterior — em `amount`=1, a saída é literalmente `x_agora − x_antes`,
> a **diferença entre uma amostra e a anterior**. Isso é um
> **diferenciador** clássico de processamento de sinal: ele mata tudo
> que é constante ou muda devagar (um tom sustentado praticamente some)
> e realça o que muda rápido (bordas, ataques, ruído de alta frequência
> ficam proporcionalmente mais fortes). Na prática soa como um filtro
> passa-alta agressivo "de graça", sem gastar um `FILTER`.

## 2. Ruptura e cicatriz

**O que acontece, exatamente, quando um cabo é rompido:** ele **não**
vira silêncio instantâneo (o que aconteceria se, por exemplo, você
descesse um `VCA` a zero). Em vez disso, o cabo guarda o **último
pequeno bloco de áudio** que estava passando por ele — um pedacinho de
poucos milissegundos, o tamanho do bloco que o motor processa de cada
vez — e fica **repetindo esse mesmo pedacinho em loop**, como uma
minúscula fita presa num ponto, enquanto o volume desse loop **decai
exponencialmente**: a cada 350 ms, o que sobra de volume cai bastante
(é uma curva, não uma reta — no começo ainda dá pra ouvir bem, depois
esvai rápido), até ficar abaixo de um piso de silêncio e a cicatriz
"fechar" de vez (não fica um chiado infinitamente baixinho tocando pra
sempre). Se aquele pedacinho de áudio tinha uma altura/textura
reconhecível, você vai ouvir **ela mesma repetindo em loop e sumindo**
— um eco cada vez mais fraco de si mesma, não um corte seco.

Essa é a diferença entre "romper" e "silenciar": silenciar apaga;
romper **segura o último instante e deixa ele morrer aos poucos** — é
"romper não é apagar, é segurar o último instante e deixá-lo sumir
devagar" (ver [`COMO_PENSAR.md`](COMO_PENSAR.md) §3). **Reconectar**
(`reconnect`) cancela tudo isso na hora: o cabo volta a passar o sinal
ao vivo, sem cicatriz nenhuma, como se nunca tivesse sido rompido.

No painel, `[espaço]` faz isso com **todos os cabos do patch de uma
vez** — um gesto grande, útil no meio de uma peça (o instrumento
"prende a respiração": cada cabo passa a repetir e apagar seu próprio
último instante, todos ao mesmo tempo, e depois volta tudo junto ao
apertar de novo). Romper **um cabo só**, deixando os outros intocados —
por exemplo, só a camada de fundo, mantendo a voz principal tocando —
hoje só dá pra fazer editando o `.rmp` (`state=`) ou em código.

## 3. Condução probabilística

**O que é, exatamente:** todo cabo tem uma **probabilidade de
conduzir** por vez, chamada `conductance` (0 a 1; **1.0 por padrão**,
que significa "sempre conduz", sem sorteio nenhum — é o comportamento
normal de um cabo comum). Quando `conductance` é menor que 1, a cada
~50 ms (20 vezes por segundo) o cabo **sorteia de novo**, como jogar
uma moeda viciada: com `conductance`=0.7, a moeda dá "conduz" 70% das
vezes; com 0.3, só 30%. O resultado desse sorteio vale até o **próximo**
sorteio, ~50 ms depois — não é um brilho contínuo nem um volume mais
baixo o tempo todo, é **liga/desliga**, decidido no acaso, em janelas de
dezenas de milissegundos. Pra não estalar quando liga/desliga, a
transição desliza (não pula) de um estado a outro em ~30 ms — mais
suave que um clique, mas ainda rápido o bastante pra parecer um
"corte", não um fade longo.

**O que se ouve, por faixa de valor:**

- **perto de 1** (ex.: 0.9): o cabo conduz quase sempre — de vez em
  quando um sumiço rápido e curto, como um mau contato ocasional. Dá
  "imperfeição orgânica" sem comprometer a voz.
- **no meio** (ex.: 0.5): moeda honesta a cada ~50 ms — o sinal
  alterna entre tocando e sumindo em blocos curtos e irregulares (às
  vezes a sorte mantém "ligado" por vários sorteios seguidos, às vezes
  "desligado"). Soa como um **gaguejo/stutter** rítmico, mas nunca
  regular — porque é sorteado, não é um padrão fixo.
- **perto de 0** (ex.: 0.1): o cabo passa a maior parte do tempo em
  silêncio, com lampejos curtos e raros de sinal — um "fantasma" de
  conexão, quase cortada.

Isto é exatamente o comportamento de um Bernoulli gate como o Mutable
Marbles/Branches (uma moeda sorteada a um ritmo fixo, decidindo se um
sinal passa) — só que embutido em **qualquer** cabo do patch, sem
precisar de um módulo à parte. A única limitação de hoje: o ritmo do
sorteio é fixo em ~20 Hz — não dá pra deixá-lo mais lento (sorteios a
cada segundo, por exemplo) nem mais rápido ainda sem editar o motor.
Cada cabo sorteia de uma semente própria (derivada de que módulos ele
liga), então o padrão de liga/desliga de cada cabo é diferente, mas
**se repete sempre igual** ao recarregar o mesmo patch — é acaso
determinístico, como o `drift` (ver [`COMO_PENSAR.md`](COMO_PENSAR.md)
§5).

O painel já tem um slider pra isto (`COND`, ver §4) — arrasta e ouve o
resultado na hora.

---

## 4. Como usar no painel

**Clique no corpo de um cabo** — no fio em si, na barriga da curva, não
numa ponta (as pontas continuam sendo pra puxar/tirar cabo, como
sempre). Abre um pequeno inspector ancorado ali perto, sem esconder o
resto do patch:

1. Uma linha de 4 botões — `NONE · RING · FOLD · DIFF` — escolhe a
   relação. O botão aceso é a relação atual do cabo.
2. Ao escolher `RING`/`FOLD`/`DIFF` pela primeira vez (vindo de
   `NONE`), o painel entra no modo de **escolher o companion**: todo
   jack de **saída** de todo módulo do patch acende um halo (o mesmo
   afordance de "destino válido" que já existe ao puxar um cabo
   normal). Clique em qualquer um — inclusive na própria origem do
   cabo, pra uma auto-relação (ver a caixa "e se o companion for o
   próprio cabo?" em cada relação, §1). `Esc` cancela e volta a `NONE`.
3. Com uma relação já ligada, dois sliders aparecem: `AMT` (a
   intensidade, `amount`) e `COND` (a condução probabilística,
   `conductance`). Clique e arraste verticalmente — a mesma física dos
   knobs de módulo (sobe = mais, desce = menos), com o som mudando em
   tempo real.
4. Um botão `ROMPER`/`RECONECTAR` no fim rompe **só aquele cabo** — a
   cicatriz e o decaimento de §2, sem afetar o resto do patch. É a
   versão seletiva do `[espaço]`.
5. Clicar fora do inspector (ou `Esc`) fecha.

Trocar de tipo de relação (por exemplo `RING`→`FOLD`) num cabo que já
tem um companion **mantém** o companion e o `amount` — só troca a
fórmula. Pra trocar o companion sem trocar o tipo, escolha o mesmo
botão de novo.

Tudo isso já é salvo por `Ctrl+S` (a serialização do patch sempre
gravou `relation=`/`companion=`/`amount=`/`conductance=`/`state=` por
cabo — o que faltava era só o jeito de editar ao vivo) e volta
exatamente igual com `--resume`/`RASGO_RESUME=1`.

---

## 5. Receitas

### Receita A — Ring-mod na própria conexão (sinos e metais)

Sem gastar um `SHAPE`: pegue o cabo que já leva uma voz até o filtro e
faça ele mesmo multiplicar por um segundo oscilador.

```
OSC (voz)  → FILTER (in)        ← este é o cabo a editar
OSC #2 (afinado numa razão não-inteira) → (só existe pra servir de companion)
```

Clique no corpo do cabo `OSC → FILTER.IN`, escolha `RING`, e no modo de
escolha de companion clique na saída `SAW` do `OSC #2`. Suba `AMT` até
~0,8. A voz que entra no filtro agora carrega bandas soma e diferença
do segundo oscilador — metálica, sem nenhum módulo novo no rack.

### Receita B — Wavefolder dirigido por um envelope

Em vez de um `FOLD` fixo (como no `SHAPE`), deixe a quantidade de dobra
**seguir** um envelope — dinamicamente, na própria conexão.

```
OSC (voz)      → SHAPE (in)     ← cabo a editar
ENVELOPE (env) → (só serve de companion; não precisa ir a lugar nenhum)
```

Clique no cabo `OSC → SHAPE.IN`, escolha `FOLD`, e clique na saída
`ENV` do `ENVELOPE` pra companion. `AMT` no talo. Cada nota agora chega
mais dobrada no ataque e mais limpa na cauda — sem precisar cabear o
`ENVELOPE` no `FOLD` do módulo (que também funcionaria, mas esta é a
versão "de graça", na própria conexão).

### Receita C — Realce de transiente por diferença

Um cabo pode subtrair uma versão atrasada de si mesmo do sinal —
cancela o que é estacionário, deixa passar o que muda rápido.

```
FILTER (all) → MIXER (ch1)        ← cabo a editar
SPACE (wet, com TIME bem curto do mesmo sinal) → (companion)
```
Clique no cabo `FILTER → MIXER.ch1`, escolha `DIFF`, e clique na saída
`WET` do `SPACE` pra companion. `AMT` ~0,5. Com o `SPACE` no `TIME`
mínimo, o companion é quase uma cópia do próprio sinal, levemente
atrasada — a subtração realça as bordas (transientes) e cava um leve
filtro em pente no resto.

### Receita D — Congelar um cabo só (o resto do patch continua)

Clique no cabo que você quer travar (por exemplo, a entrada do `MIXER`
que está recebendo uma textura de fundo) e aperte `ROMPER`. Aquele
ponto do patch fica preso na última cicatriz sonora (decaindo ~350 ms
e depois silencioso) enquanto todo o resto do patch segue tocando
normalmente — diferente do `[espaço]`, que pega tudo. `RECONECTAR`
volta ao normal.

### Receita E — Um caminho que falha de vez em quando

Num cabo secundário (uma camada, não a voz principal), clique nele e
arraste `COND` até ~0,5. Aquele caminho passa a "piscar" — ora conduz,
ora cai a silêncio por ~30 ms — um efeito de conexão intermitente,
estruturado (Marbles/Branches), sem precisar de um `DECISION`/`SWITCH`
a mais no meio.

---

## 6. Editando o arquivo à mão (avançado)

O painel (§4) é o caminho normal agora. Editar o `.rmp` direto ainda
serve pra scripts, pra reproduzir um ajuste exato sem depender do
mouse, e é como as peças de exemplo fazem (`examples/peca_generativa*.cpp`,
que falam com o `Cable` em C++ direto, sem painel nenhum).

1. Monte o patch normalmente no painel.
2. **`Ctrl+S`** salva a sessão em
   `~/.local/share/rasgo-modular/session.rmp` (texto simples).
3. Abra o arquivo. Cada cabo é uma linha:
   ```
   cable 4:2 -> 7:0 gain=1 conductance=1 feedback=0 state=intact
   ```
   `4:2` é `nó:porta` da **saída** (fonte); `7:0` é `nó:porta` da
   **entrada** (destino). Os números de nó **não são os que aparecem no
   painel** — pra saber qual módulo é qual, olhe a linha
   `panel shown <id> <id> …` no fim do arquivo: ela lista os nós **na
   mesma ordem em que os módulos aparecem no rack**, um id por módulo.
4. Pra dar uma relação a um cabo já existente, acrescente à linha (em
   qualquer ordem, depois do `->`):
   ```
   relation=ring companion=12:0 amount=0.6
   ```
   `relation` é `ring`, `fold` ou `diff`; `12:0` é `nó:porta` da saída
   companion.
5. Pra romper só este cabo, troque `state=intact` por `state=ruptured`.
6. Pra dar uma condução probabilística, mude `conductance=1` para,
   por exemplo, `conductance=0.6`.
7. Salve o arquivo e recarregue com `RASGO_RESUME=1` (ou `--resume`) —
   um `RASGO_SEED=N` normal **não** lê a sessão salva (ver
   [`../apps/panel/design.md §2.3`](../apps/panel/design.md)).

## Se você conhece o Eurorack

O paralelo mais próximo é o Mutable Warps — um módulo cuja função **é**
a relação entre duas entradas (ring-mod, wavefolder cruzado). O Rasgo
foi um passo adiante: se o cabo já é um objeto com estado (ganho,
condução, ruptura), a relação pode morar **nele**, sem gastar um módulo
e um HP a mais.
