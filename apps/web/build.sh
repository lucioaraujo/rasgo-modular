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
tmp=$raiz/.local-build/web-obj
mkdir -p "$tmp"
# o motor em -O2; o LEARN (tabelas de texto enormes) sem otimização, senão o
# compilador passa de 15 minutos nele (ver o cabeçalho de rasgo_learn.cpp).
# Sem otimização, a função que monta as tabelas usa muita pilha: daí os 4 MB
# de pilha no link (o padrão do Emscripten é 64 KB, e estourava).
versao=$(git -C "$raiz" describe --tags --always --dirty 2>/dev/null || echo dev)
nice -n 19 em++ -std=c++17 -O2 -I"$raiz/src" -I"$raiz/apps" -fwasm-exceptions \
    -DRASGO_MODULAR_BUILD="\"$versao $(date +%Y-%m-%d)\"" \
    -c "$raiz/apps/web/rasgo_web.cpp" -o "$tmp/rasgo_web.o"
if [ ! -f "$tmp/rasgo_learn.o" ] || [ "$raiz/apps/web/rasgo_learn.cpp" -nt "$tmp/rasgo_learn.o" ] \
   || [ "$raiz/apps/panel/LearnCatalog.hpp" -nt "$tmp/rasgo_learn.o" ]; then
  nice -n 19 em++ -std=c++17 -O0 -I"$raiz/src" -I"$raiz/apps" -fwasm-exceptions \
      -c "$raiz/apps/web/rasgo_learn.cpp" -o "$tmp/rasgo_learn.o"
fi
# o REC: o WAV e a versão de entrega com o código do app (src/saida)
nice -n 19 em++ -std=c++17 -O2 -I"$raiz/src" -I"$raiz/apps" -fwasm-exceptions \
    -c "$raiz/apps/web/rasgo_rec.cpp" -o "$tmp/rasgo_rec.o"
saida_o=()
for f in "$raiz"/src/saida/*.cpp; do
  o="$tmp/saida_$(basename "$f" .cpp).o"
  if [ ! -f "$o" ] || [ "$f" -nt "$o" ]; then
    nice -n 19 em++ -std=c++17 -O2 -I"$raiz/src" -fwasm-exceptions -c "$f" -o "$o"
  fi
  saida_o+=("$o")
done
nice -n 19 em++ -O2 -fwasm-exceptions "$tmp/rasgo_web.o" "$tmp/rasgo_learn.o" "$tmp/rasgo_rec.o" "${saida_o[@]}" -sSTANDALONE_WASM --no-entry \
    -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=4MB \
    -sEXPORTED_FUNCTIONS=_rw_iniciar,_rw_seed,_rw_processar,_rw_cabos,_rw_varia,_rw_descrever,_rw_texto_tamanho,_rw_num_controles,_rw_valores,_rw_cabos_estado,_rw_wordmark,_rw_param,_rw_param_usuario,_rw_ligar,_rw_cortar_entrada,_rw_cortar_saida,_rw_mao,_rw_cabo_relacao,_rw_cabo_condutancia,_rw_cabo_ganho,_rw_cabo_romper,_rw_num_visores,_rw_visor_id,_rw_visor,_rw_nome_escala,_rw_repor,_rw_seed_alto,_rw_seed_baixo,_rw_foto,_rw_pode_desfazer,_rw_desfazer,_rw_mudar,_rw_evoluir,_rw_cruzar,_rw_descabear,_rw_espera,_rw_salvar,_rw_salvo_tamanho,_rw_entrada,_rw_abrir,_rw_vu,_rw_romper_todos,_rw_adicionar,_rw_remover,_rw_ordem,_rw_banco,_rw_escolher_escala,_rw_parametro,_rw_catalogo,_rw_learn_widget,_rw_learn_modulo,_rw_learn_cabo,_rw_learn_cabecalho,_rw_learn_ocioso,_rw_learn_valor,_rw_textos,_rw_lingua,_rw_versao,_rw_lufs,_rw_compacto,_rw_rec_iniciar,_rw_rec_ativo,_rw_rec_seed_alto,_rw_rec_seed_baixo,_rw_rec_montagem,_rw_rec_param,_rw_rec_parar,_rw_rec_midi,_rw_rec_midi_tamanho,_rw_rec_buffer,_rw_rec_wav,_rw_rec_bytes,_rw_rec_entrega,_rw_rec_texto,_rw_rec_liberar,_rw_rec_arquivo,_rw_entrada_necessaria,_rw_audio_buffer,_rw_audio_in,_rw_midi \
    -o "$tmp/rasgo.wasm"
# o motor só vai para a pasta servida pronto: o link escreve um arquivo
# intermediário por minutos, e a página o pegaria pela metade
cp "$tmp/rasgo.wasm" "$saida/rasgo.wasm.novo" && mv "$saida/rasgo.wasm.novo" "$saida/rasgo.wasm"
cp "$raiz/apps/web/index.html" "$raiz/apps/web/rasgo-worklet.js" "$raiz/apps/web/rack.js" "$raiz/apps/web/visores.js" "$raiz/apps/web/textos.js" "$saida/"
ls -l "$saida"
