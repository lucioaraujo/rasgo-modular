# Rasgo Modular — website

Definição visual e editorial do site do instrumento, conforme
`RASGO_DOCUMENTATION/design/WEBSITES.md §5` (que lista o que cada site
deve documentar).

> **Estado: em preparação, não publicado.** A regra editorial da família é
> explícita: *"o site de um instrumento só será publicado junto com a
> publicação do próprio instrumento"*. O Rasgo Modular ainda não tem
> release — ver [`../PUBLICACAO.md`](../PUBLICACAO.md). Este site existe
> pronto para o dia em que tiver.

## Propósito, públicos, idiomas

**Propósito:** apresentar o instrumento a quem nunca o viu e ser honesto
sobre o que ele ainda não é — uma página de apresentação por idioma, mais
uma de guia dos módulos. Não é documentação de uso: instalação e variáveis
seguem no repositório (`INSTALL.md`), e o guia do site é o mesmo material
que o LEARN mostra dentro do app, reunido para leitura corrida.

**Públicos:** músicos e pessoas interessadas em síntese modular e em
sistemas generativos; secundariamente, quem chega pelo portal da família.

**Idiomas:** português (canônico), inglês, francês e espanhol — os mesmos
quatro do instrumento, pela mesma razão: o app já fala os quatro, e um
site em menos idiomas que o produto seria uma regressão.

## Mapa de páginas e fonte canônica

| Página | Conteúdo | Fonte canônica |
|---|---|---|
| `index.html` (pt) | apresentação completa | esta página |
| `en.html` · `fr.html` · `es.html` | traduções integrais | `index.html` |
| `modulos.html` · `modulos-en.html` · `modulos-fr.html` · `modulos-es.html` | guia dos 58 módulos, família por família | **o catálogo do app** — ver abaixo |

Duas páginas por idioma. A segunda segue o precedente do Antitotem, que já
tem `install.html` por idioma: `.page-nav` no cabeçalho ao lado do seletor
de idioma, com `aria-current="page"` na página aberta.

Quando houver release, entram: página de instalação (traduzindo
`INSTALL.md`), downloads e obras.

**O que o site NÃO duplica:** matriz de plataformas detalhada, variáveis
de ambiente, créditos completos e procedimento de retirada seguem no
repositório. O site resume e aponta; cópia editada à mão diverge.

## O guia dos módulos é GERADO, e por quê

As quatro páginas de guia não são escritas aqui. Os 58 verbetes de módulo
já existem em `apps/panel/LearnCatalog.hpp`, nos quatro idiomas, cobertos
por teste — são o mesmo texto que a caixa LEARN mostra quando o mouse passa
sobre o corpo de um módulo dentro do instrumento.

Escrever uma segunda cópia no site criaria duas versões do mesmo texto, e
duas versões divergem na primeira correção — sendo que a que o público lê
seria justamente a que nenhum teste cobre. Então:

```sh
../build/rasgo_modular_learn_coverage --despejar-modulos > modulos.json
python3 gerar_modulos.py
```

O primeiro comando despeja o catálogo do BINÁRIO (não de uma leitura do
código-fonte): tipos, famílias, ordem e os quatro idiomas vêm de onde o
instrumento os lê. O segundo escreve as quatro páginas. O wordmark é lido
de `index.html` em vez de copiado para o script, pelo mesmo motivo.

`modulos.json` fica versionado de propósito: com ele, o site se regenera
sem compilar o instrumento.

Isto também cumpre uma promessa que o app já fazia. O tutorial embutido
diz, nos quatro idiomas, que *"um guia escrito mais completo e receitas de
patch estão a caminho, como site e PDF"*. O guia é essa primeira metade.
Quando o site for publicado, esse texto do tutorial precisa passar de "a
caminho" para um apontamento.

## Tipografia

**DejaVu Sans** e **DejaVu Sans Mono**, licença livre (derivadas da
Bitstream Vera), **auto-hospedadas** em `assets/fonts/` via `@font-face`.

Nunca uma família nomeada do sistema. A razão é um bug real, encontrado ao
vivo nos sites do Antitotem e do Navalha 2 em 26 ago. 2026: nomear
"DejaVu Sans" sem embutir fazia a página cair num fallback genérico
diferente conforme o sistema do visitante — comum no Linux, rara no
Windows e no macOS. Os arquivos aqui são os mesmos do site do Antitotem.

Escala: corpo 16px/1.65; títulos em `clamp()` responsivo; largura de
leitura limitada a 46rem. Monoespaçada reservada a marca, navegação,
rótulos de tabela e trechos de código — ela sinaliza "isto é do sistema",
não é decoração.

