#!/usr/bin/env python3
"""Gera src/escalas/BancoEscalasDados.inc a partir da cópia fixada do banco
de escalas do Trioio (third_party/trioio-escalas/arquivo_trioio.txt).

Formato de saída: uma linha por escala, campos separados por TAB —
id, nome, família, período (cents), graus (cents, separados por espaço),
fonte. O texto vai em pedaços de string crua de até 12 KB (compiladores
limitam o tamanho de um literal). O id é o FNV-1a de 32 bits do nome,
dobrado em 24 bits (cabe exato num float do patch); o gerador falha se dois
nomes colidirem ou se algum id der 0.
Uso: python3 tools/gerar_banco_escalas.py
"""
import pathlib, sys

raiz = pathlib.Path(__file__).resolve().parent.parent
src = raiz / "third_party" / "trioio-escalas" / "arquivo_trioio.txt"
dst = raiz / "src" / "escalas" / "BancoEscalasDados.inc"

def fnv24(s):
    h = 0x811C9DC5
    for b in s.encode("utf-8"):
        h ^= b
        h = (h * 0x01000193) & 0xFFFFFFFF
    return ((h >> 24) ^ h) & 0xFFFFFF

blocos = src.read_text(encoding="utf-8").split("\n= ")[1:]
linhas, ids = [], {}
for b in blocos:
    ls = b.split("\n")
    nome = ls[0].strip()
    d = {}
    for l in ls[1:]:
        if ":" in l:
            k, v = l.split(":", 1)
            d.setdefault(k.strip(), v.strip())
    i = fnv24(nome)
    if i == 0 or i in ids:
        sys.exit(f"id inválido ou colisão: {nome!r} × {ids.get(i)!r}")
    ids[i] = nome
    campos = [str(i), nome, d["família"], d["período"], d["graus"], d.get("fonte", "")]
    for c in campos:
        if "\t" in c or "\n" in c or ")\"" in c:
            sys.exit(f"caractere proibido em {nome!r}")
    linhas.append("\t".join(campos))

texto = "\n".join(linhas) + "\n"
pedacos, atual = [], ""
for l in texto.splitlines(keepends=True):
    if len((atual + l).encode("utf-8")) > 12000:
        pedacos.append(atual); atual = ""
    atual += l
pedacos.append(atual)
with dst.open("w", encoding="utf-8") as f:
    f.write("// GERADO por tools/gerar_banco_escalas.py — não editar à mão.\n")
    f.write("// Fonte: third_party/trioio-escalas/arquivo_trioio.txt (ver PROVENANCE.md).\n")
    f.write(f"// {len(linhas)} escalas, {len(pedacos)} pedaços.\n")
    for p in pedacos:
        f.write('R"BANCO(' + p + ')BANCO",\n')
print(f"{dst.relative_to(raiz)}: {len(linhas)} escalas, {len(pedacos)} pedaços, {dst.stat().st_size} bytes")
