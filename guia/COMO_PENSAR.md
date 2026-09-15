# Como pensar o instrumento

O Rasgo Modular não é um sintetizador que espera você. Ele é um
instrumento que **já está tocando** quando você abre — e o seu trabalho
é ouvir o que ele propõe e conduzir dali. Este capítulo é sobre a
mentalidade: como o instrumento quer ser pensado, e como isso muda a
forma de ter e de crescer uma ideia.

---

## 1. Ele soa ao carregar

Abra o painel e há som. Não existe folha em branco silenciosa: todo
`SEED` gera um patch inteiro — vozes, cabos, forma — que faz som na
hora. Um módulo sozinho no rack, sem nada plugado, **também** faz algo:
o `SPECTRA` vira um drone que respira, o `RESONATOR` canta como uma
taça, o `PULSAR` toca um trem de grãos, o `DRUM` com `roll` se dispara
sozinho. Cada módulo tem um **modo autônomo**.

**Como isso é possível, tecnicamente:** vários módulos escondem uma
fonte de excitação **interna** — um ruído de baixo nível, um relógio
próprio — pra nunca ficarem mudos por falta de entrada (o `RESONATOR`
"arqueia sozinho" com um ruído interno, ver a página dele; o `DRUM` com
`roll` se retriga sem gate externo nenhum). Não é um preset tocando por
cima — é o próprio circuito decidindo soar quando não recebe nada de
fora, do mesmo jeito que um instrumento acústico ressoa um pouco só
com o ar da sala.

O que isso muda: você não *constrói* um som do nada, você **negocia**
com um que já existe. A pergunta de partida não é "o que eu faço?", é
"o que estou ouvindo, e para onde quero levar?".

## 2. O seed é uma hipótese, não um preset

O `SEED` é um número. O mesmo número sempre produz o mesmo patch —
timbre, cabeamento, andamento, tudo. Mas ele **não é** um preset que
você recarrega para voltar exatamente onde estava: é um **ponto de
partida** que o instrumento te propõe, e que você modifica à vontade a
partir dali.

- `⚄ SEED` (ou `[g]`) sorteia um novo — uma proposta nova.
- Digite um número na caixa ao lado + `Enter` para reproduzir um
  específico (o som é determinístico).
- Todo cabo do patch de seed **tem função sonora** — o que está cabeado,
  soa. (A vista `RACK · SAÍDA` esconde tudo que não chega à saída.)

Trabalhe com o seed como um músico de jazz trabalha com um *standard*:
o tema está dado, a peça é o que você faz com ele.

## 3. O cabo é um objeto, não um fio

No Rasgo um cabo não só liga A a B. Ele tem estado e comportamento:

- **relação** — um cabo pode multiplicar (`RingMod`), dobrar (`Fold`) ou
  subtrair (`Difference`) o sinal por um terceiro, na própria conexão;
- **ruptura e cicatriz** — um cabo pode ser *rompido* (para de conduzir)
  e depois *cicatrizado* (segura o último bloco que passou) — é forma
  musical: cortar todos os cabos aos 28 s de uma peça e reatar aos 34 s
  é um gesto composicional, não um bug;
- **condução probabilística** — um cabo pode conduzir *às vezes*, com
  falhas.

O `[espaço]` rompe e reata tudo de uma vez — experimente no meio de um
patch denso e ouça o instrumento "prender a respiração".

Os detalhes de cada um desses três — as fórmulas, o que soa, e como
usá-los (clique no corpo de um cabo no painel) — estão em
[`RELACAO_DE_CABO.md`](RELACAO_DE_CABO.md), com receitas.

## 4. A relação entre as saídas costuma ser o gesto

Vários módulos têm **mais de uma saída** que representam a *mesma* ideia
sob ângulos diferentes — e o gesto musical é **cruzar** entre elas, não
girar um knob:

- o `FILTER` tem `low`/`center`/`high` na mesma frequência; `spread`
  abre a distância entre eles — é um formante móvel;
