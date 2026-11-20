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
# saida-ref 1.1 (16/11/2026): o master de entrega da gravação
target_sources(RasgoModularApp PRIVATE
    ../../src/saida/DeliveryMaster.cpp ../../src/saida/OutputStage.cpp
    ../../src/saida/LoudnessMeter.cpp ../../src/saida/TruePeakDetector.cpp
    ../../src/saida/TakeAnalysis.cpp)

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
    juce::juce_osc   # receptor do Maestruc (MaestrucLink.h), 15/11/2026
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
    JUCE_APPLICATION_VERSION_STRING="${RASGO_MODULAR_VERSION_LABEL}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_XINPUT=0)

if (MSVC)
    # A unidade principal também agrega o catálogo inteiro; prevenir C1128.
    target_compile_options(RasgoModularApp PRIVATE /utf-8 /bigobj)
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
    # Ícone do menu e da barra de tarefas: o MONOGRAMA (`mark-source.svg`,
    # o mesmo favicon do site), não o wordmark. Até a v0.1.1 instalava
    # `rasgo_logo_2026.svg` — o wordmark master, PRETO, desenhado pra fundo
    # claro: no painel escuro do Mint quase sumia, e com proporção 4,86:1
    # virava um traço no quadrado do ícone (achado do autor, 2 out. 2026).
    # O PNG de 256 vai junto para menus que não leem SVG.
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/mark-source.svg"
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME rasgo-modular.svg)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon-256.png"
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME rasgo-modular.png)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(TARGETS RasgoModularApp RUNTIME DESTINATION .)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    install(TARGETS RasgoModularApp BUNDLE DESTINATION .)
endif()

# ---- PLUGIN (experimental, 20/11/2026) --------------------------------
# O mesmo instrumento como VST3 e LV2 (o SDK do VST3 que vem no JUCE 9.0.3
# está sob MIT, Steinberg 2025; o LV2 é ISC). DESLIGADO por padrão: o
# aplicativo, os pacotes e a CI não mudam. Ligar com
#   cmake -B build -DRASGO_MODULAR_BUILD_PLUGIN=ON
# Instrumento (IS_SYNTH) com MIDI de entrada e entrada estéreo opcional
# (o SIGNAL-IN recebe o som da faixa). Sem o logotipo "VST" (acordo de
# marca da Steinberg, ainda não pedido). Plano: DESENVOLVIMENTO.md.
option(RASGO_MODULAR_BUILD_PLUGIN "Build the experimental VST3/LV2 plugin" OFF)
if (RASGO_MODULAR_BUILD_PLUGIN)
    set(RASGO_MODULAR_PLUGIN_FORMATS VST3)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        list(APPEND RASGO_MODULAR_PLUGIN_FORMATS LV2)
    endif()
    juce_add_plugin(RasgoModularPlugin
        PRODUCT_NAME "Rasgo Modular"
        COMPANY_NAME "RASGO"
        BUNDLE_ID "org.rasgo.modular.plugin"
        PLUGIN_MANUFACTURER_CODE Rsgo
        PLUGIN_CODE RsMd
        FORMATS ${RASGO_MODULAR_PLUGIN_FORMATS}
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Instrument Synth Generator
        LV2URI "https://lucioaraujo.github.io/rasgo-modular/plugin"
        VERSION "${PROJECT_VERSION}")
    target_sources(RasgoModularPlugin PRIVATE RasgoModularApp.cpp
        ../../src/saida/DeliveryMaster.cpp ../../src/saida/OutputStage.cpp
        ../../src/saida/LoudnessMeter.cpp ../../src/saida/TruePeakDetector.cpp
        ../../src/saida/TakeAnalysis.cpp)
    target_include_directories(RasgoModularPlugin PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../apps")
    target_link_libraries(RasgoModularPlugin PRIVATE
        rasgo_modular_core
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_osc
        juce::juce_recommended_config_flags)
    target_compile_definitions(RasgoModularPlugin PUBLIC
        RASGO_PLUGIN=1
        RASGO_MODULAR_BUILD="${RASGO_BUILD_STAMP}"
        JUCE_APPLICATION_NAME_STRING="Rasgo Modular"
        JUCE_APPLICATION_VERSION_STRING="${RASGO_MODULAR_VERSION_LABEL}"
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_XINPUT=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
    if (MSVC)
        target_compile_options(RasgoModularPlugin PRIVATE /utf-8 /bigobj)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(RasgoModularPlugin PRIVATE -Wall -Wextra
            -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endif()

# Hospedeiro de teste do plugin, sem janela (tools/plugin_check.cpp).
if (RASGO_MODULAR_BUILD_PLUGIN)
    juce_add_console_app(RasgoModularPluginCheck PRODUCT_NAME "rasgo_modular_plugin_check")
    target_sources(RasgoModularPluginCheck PRIVATE ../../tools/plugin_check.cpp)
    target_compile_definitions(RasgoModularPluginCheck PRIVATE
        JUCE_PLUGINHOST_VST3=1 JUCE_PLUGINHOST_LV2=1 JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(RasgoModularPluginCheck PRIVATE
        juce::juce_audio_processors juce::juce_events
        juce::juce_recommended_config_flags)
endif()
