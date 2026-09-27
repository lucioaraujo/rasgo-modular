#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Confere um lote de tradução contra o português REAL do código.

Existe porque um `bind` digitado errado no JSON entra como verbete órfão:
nunca é consultado, e o medidor continua mostrando a família incompleta
sem dizer qual linha está errada. Aqui o erro aparece com nome e sobrenome.

    ./build/rasgo_modular_learn_coverage --despejar TIME > /tmp/TIME_pt.json
    python3 tools/traducao/conferir.py TIME /tmp/TIME_pt.json
"""
import json, sys, pathlib

familia, dump = sys.argv[1], sys.argv[2]
raiz = pathlib.Path(__file__).parent
pt = {(m, b): t for m, b, t in json.load(open(dump, encoding='utf-8'))}
lote = json.load(open(raiz / (familia + '.json'), encoding='utf-8'))

erros = []
avisos = []
vistos = set()
for i, linha in enumerate(lote):
    if len(linha) != 5:
        erros.append('linha %d: esperava [modulo, bind, en, fr, es]' % i); continue
    mod, bind, *idiomas = linha
    if (mod, bind) not in pt:
        erros.append('%s.%s não existe no código (verbete órfão)' % (mod, bind)); continue
    if (mod, bind) in vistos:
        erros.append('%s.%s repetido no lote' % (mod, bind)); continue
    vistos.add((mod, bind))
    fonte = pt[(mod, bind)]
    for nome, tres in zip(('en', 'fr', 'es'), idiomas):
        if len(tres) != 3:
            erros.append('%s.%s [%s]: esperava 3 níveis' % (mod, bind, nome)); continue
        # todo nível preenchido no pt tem de estar preenchido aqui: é a
        # mesma régua do medidor, aplicada antes de gerar em vez de depois
        for nivel, (p, x) in enumerate(zip(fonte, tres)):
            if p.strip() and not x.strip():
                erros.append('%s.%s [%s]: nível %d vazio e o pt tem texto'
                             % (mod, bind, nome, nivel))
            # Identidade NÃO é erro: "A OR B." é a mesma frase em pt, en,
            # fr e es, e forçar uma diferença só para satisfazer o
            # verificador pioraria o texto. Mas também não é invisível —
            # sai como aviso, para eu confirmar que foi intencional e não
            # uma linha esquecida no copiar-colar.
            if x.strip() and x.strip() == p.strip():
                avisos.append('%s.%s [%s]: nível %d idêntico ao português'
                              % (mod, bind, nome, nivel))

faltando = sorted(set(pt) - vistos)
for mod, bind in faltando:
    erros.append('%s.%s sem tradução no lote' % (mod, bind))

for a in avisos:
    print('  aviso: ' + a)
for e in erros:
    print('  ERRO: ' + e)
print('%s: %d/%d verbetes, %d erro(s), %d aviso(s)'
      % (familia, len(vistos), len(pt), len(erros), len(avisos)))
sys.exit(1 if erros else 0)
