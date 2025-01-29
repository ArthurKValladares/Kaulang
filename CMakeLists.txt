cmake_minimum_required(VERSION 3.25)
project(kaulang)

add_definitions(-DDEBUG=1)
add_definitions(-DRUN_SCRIPT=1)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/tokens.cpp
    src/scanner.cpp
    src/compiler.cpp
    src/parser.cpp
    src/expr.cpp
    src/environment.cpp
    src/resolver.cpp
    src/lib/string.cpp
    src/lib/arena.cpp
    src/lib/map.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)