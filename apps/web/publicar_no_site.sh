#!/bin/bash
# Põe a página do navegador no site (website/play/), a partir do build em
# .local-build/web (bash apps/web/build.sh antes). O site é publicado copiando
# website/ inteira, sem etapa de build: o motor (rasgo.wasm) vai pronto, e
# atualiza-se junto com cada versão (decisão registrada em
# dossies/ESTUDO_outras_plataformas.md §6, F4d).
set -euo pipefail
raiz=$(cd "$(dirname "$0")/../.." && pwd)
de=$raiz/.local-build/web
para=$raiz/website/play
mkdir -p "$para"
for f in index.html rack.js visores.js textos.js rasgo-worklet.js rasgo-rec-worker.js rasgo.wasm; do
  cp "$de/$f" "$para/$f"
done
ls -l "$para"
