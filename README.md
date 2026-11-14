# car-control — embedded C test project for the SCA VS Code extension

A small **embedded-style ECU** for an electric drive, written in **C99**, that also runs on the PC.
It is the test field for [SCA](https://github.com/Tomasi12/sca): the code contains **planted defects**,
**rule violations** and **safe traps** (correct code that weak tools wrongly flag), each marked in the source.

Embedded style: no heap, fixed-width types, memory-mapped registers (`volatile`), a 1 ms timer ISR,
a cooperative scheduler, CAN messages with CRC, NVM calibration, diagnostics (DTCs) and a watchdog.
On the PC the hardware is replaced by a small plant model (`src/hw_sim.c`).

## Build and run
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Run
```
- Uses the toolchain in `third_party/` (see [third_party/README.md](third_party/README.md)), CMake + Ninja.
- Writes `build/compile_commands.json` for analyzers.
- `-Clean` rebuilds from scratch.

The normal run is a 12 s drive cycle (pedal 0 → 40 → 70 → 95 → 30 %) and prints one status line per second:
```
 3000 ms | pedal  40 % | speed  49.8 km/h (true  50.1) | duty  822 | RUN    |  78 C | 15540 mV | SOC  83 % | DTC 0
```

### Trigger a planted defect at run time
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Fault 2
```
or `build\dummy_ecu.exe --fault N` (`--list` shows all):

| N | What happens | Category | Result on PC |
|---|---|---|---|
| 1 | corrupt NVM → `wheel_init(NULL)` | null_deref | crash (access violation) |
| 2 | erased calibration, `ticks_per_rev = 0` | div_zero | crash (divide by zero) |
| 3 | task with period 0 | div_zero | crash (divide by zero) |
| 4 | CAN frame with DLC 15 | out_of_bounds | silent memory corruption |
| 5 | freeze frame returns local address | stack_escape | crash |
| 6 | `abs_s32(INT32_MIN)` | overflow | wrong value |
| 7 | battery voltage above table | out_of_bounds | SOC 119 % |
| 8 | `adc_raw(9)` | out_of_bounds | reads past registers |
| 9 | more than 16 DTCs | out_of_bounds + null_deref | crash |
| 10 | `rate_limit(-30000, 30000)` | overflow | wrong value |
| 11 | `can_get_signal(length 32)` | shift | wrong value |
| 12 | `rb_raw(16)` | out_of_bounds | reads next field |
| 13 | `battery_cell_mv(cells 0)` | div_zero | crash |
| 14 | `pid_update(dt 0)` | div_zero | crash |
| 15 | `lowpass_update(shift 40)` | shift | wrong value |

## Planted findings (ground truth)
Markers sit on the exact line of the issue:
```c
return (u16)(sum / n); /* DEFECT(div_zero): n == 0 */
```
| Marker | Meaning | Count |
|---|---|---|
| `DEFECT(category)` | run-time error — an analyzer **should** report it | 33 |
| `RULE(category)` | coding-rule violation (MISRA-style) — **should** be reported | 8 |
| `SAFE(category)` | correct code — reporting it is a **false positive** | 20 |

Regenerate the full list (file, line, kind, category) into `expected_findings.csv`:
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\expected_findings.ps1
```

DEFECT categories: out_of_bounds 9, div_zero 6, overflow 5, null_deref 2, shift 2, uninit_var 1,
conversion 1, uint_wrap 1, wrong_sizeof 1, stack_escape 1, dead_code 1, missing_break 1,
non_terminating_loop 1, data_race 1.
RULE categories: switch_default, goto, recursion, float_equality, unchecked_return, missing_volatile.

### Baseline: free tools (2026-11-11, default-ish settings)
| Tool | DEFECT found (of 33) | False positives on SAFE |
|---|---|---|
| gcc 14.2 `-Wall -Wextra` | 3 (missing_break, wrong_sizeof, stack_escape) | 0 |
| clang-tidy 19.1 (`clang-analyzer-*`, `bugprone-*`) | 3 (wrong_sizeof, stack_escape, uninit_var) | 0 |
| cppcheck 2.15 (`warning,style`) | 6 (wrong_sizeof, stack_escape, uninit_var, dead_code, 2× out_of_bounds) | 0 |

Most defects depend on **unchecked inputs** (parameters, bus data, calibration). The free tools only report
errors they can prove on a concrete path; SCA's goal is to also report the *possible* ones.

## Layout
```
include/            module headers (std_types.h, hw_regs.h = register map)
src/                ECU modules
  adc.c             ADC sampling + history
  battery.c         state of charge from cell voltage
  can.c             CAN receive/send, signal unpacking, dispatch table
  crc.c             CRC-8 (J1850) and CRC-16 (CCITT)
  diag.c            DTC storage, debounce, freeze frame
  filter.c          moving average, low-pass, median, rate limiter
  motor_ctrl.c      motor state machine, derating, PWM
  nvm.c             simulated EEPROM, calibration block with CRC
  pid.c             fixed-point PID
  ring_buffer.c     lock-free style byte FIFO
  scheduler.c       1 ms tick ISR + cooperative task scheduler
  util.c            saturation, abs, table interpolation, sqrt, parsing
  watchdog.c        watchdog kick
  wheel_speed.c     encoder ticks -> wheel and vehicle speed
  hw_sim.c          PC-only plant model (not ECU code)
  main.c            init, task setup, drive cycle, fault injection
tools/              expected_findings.ps1 (ground truth extractor)
third_party/        local GCC/Clang/CMake/clang-tidy/cppcheck (not in Git)
build.ps1           build script
```
