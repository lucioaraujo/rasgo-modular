# Front-end de produção JUCE. Ver o cabeçalho de `RasgoModularApp.cpp` pra
# o porquê de existir ao lado do painel X11 de teste.
#
# ICON_BIG/ICON_SMALL: o juceaide embute um recurso de ícone de verdade no
# executável (.ico no Windows, .icns no macOS). Sem isso o exe não carrega
# ícone nenhum — Explorer/Finder, barra de tarefas e o atalho que o CPack
# cria caem num ícone genérico. Mesmo achado do ANTITOTEM e do Navalha 2
# (26 ago. 2026). Os PNGs são derivados do SVG master do RASGO; são
# opcionais no build pra um checkout limpo sem eles ainda compilar.
set(RASGO_MODULAR_ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon-256.png")
set(RASGO_MODULAR_ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon-32.png")

if (EXISTS "${RASGO_MODULAR_ICON_BIG}" AND EXISTS "${RASGO_MODULAR_ICON_SMALL}")
    juce_add_gui_app(RasgoModularApp
        PRODUCT_NAME "Rasgo Modular"
        COMPANY_NAME "RASGO"
        BUNDLE_ID "org.rasgo.modular"
        ICON_BIG "${RASGO_MODULAR_ICON_BIG}"
        ICON_SMALL "${RASGO_MODULAR_ICON_SMALL}")
else()
    message(WARNING "Ícones não encontrados em apps/juce/assets — o app "
                    "compila, mas sem ícone embutido.")
    juce_add_gui_app(RasgoModularApp
        PRODUCT_NAME "Rasgo Modular"
        COMPANY_NAME "RASGO"
        BUNDLE_ID "org.rasgo.modular")
endif()

target_sources(RasgoModularApp PRIVATE RasgoModularApp.cpp)

# A marca NÃO precisa de BinaryData: vem do mesmo cabeçalho C já commitado
# que o painel X11 usa (`apps/panel/assets/rasgo_logo_gray.h`, mapa de
# cobertura 118×15 gerado por `regen_logo.sh`). Um asset, dois front-ends,
# marca idêntica.

# `apps` no include path: `panel/ModuleCatalog.hpp` e `panel/PatchSeed.hpp`
# são headers SEM dependência de plataforma, compartilhados com o painel
# X11 (o prefixo `panel/` é histórico — são lógica de aplicação, não do
# front-end X11). `rasgo_modular_core` já expõe `src/`.
target_include_directories(RasgoModularApp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../apps")

target_link_libraries(RasgoModularApp PRIVATE
    rasgo_modular_core
    juce::juce_audio_utils
    juce::juce_recommended_config_flags)

# Política de aviso: a DO PROJETO (`-Wall -Wextra`, igual ao alvo do painel
# X11), não a `juce_recommended_warning_flags`. Aquela liga `-Wfloat-equal`
# e `-Wsign-conversion`, que disparam às centenas nos headers de DSP deste
# repositório — comparar um parâmetro com `0.0f` pra saber se está
# desligado é idioma corrente aqui, não bug. Aviso que ninguém pode zerar
# vira aviso que todo mundo ignora; o projeto já escolheu seu nível
# (`-Wall -Wextra -Wpedantic -Werror` nos testes) e este alvo segue ele.
if (NOT MSVC)
    target_compile_options(RasgoModularApp PRIVATE -Wall -Wextra)
endif()

target_compile_definitions(RasgoModularApp PRIVATE
    # mesmo carimbo de build do painel X11 (hash do git + data) — aparece
    # na faixa de crédito e no cartão SOBRE
    RASGO_MODULAR_BUILD="${RASGO_BUILD_STAMP}"
    JUCE_APPLICATION_NAME_STRING="Rasgo Modular"
    JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_XINPUT=0)

if (MSVC)
    target_compile_options(RasgoModularApp PRIVATE /utf-8)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(RasgoModularApp PRIVATE
        -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# Instalação por plataforma (o CPack do CMakeLists raiz empacota isto).
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # binário sem espaços é a convenção Debian; o título da janela continua
    # "Rasgo Modular" (JUCE_APPLICATION_NAME_STRING acima)
    install(PROGRAMS "$<TARGET_FILE:RasgoModularApp>"
        DESTINATION bin RENAME rasgo-modular)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/linux/rasgo-modular.desktop")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/linux/rasgo-modular.desktop"
            DESTINATION share/applications)
    endif()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../panel/assets/rasgo_logo_2026.svg")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../panel/assets/rasgo_logo_2026.svg"
            DESTINATION share/icons/hicolor/scalable/apps
            RENAME rasgo-modular.svg)
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(TARGETS RasgoModularApp RUNTIME DESTINATION .)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    install(TARGETS RasgoModularApp BUNDLE DESTINATION .)
endif()
