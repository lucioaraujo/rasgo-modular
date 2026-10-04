#!/bin/sh
# Instala o Rasgo Modular na sua pasta pessoal (~/.local), sem root.
#   ./install.sh            instala (ou atualiza)
#   ./install.sh --remove   remove o que este script instalou
# Outro destino: PREFIX=/opt/rasgo ./install.sh
set -eu
AQUI=$(cd "$(dirname "$0")" && pwd)
PREFIX=${PREFIX:-"$HOME/.local"}
APPS="$PREFIX/share/applications"
ICONES="$PREFIX/share/icons/hicolor"

if [ "${1:-}" = "--remove" ]; then
    rm -f "$PREFIX/bin/rasgo-modular" \
          "$APPS/rasgo-modular.desktop" \
          "$ICONES/scalable/apps/rasgo-modular.svg" \
          "$ICONES/256x256/apps/rasgo-modular.png"
    echo "Rasgo Modular removido de / removed from $PREFIX."
    echo "Preferências e patches / settings and patches: ~/.config/rasgo-modular"
    exit 0
fi

mkdir -p "$PREFIX/bin" "$APPS" "$ICONES/scalable/apps" "$ICONES/256x256/apps"
cp "$AQUI/bin/rasgo-modular" "$PREFIX/bin/"
cp "$AQUI/share/icons/hicolor/scalable/apps/rasgo-modular.svg" "$ICONES/scalable/apps/"
cp "$AQUI/share/icons/hicolor/256x256/apps/rasgo-modular.png" "$ICONES/256x256/apps/"
# caminho absoluto no Exec: ~/.local/bin nem sempre está no PATH do menu
sed "s|^Exec=.*|Exec=\"$PREFIX/bin/rasgo-modular\"|" \
    "$AQUI/share/applications/rasgo-modular.desktop" > "$APPS/rasgo-modular.desktop"
command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database "$APPS" 2>/dev/null || true
echo "Rasgo Modular instalado em / installed to $PREFIX/bin/rasgo-modular"
echo "(no menu de aplicativos / in the application menu)"
