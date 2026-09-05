#!/usr/bin/env bash
# Regenera rasgo_logo_gray.h a partir do SVG da marca RASGO.
# Precisa de inkscape + ImageMagick (`convert`) + python3. O .h é
# COMMITADO — o build do painel não depende destas ferramentas.
#
# O .h traz um MAPA DE COBERTURA em tons de cinza (0 = fundo, 255 = marca
# cheia) COM anti-aliasing — o painel mistura isso do fundo pra a cor de
# acento numa passada, então a marca não fica serrilhada.
set -euo pipefail
cd "$(dirname "$0")"

H=15   # altura em px no cabeçalho (a largura sai da proporção do traço)

inkscape rasgo_logo_2026.svg --export-type=png --export-filename=l.png -h $((H * 3))
convert l.png -background white -flatten -trim +repage \
        -resize x$H -colorspace Gray -depth 8 -negate PGM:- \
| python3 - << 'PY'
import sys
d = sys.stdin.buffer.read()
assert d[:2] == b"P5", "esperava PGM binario (P5)"
i = 2
def tok():
    global i
    while d[i:i+1].isspace(): i += 1
    s = i
    while not d[i:i+1].isspace(): i += 1
    return d[s:i]
w = int(tok()); h = int(tok()); int(tok()); i += 1
px = d[i:i + w * h]
with open("rasgo_logo_gray.h", "w") as f:
    f.write("// gerado por regen_logo.sh -- cobertura (0=fundo, 255=marca cheia)\n")
    f.write("// do SVG da familia RASGO, com anti-aliasing. NAO editar a mao.\n")
    f.write(f"static const int rasgo_logo_w = {w};\n")
    f.write(f"static const int rasgo_logo_h = {h};\n")
    f.write("static const unsigned char rasgo_logo_gray[] = {\n")
    for r in range(h):
        f.write("  " + ",".join(str(b) for b in px[r*w:(r+1)*w]) + ",\n")
    f.write("};\n")
print(f"rasgo_logo_gray.h regenerado ({w}x{h})")
PY

rm -f l.png