- o `RESONATOR` e o `FORMANT` têm `low`/`mid`/`high`; varrer `tilt`
  migra a energia de uma saída pra outra;
- o `SHIFTER` entrega `up` e `down` (espectro subido e descido)
  simultâneos;
- o `NOISE` tem 8 cores ao mesmo tempo; o `OSC`, 5 formas.

Quando você cabear duas saídas de um módulo para dois destinos e depois
mexer o knob que as relaciona, o patch inteiro se move junto. Isso é
mais "Rasgo" do que um knob de "mix".

**Por que isso é diferente de um "mix" comum:** um knob de mix cruza
entre duas coisas **não relacionadas** (seco/molhado, A/B) — cada lado é
independente do outro. Cruzar `LOW`/`HIGH` do mesmo `FILTER`, ou
`up`/`down` do mesmo `SHIFTER`, cruza entre **duas leituras do mesmo
evento físico** — a mesma nota, vista de dois ângulos. Mover o knob não
troca de fonte, **redistribui** onde a energia de uma única coisa
aparece — por isso o timbre se move de um jeito coerente (nunca solta
duas coisas estranhas uma da outra), em vez de misturar dois materiais
diferentes.

## 5. O `drift` faz o patch respirar

Quase todo módulo tem um knob **`drift`** (ou `jitter`, `wear`). Em 0, a
saída é exatamente determinística — dois renders idênticos. Subindo um
pouco, o módulo passa a **derivar**: a afinação passeia meio semitom, o
timbre cintila, o groove escorrega. Não é ruído aleatório — é um
movimento lento e correlacionado, e é **semeado**, então continua
reprodutível.

Um seno digital parado soa "morto"; um toque de `drift` o deixa vivo sem
você mexer em nada. Use pouco em tudo, e o patch inteiro ganha um
tremor orgânico.

**Por que "passeio lento e correlacionado" soa vivo e "ruído" soa
quebrado:** um valor que muda **bruscamente**, sem relação com o
instante anterior, é ouvido como defeito/estática — não é assim que
nada físico se move. Um valor que **deriva** (o próximo passo depende
um pouco do anterior, como uma mão que não consegue segurar
perfeitamente parada) soa orgânico porque é literalmente o mesmo tipo
de imperfeição de qualquer sistema físico real — um motor, uma corda,
uma respiração. `drift` é essa derivação, com pouca amplitude — é por
isso que soa "vivo" em vez de "quebrado".

## 6. O patch pode evoluir sozinho — o `DRIFT` e a mão caótica

Dois recursos levam o "respirar" além do módulo:

- o **módulo `DRIFT`** é uma fonte de CV que se move em escala de
  **minutos**, com memória (o movimento acumula) e correlação (as 4
  saídas são leituras diferentes do mesmo movimento). Plugue `a`/`b`/`c`
  em três parâmetros estruturais e o patch **se transforma sozinho** ao
  longo da peça;
- o **`VARIA`** (botão do cabeçalho) liga a *mão caótica*: um campo que
  move todas as fibras do patch em relação, na velocidade da energia do
  som. Não é aleatório e não é seguro só por sorte — é musical por
  construção (o `MIXER` e o `MASTER` ficam de fora; a voz é preservada).

Pensar composição aqui é **decidir o que deriva e o que fica fixo** —
não escrever cada evento.

## 7. As 8 famílias são verbos

Quando você não sabe qual módulo usar, pense no **verbo**:

| Preciso… | Família | Exemplos |
|---|---|---|
| de uma voz / de tensão do zero | **SOURCE** (gerar) | `OSC`, `MATTER`, `NOISE`, `CHORD`, `DRUM`, `SPECTRA` |
| modificar um som que passa | **TRANSFORM** (transformar) | `FILTER`, `SHAPE`, `LPG`, `RESONATOR`, `SHIFTER`, `VCA` |
| de um envelope / LFO / aleatório | **MODULATE** (mover) | `ENVELOPE`, `FUNCTION`, `SH`, `DRIFT`, `CHAOS`, `STAGES` |
| marcar o tempo | **TIME** | `CLOCK`, `SEQUENCE`, `TURING`, `LOGIC`, `TRIGSEQ` |
| escolher / quantizar / harmonizar | **DECISION** | `QUANTIZER`, `HARMONY`, `DECISION`, `ABACUS`, `BOXCAR` |
| rotear sinais | **ROUTE** | `SWITCH`, `MATRIX`, `MULT`, `PLANAR` |
| espaço / eco / memória | **SPACE** | `SPACE`, `HALL`, `MEMORY`, `LOOPER`, `SWIRL` |
| misturar / medir / enviar | **OUT** | `MIXER`, `MASTER`, `SCOPE`, `NOTE-OUT` |

A paleta do painel é organizada assim, e a ordem das famílias acima é a
ordem do fluxo do sinal.

## 8. O instrumento se ouve

O `SCOPE` e o `BOXCAR` **medem o próprio patch e devolvem a medição como
CV**: a altura que está soando (`SCOPE.pitch`), o ataque de um transiente
(`SCOPE.onset`), o brilho, o nível. Num scope de hardware a tela é um
beco sem saída; aqui a leitura sai por um jack.

Isso abre um tipo de patch onde o instrumento **reage a si mesmo**:
`SCOPE.onset` de uma bateria disparando um envelope no compasso do
áudio, `SCOPE.pitch` afinando um oscilador pelo que entra. É uma forma
de composição — o sistema fechando o próprio laço. (Como cada medição é
extraída de verdade — sem FFT, por diferenciação e autocorrelação —
está em `29_scope.md`.)

## 9. Autônomo não quer dizer fechado

O Rasgo soa sozinho, sem teclado nem entrada. Mas ele **acopla**: o
`SIGNAL-IN` deixa um teclado MIDI ou outro instrumento tocá-lo; o
`NOTE-OUT` deixa o Rasgo tocar um sequenciador externo; e há um plano
de barramento (Ensemble Bus) pra os instrumentos da família RASGO se
juntarem. Você pode começar generativo e ir amarrando controle à mão —
ou o contrário.

---

## Como ter uma ideia

Três pontos de entrada, todos válidos:

1. **Do seed.** Sorteie até ouvir algo que te interessa. Ligue
   `RACK · SAÍDA` pra ver só o que está tocando. Mexa um knob de cada
   vez e ouça o que ele controla. Quando algo pedir mudança, siga.

2. **De uma voz.** Escolha uma fonte pelo caráter (uma corda? um sino?
   fala? um trem de grãos?), cabeie-a até o `MIXER`, e trabalhe pra
   fora dela — um filtro, um envelope, um espaço.

3. **De um gesto.** Escolha um movimento — "quero que isto cruze de
   grave pra agudo devagar", "quero que a densidade suba e desça". Ache
   o módulo cujas saídas ou cujo knob fazem esse movimento, e construa
   o resto em volta.

## Como crescer a ideia

- **Uma segunda voz** num canal livre do mixer, mais baixa, com um
  caráter contrastante.
- **Uma modulação** — um `FUNCTION` lento no corte do filtro, um
  `ENVELOPE` no timbre. O patch deixa de ser estático.
- **Um `drift`** em dois ou três parâmetros — a peça passa a andar
  sozinha em segundos.
- **Um `DRIFT`** (o módulo) em parâmetros estruturais — a peça anda
  sozinha em minutos.
- **Uma seção** — rompa os cabos (`[espaço]`) e reate; ou automatize
  isso com o `Cable` e a ruptura; ou deixe o `DECISION` abrir e fechar
  uma sala.
- **O instrumento se ouvindo** — um `SCOPE` medindo a saída e
  realimentando o patch.

O caderno de [`RECEITAS.md`](RECEITAS.md) traz montagens inteiras nesse
espírito.