## Cores e tokens

A paleta é **extraída do app real** (`apps/juce/RasgoModularApp.cpp`,
struct `Tokens` — os mesmos valores do painel X11). O site herda a cor do
instrumento, não o contrário: se os tokens do app mudarem, `styles.css`
é que segue.

| Token | Valor | Papel |
|---|---|---|
| `--ground` | `#131a1a` | fundo da página |
| `--surface` | `#262b36` | seções destacadas |
| `--recessed` | `#0e1015` | cartões, código, faixa do portal |
| `--line` | `#485060` | bordas e divisões |
| `--ink` | `#e0e4ec` | texto |
| `--muted` | `#8890a0` | texto secundário |
| `--accent` | `#ff9d4c` | acento único (links, foco, estado) |
| `--warning` | `#ff6b5b` | aviso — usado com parcimônia |

Não compartilha a paleta do portal geral nem a âmbar/madeira do
Antitotem: cada instrumento mantém identidade visual própria
(`WEBSITES.md §3`).

**Contraste**, medido (WCAG 2, mínimo 4,5:1 para texto normal):

| Par | Razão |
|---|---|
| `--ink` sobre `--ground` | 13,84:1 |
| `--ink` sobre `--surface` | 11,12:1 |
| `--accent` sobre `--recessed` | 9,22:1 |
| `--accent` sobre `--ground` | 8,55:1 |
| `--warning` sobre `--recessed` | 6,80:1 |
| `--muted` sobre `--recessed` | 5,93:1 |
| `--muted` sobre `--ground` | 5,50:1 |

Pares acrescentados pelo guia dos módulos, medidos do mesmo jeito:

| Par | Razão | Onde |
|---|---|---|
| `--accent` sobre `--surface` | 6,87:1 | nome do módulo em seção escura |
| `--cable-ctrl` sobre `--ground` | 7,24:1 | rótulo do nível (RÁPIDO / COMO FUNCIONA / EXPERIMENTE) |
| `--cable-ctrl` sobre `--surface` | 5,82:1 | o mesmo rótulo em seção escura |
| `--recessed` sobre `--accent` | 9,22:1 | texto do botão primário |

O rótulo do nível usa `--cable-ctrl` — o azul frio que no rack marca
*controle*, não áudio. Não é decoração: o rótulo é metadado sobre o texto,
da mesma natureza que um cabo de controle é sobre o sinal, e reusar a cor
que o instrumento já deu a esse papel mantém uma gramática só entre app e
site.

Uma combinação **reprovava**: `--muted` sobre `--surface`, 4,42:1. Hoje ela
não ocorre — o texto secundário vive em cartões e notas, que têm fundo
`--recessed` —, mas bastaria mover uma tabela ou uma nota para uma seção
escura e o contraste cairia em silêncio. `.section-dark` passou a
redefinir `--muted` no próprio escopo (`#8a92a2`, 4,53:1, mesmo matiz):
qualquer componente que use o token ali dentro recebe a variante
acessível automaticamente. Fechar a porta é melhor que lembrar de não
entrar nela.

## Grid, breakpoints, movimento

Coluna única, `max-width: 62rem`, respiro lateral por
`clamp(1rem, 4vw, 3rem)`. Cartões em `auto-fit`/`minmax(15rem, 1fr)`:
reflui sozinho, sem breakpoint por dispositivo. Um único breakpoint em
40rem, só para a navegação.

Tabelas ficam dentro de `.wrap` com `overflow-x: auto` — a página nunca
rola na horizontal.

`prefers-reduced-motion: reduce` desliga rolagem suave, animação e
transição. A única "animação" do site é o `scroll-behavior`, então
respeitar a preferência é barato e não há desculpa para não fazê-lo.

## Componentes e navegação por teclado

- **faixa do portal** (`.rasgo-strip`) — persistente e fixa no topo, acima
  do cabeçalho do instrumento. Regra da família (27 ago. 2026): o
  cabeçalho do portal nunca deixa de existir. Deliberadamente neutra, para
  ler como camada compartilhada e não como segunda marca competindo;
- **cabeçalho do instrumento** — marca, e o seletor de idioma com
  `aria-current` marcando a página atual;
- **`.skip-link`** — primeiro elemento focável, some visualmente até
  receber foco;
- **`:focus-visible`** — anel de 2px no acento, com deslocamento, em todo
  elemento interativo. Nada de `outline: none`;
