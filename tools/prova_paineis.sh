#!/bin/bash
# Motor de PROVA para a auditoria: aplica as propostas de painel (patches em
# dossies/propostas_paineis/*.patch) numa cópia da árvore e compila para
# .local-build/web-prova/, servida à parte. A página do autor não muda.
set -euo pipefail
raiz=$(cd "$(dirname "$0")/.." && pwd)
tmp=$raiz/.local-build/prova-src
rm -rf "$tmp"; mkdir -p "$tmp"
cp -r "$raiz/src" "$raiz/apps" "$tmp/"
# argumentos: só esses patches (nomes sem .patch); sem argumentos, todos
if [ $# -gt 0 ]; then lista=(); for n in "$@"; do lista+=("$raiz/dossies/propostas_paineis/$n.patch"); done
else lista=("$raiz"/dossies/propostas_paineis/*.patch); fi
for p in "${lista[@]}"; do (cd "$tmp" && patch -p1 -s < "$p"); done
source "$raiz/../../third_party/emsdk/emsdk_env.sh" >/dev/null 2>&1
obj=$raiz/.local-build/web-obj; out=$raiz/.local-build/web-prova
mkdir -p "$out"
nice -n 19 em++ -std=c++17 -O2 -I"$tmp/src" -I"$tmp/apps" -fwasm-exceptions -c "$tmp/apps/web/rasgo_web.cpp" -o "$obj/prova_web.o"
exp=$(grep -o 'EXPORTED_FUNCTIONS=[^ ]*' "$tmp/apps/web/build.sh")
nice -n 19 em++ -O2 -fwasm-exceptions "$obj/prova_web.o" "$obj/rasgo_learn.o" "$obj/rasgo_rec.o" "$obj"/saida_*.o -sSTANDALONE_WASM --no-entry \
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=4MB -s$exp -o "$out/rasgo.wasm"
cp "$tmp"/apps/web/{index.html,rack.js,visores.js,textos.js,rasgo-worklet.js} "$out/"
echo pronto
