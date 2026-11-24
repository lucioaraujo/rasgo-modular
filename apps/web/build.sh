#!/bin/bash
# Compila o motor para o navegador (WebAssembly autônomo) e junta a página de
# teste em .local-build/web/. Uso, da raiz do projeto:
#   bash apps/web/build.sh
#   python3 -m http.server -d .local-build/web 8765   # e abra http://localhost:8765
# Precisa do Emscripten (third_party/emsdk; ver PROVENANCE-RASGO.md lá).
set -euo pipefail
raiz=$(cd "$(dirname "$0")/../.." && pwd)
emsdk=${EMSDK_DIR:-$raiz/../../third_party/emsdk}
# shellcheck disable=SC1091
source "$emsdk/emsdk_env.sh" >/dev/null 2>&1
saida=$raiz/.local-build/web
mkdir -p "$saida"
nice -n 19 em++ -std=c++17 -O2 -I"$raiz/src" -I"$raiz/apps" -fwasm-exceptions \
    "$raiz/apps/web/rasgo_web.cpp" -sSTANDALONE_WASM --no-entry \
    -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB \
    -sEXPORTED_FUNCTIONS=_rw_iniciar,_rw_seed,_rw_processar,_rw_cabos,_rw_varia \
    -o "$saida/rasgo.wasm"
cp "$raiz/apps/web/index.html" "$raiz/apps/web/rasgo-worklet.js" "$saida/"
ls -l "$saida"
