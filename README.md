# car-control — embedded C test project for the SCA VS Code extension

A small **embedded-style ECU** for an electric drive, written in **C99**, that also runs on the PC.
It is the test field for [SCA](https://github.com/Tomasi12/sca): the code contains **planted defects**,
**rule violations** and **safe traps** (correct code that weak tools wrongly flag), each marked in the source.

Embedded style: fixed-width types, memory-mapped registers (`volatile`), a 1 ms timer ISR,
a cooperative scheduler, CAN messages with CRC, NVM calibration, diagnostics (DTCs) and a watchdog.
The control path has no heap; only the service-session snapshot buffer (`src/diag_buffer.c`) uses `malloc`.
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
| `DEFECT(category)` | run-time error — an analyzer **should** report it | 60 |
| `RULE(category)` | coding-rule violation (MISRA-style) — **should** be reported | 22 |
| `SAFE(category)` | correct code — reporting it is a **false positive** | 45 |

Regenerate the full list (file, line, kind, category) into `expected_findings.csv`:
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\expected_findings.ps1
```

### Categories
Numbers are DEFECT / RULE / SAFE markers per category.

**Run-time errors (DEFECT)**

| Group | Category | D | S | Example |
|---|---|---|---|---|
| Arithmetic | `div_zero` | 6 | 3 | integer and float division by an unchecked value |
| | `overflow` | 5 | 1 | signed overflow (abs of the minimum, unbounded integral) |
| | `uint_wrap` | 1 | 3 | unsigned counter wraps to 0 |
| | `shift` | 2 | – | shift count ≥ width |
| | `conversion` | 1 | 1 | negative value to unsigned |
| | `float_overflow` | 1 | 1 | `expf` of a large exponent gives infinity |
| | `float_invalid` | 1 | 1 | square root of a negative number (NaN) |
| | `float_conversion` | 1 | 1 | float to `s16` out of range |
| | `integer_division_in_float` | 1 | 1 | `u8 / 100U` stored in a float |
| | `invalid_enum` | 1 | 1 | integer cast to an enum without a range check |
| Memory / pointers | `out_of_bounds` | 9 | 12 | array index, `memcpy` length, table lookup |
| | `null_deref` | 2 | – | NULL from a lookup or from invalid NVM |
| | `ptr_arith` | 1 | 1 | pointer moved past the end of its array |
| | `stack_escape` | 1 | – | returns the address of a local |
| | `dangling_ptr` | 1 | 1 | pointer to a block-scope array used after the block |
| | `uninit_var` | 1 | – | local not set on one path |
| | `uninit_ptr` | 1 | 1 | local pointer not set on one path |
| | `wrong_sizeof` | 1 | – | `sizeof(pointer)` instead of the object |
| | `bad_func_ptr` | 1 | 1 | function pointer that points to data |
| Strings / formats | `string_overflow` | 2 | 1 | `strcpy` / `sprintf` into a small buffer |
| | `missing_null_terminator` | 1 | 1 | `strncpy` with the full buffer size |
| | `overlapping_copy` | 1 | 1 | `memcpy` with overlapping ranges |
| | `format_string` | 1 | 1 | `%d` with a `double` argument |
| | `uncontrolled_format` | 1 | 1 | external text used as the format |
| Heap | `use_after_free` | 1 | 1 | field read after `free` |
| | `double_free` | 1 | 1 | same pointer freed twice |
| | `memory_leak` | 1 | 1 | early return loses an allocation |
| | `bad_free` | 1 | 1 | `free` of a static array |
| | `unchecked_alloc` | 1 | 1 | `malloc` result used without a NULL check |
| | `alloc_size` | 1 | 1 | byte count instead of element count |
| Control flow | `dead_code` | 1 | – | condition that can never be true |
| | `unreachable_code` | 1 | – | statement after `return` |
| | `missing_return` | 1 | 1 | non-void function falls off its end |
| | `missing_break` | 1 | – | unintended switch fall-through |
| | `empty_body` | 1 | – | stray `;` after `if` |
| | `operator_precedence` | 1 | 1 | `a & B != 0` |
| | `non_terminating_loop` | 1 | – | busy-wait without a timeout |
| | `non_terminating_call` | 1 | – | call to a function that never returns |
| Concurrency | `data_race` | 1 | – | ISR and task write the same data |
| | `missing_unlock` | 1 | 1 | return path keeps the lock |

**Coding rules (RULE)**

| Category | R | S | Example |
|---|---|---|---|
| `switch_default` | 1 | – | switch without `default` |
| `goto` | 3 | – | `goto` used |
| `recursion` | 1 | – | function calls itself |
| `float_equality` | 1 | – | floats compared with `==` |
| `unchecked_return` | 1 | – | return value ignored |
| `missing_volatile` | 1 | – | ISR-shared data not `volatile` |
| `dynamic_memory` | 1 | – | `malloc` in ECU code |
| `assignment_in_condition` | 1 | – | `if ((p = f()) == NULL)` |
| `side_effect_in_condition` | 1 | – | `++` in the right operand of `\|\|` |
| `implicit_conversion` | 1 | – | `s32` assigned to `s16` without a cast |
| `signed_unsigned_compare` | 1 | 1 | `s32 <= u32` |
| `signed_bitwise` | 1 | – | shift of a signed value |
| `invariant_condition` | 1 | – | `u8 <= 255U` is always true |
| `unused_variable` | 1 | – | local never used |
| `unused_parameter` | 1 | 1 | parameter never used |
| `shadowing` | 1 | – | inner local hides an outer one |
| `magic_number` | 1 | – | unnamed numeric constant |
| `octal_constant` | 1 | – | `010` |
| `missing_braces` | 1 | – | `if` body without braces |
| `vla` | 1 | – | variable-length array |

### Baseline: free tools (2026-11-11, default-ish settings, before the categories above were extended)
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
  config_parser.c   "key=value" config lines from the end-of-line tester
  crc.c             CRC-8 (J1850) and CRC-16 (CCITT)
  diag.c            DTC storage, debounce, freeze frame
  diag_buffer.c     heap-based snapshot buffer for the service session
  filter.c          moving average, low-pass, median, rate limiter
  logger.c          text event log in fixed-size lines
  motor_ctrl.c      motor state machine, derating, PWM
  nvm.c             simulated EEPROM, calibration block with CRC
  os_lock.c         minimal lock (interrupt disable on the MCU)
  pid.c             fixed-point PID
  ring_buffer.c     lock-free style byte FIFO
  scheduler.c       1 ms tick ISR + cooperative task scheduler
  torque_map.c      pedal + mode + temperature -> torque request (float)
  util.c            saturation, abs, table interpolation, sqrt, parsing
  watchdog.c        watchdog kick
  wheel_speed.c     encoder ticks -> wheel and vehicle speed
  hw_sim.c          PC-only plant model (not ECU code)
  main.c            init, task setup, drive cycle, fault injection
tools/              expected_findings.ps1 (ground truth extractor)
third_party/        local GCC/Clang/CMake/clang-tidy/cppcheck (not in Git)
build.ps1           build script
```
