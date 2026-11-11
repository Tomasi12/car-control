cmake_minimum_required(VERSION 3.20)
project(dummy_ecu VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
# compile_commands.json lets analyzers (clang-tidy, SCA) see the real compile flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ecu_core STATIC
    src/sensor.cpp
    src/stats.cpp
    src/controller.cpp
    src_c/crc8.c
)
target_include_directories(ecu_core PUBLIC include)

add_executable(dummy_ecu src/main.cpp)
target_link_libraries(dummy_ecu PRIVATE ecu_core)
