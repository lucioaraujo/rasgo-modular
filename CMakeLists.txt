cmake_minimum_required(VERSION 3.16)
project(RasgoModularPrototype LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rasgo_modular_core INTERFACE)
target_include_directories(rasgo_modular_core INTERFACE src)

if(BUILD_TESTING)
    add_executable(rasgo_modular_graph_tests tests/test_graph_engine.cpp)
    target_link_libraries(rasgo_modular_graph_tests PRIVATE rasgo_modular_core)
    if(NOT MSVC)
        target_compile_options(rasgo_modular_graph_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME rasgo_modular_graph_tests COMMAND rasgo_modular_graph_tests)
endif()
