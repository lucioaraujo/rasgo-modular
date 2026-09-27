# Rasgo Modular — prontidão para publicação

Auditoria contra `RASGO_DOCUMENTATION/ESTRATEGIA_DE_PUBLICACAO.md`: as
cinco camadas e o gate editorial comum. Cada item está marcado com o que
**foi feito**, não com o que deveria estar pronto.

**Data:** 24 set. 2026 · **Camadas 1 e 2 fechadas.** A CI de três
sistemas está verde e gera os três instaladores; a sessão de escuta foi
executada e documentada. O que falta é operacional: cortar a tag, tornar
o repositório público e liberar site e portal juntos.

---

## Camadas

| Camada | Critério | Estado |
|---|---|---|
| 0. Pesquisa local | estado documentado; nada apresentado como release | ✅ `RASGO_MODULAR.md`, `TAREFAS.md`, `dossies/`, `PESQUISA_MODULOS.md` |
| 1. Candidato publicável | build/execução multiplataforma, licença, créditos, documentação e limitações revisados | ✅ **fechada em 21 set. 2026** — CI verde nos três sistemas, com instaladores gerados |
| 2. Release do instrumento | pacote por plataforma, testes relevantes, **evidência de validação** | ✅ **destravada em 24 set. 2026** — pacotes ✅ (.deb/.dmg/.exe) · 78 testes ✅ nos 3 sistemas · **sessão de escuta documentada** ✅ (4 estudos com achados) |
| 3. Página editorial | texto, autoria, imagens, links e estado correspondem à release | 🟡 site **existe** em `website/` (4 idiomas, captura de execução real, **guia dos 58 módulos** e contato no padrão da família), em preparação — não publica antes do instrumento |
| 4. Publicação | portal, repositório e página liberados juntos | ❌ |

A regra é explícita: **não se pula da camada 0 para a 4.**

---

## Gate editorial — item a item

| Item do gate | Estado |
|---|---|
| nome, descrição curta e estado do projeto | ✅ `README.md` |
| autoria, créditos, fontes e licença | ✅ `CREDITS_AND_SOURCES.md`, `LICENSE`, `apps/juce/LICENSE_STATUS.md` |
| instruções de build/instalação e plataforma suportada | ✅ `INSTALL.md` |
| matriz de plataformas (testada / parcial / planejada) | ✅ `INSTALL.md` — e ela diz que Windows e macOS **nunca foram abertos** |
| testes automatizados e validações humanas **realmente executados** | ✅ 78 testes **executados nos três sistemas** · **validação humana executada** (24 set. 2026): 4 estudos de escuta, 7 achados, 3 corrigidos antes de publicar |
| screenshots com origem autorizada | ✅ uma, de execução real (18 set. 2026) — original em `screenshots/`, derivados no site |
| links corretos para repositório, documentação e release | 🟡 repositório existe (`lucioaraujo/rasgo-modular`, privado); os links de download **já estão escritos e conferidos** (`website/estado.py --depois`, nomes amarrados ao `CPACK_PACKAGE_FILE_NAME` por `website/verificar.py`) e passam a funcionar quando a tag criar a release |
| ausência de áudio privado, recordings, testes | ✅ verificado: nenhum áudio rastreado pelo git; os 19 MB de renders de referência passaram a ser ignorados explicitamente |
| contato oficial | ✅ **`rasgo.instruments@gmail.com`** — confirmado pelo autor em 21 set. 2026; já era o `CPACK_PACKAGE_CONTACT` dos instaladores |
| correspondência entre versão publicada e página editorial | ❌ depende da página |
| **procedimento de correção/retirada conhecido** | ✅ ver abaixo |

---

## O que falta, separado por quem resolve

### Depende de mim (código/documento)

