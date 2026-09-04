#!/usr/bin/env bash
# Compila (Release) e abre o painel de teste do RASGO Modular.
#
#   ./.run_rasgo_modular.sh            # build + abre o painel
#   ./.run_rasgo_modular.sh --seed 42  # abre já num patch de cabeamento (seed 42)
#   ./.run_rasgo_modular.sh --tests    # roda os testes e sai
#   ./.run_rasgo_modular.sh --clean    # apaga o build e recompila do zero
#
# Release, não Debug: com o rack cheio (20+ módulos, reverb e modelos
# físicos) o Debug não fecha o tempo real no dispositivo do autor e a
# ALSA dá underrun — mesma contingência já mapeada no ANTITOTEM.
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$project_dir/build"
panel="$build_dir/rasgo_modular_panel"

run_tests=0
seed=""
want_seed=0
for arg in "$@"; do
    if [[ "$want_seed" -eq 1 ]]; then seed="$arg"; want_seed=0; continue; fi
    case "$arg" in
        --tests|-t) run_tests=1 ;;
        --clean|-c) rm -rf "$build_dir" ;;
        --seed|-s) want_seed=1 ;;
        --help|-h)
            sed -n '2,11p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        *) echo "argumento desconhecido: $arg" >&2; exit 2 ;;
    esac
done

cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -j"$(nproc)"

if [[ "$run_tests" -eq 1 ]]; then
    ctest --test-dir "$build_dir" --output-on-failure
    exit $?
fi

if [[ ! -x "$panel" ]]; then
    echo "painel não compilou: $panel" >&2
    exit 1
fi

echo "patches e gravações: ${XDG_DATA_HOME:-$HOME/.local/share}/rasgo-modular/"
if [[ -n "$seed" ]]; then
    echo "abrindo no seed $seed"
    exec env RASGO_SEED="$seed" "$panel"
fi
exec "$panel"
