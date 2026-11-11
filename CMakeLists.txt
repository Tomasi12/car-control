cmake_minimum_required(VERSION 3.20)
project(car_control VERSION 0.2.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# compile_commands.json lets analyzers (clang-tidy, cppcheck, SCA) see the real compile flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ECU code under test (would run on the microcontroller)
add_library(ecu STATIC
  src/adc.c
  src/battery.c
  src/can.c
  src/crc.c
  src/diag.c
  src/filter.c
  src/motor_ctrl.c
  src/nvm.c
  src/pid.c
  src/ring_buffer.c
  src/scheduler.c
  src/util.c
  src/watchdog.c
  src/wheel_speed.c
)
target_include_directories(ecu PUBLIC include)
target_compile_options(ecu PRIVATE -Wall -Wextra)

# PC simulation of the hardware + main loop
add_executable(dummy_ecu src/main.c src/hw_sim.c)
target_link_libraries(dummy_ecu PRIVATE ecu)
target_compile_options(dummy_ecu PRIVATE -Wall -Wextra)
