#!/usr/bin/env bash
# Monta o AppImage e o .tar.gz do Rasgo Modular a partir de um build pronto.
#
#   packaging/linux/empacotar.sh <pasta-de-build> <pasta-de-saída>
#
# O .deb continua vindo do CPack; este script cobre as distribuições que não
# instalam .deb (Fedora, Arch, openSUSE...). Os dois partem do MESMO
# `cmake --install` — binário, .desktop e ícones —, então não há uma segunda
# lista de arquivos para manter em dia.
#
# - AppImage: um arquivo só, com as bibliotecas que não são de sistema
#   embutidas pelo linuxdeploy. Baixar, marcar como executável, abrir.
# - .tar.gz: o binário puro, com as bibliotecas do sistema (as mesmas que o
#   .deb pede). Para quem prefere não usar AppImage; traz um `install.sh`
#   que instala em ~/.local, sem root.
#
# Compilado no Ubuntu 22.04 pela CI (glibc 2.35): roda em distribuições de
# 2022 em diante. Decisão registrada em DESENVOLVIMENTO.md ("Planos e
# pedidos registrados", 4 out. 2026).
set -euo pipefail

BUILD=${1:?uso: empacotar.sh <pasta-de-build> <pasta-de-saída>}
SAIDA=${2:?uso: empacotar.sh <pasta-de-build> <pasta-de-saída>}
RAIZ=$(cd "$(dirname "$0")/../.." && pwd)

# linuxdeploy FIXADO numa versão e conferido por hash: é ferramenta de
# terceiro dentro do caminho de publicação, e o "continuous" muda sem aviso.
LINUXDEPLOY_VERSAO=1-alpha-20251107-1
LINUXDEPLOY_SHA256=c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d

VERSAO=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$BUILD/CMakeCache.txt")
[ -n "$VERSAO" ] || { echo "versão não encontrada em $BUILD/CMakeCache.txt" >&2; exit 1; }
ARQ=$(uname -m)
NOME="rasgo-modular-${VERSAO}-linux-${ARQ}"

mkdir -p "$SAIDA"
SAIDA=$(cd "$SAIDA" && pwd)
TRAB=$(mktemp -d)
trap 'rm -rf "$TRAB"' EXIT

# ---- árvore instalada (base dos dois pacotes) ---------------------------
cmake --install "$BUILD" --config Release --prefix "$TRAB/AppDir/usr"
BIN="$TRAB/AppDir/usr/bin/rasgo-modular"
[ -x "$BIN" ] || { echo "binário não instalado em $BIN" >&2; exit 1; }
strip --strip-unneeded "$BIN"

# ---- .tar.gz ------------------------------------------------------------
T="$TRAB/$NOME"
mkdir -p "$T"
cp -a "$TRAB/AppDir/usr/bin" "$TRAB/AppDir/usr/share" "$T/"
cp "$RAIZ/LICENSE" "$T/"
cp "$RAIZ/packaging/linux/install.sh" "$T/"
cp "$RAIZ/packaging/linux/LEIA-ME.txt" "$T/README.txt"
chmod +x "$T/install.sh"
tar -C "$TRAB" --owner=0 --group=0 -czf "$SAIDA/$NOME.tar.gz" "$NOME"

# ---- AppImage -----------------------------------------------------------
LD="$TRAB/linuxdeploy-${ARQ}.AppImage"
if [ -n "${LINUXDEPLOY:-}" ]; then
    cp "$LINUXDEPLOY" "$LD"
else
    curl -fsSL -o "$LD" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VERSAO}/linuxdeploy-${ARQ}.AppImage"
fi
echo "${LINUXDEPLOY_SHA256}  $LD" | sha256sum -c -
chmod +x "$LD"

# Sem FUSE (contêiner, CI) o AppImage do linuxdeploy precisa se extrair.
export APPIMAGE_EXTRACT_AND_RUN=1
export LDAI_OUTPUT="$SAIDA/$NOME.AppImage"
export ARCH="$ARQ"
(cd "$TRAB" && "$LD" --appdir AppDir \
    --executable "$BIN" \
    --desktop-file AppDir/usr/share/applications/rasgo-modular.desktop \
    --icon-file AppDir/usr/share/icons/hicolor/256x256/apps/rasgo-modular.png \
    --output appimage)
chmod +x "$SAIDA/$NOME.AppImage"

ls -l "$SAIDA/$NOME.tar.gz" "$SAIDA/$NOME.AppImage"
