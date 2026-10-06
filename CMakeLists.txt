cmake_minimum_required(VERSION 3.22)

# Windows: runtime do Visual C++ ESTÁTICO (/MT), embutido no executável.
# Sem isto o `Rasgo Modular.exe` exige MSVCP140.dll, VCRUNTIME140.dll e
# VCRUNTIME140_1.dll (o "Visual C++ Redistributable"), que o instalador não
# leva: num Windows sem esse pacote o app simplesmente não abre. A CI nunca
# pegou porque o runner do GitHub já tem o pacote instalado. Achado em 6 out.
# 2026, depois de o autor não conseguir abrir a v0.1.3 num Windows; conferido
# listando as DLLs importadas do .exe publicado. Vale para o app e para o
# JUCE (variável global, definida antes de qualquer alvo). A CI agora falha
# se essas DLLs voltarem a aparecer (package.yml).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# PRECISA vir antes do `project()` (exigência do CMake/Apple). Mesmo bug
# que o ANTITOTEM e o Navalha 2 pegaram ao vivo (26 ago. 2026): sem isto o
# .dmg sai com uma fatia só (a arquitetura do runner) e com `minos` = a
# versão do macOS do runner, e recusa abrir em Mac mais antigo.
# x86_64;arm64 dá UM binário Universal 2 em vez de obrigar a escolher
# entre dois downloads.
#
# 10.15, e NÃO 10.13. Corrigido em 21 set. 2026, no primeiro dia em que a
# CI rodou de verdade (até a extração pra repositório próprio o workflow
# estava inerte, porque o Actions só lê `.github/workflows/` da raiz). O
# job do macOS não passou nem da compilação:
#
#   ArchiveStorage.hpp:25: error: 'path' is unavailable:
#   introduced in macOS 10.15
#
# `std::filesystem` só existe na libc++ da Apple a partir do 10.15, e o
# projeto o usa em `src/core/ArchiveStorage.hpp`, `apps/panel/panel_main.cpp`
# e num teste. Com alvo 10.13 este código NUNCA compilou pra macOS — o
# número era aspiracional, e ninguém tinha como descobrir: não há máquina
# Apple neste ambiente e a CI não rodava.
#
# Subir o alvo, e não arrancar o `std::filesystem`: 10.15 é de 2019, a
# fatia arm64 exige 11.0 de qualquer forma, e trocar biblioteca padrão
# testada por manipulação de caminhos à mão seria arriscar defeito real
# para atender um alvo que nunca funcionou.
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build a Universal 2 binary" FORCE)
endif()

# `C` além de `CXX`: o JUCE compila fontes C nos seus módulos.
project(RasgoModularPrototype VERSION 0.1.5 LANGUAGES C CXX)

include(CTest)

# `M_PI` NÃO é padrão C++. Ele vem do POSIX, e o GCC e o Clang o expõem
# por hábito — o MSVC, não, a menos que `_USE_MATH_DEFINES` esteja
# definido ANTES de `<cmath>`. O projeto usa `M_PI` em 83 lugares, de 28
# arquivos (DSP, testes, exemplos, o painel), então **nunca compilou no
# Windows**:
#
#   test_additive.cpp(59): error C2065: 'M_PI': undeclared identifier
#   Loudness.hpp(117):     error C2065: 'M_PI': undeclared identifier
#
# Achado da CI em 21 set. 2026, no primeiro dia em que ela rodou de
# verdade. Uma definição global resolve os 83 usos de uma vez, sem tocar
# em 28 arquivos nem inventar uma constante paralela à da biblioteca
# padrão. É inofensiva fora do MSVC: nos outros compiladores a macro
# simplesmente não muda nada.
add_compile_definitions(_USE_MATH_DEFINES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rasgo_modular_core INTERFACE)
target_include_directories(rasgo_modular_core INTERFACE src)

# camada io/ com dependência de terceiro (dr_wav) — FORA do core.
# Compilada sem -Werror (o header do dr_wav tem warnings próprios).
add_library(rasgo_modular_io STATIC src/io/AudioFile.cpp)
target_include_directories(rasgo_modular_io PUBLIC src PRIVATE third_party)
if(NOT MSVC)
    target_compile_options(rasgo_modular_io PRIVATE -w)
endif()

find_package(Threads REQUIRED)

