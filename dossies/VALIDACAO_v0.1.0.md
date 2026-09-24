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
5. ✅ **FEITO em 22 set. 2026** — saíram os dois `.wav` com o tap no
   nome e o `.score.txt`, em PCM 24 bits a 44,1 kHz.

   **Cuidado ao julgar pelo ouvido:** o `pre-safety` só soa mais alto e
   mais dinâmico quando o sinal está **quente o bastante** para a
   proteção agir, E quando o MASTER é o único caminho até a saída. Nesta
   tomada os picos ficaram em −10 dB (longe do teto) e havia **quatro**
   fontes chegando no OUT — então o `post-safety` saiu 1,6 dB mais ALTO
   que o `pre`, o que parece defeito e não é: o post é a soma final, e o
   pre enxerga só o MASTER. Medido direto no MASTER com sinal quente, a
   relação esperada aparece limpa: pre em +17,9 dB de pico contra post
   cravado em −1,0 dB (o teto), com 11,9 dB de crista contra 6,2 dB.

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

**Achados** (autor, 23 set. 2026):

- nenhum seed abre em silêncio total, mas **vários abrem muito fracos** —
  foi preciso subir o volume das caixas para perceber que havia som.
  **A faixa de volume entre seeds precisa ser revista.**
- nenhum seed assustou.
- seed gravado: **1276993369095270621**
- **estalos** nesse seed.

**Confirmação por medição (renderizado aqui, sem janela):** o seed
1276993369095270621 tem pico em **−18,7 dBFS** em 60 s. Isso corrobora o
achado de volume baixo com um número, e não só com impressão.

Sobre os estalos: **não se reproduzem no render**, nem com VARIA ligado —
zero descontinuidades em 60 s, nos dois casos. Ou seja, **não são do
DSP**. Ver a seção "Estalos" no fim deste documento.

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

**Achados** (autor, 23 set. 2026):

- VARIA funciona; MUDA, EVOLUI e CRUZA funcionam;
- a variação **continua musical**, mas **às vezes é discreta demais**.
  Pedido do autor: **um knob para regular a intensidade do VARIA**;
- **WIDTH: nenhuma diferença audível.** BODY: idem;
- **estalos em alguns seeds.**

**WIDTH — medido, e o achado é real:**

| Fonte | WIDTH=0 | WIDTH=1 | WIDTH=2 | variação |
|---|---|---|---|---|
| **mono** | −13,46 dB | −13,46 dB | −13,46 dB | **0,00 dB** |
| estéreo | −240 dB | −13,46 dB | −7,44 dB | 232 dB |

O código está **correto**: WIDTH é mid/side, e num sinal mono o lado é
zero — não há o que escalar. Mas o MASTER tem **uma** entrada e a maioria
dos módulos é mono, então na prática o knob é **inerte na maior parte dos
patches**. Não é defeito de implementação; é um problema de projeto e de
expectativa, e precisa de decisão.

**BODY — não é defeito, é desenho.** É o governador de corpo do
`OutputStage`: só age sobre energia **alta, sustentada e concentrada** em
2,5–8 kHz (a faixa de fadiga do ouvido), com ataque de ~250 ms e limiar
alto. O próprio projeto diz que "música de ruído passa com zero redução
na esmagadora maioria dos casos". Girar e não ouvir nada é o
comportamento esperado. O que falta é a interface **mostrar quando ele
está agindo** — a telemetria existe (`bodyGuardDb()`) e não é exibida.

### Estudo 3 — Cabo

*A ideia que separa este instrumento: o cabo como objeto.*

1. `n` para descabear tudo. Construa um patch do zero, ligação por ligação.
2. **CLIQUE SOBRE UM CABO** (no meio dele, não nos jacks) para abrir o
   inspector. Ali estão RING, FOLD, DIFF, AMT e COND — eles **não são
   módulos**, são propriedades do cabo. Esta instrução dizia só "abra o
   inspector num cabo", sem dizer que é clicando, e foi por isso que o
   autor os procurou no rack e não achou.
3. Rompa e reate cabos, um a um e com `[espaço]`.
4. Grave uma tomada do processo inteiro — aqui o `.score.txt` é o
   documento principal, porque registra cada ligação.

**Escutar:** a cicatriz da ruptura soa como decaimento ou como corte
seco? A condutância probabilística produz variação musical ou intermitência
irritante? As relações mudam o timbre de forma previsível?

**Achados** (autor, 23 set. 2026):

- `[espaço]` funciona: rompe e reata;
- **"RING, FOLD, DIFF, AMT e COND — não achei esses módulos."**
- **o `.score.txt` é ininteligível**: "não dá pra entender que cabo está
  conectado onde, ou quais módulos estão acionados, nem qual a regulagem
  empregada".

**Os dois últimos são achados de verdade, e nenhum é do instrumento:**

1. **RING/FOLD/DIFF/AMT/COND não são módulos** — são as propriedades do
   CABO, no **inspector**, que abre ao clicar sobre um cabo. O autor
   procurou entre os módulos porque nada o levou até lá. É falha de
   descoberta: a ideia central do instrumento — o cabo como objeto com
   estado — está escondida atrás de um clique que ninguém anuncia. **As
   instruções que eu mesmo escrevi diziam "abra o inspector num cabo"
   sem dizer que é clicando no cabo.**
