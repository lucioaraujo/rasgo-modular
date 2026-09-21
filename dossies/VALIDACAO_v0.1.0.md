# Validação para a v0.1.0 — protocolo

Este documento existe para transformar o teste do autor em **evidência de
validação** no formato que o gate da camada 2 exige
(`ESTRATEGIA_DE_PUBLICACAO.md`) — não em impressão solta. Segue o
precedente do Antitotem, que fechou **quatro estudos formais** antes de
publicar e, no processo, achou um bug de sinal real.

Duas partes. A **A** é a que trava a publicação e só o autor pode fazer.
A **B** é verificação funcional do que mudou e eu não pude observar.

> **Como preencher:** cada estudo tem um espaço `Achados`. Escreva mesmo
> que seja "nada a relatar" — um estudo sem achado registrado é
> indistinguível de um estudo não feito.

---

## Por onde começar — a ordem que fecha a publicação

Escrito em 21 set. 2026, quando o autor perguntou "o que preciso testar
para validar de uma vez por todas a publicação?". A resposta honesta é
que **só duas coisas travam**, e uma delas é curta.

### Sessão 1 — uma hora, e destrava a camada 2 pela metade

Faça numa sentada só, com o app aberto e gravando:

1. ✅ **FEITO em 22 set. 2026** — `g` sorteia seed ao abrir, várias vezes
   seguidas. O defeito de foco está morto.
2. ✅ **FEITO em 22 set. 2026** — `Ctrl+Z` várias vezes seguidas,
   funcionando.
3. 🟡 **PARCIAL** — `Ctrl+S` (salvar) e `Ctrl+O` (abrir) confirmados em
   22 set. 2026. **Falta:** fechar o app e reabrir com `./run --resume`,
   e o `Ctrl+B` (arquivar no banco) seguido de abrir aquele arquivo. Se
   o patch abrir mudo, o alvo de saída não reancorou — é a falha que
   este passo procura, e ela só aparece no ciclo completo.
4. ✅ **FEITO em 22 set. 2026** — arrastar módulo pelo corpo na vista
   RACK·SAÍDA reposiciona. A correção que estava sem confirmação desde
   20 set. está validada.
5. **`./run --rec-both` por um minuto.** Dois `.wav` com o tap no nome
   mais um `.score.txt`; o `pre-safety` deve soar mais dinâmico.

Se os cinco passarem, o instrumento está funcionalmente verificado.

### Sessão 2 — a que não tem atalho

Os **quatro estudos de escuta da Parte A**. É o único bloqueio duro da
camada 2, e nenhum trabalho de código o remove: alguém precisa ouvir e
registrar o que ouviu. O Antitotem fechou quatro antes de publicar e
**achou um bug de sinal real no processo**.

Pode ser em dias diferentes. O que não pode é ser pulado, nem ser
"escutei e achei bom" sem achado escrito — um estudo sem achado
registrado é indistinguível de um estudo não feito.

### O que NÃO precisa mais ser testado

- **Teclado** — virou tabela única com teste que roda nos três sistemas;
- **MATRIX** — confirmado no uso;
- **Build e empacotamento nos três sistemas** — a CI provou, com
  `.deb`/`.dmg`/`.exe` gerados e o `.app` verificado como Universal 2 de
  verdade;
- **Comportamento do motor** — 77 testes, executados em Linux, Windows e
  macOS.

### O que fica declarado como limitação, não testado

O app **nunca foi aberto** em Windows nem em macOS, e não há como fazê-lo
aqui. A decisão de publicar assim já foi tomada e registrada — o que o
gate exige é que isso seja dito em voz alta, e está.

---

## Parte A — escuta (4 estudos)

### Preparação comum

- Fones ou monitores em que você confie, volume de trabalho.
- Grave **os dois taps**: `RASGO_REC_TAP=both ./RasgoModularApp`. O
  `pre-safety` mostra a dinâmica que o limitador esconde; o `post-safety`
  é o que sai de fato. Comparar os dois é metade do estudo.
- Cada tomada gera `.wav` + `.score.txt`. **Anote o número do seed** — ele
  reproduz o patch exatamente, e é o que torna o estudo repetível.
- A leitura está no cartão **SOBRE**: LUFS (momentary / short-term /
  integrated), **true-peak em dBTP**, a **taxa de amostragem real** e a
  distância até o alvo. Anote a integrada e o true-peak ao fim de cada
  tomada.
- **Alvo declarado (21 set. 2026): streaming** — −14 LUFS integrado, teto
  de −1 dBTP. A linha do alvo fica em cor de aviso quando o true-peak
  passa do teto. Isso **não corrige nada sozinho**: o medidor não toca no
  sinal. É informação pra você decidir.
- As gravações saem em **PCM 24 bits**.

### Estudo 1 — Semente