if(BUILD_TESTING)
    set(rasgo_modular_warnings)
    if(NOT MSVC)
        set(rasgo_modular_warnings -Wall -Wextra -Wpedantic -Werror)
    endif()

    add_executable(rasgo_modular_graph_tests tests/test_graph_engine.cpp)
    target_link_libraries(rasgo_modular_graph_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_graph_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_graph_tests COMMAND rasgo_modular_graph_tests)

    add_executable(rasgo_modular_signal_graph_tests tests/test_signal_graph.cpp)
    target_link_libraries(rasgo_modular_signal_graph_tests
        PRIVATE rasgo_modular_core Threads::Threads)
    target_compile_options(rasgo_modular_signal_graph_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_signal_graph_tests COMMAND rasgo_modular_signal_graph_tests)

    add_executable(rasgo_modular_function_generator_tests tests/test_function_generator.cpp)
    target_link_libraries(rasgo_modular_function_generator_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_function_generator_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_function_generator_tests COMMAND rasgo_modular_function_generator_tests)

    add_executable(rasgo_modular_filter_tests tests/test_filter.cpp)
    target_link_libraries(rasgo_modular_filter_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_filter_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_filter_tests COMMAND rasgo_modular_filter_tests)

    add_executable(rasgo_modular_decision_tests tests/test_decision.cpp)
    target_link_libraries(rasgo_modular_decision_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_decision_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_decision_tests COMMAND rasgo_modular_decision_tests)

    add_executable(rasgo_modular_clock_tests tests/test_clock.cpp)
    target_link_libraries(rasgo_modular_clock_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_clock_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_clock_tests COMMAND rasgo_modular_clock_tests)

    add_executable(rasgo_modular_envelope_tests tests/test_envelope.cpp)
    target_link_libraries(rasgo_modular_envelope_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_envelope_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_envelope_tests COMMAND rasgo_modular_envelope_tests)

    add_executable(rasgo_modular_memory_tests tests/test_memory.cpp)
    target_link_libraries(rasgo_modular_memory_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_memory_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_memory_tests COMMAND rasgo_modular_memory_tests)

    add_executable(rasgo_modular_turing_tests tests/test_turing.cpp)
    target_link_libraries(rasgo_modular_turing_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_turing_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_turing_tests COMMAND rasgo_modular_turing_tests)

    add_executable(rasgo_modular_matter_tests tests/test_matter.cpp)
    target_link_libraries(rasgo_modular_matter_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_matter_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_matter_tests COMMAND rasgo_modular_matter_tests)

    add_executable(rasgo_modular_space_tests tests/test_space.cpp)
    target_link_libraries(rasgo_modular_space_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_space_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_space_tests COMMAND rasgo_modular_space_tests)

    add_executable(rasgo_modular_string_tests tests/test_string.cpp)
    target_link_libraries(rasgo_modular_string_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_string_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_string_tests COMMAND rasgo_modular_string_tests)

    add_executable(rasgo_modular_quantizer_tests tests/test_quantizer.cpp)
    target_link_libraries(rasgo_modular_quantizer_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_quantizer_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_quantizer_tests COMMAND rasgo_modular_quantizer_tests)

    add_executable(rasgo_modular_parametric_tests tests/test_parametric.cpp)
    target_link_libraries(rasgo_modular_parametric_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_parametric_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_parametric_tests COMMAND rasgo_modular_parametric_tests)

    add_executable(rasgo_modular_harmony_tests tests/test_harmony.cpp)
    target_link_libraries(rasgo_modular_harmony_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_harmony_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_harmony_tests COMMAND rasgo_modular_harmony_tests)

    add_executable(rasgo_modular_sequence_tests tests/test_sequence.cpp)
    target_link_libraries(rasgo_modular_sequence_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_sequence_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_sequence_tests COMMAND rasgo_modular_sequence_tests)

    add_executable(rasgo_modular_mix_tests tests/test_mix.cpp)
    target_link_libraries(rasgo_modular_mix_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_mix_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_mix_tests COMMAND rasgo_modular_mix_tests)

    add_executable(rasgo_modular_oscillator_tests tests/test_oscillator.cpp)
    target_link_libraries(rasgo_modular_oscillator_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_oscillator_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_oscillator_tests COMMAND rasgo_modular_oscillator_tests)

    add_executable(rasgo_modular_noise_tests tests/test_noise.cpp)
    target_link_libraries(rasgo_modular_noise_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_noise_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_noise_tests COMMAND rasgo_modular_noise_tests)

    add_executable(rasgo_modular_vca_tests tests/test_vca.cpp)
    target_link_libraries(rasgo_modular_vca_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_vca_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_vca_tests COMMAND rasgo_modular_vca_tests)

    add_executable(rasgo_modular_control_tests tests/test_control.cpp)
    target_link_libraries(rasgo_modular_control_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_control_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_control_tests COMMAND rasgo_modular_control_tests)

    add_executable(rasgo_modular_logic_tests tests/test_logic.cpp)
    target_link_libraries(rasgo_modular_logic_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_logic_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_logic_tests COMMAND rasgo_modular_logic_tests)

    add_executable(rasgo_modular_sample_hold_tests tests/test_sample_hold.cpp)
    target_link_libraries(rasgo_modular_sample_hold_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_sample_hold_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_sample_hold_tests COMMAND rasgo_modular_sample_hold_tests)
    add_executable(rasgo_modular_shape_tests tests/test_shape.cpp)
    target_link_libraries(rasgo_modular_shape_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_shape_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_shape_tests COMMAND rasgo_modular_shape_tests)

    add_executable(rasgo_modular_lpg_tests tests/test_lpg.cpp)
    target_link_libraries(rasgo_modular_lpg_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_lpg_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_lpg_tests COMMAND rasgo_modular_lpg_tests)

    add_executable(rasgo_modular_chord_tests tests/test_chord.cpp)
    target_link_libraries(rasgo_modular_chord_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_chord_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_chord_tests COMMAND rasgo_modular_chord_tests)

    add_executable(rasgo_modular_drift_tests tests/test_drift.cpp)
    target_link_libraries(rasgo_modular_drift_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_drift_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_drift_tests COMMAND rasgo_modular_drift_tests)

    add_executable(rasgo_modular_switch_tests tests/test_switch.cpp)
    target_link_libraries(rasgo_modular_switch_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_switch_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_switch_tests COMMAND rasgo_modular_switch_tests)

    add_executable(rasgo_modular_scope_tests tests/test_scope.cpp)
    target_link_libraries(rasgo_modular_scope_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_scope_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_scope_tests COMMAND rasgo_modular_scope_tests)

    add_executable(rasgo_modular_trigseq_tests tests/test_trigseq.cpp)
    target_link_libraries(rasgo_modular_trigseq_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_trigseq_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_trigseq_tests COMMAND rasgo_modular_trigseq_tests)

    add_executable(rasgo_modular_abacus_tests tests/test_abacus.cpp)
    target_link_libraries(rasgo_modular_abacus_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_abacus_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_abacus_tests COMMAND rasgo_modular_abacus_tests)

    add_executable(rasgo_modular_chaos_tests tests/test_chaos.cpp)
    target_link_libraries(rasgo_modular_chaos_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_chaos_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_chaos_tests COMMAND rasgo_modular_chaos_tests)

    add_executable(rasgo_modular_pll_tests tests/test_pll.cpp)
    target_link_libraries(rasgo_modular_pll_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_pll_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_pll_tests COMMAND rasgo_modular_pll_tests)

    add_executable(rasgo_modular_note_out_tests tests/test_note_out.cpp)
    target_link_libraries(rasgo_modular_note_out_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_note_out_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_note_out_tests COMMAND rasgo_modular_note_out_tests)

    add_executable(rasgo_modular_wasp_tests tests/test_wasp.cpp)
    target_link_libraries(rasgo_modular_wasp_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_wasp_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_wasp_tests COMMAND rasgo_modular_wasp_tests)

    add_executable(rasgo_modular_panel_layout_tests tests/test_panel_layout.cpp)
    target_link_libraries(rasgo_modular_panel_layout_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_panel_layout_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_panel_layout_tests COMMAND rasgo_modular_panel_layout_tests)

    add_executable(rasgo_modular_cable_geometry_tests tests/test_cable_geometry.cpp)
    target_link_libraries(rasgo_modular_cable_geometry_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_cable_geometry_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_cable_geometry_tests COMMAND rasgo_modular_cable_geometry_tests)

    add_executable(rasgo_modular_matrix_tests tests/test_matrix.cpp)
    target_link_libraries(rasgo_modular_matrix_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_matrix_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_matrix_tests COMMAND rasgo_modular_matrix_tests)

    add_executable(rasgo_modular_mult_tests tests/test_mult.cpp)
    target_link_libraries(rasgo_modular_mult_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_mult_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_mult_tests COMMAND rasgo_modular_mult_tests)

    add_executable(rasgo_modular_glide_tests tests/test_glide.cpp)
    target_link_libraries(rasgo_modular_glide_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_glide_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_glide_tests COMMAND rasgo_modular_glide_tests)

    add_executable(rasgo_modular_wavetable_tests tests/test_wavetable.cpp)
    target_link_libraries(rasgo_modular_wavetable_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_wavetable_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_wavetable_tests COMMAND rasgo_modular_wavetable_tests)

    add_executable(rasgo_modular_looper_tests tests/test_looper.cpp)
    target_link_libraries(rasgo_modular_looper_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_looper_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_looper_tests COMMAND rasgo_modular_looper_tests)

    add_executable(rasgo_modular_additive_tests tests/test_additive.cpp)
    target_link_libraries(rasgo_modular_additive_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_additive_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_additive_tests COMMAND rasgo_modular_additive_tests)

    add_executable(rasgo_modular_planar_tests tests/test_planar.cpp)
    target_link_libraries(rasgo_modular_planar_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_planar_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_planar_tests COMMAND rasgo_modular_planar_tests)

    add_executable(rasgo_modular_operator_tests tests/test_operator.cpp)
    target_link_libraries(rasgo_modular_operator_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_operator_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_operator_tests COMMAND rasgo_modular_operator_tests)

    add_executable(rasgo_modular_formant_tests tests/test_formant.cpp)
    target_link_libraries(rasgo_modular_formant_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_formant_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_formant_tests COMMAND rasgo_modular_formant_tests)

    add_executable(rasgo_modular_hall_tests tests/test_hall.cpp)
    target_link_libraries(rasgo_modular_hall_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_hall_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_hall_tests COMMAND rasgo_modular_hall_tests)

    add_executable(rasgo_modular_drum_tests tests/test_drum.cpp)
    target_link_libraries(rasgo_modular_drum_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_drum_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_drum_tests COMMAND rasgo_modular_drum_tests)

    add_executable(rasgo_modular_audio_file_tests tests/test_audio_file.cpp)
    target_link_libraries(rasgo_modular_audio_file_tests PRIVATE rasgo_modular_core rasgo_modular_io)
    target_compile_options(rasgo_modular_audio_file_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_audio_file_tests COMMAND rasgo_modular_audio_file_tests)

    add_executable(rasgo_modular_pitch_shift_tests tests/test_pitch_shift.cpp)
    target_link_libraries(rasgo_modular_pitch_shift_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_pitch_shift_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_pitch_shift_tests COMMAND rasgo_modular_pitch_shift_tests)

    add_executable(rasgo_modular_sampler_tests tests/test_sampler.cpp)
    target_link_libraries(rasgo_modular_sampler_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_sampler_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_sampler_tests COMMAND rasgo_modular_sampler_tests)

    add_executable(rasgo_modular_turntable_tests tests/test_turntable.cpp)
    target_link_libraries(rasgo_modular_turntable_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_turntable_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_turntable_tests COMMAND rasgo_modular_turntable_tests)

    add_executable(rasgo_modular_boxcar_tests tests/test_boxcar.cpp)
    target_link_libraries(rasgo_modular_boxcar_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_boxcar_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_boxcar_tests COMMAND rasgo_modular_boxcar_tests)

    add_executable(rasgo_modular_swirl_tests tests/test_swirl.cpp)
    target_link_libraries(rasgo_modular_swirl_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_swirl_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_swirl_tests COMMAND rasgo_modular_swirl_tests)

    add_executable(rasgo_modular_crush_tests tests/test_crush.cpp)
    target_link_libraries(rasgo_modular_crush_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_crush_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_crush_tests COMMAND rasgo_modular_crush_tests)

    add_executable(rasgo_modular_stages_tests tests/test_stages.cpp)
    target_link_libraries(rasgo_modular_stages_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_stages_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_stages_tests COMMAND rasgo_modular_stages_tests)

    add_executable(rasgo_modular_resonator_tests tests/test_resonator.cpp)
    target_link_libraries(rasgo_modular_resonator_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_resonator_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_resonator_tests COMMAND rasgo_modular_resonator_tests)

    add_executable(rasgo_modular_pulsar_tests tests/test_pulsar.cpp)
    target_link_libraries(rasgo_modular_pulsar_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_pulsar_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_pulsar_tests COMMAND rasgo_modular_pulsar_tests)

    add_executable(rasgo_modular_spectra_tests tests/test_spectra.cpp)
    target_link_libraries(rasgo_modular_spectra_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_spectra_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_spectra_tests COMMAND rasgo_modular_spectra_tests)

    add_executable(rasgo_modular_shifter_tests tests/test_shifter.cpp)
    target_link_libraries(rasgo_modular_shifter_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_shifter_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_shifter_tests COMMAND rasgo_modular_shifter_tests)

    add_executable(rasgo_modular_vca4_tests tests/test_vca4.cpp)
    target_link_libraries(rasgo_modular_vca4_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_vca4_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_vca4_tests COMMAND rasgo_modular_vca4_tests)

    add_executable(rasgo_modular_vocoder_tests tests/test_vocoder.cpp)
    target_link_libraries(rasgo_modular_vocoder_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_vocoder_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_vocoder_tests COMMAND rasgo_modular_vocoder_tests)

    # usa apps/panel/MotionEngine.hpp — include dir extra `apps`, como o
    # painel gráfico e a peça 4.
    add_executable(rasgo_modular_motion_engine_tests tests/test_motion_engine.cpp)
    target_include_directories(rasgo_modular_motion_engine_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_motion_engine_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_motion_engine_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_motion_engine_tests COMMAND rasgo_modular_motion_engine_tests)

    # regressão: o Motion Engine nunca clipa a saída (usa ModuleCatalog +
    # PatchSeed + MotionEngine — mesmo include dir `apps` do painel).
    add_executable(rasgo_modular_motion_no_clip_tests tests/test_motion_no_clip.cpp)
    target_include_directories(rasgo_modular_motion_no_clip_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_motion_no_clip_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_motion_no_clip_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_motion_no_clip_tests COMMAND rasgo_modular_motion_no_clip_tests)

    # usa apps/panel/PatchGenetics.hpp
    add_executable(rasgo_modular_patch_genetics_tests tests/test_patch_genetics.cpp)
    target_include_directories(rasgo_modular_patch_genetics_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_patch_genetics_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_patch_genetics_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_patch_genetics_tests COMMAND rasgo_modular_patch_genetics_tests)

    # usa apps/panel/ScoreRecorder.hpp (não depende do core; include dir
    # `apps` mantido pra consistência com os outros protótipos)
    add_executable(rasgo_modular_score_recorder_tests tests/test_score_recorder.cpp)
    target_include_directories(rasgo_modular_score_recorder_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_score_recorder_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_score_recorder_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_score_recorder_tests COMMAND rasgo_modular_score_recorder_tests)

    add_executable(rasgo_modular_seed_patch_tests tests/test_seed_patch.cpp)
    target_include_directories(rasgo_modular_seed_patch_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_seed_patch_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_seed_patch_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_seed_patch_tests COMMAND rasgo_modular_seed_patch_tests)

    # usa apps/panel/LearnCatalog.hpp
    add_executable(rasgo_modular_learn_catalog_tests tests/test_learn_catalog.cpp)
    target_include_directories(rasgo_modular_learn_catalog_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_learn_catalog_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_learn_catalog_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_learn_catalog_tests COMMAND rasgo_modular_learn_catalog_tests)

    # usa apps/panel/StartupPolicy.hpp (como o app abre: preferência ABRE)
    add_executable(rasgo_modular_startup_policy_tests tests/test_startup_policy.cpp)
    target_include_directories(rasgo_modular_startup_policy_tests PRIVATE src apps)
    target_link_libraries(rasgo_modular_startup_policy_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_startup_policy_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_startup_policy_tests COMMAND rasgo_modular_startup_policy_tests)

    # usa apps/panel/UiLanguage.hpp
    # Guarda de FIAÇÃO: o catálogo traduzido só serve se os front-ends
    # passarem o idioma na consulta. Em 27 set. 2026 eles não passavam, e
    # nada avisou — compilava, o ctest passava e o medidor dizia 100%,
    # porque o parâmetro tem padrão `Lang::pt`. Ver o cabeçalho do teste.
    add_executable(rasgo_modular_learn_idioma_tests
                   tests/test_learn_idioma_ligado.cpp)
    target_compile_definitions(rasgo_modular_learn_idioma_tests PRIVATE
                   RASGO_MODULAR_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(rasgo_modular_learn_idioma_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_learn_idioma_tests
             COMMAND rasgo_modular_learn_idioma_tests)

    add_executable(rasgo_modular_ui_language_tests tests/test_ui_language.cpp)
    target_include_directories(rasgo_modular_ui_language_tests PRIVATE src apps)
    target_compile_options(rasgo_modular_ui_language_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_ui_language_tests COMMAND rasgo_modular_ui_language_tests)

    # Auditoria dos atalhos de teclado: a tabela de src/ui/Shortcuts.hpp
    # contra o cartão TECLADO de apps/panel/UiLanguage.hpp. Sem janela e
    # sem JUCE — por isso roda no ctest, que é onde precisava estar: o
    # teclado quebrou três vezes e nenhuma quebra tinha teste.
    add_executable(rasgo_modular_shortcuts_tests tests/test_shortcuts.cpp)
    target_include_directories(rasgo_modular_shortcuts_tests PRIVATE src apps)
    target_compile_options(rasgo_modular_shortcuts_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_shortcuts_tests COMMAND rasgo_modular_shortcuts_tests)

    # Equilíbrio de volume entre seeds. Achado da sessão de ESCUTA, não
    # dos testes — por isso ganhou teste depois, e o caso que mais importa
    # é o determinismo: o seed é a identidade do patch.
    add_executable(rasgo_modular_seed_balance_tests tests/test_seed_balance.cpp)
    target_include_directories(rasgo_modular_seed_balance_tests PRIVATE src apps)
    target_compile_options(rasgo_modular_seed_balance_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_seed_balance_tests COMMAND rasgo_modular_seed_balance_tests)

    # FERRAMENTA de auditoria dos módulos — não é teste de regressão, e
    # por isso NÃO entra no ctest. Ela acusa 32 itens dos quais 13 já se
    # explicam como comportamento correto; um teste que falha sempre
    # ensina a ignorar falhas. Ver `dossies/AUDITORIA_MODULOS.md`.
    add_executable(rasgo_modular_module_audit tests/tool_module_audit.cpp)
    target_include_directories(rasgo_modular_module_audit PRIVATE src apps)
    target_compile_options(rasgo_modular_module_audit PRIVATE ${rasgo_modular_warnings})

    # Exame caso a caso dos itens que a triagem deixa em aberto, com
    # excitação própria de cada módulo. Também ferramenta, não teste.
    add_executable(rasgo_modular_param_exam tests/tool_param_exam.cpp)
    target_include_directories(rasgo_modular_param_exam PRIVATE src apps)
    target_compile_options(rasgo_modular_param_exam PRIVATE ${rasgo_modular_warnings})

    # Medidor de cobertura da tradução do LEARN. Ferramenta, não teste:
    # enquanto os lotes avançam ele reporta progresso parcial, e um teste
    # que falha por estar "em andamento" ensina a ignorar falhas.
    add_executable(rasgo_modular_learn_coverage tests/tool_learn_coverage.cpp)
    target_include_directories(rasgo_modular_learn_coverage PRIVATE src apps)
    target_compile_options(rasgo_modular_learn_coverage PRIVATE ${rasgo_modular_warnings})
    # Entra no ctest como GUARDA: `--exigir` falha se algum verbete de widget
    # perder a tradução. Sem cabo no ctest, os 803 verbetes apodreciam em
    # silêncio no primeiro módulo novo. Sem o argumento continua sendo o
    # medidor legível de sempre (`./build/rasgo_modular_learn_coverage`).
    add_test(NAME rasgo_modular_learn_coverage_tests
             COMMAND rasgo_modular_learn_coverage --exigir)

    add_executable(rasgo_modular_port_exam tests/tool_port_exam.cpp)
    target_include_directories(rasgo_modular_port_exam PRIVATE src apps)
    target_compile_options(rasgo_modular_port_exam PRIVATE ${rasgo_modular_warnings})

    add_executable(rasgo_modular_wav_writer_tests tests/test_wav_writer.cpp)
    target_link_libraries(rasgo_modular_wav_writer_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_wav_writer_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_wav_writer_tests COMMAND rasgo_modular_wav_writer_tests)

    add_executable(rasgo_modular_true_peak_tests tests/test_true_peak.cpp)
    target_link_libraries(rasgo_modular_true_peak_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_true_peak_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_true_peak_tests COMMAND rasgo_modular_true_peak_tests)

    add_executable(rasgo_modular_output_stage_tests tests/test_output_stage.cpp)
    target_link_libraries(rasgo_modular_output_stage_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_output_stage_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_output_stage_tests COMMAND rasgo_modular_output_stage_tests)

    add_executable(rasgo_modular_signal_in_tests tests/test_signal_in.cpp)
    target_link_libraries(rasgo_modular_signal_in_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_signal_in_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_signal_in_tests COMMAND rasgo_modular_signal_in_tests)

    # guarda de segurança do sink: precisa de `apps/` no include path
    # (o `SinkOut.hpp` é header de aplicação, framework-free)
    add_executable(rasgo_modular_sink_guard_tests tests/test_sink_guard.cpp)
    target_include_directories(rasgo_modular_sink_guard_tests PRIVATE apps)
    target_link_libraries(rasgo_modular_sink_guard_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_sink_guard_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_sink_guard_tests COMMAND rasgo_modular_sink_guard_tests)

    # bateria de casos-limite da saída (SAIDA_AUDIO_COMUM.md §5)
    add_executable(rasgo_modular_output_excellence_tests
        tests/test_output_excellence.cpp)
    target_include_directories(rasgo_modular_output_excellence_tests PRIVATE apps)
    target_link_libraries(rasgo_modular_output_excellence_tests
        PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_output_excellence_tests
        PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_output_excellence_tests
             COMMAND rasgo_modular_output_excellence_tests)

    # medição BS.1770/EBU R128 — fixtures de conformidade
    add_executable(rasgo_modular_loudness_tests tests/test_loudness.cpp)
    target_link_libraries(rasgo_modular_loudness_tests PRIVATE rasgo_modular_core)
    target_compile_options(rasgo_modular_loudness_tests PRIVATE ${rasgo_modular_warnings})
    add_test(NAME rasgo_modular_loudness_tests COMMAND rasgo_modular_loudness_tests)
endif()

add_executable(rasgo_modular_primeiro_fragmento examples/primeiro_fragmento.cpp)
target_link_libraries(rasgo_modular_primeiro_fragmento PRIVATE rasgo_modular_core)
if(NOT MSVC)
    target_compile_options(rasgo_modular_primeiro_fragmento PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(rasgo_modular_peca_generativa examples/peca_generativa.cpp)
target_link_libraries(rasgo_modular_peca_generativa PRIVATE rasgo_modular_core)
if(NOT MSVC)
    target_compile_options(rasgo_modular_peca_generativa PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(rasgo_modular_peca_generativa_2 examples/peca_generativa_2.cpp)
target_link_libraries(rasgo_modular_peca_generativa_2 PRIVATE rasgo_modular_core)
if(NOT MSVC)
    target_compile_options(rasgo_modular_peca_generativa_2 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(rasgo_modular_peca_generativa_3 examples/peca_generativa_3.cpp)
target_link_libraries(rasgo_modular_peca_generativa_3 PRIVATE rasgo_modular_core)
if(NOT MSVC)
    target_compile_options(rasgo_modular_peca_generativa_3 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# usa apps/panel/MotionEngine.hpp (protótipo do Motion Engine, fora do
# core) — por isso o include dir extra `apps`, como o painel gráfico.
add_executable(rasgo_modular_peca_generativa_4 examples/peca_generativa_4.cpp)
target_include_directories(rasgo_modular_peca_generativa_4 PRIVATE src apps)
target_link_libraries(rasgo_modular_peca_generativa_4 PRIVATE rasgo_modular_core)
if(NOT MSVC)
    target_compile_options(rasgo_modular_peca_generativa_4 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# ---------------------------------------------------------------------------
# Front-end de TESTE: painel gráfico (X11 + ALSA). Não faz parte do core.
# Só compila em Linux com libX11 e libasound disponíveis.
# ---------------------------------------------------------------------------
find_library(RASGO_X11 X11)
find_library(RASGO_ASOUND asound)
find_library(RASGO_XRANDR Xrandr)
if(RASGO_X11 AND RASGO_ASOUND AND RASGO_XRANDR)
    # carimbo de build pro "SOBRE" — hash curto do git + data (fallback: só a data)
    execute_process(
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse --short HEAD
        OUTPUT_VARIABLE RASGO_GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    # Reconfigurar quando o hash muda. Observar só `.git/HEAD` não basta:
    # ele muda ao trocar de BRANCH, não a cada commit — o carimbo ficava
    # parado num hash antigo (a tela mostrava 49eb4ad com o main em 1754f72,
    # 1 out. 2026), e não havia como saber pela janela qual binário estava
    # aberto. Observa também o arquivo do branch atual e o packed-refs, por
    # caminho ABSOLUTO (`--git-path` devolve relativo, e `if(EXISTS)` com
    # caminho relativo não é confiável).
    execute_process(
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse --absolute-git-dir
        OUTPUT_VARIABLE RASGO_GIT_DIR OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} symbolic-ref -q HEAD
        OUTPUT_VARIABLE RASGO_GIT_REF OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(RASGO_GIT_DIR)
        foreach(rasgo_git_file IN ITEMS HEAD packed-refs ${RASGO_GIT_REF})
            if(EXISTS "${RASGO_GIT_DIR}/${rasgo_git_file}")
                set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                    "${RASGO_GIT_DIR}/${rasgo_git_file}")
            endif()
        endforeach()
    endif()
    string(TIMESTAMP RASGO_BUILD_DATE "%Y")
    if(RASGO_GIT_HASH)
        set(RASGO_BUILD_STAMP "v${PROJECT_VERSION} · ${RASGO_GIT_HASH} · ${RASGO_BUILD_DATE}")
    else()
        set(RASGO_BUILD_STAMP "v${PROJECT_VERSION} · ${RASGO_BUILD_DATE}")
    endif()
    add_executable(rasgo_modular_panel apps/panel/panel_main.cpp)
    target_compile_definitions(rasgo_modular_panel
        PRIVATE RASGO_MODULAR_BUILD="${RASGO_BUILD_STAMP}")
    target_include_directories(rasgo_modular_panel PRIVATE src apps)
    target_link_libraries(rasgo_modular_panel
        PRIVATE rasgo_modular_core Threads::Threads
                ${RASGO_X11} ${RASGO_XRANDR} ${RASGO_ASOUND})
    if(NOT MSVC)
        target_compile_options(rasgo_modular_panel PRIVATE -Wall -Wextra)
    endif()
    message(STATUS "rasgo_modular_panel: painel gráfico de teste habilitado")
else()
    message(STATUS "rasgo_modular_panel: pulado (falta libX11 ou libasound)")
endif()

# ---- front-end de produção (JUCE), multiplataforma ---------------------
# O JUCE NÃO é versionado aqui (nem submodule, nem FetchContent) — é um
# checkout local que o desenvolvedor/CI aponta, igual ao ANTITOTEM, ao
# Navalha 2 e ao Rasgo Synth Performance. Localmente:
#   cmake -B build -DRASGO_MODULAR_JUCE_PATH=/caminho/para/JUCE
option(RASGO_MODULAR_BUILD_JUCE_APP "Build the JUCE production front-end" ON)
set(RASGO_MODULAR_JUCE_PATH "" CACHE PATH "Path to a JUCE source checkout")

if(RASGO_MODULAR_BUILD_JUCE_APP)
    if(RASGO_MODULAR_JUCE_PATH AND EXISTS "${RASGO_MODULAR_JUCE_PATH}/CMakeLists.txt")
        add_subdirectory("${RASGO_MODULAR_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE"
                         EXCLUDE_FROM_ALL)
        add_subdirectory(apps/juce)
        message(STATUS "rasgo_modular_juce: front-end JUCE habilitado")
    else()
        message(STATUS "rasgo_modular_juce: pulado — defina "
                       "-DRASGO_MODULAR_JUCE_PATH=/caminho/para/JUCE")
    endif()
endif()

# ---- empacotamento (CPack) ---------------------------------------------
# Só faz sentido quando o app JUCE existe: é ele o artefato publicável (o
# painel X11 é ferramenta de teste, não vai em release).
if(RASGO_MODULAR_BUILD_JUCE_APP AND TARGET RasgoModularApp)
    set(CPACK_PACKAGE_NAME "rasgo-modular")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR "RASGO")
    set(CPACK_PACKAGE_CONTACT "Lúcio Araújo <rasgo.instruments@gmail.com>")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Rasgo Modular — instrumento modular generativo")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    # Nome de arquivo DETERMINÍSTICO e que diz a arquitetura.
    #
    # O padrão do CPack daria `rasgo-modular-0.1.0-Linux.deb`,
    # `-Windows.exe` e `-Darwin.dmg`: previsível, mas silencioso sobre a
    # arquitetura — e é justamente o que alguém precisa saber antes de
    # baixar. Pior: o site publica esses nomes como link direto de
    # download, então eles passam a ser CONTRATO. Um nome que mude entre
    # versões do CPack quebraria a página sem avisar.
    #
    # `universal` no macOS não é enfeite: o alvo fixa
    # `CMAKE_OSX_ARCHITECTURES` com as duas arquiteturas e a CI verifica
    # que o binário é Universal 2 de verdade. O nome diz o que foi
    # verificado.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(LENGTH CMAKE_OSX_ARCHITECTURES _rasgo_n_arch)
        if(_rasgo_n_arch GREATER 1)
            set(_rasgo_plat "macos-universal")
        elseif(CMAKE_OSX_ARCHITECTURES)
            set(_rasgo_plat "macos-${CMAKE_OSX_ARCHITECTURES}")
        else()
            set(_rasgo_plat "macos-${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_rasgo_plat "windows-x64")
        else()
            set(_rasgo_plat "windows-x86")
        endif()
    else()
        string(TOLOWER "${CMAKE_SYSTEM_NAME}" _rasgo_os)
        set(_rasgo_plat "${_rasgo_os}-${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    set(CPACK_PACKAGE_FILE_NAME
        "rasgo-modular-${PROJECT_VERSION}-${_rasgo_plat}")

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lúcio Araújo <rasgo.instruments@gmail.com>")
        set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CPACK_GENERATOR "NSIS")
        set(CPACK_NSIS_PACKAGE_NAME "Rasgo Modular")
        set(CPACK_NSIS_DISPLAY_NAME "Rasgo Modular")
        set(CPACK_NSIS_MODIFY_PATH OFF)
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        # SEM isto o template NSIS do CPack registra a desinstalação mas
        # não cria atalho NENHUM no Menu Iniciar — bug real, achado e
        # corrigido igual no ANTITOTEM e no Navalha 2 no mesmo dia.
        set(CPACK_PACKAGE_EXECUTABLES "Rasgo Modular" "Rasgo Modular")
        # O .exe fica na RAIZ da pasta instalada (DESTINATION .), mas o CPack
        # cria os atalhos do Menu Iniciar e da área de trabalho (e o
        # "Executar" da última tela) apontando para $INSTDIR\bin\ — o
        # padrão dele quando esta variável falta. Resultado, visto num
        # Windows 10 real em 7 out. 2026: o programa abria pela pasta e
        # NÃO abria pelo menu. A CI agora confere o alvo de cada atalho.
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
        set(CPACK_CREATE_DESKTOP_LINKS "Rasgo Modular")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "Rasgo Modular.exe")
        # última tela do instalador: "Executar o Rasgo Modular" já marcado
        set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Rasgo Modular.exe")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/rasgo-modular.ico")
            set(CPACK_NSIS_MUI_ICON
                "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/rasgo-modular.ico")
            set(CPACK_NSIS_MUI_UNIICON
                "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/rasgo-modular.ico")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_DMG_VOLUME_NAME "Rasgo Modular")
    endif()
    include(CPack)
endif()