2. **o score usa números de nó crus** (`33:1 -> 38:2`), sem nome de
   módulo, sem nome de porta, sem os parâmetros, **e sem o seed**. Para
   um documento que o protocolo chama de "o documento principal" do
   Estudo 3, isso é inútil: não se reconstrói nada a partir dele.

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

**Achados** (autor, 23 set. 2026): **não conseguiu executar** — pediu um
patch pronto para o teste.

Era falha do protocolo, não de quem o executou: pedir que se cabeie dez
módulos antes de ouvir a primeira nota, num instrumento onde ligar módulo
a módulo é o trabalho inteiro, transforma um estudo de escuta em
exercício de montagem.

**RESOLVIDO em 24 set. 2026.** Os quatro casos viraram patches prontos em
[`patches-estudo4/`](patches-estudo4/LEIA-ME.md), abríveis por `Ctrl+O`.
Cada um foi **medido antes de ser entregue**, e a ida-e-volta pelo mesmo
caminho do `Ctrl+O` foi verificada:

| patch | pico | LUFS | no teto | finito |
|---|---|---|---|---|
| 1 · matéria (ressoadores no limite) | −5,9 dBFS | −10,4 | 0 | sim |
| 2 · espaço (realimentação alta) | −2,4 dBFS | −14,6 | 0 | sim |
| 3 · **direto no OUT** | **−1,0 dBFS** | −13,0 | **368** | sim |
| 4 · signal-in | mudo sem entrada | — | 0 | sim |

O caso 3 já confirma por medição o que o estudo procura: **por fora do
MASTER o sinal bate no teto de segurança** (−1,0 dBFS é exatamente
`0,891251`) e recorta, **sem nunca estourar nem produzir NaN**. Falta o
julgamento do ouvido sobre se o recorte soa como aviso.

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

---

## Síntese dos quatro estudos — 23 set. 2026

**Os estudos funcionaram**: produziram sete achados que nem os 77 testes
nem a minha leitura do código tinham encontrado. É exatamente o que o
precedente do Antitotem previa — lá a sessão de escuta achou um bug de
sinal real.

### Estalos — a pista mais séria, e o que já se sabe

Relatados em mais de um seed. **Não se reproduzem no render headless**:
zero descontinuidades em 60 s do seed 1276993369095270621, com e sem
VARIA. Logo, **não são do DSP**.

O que sobra é o comportamento em tempo real, e há uma cadeia que liga os
estalos ao achado de CPU do mesmo dia:

1. a interface repinta tudo a 30 Hz e consome **47% de um núcleo**;
2. para desenhar, ela toma o `gmx` (fotografia de osciloscópios e cabos);
3. o thread de áudio usa `try_to_lock` e, **quando falha, reemite o
   último bloco** com uma rampa (`starve_ *= 0.86`);
4. reemitir bloco é descontinuidade — e descontinuidade é estalo.

Isso explicaria por que são **intermitentes** e por que aparecem "em
alguns seeds" (os mais pesados disputam mais o lock). **É hipótese, não
conclusão** — falta contar quantas vezes o `try_to_lock` falha. É uma
instrumentação pequena e decide a questão.

**Consequência para a publicação:** se confirmada, os estalos e o custo de
CPU são **o mesmo problema**, e a decisão de "publicar agora e otimizar
depois" merece ser reconsiderada — estalo audível é defeito de
qualidade, não questão de desempenho.

### Tabela dos achados

| # | Achado | Natureza | Onde |
|---|---|---|---|
| 1 | **Estalos** em alguns seeds | possível defeito de RT | hipótese do lock, a confirmar |
| 2 | **Faixa de volume entre seeds** muito ampla; vários abrem fracos (medido: −18,7 dBFS) | ajuste de projeto | `PatchSeed.hpp` |
| 3 | **WIDTH inerte** em fonte mono (medido: 0,00 dB de variação) | projeto/expectativa | `Master.hpp` + decisão |
| 4 | **`.score.txt` ininteligível**: nós crus, sem nomes, sem parâmetros, **sem o seed** | defeito de utilidade | `ScoreRecorder` |
| 5 | **Cabo como objeto não se descobre** — RING/FOLD/DIFF/AMT/COND escondidos atrás de um clique não anunciado | defeito de descoberta | UI + tutorial |
| 6 | **BODY parece não fazer nada** (é desenho, mas não se vê) | falta telemetria na tela | UI |
| 7 | **Estudo 4 inexecutável** sem patches prontos | falha do protocolo | preparar `.rmp` |

### O que NÃO é defeito, verificado

- **BODY**: governador de corpo, age só em energia alta/sustentada/
  concentrada em 2,5–8 kHz, ataque de 250 ms, limiar alto. Não agir é o
  comportamento correto na esmagadora maioria do material.
- **WIDTH**: o código mid/side está certo; é inerte porque a fonte é
  mono.
- **Memória**: não vaza (30 min de vigia: +28 kB nos últimos 5 min).
- **Motor**: 6,6–14,5% de um núcleo; medição de loudness 0,55%.
