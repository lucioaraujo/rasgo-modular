#!/bin/bash
# Exporta os painéis dos 58 módulos com 13 seeds (3 de demonstração + 10
# comuns) para escolher, por módulo, a imagem de display mais movimentada
# (`escolher_paineis.py`). Uso, da raiz do projeto, com o app compilado:
#   website/exportar_paineis.sh /tmp/paineis
# O app toca durante a exportação: rode com a saída de áudio desviada, por
# exemplo para um null-sink do PipeWire (PIPEWIRE_NODE/PULSE_SINK), ou com
# o volume baixo. HOME isolado para não tocar nas preferências de quem usa.
set -euo pipefail
dest=${1:?pasta de destino}
app="build/apps/juce/RasgoModularApp_artefacts/Release/Rasgo Modular"
home=$(mktemp -d)
for s in 4303935450909092226 424242 1234567 1 2 3 4 5 6 7 8 9 10; do
    mkdir -p "$dest/$s"
    HOME=$home RASGO_SEED=$s RASGO_EXPORTAR_PAINEIS="$dest/$s" \
        timeout 60 "$app" >/dev/null 2>&1 || true
    echo "seed $s: $(ls "$dest/$s" | wc -l) painéis"
done
rm -rf "${home:?}"
