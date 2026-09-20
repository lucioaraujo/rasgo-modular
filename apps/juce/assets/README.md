# Ícones do app

Dois arquivos entram no executável pelo `juceaide`
(`ICON_BIG`/`ICON_SMALL` em `../CMakeLists.txt`), e são eles que aparecem
no Explorer/Finder, na barra de tarefas e no atalho que o instalador
cria.

| Arquivo | Conteúdo | Fonte |
|---|---|---|
| `icon-256.png` | wordmark RASGO completo | `icon-source.svg` |
| `icon-32.png` | monograma (a letra `r` do próprio wordmark) | `mark-source.svg` |

Regenerar: `inkscape --export-type=png --export-filename=icon-256.png -w 256 -h 256 icon-source.svg`
(e o equivalente 32×32 a partir de `mark-source.svg`).

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