- **contato** (`.contact-row` + `.contact-dialog`) — padrão da família,
  igual ao do Antitotem e ao do portal: **não há `href="mailto:"` em lugar
  nenhum**. O endereço aparece como texto do botão e `assets/contact.js` o
  remonta a partir de códigos de caractere, para o diálogo e para a área de
  transferência.

  O que isso evita é o link `mailto:` — que coletor ingênuo segue, e que
  exige um cliente de e-mail configurado no visitante. **Não** esconde o
  endereço de quem lê o HTML, e não é para isso que serve; dizer o
  contrário seria vender uma proteção que não existe. O `<dialog>` nativo
  traz foco preso e fechamento por Esc de graça;
- **estados vazios/erro:** o site é estático e sem formulário, então não
  há nenhum. Quando entrar a página de downloads, ela precisa de um.

## Imagem, áudio, vídeo, download

**Uma captura de tela**, do instrumento em execução real (18 set. 2026).
Nada de áudio ou vídeo ainda; downloads, nenhum.

**Onde mora o quê.** O PNG ORIGINAL (1920×1006, 832 KB) fica em
`../screenshots/`, no instrumento. O site guarda só derivados otimizados
em `assets/images/`: WebP em 1000 e 1600 px de largura, com JPEG de
reserva. É a regra do `WEBSITES.md §7` — "o website guarda derivados
otimizados; logos master, screenshots originais e masters de áudio
permanecem no instrumento ou acervo de origem" —, e ela existe pra o site
não virar o arquivo de ninguém.

Peso: ~1 MB no total, dos quais 240 KB são a captura grande, servida só a
quem tem tela larga (`srcset`/`sizes`). Sem a imagem o site tem 192 KB.

A compressão foi conferida, não presumida: a captura tem setenta cabos
finos coloridos sobre fundo escuro, que é justamente onde compressão com
perda costuma borrar. Comparei um recorte denso do original com o WebP a
88 de qualidade — cabos nítidos, texto legível, sem artefato visível.

Política para o que ainda não existe: áudio de demonstração precisa ser
obra do autor, com seed e versão registrados. **Downloads** só aparecem
quando existir release, apontando pro artefato versionado e nunca pra um
build solto — a versão pós-release de `estado.py` aponta exatamente para
os anexos da release da tag, e `verificar.py` confere que os nomes batem
com o empacotamento.

## Qualidade e validação

A captura tem **texto alternativo descritivo** nos quatro idiomas — não
"captura de tela do app", mas o que se vê nela: as três fileiras, os
setenta cabos curvos, a paleta à esquerda, a barra de comandos no topo.
Quem usa leitor de tela precisa da imagem, não do rótulo dela.

**Verificado** nas **oito** páginas, por script: marcação bem-formada
(nenhuma tag fechada fora de ordem, nenhuma aberta sem fechar); todo
`href`/`src` interno aponta para arquivo que existe; **nenhum recurso
externo** além dos links para o portal e para o GitHub — nada de CDN, nada
de fonte remota; seletor de idioma e `.page-nav` marcando a página certa em
cada uma; os 58 módulos presentes nas quatro páginas de guia; nenhum
resíduo de português nas traduções.

**Ainda não verificado, e é honesto dizer:** nenhum navegador real abriu
esta página. Não há navegador gráfico neste ambiente, e não abro janela na
máquina do autor. Antes de publicar, é preciso conferir em pelo menos um
navegador de cada motor (Blink, Gecko, WebKit), em desktop e em telefone,
mais uma passada de leitor de tela na ordem de foco.

## Os dois estados da página: antes e depois da release

A seção de estado e a pílula do topo existem em **duas versões**, e
`estado.py` é o dono das duas:

```sh
python3 estado.py --antes          # sem release — o estado de hoje
python3 estado.py --depois 0.1.0   # com release — downloads por plataforma
```

São duas regiões × quatro idiomas = **oito trechos que precisam mudar
juntos**. No dia de publicar, editar isso à mão é como se erra: uma página
fica anunciando "ainda não há release" com o resto do site já no ar, e
ninguém nota porque ninguém relê as quatro línguas. Com o script, o estado
é um comando e não uma lembrança.

Fora das regiões marcadas (`<!-- PILULA:… -->`, `<!-- ESTADO:… -->`) o
script não toca em nada.

### Os downloads, e por que os nomes são contrato

A versão pós-release publica **link direto para cada instalador**:

| Sistema | Arquivo |
|---|---|
| Linux | `rasgo-modular-<versão>-linux-x86_64.deb` |
| Windows | `rasgo-modular-<versão>-windows-x64.exe` |
| macOS | `rasgo-modular-<versão>-macos-universal.dmg` |

Esses nomes vêm do `CPACK_PACKAGE_FILE_NAME` do `CMakeLists.txt`, que
passou a ser **explícito** por causa desta página: o padrão do CPack daria
`rasgo-modular-0.1.0-Linux.deb`, que não diz a arquitetura e poderia mudar
entre versões do CPack, quebrando três botões sem avisar.

