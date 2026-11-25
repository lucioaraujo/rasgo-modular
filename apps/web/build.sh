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
    -sEXPORTED_FUNCTIONS=_rw_iniciar,_rw_seed,_rw_processar,_rw_cabos,_rw_varia,_rw_descrever,_rw_texto_tamanho,_rw_num_controles,_rw_valores,_rw_cabos_estado,_rw_wordmark,_rw_param,_rw_param_usuario,_rw_ligar,_rw_cortar_entrada,_rw_cortar_saida,_rw_mao,_rw_cabo_relacao,_rw_cabo_condutancia,_rw_cabo_ganho,_rw_cabo_romper,_rw_num_visores,_rw_visor_id,_rw_visor,_rw_nome_escala \
    -o "$saida/rasgo.wasm"
cp "$raiz/apps/web/index.html" "$raiz/apps/web/rasgo-worklet.js" "$raiz/apps/web/rack.js" "$raiz/apps/web/visores.js" "$saida/"
ls -l "$saida"