*O comportamento generativo por omissão: o instrumento tocando sozinho.*

1. Abra o app. Ouça por ~2 min sem tocar em nada.
2. Aperte SEED umas dez vezes, ouvindo ~30 s cada.
3. Grave uma tomada de 3–5 min de um seed que tenha gostado.

**Escutar:** algum seed abre em silêncio? Algum abre agressivo ou
estridente a ponto de assustar? A faixa de volume entre seeds é aceitável
sem mexer no MASTER? O limitador está trabalhando o tempo todo (indicador
de clipe aceso com frequência)?

**Achados:**

### Estudo 2 — Deriva

*Variação ao vivo e as operações genéticas.*

1. Com o VARIA ligado, ouça um patch por 5 min sem intervir.
2. Aperte MUDA algumas vezes; depois EVOLUI; depois CRUZA.
3. Arraste o GAIN e o WIDTH do MASTER de um extremo ao outro, rápido.
4. Grave uma tomada cobrindo isso.

**Escutar:** a variação continua musical ou vira ruído? Algum salto
abrupto, clique ou estalo? O passo 3 é teste dirigido: GAIN e WIDTH não
tinham rampa e clicavam — a bateria automática pegou e corrigiu, mas **o
ouvido é o juiz**.

**Achados:**

### Estudo 3 — Cabo

*A ideia que separa este instrumento: o cabo como objeto.*

1. `n` para descabear tudo. Construa um patch do zero, ligação por ligação.
2. Abra o inspector num cabo: experimente RING, FOLD, DIFF; mexa em AMT e
   COND.
3. Rompa e reate cabos, um a um e com `[espaço]`.
4. Grave uma tomada do processo inteiro — aqui o `.score.txt` é o
   documento principal, porque registra cada ligação.

**Escutar:** a cicatriz da ruptura soa como decaimento ou como corte
seco? A condutância probabilística produz variação musical ou intermitência
irritante? As relações mudam o timbre de forma previsível?

**Achados:**

### Estudo 4 — Matéria e espaço

*As famílias de DSP pesado, e os extremos.*

1. Patch com STRING, MATTER, RESONATOR; leve `res`/`drive` ao máximo.
2. Patch com SPACE, HALL, LOOPER, SAMPLER; realimentação alta.
3. Cabeie um módulo **direto no OUT**, por fora do MASTER.
4. Se tiver SIGNAL-IN, entre com microfone ou instrumento.

**Escutar:** algo explode, trava ou produz silêncio permanente? Alguma
auto-oscilação foge de controle? O passo 3 é teste dirigido da **guarda
de segurança do sink**: por fora do MASTER, o som deve recortar de forma
feia e audível — isso é proposital, é o aviso de que falta um MASTER — mas
**nunca** estourar sem limite nem produzir estalo de NaN.

**Achados:**

### Fechamento

- [ ] Os quatro estudos têm achados escritos.
- [ ] Ao menos uma tomada por estudo, com seed anotado.
- [ ] Bugs achados viraram entrada em `TAREFAS.md`.
- [ ] LUFS integrado e true-peak anotados por tomada.
- [ ] Alguma tomada passou de −1 dBTP? Se sim, em que situação — é o dado
      que diz se a guarda de saída precisa de ajuste antes de publicar.

---

## Parte B — verificação funcional

O que mudou recentemente e **eu não tenho como observar**: não há
navegador nem display gráfico neste ambiente, e não abro janela na máquina
do autor. Marque o que conferir.

### Prioridade — onde a falha seria silenciosa ou cara

Estes vêm antes do resto. Não são os que mais mudaram, e sim aqueles cujo
defeito **não se anuncia**: perde-se trabalho sem aviso, ou o dano só
aparece depois.

**1. Desfazer, nos casos difíceis.** É o recurso mais novo e mais
estrutural — cada passo substitui o grafo inteiro por uma fotografia
serializada. Testar: ligar um cabo e desfazer; MUDA e desfazer; adicionar
módulo e desfazer; descabear (`n`) e desfazer; **doze ações seguidas e
doze desfazeres** (o anel guarda 24). A cada desfazer: o som continua sem
estalo? o número do seed na caixa acompanha? o patch volta ao que era
mesmo, e não a algo parecido?

**2. Salvar, abrir e a ida-e-volta entre os dois front-ends.** Envolve
disco e um diálogo nativo. Testar: SALVA, feche, `./run --resume`; BANCO e
depois ABRIR aquele arquivo; e — isto eu afirmei na documentação e **nunca
verifiquei** — salvar no painel X11 e abrir no app JUCE, e o contrário.
Falha esperada se algo estiver errado: o patch abre mudo (o alvo de saída
não reancorou).

