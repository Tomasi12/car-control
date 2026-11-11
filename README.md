# car-control — dummy C/C++ project for testing the SCA VS Code extension

A small, compiling C++17 (+ one C11 file) "ECU" program: a wheel-speed sensor,
statistics helpers, a controller and a CRC-8. It contains **intentional defects**,
each marked with a `DEFECT` comment, so we can see whether SCA finds them in a real project.

## Build
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Run
```
Uses CMake if installed (and writes `build/compile_commands.json` for analyzers);
otherwise compiles with `g++` directly. `-Clean` rebuilds from scratch.

## Layout
```
CMakeLists.txt       ecu_core library + dummy_ecu executable
include/             headers (.hpp, and crc8.h for C)
src/                 C++ sources
src_c/crc8.c         C source (tests mixed C/C++)
build.ps1            build script (uses third_party/ toolchain)
third_party/         local GCC/Clang/CMake/clang-tidy/cppcheck (see third_party/README.md)
```

## Intentional defects (expected findings)
| File | Function | Defect |
|---|---|---|
| src/sensor.cpp | `WheelSensor::sample` | out-of-bounds (`<=` instead of `<`) |
| src/sensor.cpp | `WheelSensor::speed_kmh` | division by zero (`count_`, `ticks_per_rev`) |
| src/stats.cpp | `average` | signed overflow; division by zero on empty vector |
| src/stats.cpp | `scale` | possible overflow in `value * numerator` |
| src/stats.cpp | `to_percent` | uninitialized read of `pct`; narrowing to `uint8_t` |
| src/controller.cpp | `Controller::torque_request` | null dereference of `cfg_` |
| src/controller.cpp | `Controller::mode_limit` | array index not range-checked |
| src_c/crc8.c | `frame_checksum_ok` | `len == 0` reads `frame[-1]` |
| src/main.cpp | `main` | calls the bad paths inside a never-taken `if` |

The program runs cleanly with its normal input; the defects only show on bad input.
