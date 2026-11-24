#!/usr/bin/env python3
"""Escolhe, por módulo, o painel de display mais movimentado entre as
exportações de `exportar_paineis.sh` e grava `assets/modulos/<TIPO>.webp`.

Medida: pixels de traço na faixa do display, fora da linha mais cheia —
que é a linha de repouso. Desde 24/11/2026 o traço tem a cor da família
do módulo; conta-se qualquer pixel claro e saturado (a faixa da família
no topo soma o mesmo a todos os candidatos do módulo). Uso:
    python3 website/escolher_paineis.py /tmp/paineis
"""
import pathlib, sys
from PIL import Image

src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(__file__).parent / "assets" / "modulos"
runs = sorted(p for p in src.iterdir() if p.is_dir())

def movimento(png):
    im = Image.open(png).convert("RGB")
    w, h = im.size
    y0, y1 = int(h * 0.045), int(h * 0.18)      # faixa do display
    linhas = []
    px = im.load()
    for y in range(y0, y1):
        n = 0
        for x in range(w):
            r, g, b = px[x, y]
            if max(r, g, b) > 120 and max(r, g, b) - min(r, g, b) > 60:
                n += 1
        linhas.append(n)
    return sum(linhas) - max(linhas, default=0)

tipos = sorted({p.stem for r in runs for p in r.glob("*.png")})
for t in tipos:
    cands = [r / f"{t}.png" for r in runs if (r / f"{t}.png").exists()]
    melhor = max(cands, key=movimento)
    Image.open(melhor).convert("RGB").save(dst / f"{t}.webp", "WEBP",
                                           quality=82, method=6)
    print(f"{t:12s} {melhor.parent.name}")