**3. REC com os dois taps.** `./run --rec-both`, grave 1 min, pare.
Devem sair dois `.wav` com o tap no nome, mais um `.score.txt`. Os dois
tocam? O `pre-safety` soa mais dinâmico (e mais alto nos picos) que o
`post-safety`? Se soarem idênticos, o tap não está saindo de onde deveria.

**4. Estabilidade longa.** Deixe tocando 20–30 min com VARIA ligado,
fazendo outra coisa. Volte e ouça. Procurando: cliques esporádicos,
o som "afinando" ou perdendo corpo com o tempo, consumo de memória
subindo. É o teste que nenhuma bateria automática substitui, porque o
defeito é raro por definição.

### Teclado — ✅ **FECHADO em 21 set. 2026**

Era a área mais quebrada do app. Hoje não precisa mais ser testada tecla
a tecla, e a razão é estrutural: o mapeamento saiu do `keyPressed` e
virou **uma tabela única** (`src/ui/Shortcuts.hpp`) coberta por um teste
que executa as 17 combinações em todas as formas em que o sistema pode
entregar uma tecla — minúscula, maiúscula (Shift/CapsLock), sem
caractere, e com Ctrl mandando caractere de controle. O teste foi
verificado contra o bug (reintroduzindo a falha histórica ele acusa 31
falhas), e roda nos três sistemas.

Confirmado no uso pelo autor: `n`, `r`, `Ctrl+R`, `Ctrl+Z`, `Ctrl+O`, e
"todos os botões acendem".

**O que o teste NÃO prova, e só o uso mostra** — confira uma vez, ao
abrir o app, sem clicar em nada antes (era esse o caso que falhava):

- [x] ✅ logo ao abrir, **antes de clicar em qualquer lugar**, `g` sorteia
      um seed — confirmado em 22 set. 2026;
- [ ] depois de **digitar** um número na caixa de seed, os atalhos voltam
      a funcionar sem precisar clicar no rack;
- [ ] `Esc` cancela um cabo que está sendo puxado.

### Afordância de cabeamento

- [ ] Puxando um cabo, destinos que **chegam à saída** têm halo cheio; os
      que não chegam, halo apagado (válidos, mas ainda sem som)
- [ ] Puxando de uma saída **muda**, o cabo elástico sai acinzentado
- [ ] Ligar num caminho que não alcança a saída escreve um aviso na caixa
      LEARN, que some sozinho
- [ ] Depois de digitar na caixa de seed, os atalhos voltam a funcionar

### Módulo MATRIX — ✅ confirmado pelo autor em 20 set. 2026

("matrix está funcionando"). A grade 4×4 aparece como células, o arrasto
vertical muda o ganho e a barra sobe/desce do centro.

### Janela e layout

- [ ] A janela abre em ~88% do monitor (antes era fixa e baixa demais)
- [ ] Com 3 fileiras **não** aparece barra de rolagem
- [ ] O vão entre a frase de crédito e a 1ª fileira está justo
- [ ] Botão do MEIO arrasta o rack verticalmente

### Vista e idioma

- [ ] Em RACK·SAÍDA, depois de `n`, aparece a **dica** em vez de tela vazia
- [ ] Em RACK·SAÍDA, arrastar um módulo novo da paleta o deixa **visível**,
      com borda tracejada — e a borda some assim que ele é cabeado até o som
- [ ] Em RACK·SAÍDA, arrastar um módulo pelo corpo **reposiciona**. Este
      tem prioridade: o autor relatou exatamente esta falha em 20 set.
      2026 ("inseri o módulo com o rack saída, ele entrou por último,
      porém tentei desloca-lo para uma outra posição e não consegui"), foi
      corrigido, e a correção **nunca foi confirmada no uso**
- [ ] Trocar para EN muda `RACK · OUTPUT`, `BREAK`/`RECONNECT` no
      inspector e `built` no SOBRE
- [ ] `RING`/`FOLD`/`AMT`/`COND` seguem em inglês em qualquer idioma —
      é vocabulário técnico, decisão registrada

### Ícone e marca

- [ ] O ícone na barra de tarefas está **visível** (era preto sobre
      quase-preto)
- [ ] O ícone pequeno mostra o monograma, não um borrão

### Site (`website/index.html`)

- [ ] Abre em um navegador de cada motor (Blink, Gecko, WebKit)
- [ ] A captura de tela carrega e não estoura a largura
- [ ] A marca RASGO aparece laranja, alinhada com MODULAR pela base
- [ ] Os quatro idiomas trocam corretamente
- [ ] No telefone o texto é legível sem rolagem horizontal

---

## Depois

Fechada a Parte A, a camada 2 destrava e o caminho é: cortar `v0.1.0`,
extrair o projeto pra repositório próprio (é o que torna a CI de três
sistemas viva), gerar os instaladores e então publicar site e portal
juntos.