Duas coisas foram consertadas no instrumento para que esses links possam
existir:

- o `CMakeLists.txt` fixa o nome do pacote, com arquitetura;
- o workflow ganhou um job **`release`**: até 28 set. 2026 a CI só fazia
  `upload-artifact`, que exige login no GitHub, vem zipado por cima do
  instalador e expira em 90 dias. Não era download público — criar a tag
  não produziria nada instalável. Agora a tag cria a release e anexa os
  três pacotes.

A versão pós-release também diz, no próprio bloco de download, que Windows
e macOS nunca foram abertos pelo autor e que o `.dmg` tem assinatura
ad-hoc. O gate editorial exige que a limitação seja dita onde a decisão é
tomada — e a decisão de baixar é tomada ali.

## Verificação

```sh
python3 verificar.py
```

Confere as oito páginas (marcação bem-formada, todo `href`/`src` interno
existindo, nenhum recurso externo além do portal e do GitHub) **e** os
nomes dos downloads contra o empacotamento: a versão do `project()` do
`CMakeLists.txt`, os moldes do `estado.py` e — quando há `build/` — o nome
que o CPack gerou de fato.

Esta segunda parte é a que justifica o arquivo. Uma versão que sobe para
0.2.0 sem que o site saiba deixaria os três botões apontando para o vazio,
e isso não se descobre lendo o HTML: só clicando, depois de publicado.
Verificado que o guarda acusa — subi a versão de propósito e ele reclamou
nas quatro páginas.

## Build

Não há build para servir: são arquivos estáticos que funcionam abrindo o
`index.html`, e essa simplicidade é deliberada — um site que precisa de
pipeline para existir é um site que apodrece quando o pipeline quebra.
`dist/` não existe porque não é necessário.

Há **dois** scripts, e nenhum é obrigatório para servir o site (as páginas
ficam versionadas e prontas): `gerar_modulos.py`, quando um verbete de
módulo muda no instrumento, e `estado.py`, no dia de publicar.

## Metadados de busca e compartilhamento — `seo.py`

Desde 29 set. 2026. Em cada página, um bloco entre `<!-- SEO:INICIO -->` e
`<!-- SEO:FIM -->` antes de `</head>`: canonical, hreflang entre os idiomas,
Open Graph/Twitter (prévia ao compartilhar) e JSON-LD schema.org (o que o site
é, para buscadores e IAs). Título e descrição são lidos da própria página.
Gera também `sitemap.xml`. **Não editar o bloco à mão:** mudar a
configuração no topo de `seo.py` e rodar `python3 seo.py`
(`--verificar` só confere). A imagem de compartilhamento é `assets/images/og-rasgo-modular.jpg`,
1200×630, derivada de `../screenshots/rack-completo-2026-09-18.png`. O mesmo `seo.py` existe nos sites de toda a família
RASGO; só a configuração muda.

Ordem ao mexer nas páginas: `gerar_modulos.py` → `estado.py` → `seo.py`. O
`gerar_modulos.py` reescreve as páginas de módulos inteiras e apaga o bloco;
`verificar.py` acusa isso, e acusa também `VERSAO` do `seo.py` diferente do
CMake.

## Imagens dos painéis no guia de módulos

Desde 3 out. 2026 (pedido do autor), cada módulo do guia mostra o próprio
painel, em `assets/modulos/<TIPO>.webp`. As imagens são **desenhadas pelo
app**, não recortadas de capturas de tela:

```sh
RASGO_SEED=4303935450909092226 RASGO_EXPORTAR_PAINEIS=/tmp/paineis \
  "build/apps/juce/RasgoModularApp_artefacts/Release/Rasgo Modular"
```

Nesse modo o app liga uma fonte de teste em toda entrada livre (áudio ←
serra do OSC, disparo/gate ← CLK do CLOCK a 140 BPM, controle ← BI do
FUNCTION a 3 Hz), faz o motor calcular todos os módulos, espera 4 s para os
displays encherem, desenha cada painel em 2× e fecha. Os displays mostram,
portanto, o módulo processando sinal de verdade. Para o site foram feitas
três exportações assim (seeds `4303935450909092226`, `424242`, `1234567`) e
dez com seeds comuns, e para cada módulo ficou a imagem de display mais
movimentado, medido pelos pixels de traço fora da linha de repouso. Os de
lógica e decisão (QUANTIZER, LOGIC, TURING…) mostram estado, não onda, e
ficam quietos — é o painel deles. WebP 82, 58 imagens, ~500 KB.
