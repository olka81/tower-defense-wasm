cmake_minimum_required(VERSION 3.20)
project(tower_defense)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT EMSCRIPTEN)
    message(FATAL_ERROR
        "This project must be built with Emscripten.\n"
        "Run:  emcmake cmake -S . -B build && cmake --build build")
endif()

# ---- Sources ---------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/map.cpp
    src/enemy.cpp
    src/tower.cpp
)

add_executable(tower_defense ${SOURCES})

# ---- Output ----------------------------------------------------------------
# Produce tower_defense.js + tower_defense.wasm in web/
set_target_properties(tower_defense PROPERTIES
    SUFFIX ".js"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
)

# ---- Emscripten link flags -------------------------------------------------
target_link_options(tower_defense PRIVATE
    "--bind"                          # emscripten/bind.h bindings
    "-sALLOW_MEMORY_GROWTH=1"
    "-sINITIAL_MEMORY=33554432"       # 32 MB
    "-sMODULARIZE=1"                  # wraps output in a factory function
    "-sEXPORT_NAME=TowerDefense"      # Module factory name used in index.html
    "-sENVIRONMENT=web"
)
