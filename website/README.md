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

**Propósito:** apresentar o instrumento a quem nunca o viu, em uma página
só, e ser honesto sobre o que ele ainda não é. Não é documentação de uso
— essa mora no repositório (`guia/`, `INSTALL.md`) e no tutorial embutido
no próprio app.

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

Uma página por idioma, sem subpáginas. Quando houver release, entram:
página de instalação (traduzindo `INSTALL.md`), downloads e obras.

**O que o site NÃO duplica:** matriz de plataformas detalhada, variáveis
de ambiente, créditos completos e procedimento de retirada seguem no
repositório. O site resume e aponta; cópia editada à mão diverge.

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
obra do autor, com seed e versão registrados; downloads só aparecem
quando existir release, apontando pro artefato versionado e nunca pra um
build solto.

## Qualidade e validação

A captura tem **texto alternativo descritivo** nos quatro idiomas — não
"captura de tela do app", mas o que se vê nela: as três fileiras, os
setenta cabos curvos, a paleta à esquerda, a barra de comandos no topo.
Quem usa leitor de tela precisa da imagem, não do rótulo dela.

**Verificado:** estrutura HTML dos quatro idiomas (um `<title>`, uma faixa
do portal, um `<main>`, um `<footer>`, seis seções em cada); seletor de
idioma marcando a página certa em todas; nenhum arquivo referenciado
ausente; nenhum resíduo de português nas traduções.

**Ainda não verificado, e é honesto dizer:** nenhum navegador real abriu
esta página. Não há navegador gráfico neste ambiente, e não abro janela na
máquina do autor. Antes de publicar, é preciso conferir em pelo menos um
navegador de cada motor (Blink, Gecko, WebKit), em desktop e em telefone,
mais uma passada de leitor de tela na ordem de foco.

## Build

Não há build. São arquivos estáticos que funcionam abrindo o
`index.html` — e essa simplicidade é deliberada: um site que precisa de
pipeline para existir é um site que apodrece quando o pipeline quebra.
`dist/` não existe porque não é necessário.
