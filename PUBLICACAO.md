# Rasgo Modular — prontidão para publicação

Auditoria contra `RASGO_DOCUMENTATION/ESTRATEGIA_DE_PUBLICACAO.md`: as
cinco camadas e o gate editorial comum. Cada item está marcado com o que
**foi feito**, não com o que deveria estar pronto.

**Data:** 21 set. 2026 · **Camada atual: 1 (candidato publicável), quase
fechada.** O que falta pra camada 2 depende de escuta humana, não de
código.

---

## Camadas

| Camada | Critério | Estado |
|---|---|---|
| 0. Pesquisa local | estado documentado; nada apresentado como release | ✅ `RASGO_MODULAR.md`, `TAREFAS.md`, `dossies/`, `PESQUISA_MODULOS.md` |
| 1. Candidato publicável | build/execução multiplataforma, licença, créditos, documentação e limitações revisados | 🟡 **quase** — ver abaixo |
| 2. Release do instrumento | pacote por plataforma, testes relevantes, **evidência de validação** | ❌ falta a sessão de escuta documentada |
| 3. Página editorial | texto, autoria, imagens, links e estado correspondem à release | 🟡 site **existe** em `website/` (4 idiomas, captura de execução real), em preparação — não publica antes do instrumento |
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
| testes automatizados e validações humanas **realmente executados** | 🟡 77 testes automatizados ✅ · validação humana ❌ |
| screenshots com origem autorizada | ✅ uma, de execução real (18 set. 2026) — original em `screenshots/`, derivados no site |
| links corretos para repositório, documentação e release | ❌ dependem da extração e da release |
| ausência de áudio privado, recordings, testes | ✅ verificado: nenhum áudio rastreado pelo git; os 19 MB de renders de referência passaram a ser ignorados explicitamente |
| contato oficial | ❌ decisão do autor |
| correspondência entre versão publicada e página editorial | ❌ depende da página |
| **procedimento de correção/retirada conhecido** | ✅ ver abaixo |

---

## O que falta, separado por quem resolve

### Depende de mim (código/documento)

- **extrair o projeto pra repositório próprio, preservando o histórico**
  (decidido em 21 set. 2026) — é o que torna a CI de três sistemas viva
  (hoje ela está inerte: o Actions só lê o `.github/workflows/` da raiz do
  repositório). Preservar o histórico é a postura arquivística do resto do
  RASGO: o rastro de como o instrumento chegou aqui é parte do acervo;
- ~~`CHANGELOG.md` / notas da `v0.1.0`~~ **feito** — [`CHANGELOG.md`](CHANGELOG.md),
  escrito e marcado como *preparada, ainda não cortada*, com as limitações
  declaradas (Windows/macOS só na CI, CI inerte até a extração). Falta só
  criar a tag quando a Parte A fechar;
- ~~site do instrumento~~ **feito** (`website/`, 4 idiomas, faixa
  `.rasgo-strip`, DejaVu auto-hospedada, captura de execução real). Falta
  só abri-lo num navegador de cada motor antes de publicar;
- entrada do Modular no portal da família — mas o portal é território do
  Codex, então é combinação, não tarefa minha.

### Depende de você

1. **Sessão de escuta documentada.** É o bloqueio duro da camada 2, e
   nenhum front-end resolve: alguém tem que ouvir e registrar o que
   ouviu. O Antitotem fechou quatro estudos antes de publicar; o Modular
   não tem nenhum. **O protocolo está pronto** em
   [`dossies/VALIDACAO_v0.1.0.md`](dossies/VALIDACAO_v0.1.0.md): quatro
   estudos (Semente, Deriva, Cabo, Matéria e espaço), com o que escutar em
   cada um e espaço para os achados. Falta executá-lo.
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
4. **Contato oficial** e, se aplicável, campanha de apoio.
5. **Mais screenshots**, se quiser mostrar o instrumento em outros
   estados (rack vazio, inspector de cabo aberto, vista SAÍDA). A
   primeira já existe — você a capturou em 18 set. 2026.

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

Autor e responsável pela decisão de retirada: **Lúcio de Araújo**. O
canal oficial de contato entra aqui quando for definido (item pendente do
gate).

### Retenção

Instaladores retirados saem da distribuição mas permanecem no arquivo
local do autor, com o motivo registrado, pra que uma investigação futura
possa reconstruir o que foi publicado e quando.
