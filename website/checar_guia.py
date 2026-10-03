#!/usr/bin/env python3
# Confere os verbetes do guia (guia/<idioma>/*.md) contra a ficha técnica do
# motor: todo controle listado é um rótulo do painel do módulo, e toda
# palavra em maiúsculas é um módulo, um rótulo ou um termo conhecido.
#   ../build/rasgo_modular_learn_coverage --referencia > /tmp/ref.txt
#   python3 checar_guia.py /tmp/ref.txt pt      (rodar da raiz do projeto)
import re, sys, pathlib
ref=open(sys.argv[1],encoding='utf-8').read(); lang=sys.argv[2] if len(sys.argv)>2 else 'pt'
rot={}
for m in re.finditer(r'^## (\S+)  \(.*?\)\n(.*?)(?=^## |\Z)', ref, re.S|re.M):
    rot[m.group(1)]=set(re.findall(r'\[([^\]]+)\]', m.group(2)))
probs=0; n=0
for f in sorted(pathlib.Path('website/guia/'+lang).glob('*.md')):
    t=f.stem; n+=1; txt=f.read_text(encoding='utf-8')
    ctrl=re.search(r'^## Controles\n(.*?)(?=^## |\Z)', txt, re.S|re.M).group(1)
    for nome in re.findall(r'^- ([^:]+):', ctrl, re.M):
        partes=[]
        for p in nome.split(','):
            p=p.strip(); m=re.fullmatch(r'([A-Z]+)(\d+) \S+ \1(\d+)', p)  # "P1 a P8"
            partes+= [f'{m.group(1)}{i}' for i in range(int(m.group(2)),int(m.group(3))+1)] if m else [p]
        for parte in partes:
            if parte not in rot.get(t,set()):
                print(f'{t}: controle "{parte}" não existe no painel ({sorted(rot.get(t,set()))})'); probs+=1
print(f'{n} verbetes conferidos, {probs} problema(s)')
# --- segunda verificação: palavras em maiúsculas ---
todos=set().union(*rot.values())|set(rot.keys())
conhecidas={'MIDI','LFO','ADSR','RASGO','VARIA','SEED','DESCABEIA','ESPERA','REC','TODOS','SAÍDA','RACK','LEARN','CV','USB','DAW','AD','AR','VCO','VCA','PWM','DX','TB-303','DJ'}
probs2=0
for f in sorted(pathlib.Path('website/guia/'+lang).glob('*.md')):
    txt=re.sub(r'^## .*$','',f.read_text(encoding='utf-8'),flags=re.M)
    for w in sorted(set(re.findall(r'\b[A-Z][A-Z0-9&#/-]{1,}\b', txt))):
        if w not in todos and w not in conhecidas:
            print(f'{f.stem}: palavra "{w}" não é módulo nem rótulo conhecido'); probs2+=1
print(f'maiúsculas: {probs2} suspeita(s)')