- ~~extrair o projeto pra repositório próprio~~ **feito em 21 set. 2026**:
  [`lucioaraujo/rasgo-modular`](https://github.com/lucioaraujo/rasgo-modular),
  **privado por ora** — a estratégia manda liberar repositório, site e
  portal juntos, e a v0.1.0 ainda não foi cortada. O histórico foi
  preservado (`git subtree split`: 71 commits, só os que tocaram o
  instrumento), que é a postura arquivística do resto do RASGO — o rastro
  de como ele chegou aqui é parte do acervo. **A CI de três sistemas saiu
  da inércia e passou a rodar de verdade.** Falta tornar público no
  momento da publicação;
- ~~**a tag não produziria nada baixável**~~ **corrigido em 28 set. 2026**:
  a CI só fazia `upload-artifact`, que exige login no GitHub, vem zipado por
  cima do instalador e expira em 90 dias — não é download público, e o site
  não podia apontar para ele. O workflow ganhou um job `release` que, na
  tag, cria a release e anexa os três instaladores; e o
  `CPACK_PACKAGE_FILE_NAME` passou a ser explícito e com arquitetura, porque
  esses nomes são o link da página. Era um buraco entre "CI verde" e
  "alguém consegue instalar";
- ~~**área de download no site**~~ **feita**: `website/estado.py` tem as
  duas versões da página (antes e depois da release) nos quatro idiomas, e
  trocar no dia de publicar é `python3 estado.py --depois 0.1.0`. A versão
  pós-release traz link direto por plataforma e diz ali mesmo que Windows e
  macOS nunca foram abertos pelo autor e que o `.dmg` tem assinatura
  ad-hoc — a limitação onde a decisão de baixar é tomada;
- ~~`CHANGELOG.md` / notas da `v0.1.0`~~ **feito** — [`CHANGELOG.md`](CHANGELOG.md),
  escrito e marcado como *preparada, ainda não cortada*, com as limitações
  declaradas (Windows/macOS só na CI, CI inerte até a extração). Falta só
  criar a tag quando a Parte A fechar;
- ~~site do instrumento~~ **feito** (`website/`, 4 idiomas, faixa
  `.rasgo-strip`, DejaVu auto-hospedada, captura de execução real).
  Ampliado em 27 set. 2026 com **o guia dos 58 módulos** (`modulos.html` +
  três traduções) e com **o contato no padrão da família** — `<dialog>` e
  `contact.js` como no Antitotem e no portal, em lugar do `mailto:` que
  expunha o endereço. O guia é **gerado** do catálogo do instrumento
  (`--despejar-modulos` + `gerar_modulos.py`), então página e programa não
  divergem. Falta só abri-lo num navegador de cada motor antes de publicar;
- entrada do Modular no portal da família — o portal é território do
  Codex, então é combinação, não tarefa minha. O **texto pronto** do card
  nos quatro idiomas, no formato exato dos cards que já existem lá, está em
  [`website/PORTAL.md`](website/PORTAL.md), com a moldura correta e a
  indicação de que só entra no ar junto com o resto.

### Depende de você

1. ~~**Sessão de escuta documentada.**~~ **FEITA em 23–24 set. 2026.** Os
   quatro estudos executados e registrados em
   [`dossies/VALIDACAO_v0.1.0.md`](dossies/VALIDACAO_v0.1.0.md), com
   **sete achados** que nem os 78 testes nem revisão de código tinham
   encontrado — o mesmo que aconteceu no Antitotem, onde a escuta achou um
   bug de sinal real.

   Três foram corrigidos antes de publicar, por decisão do autor: a faixa
   de volume entre seeds (52,7 → 43,6 LU de dispersão), o `.score.txt`
   ilegível (formato 2, com nomes, regulagem e o seed) e o cabo-objeto que
   não se descobria (LEARN + tutorial nos 4 idiomas). Os outros quatro
   estão registrados em `TAREFAS.md` e nenhum bloqueia.

   Um caso não foi executado: o `signal-in` do Estudo 4, que depende de
   hardware de entrada e é opcional no protocolo.
2. ~~**Decisão sobre Windows e macOS.**~~ **DECIDIDO em 21 set. 2026:**
   publicar nessa condição, com verificação só pela CI — mesmo precedente
   do Antitotem. Registrado no `INSTALL.md`; a página editorial dirá o
   mesmo, porque o gate exige que a limitação seja dita e não suposta.
3. ~~**Alvo de publicação.**~~ **DECIDIDO em 21 set. 2026: streaming /
   plataformas.** Daí saíram **−14 LUFS integrado** e teto de **−1 dBTP**
   (`LoudnessMeter::kTargetLufs` / `kTargetDbtp`), a medição de
   **true-peak** que faltava, a leitura de distância até o alvo no cartão
   SOBRE, e a **gravação em PCM 24 bits** — que era o item que dependia
   justamente desta decisão.
4. ~~**Contato oficial**~~ **DECIDIDO em 21 set. 2026:**
   `rasgo.instruments@gmail.com`, o mesmo que os instaladores já
   declaravam. Resta, se aplicável, decidir sobre campanha de apoio.
5. **Mais screenshots**, se quiser mostrar o instrumento em outros
   estados (rack vazio, inspector de cabo aberto, vista SAÍDA). A
   primeira já existe — você a capturou em 18 set. 2026.

---

## Limitações declaradas da v0.1.0

O gate exige que as limitações sejam ditas, não supostas.

### ~~O LEARN segue em português~~ — RESOLVIDO em 27 set. 2026

Esta era a limitação mais séria da v0.1.0, e deixou de existir. O
instrumento está **inteiro** em português, inglês, francês e espanhol:
interface, tutorial, cartões, os 58 verbetes de módulo e os **803 verbetes
de widget** — a explicação de cada knob e cada jack individual, a camada
mais profunda do LEARN.

São 73.000 caracteres de origem, ~220.000 escritos nos três idiomas. O
medidor `rasgo_modular_learn_coverage` reporta 100% nas oito famílias.

**A tradução foi feita à mão, não por máquina**, e é o que a tornou lenta:
o conteúdo descreve comportamento real — a assimetria de vactrol do LPG, o
alcance de captura do PLL, o cruzamento das saídas do RESONATOR ao varrer
TILT. Tradução automática produziria texto que **parece** explicação sem
ser, e num instrumento didático isso é pior que não ter texto.

Nomes de módulo, siglas e rótulos de knob ficam como estão em todas as
línguas (OSC, VCA, 1 V/oct, TORQ, SCR): são o vocabulário do modular em
qualquer idioma, e traduzi-los esconderia o que o painel mostra. Um francês
lendo "TORQ" no texto acha o knob; lendo "COUPLE", não.

**O que impede o trabalho de apodrecer.** O medidor entra no `ctest` como
guarda (`--exigir`): acrescentar um widget passa a exigir seus três idiomas
no mesmo incremento. Sem esse cabo, o próximo módulo novo entraria com o
painel em português e nada avisaria — nem o compilador (o `lookupLearn`
cai no português por projeto), nem a interface (a caixa aparece, só na
língua errada).

### Custo de CPU

~70% de um núcleo num patch comum; ~38% é desenho da interface. Ver
`INSTALL.md`, e a tarefa da v0.1.1 em `TAREFAS.md`.

### Windows e macOS nunca foram abertos

A CI prova que constrói, testa e empacota; não prova que abre e soa numa
máquina real. Decisão do autor: publicar assim, com a limitação dita.

---

## Procedimento de correção e retirada

Exigido pelo gate ("procedimento de correção/retirada conhecido"). Vale a
partir da primeira release pública.

### Princípio

**Nada some sem deixar rastro.** Uma versão retirada continua existindo
no histórico e no arquivo; o que muda é que ela deixa de ser oferecida. É
a mesma postura arquivística do resto do RASGO: corrigir é acrescentar
uma correção, não apagar o erro.

### Correção (o caso comum)

1. Registrar o defeito em `TAREFAS.md` com data, sintoma e como foi
   observado — antes de corrigir, pra o relato não ser reescrito pela
   solução.
2. Corrigir, com teste que falhe sem a correção sempre que o defeito for
   testável.
3. Cortar uma versão nova (patch), com nota do que mudou e de quem é
   afetado.
4. Atualizar a página editorial **junto** com a release — o gate exige
   correspondência entre as duas, e uma página que anuncia uma versão que
   não é a oferecida é pior que página desatualizada.

### Retirada (quando a versão não deve continuar em uso)

Usar quando houver risco a equipamento ou audição, problema de licença ou
crédito, ou defeito que corrompa trabalho do usuário.

1. **Despublicar o artefato**, não o histórico: tirar o instalador da
   release e marcá-la como retirada, com o motivo em uma frase.
2. **Dizer o motivo na página** do instrumento, com data. Sem eufemismo:
   quem baixou precisa saber se foi afetado e como verificar.
3. **Dizer o que fazer** — atualizar, ou como reverter. Se houver risco
   de dano a arquivos do usuário, dizer onde eles ficam
   (`~/.local/share/rasgo-modular/`, `~/Music/RasgoModular/`) e o que
   conferir.
4. **Registrar em `TAREFAS.md` e no `HISTORICO_GLOBAL.md`** da família — a
   retirada é fato histórico do projeto, não incidente a esquecer.
5. Publicar a versão corrigida assim que houver, referenciando a retirada.

### Contatos e responsabilidade

Autor e responsável pela decisão de retirada: **Lúcio de Araújo**.

**Canal oficial de contato: `rasgo.instruments@gmail.com`** (confirmado em
21 set. 2026). É por ele que chegam relatos de defeito, questões de
licença ou crédito, e é o endereço que os instaladores já declaram como
mantenedor — `CPACK_PACKAGE_CONTACT` no `.deb`, no `.dmg` e no `.exe`.
Coincidirem não é detalhe: quem recebe um pacote quebrado procura o
contato que está DENTRO dele, não o da página.

### Retenção

Instaladores retirados saem da distribuição mas permanecem no arquivo
local do autor, com o motivo registrado, pra que uma investigação futura
possa reconstruir o que foi publicado e quando.
