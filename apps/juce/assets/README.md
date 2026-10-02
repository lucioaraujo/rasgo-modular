# Ícones do app

Dois arquivos entram no executável pelo `juceaide`
(`ICON_BIG`/`ICON_SMALL` em `../CMakeLists.txt`), e são eles que aparecem
no Explorer/Finder, na barra de tarefas e no atalho que o instalador
cria.

| Arquivo | Conteúdo | Fonte |
|---|---|---|
| `icon-256.png` | monograma (a letra `r` do próprio wordmark) | `mark-source.svg` |
| `icon-32.png` | monograma | `mark-source.svg` |

Regenerar: `inkscape --export-type=png --export-filename=icon-256.png -w 256 -h 256 mark-source.svg`
(e o equivalente 32×32). O `.ico` do instalador Windows
(`../../../packaging/windows/rasgo-modular.ico`) junta 256/64/48/32/16 do
mesmo desenho (`convert` do ImageMagick). No Linux, `mark-source.svg` é
instalado como ícone do menu e da barra (`../CMakeLists.txt`). O site usa
os mesmos arquivos (`website/assets/identity/favicon*`).

## Desde 2 out. 2026: o monograma em todos os tamanhos

O autor não gostou de ver o wordmark no menu e na barra de tarefas. O
ícone grande, que até então era o wordmark inteiro (`icon-source.svg`,
mantido aqui como fonte), passou a ser o monograma também; e o Linux,
que instalava o wordmark master preto em vez de qualquer destes,
passou a instalar o monograma.

## Por que dois desenhos diferentes

O wordmark tem proporção 4,86:1. Num quadrado de 32×32 ele fica com
**menos de 4 px de altura de tinta** — um borrão, não uma marca.
Aumentar a largura não resolve: é geometria, não ajuste. Por isso o
tamanho pequeno usa um monograma extraído do próprio wordmark, sem
inventar desenho novo. É o mesmo caminho do Antitotem, cujo favicon
também é um glifo isolado e não o logotipo inteiro.

## Por que eles foram refeitos (17 set. 2026)

A versão anterior era o wordmark **preto** (`fill:#000000`, herdado do SVG
master, que foi desenhado pra fundo claro) sobre o fundo `#131a1a` do
instrumento: preto sobre quase-preto, praticamente invisível em qualquer
tamanho. Eu tinha corrigido antes a distorção de proporção deste mesmo
ícone e deixei a cor passar — o autor viu e reportou. Agora a marca é
pintada no acento `#ff9d4c`, o mesmo que o app usa no cabeçalho.
