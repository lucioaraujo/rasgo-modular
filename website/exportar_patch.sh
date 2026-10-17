#!/usr/bin/env bash
# Linux: painéis reais de um patch, sem áudio/MIDI e sem alterar preferências.
# Uso: bash website/exportar_patch.sh examples/escuta_frase.rmp /pasta/nova LISTEN,PHRASE
set -euo pipefail
project_dir=$(cd "$(dirname "$0")/.." && pwd -P)
patch_file=$(realpath "${1:?informe o patch .rmp}")
output_dir=$(realpath -m "${2:?informe a pasta de saída}")
types=${3:-LISTEN,PHRASE}
extra_env=()
if [[ ${4:-} == --check-guidance ]]; then
    extra_env+=(RASGO_TEST_GUIDANCE=1)
elif [[ ${4:-} == --check-load-swap ]]; then
    extra_env+=(RASGO_TEST_PATCH_SWAP=1)
elif [[ ${4:-} == --check-rack-view ]]; then
    extra_env+=(RASGO_TEST_RACK_VIEW=1)
elif [[ -n ${4:-} ]]; then
    echo "Opção desconhecida: $4" >&2; exit 1
fi
app="$project_dir/build/apps/juce/RasgoModularApp_artefacts/Release/Rasgo Modular"
[[ -f "$patch_file" && -x "$app" ]] || { echo "Patch ou app ausente" >&2; exit 1; }
[[ $(uname -s) == Linux ]] || { echo "Isolamento XDG deste script exige Linux" >&2; exit 1; }
mkdir -p "$output_dir"
capture_state=$(mktemp -d "$output_dir/.capture-state.XXXXXX")
mkdir -p "$capture_state/config" "$capture_state/cache" "$capture_state/tmp"
ulimit -v 5242880
ulimit -t 20
env "${extra_env[@]}" XDG_CONFIG_HOME="$capture_state/config" XDG_CACHE_HOME="$capture_state/cache" \
    TMPDIR="$capture_state/tmp" RASGO_EXPORTAR_PATCH="$patch_file" \
    RASGO_EXPORTAR_CONFIG="$capture_state/config/rasgo-modular" \
    RASGO_EXPORTAR_PAINEIS="$output_dir" RASGO_EXPORTAR_TIPOS="$types" \
    timeout 30 "$app"
# Estado fica junto da exportação para verificar que não abriu áudio nem
# salvou uma sessão. Não apagamos diretórios fornecidos por quem executa.
echo "Painéis: $output_dir"
echo "Diagnóstico isolado: $capture_state"
