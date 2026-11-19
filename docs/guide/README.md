# Start here: how this project works

This guide is for someone who wants to understand the code without being a C++
expert. It explains what exists today (milestone M0), how to build it, what each
part does, and the C++ ideas the code uses. It is updated as the engine grows.

If you only read one section, read [Build and run](#build-and-run) and
[The 60-second mental model](#the-60-second-mental-model).

---

## What exists today

There is no game window yet. M0 is the foundation:

| Piece | What it does | Where |
| --- | --- | --- |
| Build system | Turns source files into libraries, tests and a benchmark | `CMakeLists.txt`, `CMakePresets.json`, `cmake/` |
| `eng_core` | Math, errors, logging, memory tracking, time, fixed-step timing | `engine/core/` |
| `eng_platform` | Window, OS events, input, application loop, file system; wraps SDL | `engine/platform/` |
| Tests | 40 cases that check the core and platform on every build | `tests/` |
| Benchmark | `bench_blank`: measures an empty frame | `benchmarks/` |
| Tooling | Formatting, static analysis, CI | `tools/`, `.clang-format`, `.clang-tidy`, `.github/workflows/ci.yml` |
| Documentation | Plan, architecture, rules, decisions, this guide | `docs/`, `AGENTS.md`, `README.md` |

The first visual result (a window with a loop and input) works. The renderer,
and with it anything you can see inside the window, is milestone M2.

---

## The 60-second mental model

1. You write **source files** (`.cpp`) and **headers** (`.hpp`).
2. A **compiler** (MSVC on Windows, GCC/Clang on Linux) turns each `.cpp` into an
   object file. It reads headers to learn what exists in other files.
3. A **linker** combines object files into either:
   - a **static library** — a bag of compiled code (`eng_core.lib` / `libeng_core.a`), or
   - an **executable** — a program with a `main()` (`eng_tests.exe`).
4. **CMake** does not compile anything itself. It reads `CMakeLists.txt` and
   generates build files (here: Ninja files) that tell Ninja how to call the
   compiler and linker in the right order.
5. **CTest** runs the test executable and reports pass/fail.

There is no `.sln` checked in and no IDE project file to maintain: the build is
described once in CMake and works on both platforms.

---

## Build and run

### Windows, easiest path

Open **Developer PowerShell for VS 2026** from the Start menu (it sets up the
compiler for you), then from the project folder:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

`--preset windows-release` builds an optimized version. For Clang, use
`windows-clang-debug` (the Developer PowerShell must have LLVM on PATH; the
helper below adds it).

### Windows, from Git Bash or any shell

The helper finds Visual Studio, enters its compiler environment, adds the
CMake/Ninja/LLVM that ship with Visual Studio to `PATH`, then runs your command:

```bash
cmd //c 'tools\dev\msvc_env.bat cmake --preset windows-debug'
cmd //c 'tools\dev\msvc_env.bat cmake --build --preset windows-debug'
cmd //c 'tools\dev\msvc_env.bat ctest --preset windows-debug'
```

### Linux

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

`linux-asan` adds AddressSanitizer and UndefinedBehaviorSanitizer, which catch
memory errors and undefined behavior that normal builds hide.

### Where the output goes

Builds are per-preset, so Debug and Release never overwrite each other:

```
build/windows-debug/bin/eng_tests.exe
build/windows-release/bin/eng_tests.exe
build/linux-asan/bin/eng_tests
```

The test program also accepts a filter:
`eng_tests.exe memory` runs only tests whose name contains "memory".

---

## Tour of the code

### `engine/core/include/eng/core/`

Headers: the *interface* of the core. Other code includes these.

| File | Purpose |
| --- | --- |
| `types.hpp` | `u8`/`i32`/`usize` etc. so integer sizes are identical on Windows and Linux |
| `math.hpp` | `Vec2`, `Rect`, `Color`; small, inline, no dependencies |
| `result.hpp` | `Result<T>` and `Status`: how failures are returned instead of thrown |
| `log.hpp` | Logging API, levels, replaceable sink |
| `memory.hpp` | Tracked allocation API and memory statistics |
| `time.hpp` | Monotonic clock and `Stopwatch` |
| `assert.hpp` | `ENG_ASSERT` for programmer bugs only |
| `version.hpp` | Generated from the root `CMakeLists.txt`; do not edit the generated copy |

### `engine/core/src/`

Implementations (`.cpp`). Each one is compiled once into `eng_core`.

- `log.cpp` — formats records into a fixed buffer, then calls the sink. It uses
  a mutex so log calls from different threads do not interleave mid-line.
- `memory.cpp` — a thin layer over `malloc`/`_aligned_malloc` that records size,
  tag and a magic number before the returned pointer. `deallocate` reads that
  header back. Counters are atomics because several threads may allocate.
- `time.cpp` — `std::chrono::steady_clock`, which never jumps when the system
  clock is adjusted.

### `engine/platform/`

This is the only module that talks to the operating system directly (through
SDL). Everything above it uses engine types.

| File | Purpose |
| --- | --- |
| `window.hpp` | Creates a window, pumps OS events, answers size/focus/fullscreen questions |
| `input.hpp` | Keyboard/mouse/gamepad state with `down`/`pressed`/`released` queries |
| `app.hpp` | Owns the window and runs the loop: events → fixed updates → render → limit |
| `fs.hpp` | UTF-8 file read/write, atomic replace, directories, executable path |
| `src/window.cpp` | SDL event translation, key/gamepad mapping, focus-loss handling |
| `src/app.cpp` | Fixed-step accumulator, FPS limiter, optional CSV frame log |

SDL is pinned in `third_party/` and downloaded on the first configure. It is a
dependency of the platform layer, not part of the engine's public API: no SDL
type appears in these headers.

### `tests/`

- `framework.hpp` — ~130 lines: registers test cases, `CHECK`/`REQUIRE`/`SKIP`,
  a filter and a summary. No third-party dependency.
- `test_main.cpp` — runs the registered cases, then fails if the engine leaked
  any tracked allocation.
- `test_*.cpp` — one file per area (math, result, memory, log, time, fixed step,
  version).
- `platform/` — window, file system and app-loop tests. They skip when no video
  driver exists instead of failing.

---

## The C++ ideas used here

You do not need to master these to follow the code; this is a working map.

### Headers, includes and translation units

A `.cpp` file plus everything it `#include`s is a **translation unit**. It is
compiled independently. A header should describe what other files may use, not
contain implementation. `#pragma once` stops a header being read twice in one
translation unit.

### Namespaces

`namespace eng { ... }` puts names in a named scope, so `eng::Result` cannot
clash with another library's `Result`. `eng::mem`, `eng::time` group related
functions. Internal helpers live in an unnamed `namespace { ... }` inside a
`.cpp`, which makes them private to that file.

### `constexpr` and `inline`

`constexpr` means "can be computed at compile time"; these functions are also
usable at runtime. `inline` on a function defined in a header avoids duplicate
symbol errors when several `.cpp` files include it.

### RAII and destructors

C++ runs a destructor when an object goes out of scope ("Resource Acquisition Is
Initialization"). `Result<T>` uses this: its destructor destroys whichever value
it currently holds. The test `result_error_destructor_is_called` proves it.

### Templates (lightly)

`Result<T>` is a template: one written class that works for `int`, `std::string`
or a move-only type. The compiler generates a version per type. Errors are
caught at compile time — a template's body is only checked in detail when
instantiated, which is why one test deliberately exercises move-only values.

### Unions and placement new (the one spiky part)

`Result<T>` must hold *either* a `T` or an `Error`, not both. A `union` overlaps
them in memory. Because `T` may be a `std::string` with its own constructor and
destructor, the code uses **placement new** (`new (&storage_) T(...)`) to build
the value in that memory and calls the destructor explicitly when done. This is
the only place in the core where that complexity is needed; if it looks dense,
that is expected, and the tests around it are the safety net.

### Variadic and printf-style logging

`log_message(level, category, format, ...)` takes extra arguments like
`printf`. The compiler checks the format string on GCC/Clang, and the result is
formatted into a fixed 1024-byte buffer, so logging does not allocate. Long
messages are visibly marked `...[truncated]`.

### Atomics and threads

Memory counters are `std::atomic`: several threads can update them without a
lock. Logging uses `std::mutex` because records are written to a shared sink.
The audio thread rules (no locks, no allocation) come later and are stricter.

### Static local initialization

A `static` variable inside a function is created on first use, exactly once,
thread-safely. `time.cpp` uses that for the clock epoch. Be careful with
expressions that combine such a call with another call: C++ does not promise
which operand is evaluated first. That exact bug produced a negative timestamp
and is now fixed and tested.

---

## Window, input and the main loop

`eng::App` is the smallest useful game loop:

```cpp
eng::AppConfig config;               // window size, title, fixed step, FPS limit
config.window.title = "my game";

auto app = eng::App::create(config);
if (app.is_error()) { /* log and exit */ }

app.value()->run(
    [](double dt) { /* gameplay: always dt == fixed step */ },
    [](double alpha) { /* draw; alpha blends between simulation states */ });
```

What happens each iteration in `engine/platform/src/app.cpp`:

1. `pump_events` clears one-frame input edges (pressed/released, mouse delta,
   wheel) and translates every pending SDL event.
2. The frame's duration goes into `FixedTimestep`, which returns 0..8 fixed
   steps. Gameplay never sees a variable `dt`, so behaviour is stable.
3. `render(alpha)` is called once, where `alpha` is the fraction of the next
   step already elapsed (used to interpolate positions).
4. If frame limiting is on, the loop sleeps until the 60 FPS target using
   `SDL_DelayNS`, which paces better than a naive `sleep_for` on Windows.

Why fixed steps: physics and animation remain deterministic and independent of
the monitor's refresh rate. Why a cap: after a debugger pause or a window drag,
the loop must not try to simulate seconds of catch-up in one frame; extra steps
are dropped and counted.

A game should not call `SDL_*` directly. If a feature is missing, add it to
`eng_platform` and expose an engine type.

## Measuring an empty frame

After building `windows-release` (or `linux-release`):

```bash
build/windows-release/bin/bench_blank.exe --seconds 5 --warmup 1
```

It prints p50/p99 frame time, dropped steps and process memory. The unlimited
mode (`--no-limit`) measures pure loop overhead; paced mode checks the limiter.
Targets and the recording format are in `docs/performance.md`. Numbers are only
meaningful from a Release build, and the machine plus commit must be recorded.

## How the core pieces behave

### Memory tracking

Every allocation goes through `eng::mem::allocate(bytes, alignment, tag)`.
The pointer it returns is preceded by a small header:

```
[ raw pointer | size | magic | tag ][ your bytes ... ]
                                    ^ pointer returned to you
```

`deallocate` steps back by the header size, checks the magic number, updates the
counters, then frees the original pointer. `stats(tag)` reports current and peak
bytes plus allocation/free counts. Tests assert that every allocation is
released and that the test run leaves zero live allocations.

Tags (`render`, `audio`, `assets`, ...) exist so a future memory overlay can say
"audio is using 4 MB" instead of only showing a total.

Why this design: it is simple, works on both platforms, and makes leaks
visible. Arenas and pools (M6) will be built on top of it.

### Errors

Functions that can fail return `Result<T>` or `Status`. The caller must look at
the result; `[[nodiscard]]` makes ignoring it a compiler warning. No exceptions
cross a module boundary, so the C ABI (M8) and callbacks stay simple. An
`ENG_ASSERT` is only for "this is a programming mistake", never for a missing
file or a lost audio device.

### Logging

Call `ENG_LOG_INFO("render", "loaded %d sprites", count)`. The category is a
short static string. Tests install their own sink, so they can assert on records
without parsing stderr. In M1 the default sink gains a rotating file; the API
does not change.

### Time

`now_seconds()` is monotonic and useful only for differences. Wall-clock time
surfaces later for save files, where jumps are handled explicitly.

---

## Common tasks

### Add a math function

1. Put it in `engine/core/include/eng/core/math.hpp`, marked `constexpr`/`inline`
   where possible.
2. Add a `TEST_CASE` in `tests/test_math.cpp`.
3. Build and run the tests; formatting is checked by `tools/check_format.py`.

### Add a test

Add a `TEST_CASE(name)` block in the relevant `tests/test_*.cpp`. Use `CHECK` for
a failure that should not stop the test, `REQUIRE` when continuing makes no
sense. New test files must be listed in `tests/CMakeLists.txt`.

### Add a new core module

Create `engine/core/include/eng/core/<name>.hpp` and
`engine/core/src/<name>.cpp`, add the `.cpp` to `engine/core/CMakeLists.txt`,
add tests, and update this guide's file table.

### Change the engine version

Edit `VERSION` in the root `CMakeLists.txt` only. The generated header and
`k_engine_version` follow it. Add a `CHANGELOG.md` entry.

---

## Glossary

| Term | Meaning |
| --- | --- |
| **Preset** | A named, checked-in build configuration (`windows-debug`) |
| **Target** | A buildable thing: library or executable |
| **Translation unit** | One `.cpp` after preprocessing |
| **Linker** | Combines compiled objects into a library/executable |
| **Static library** | Compiled code bundled into the executable at link time |
| **Sanitizer** | Runtime checker (ASan/UBSan) for memory errors and UB |
| **Monotonic clock** | A clock that only moves forward |
| **RAII** | Tying a resource's lifetime to an object's scope |
| **UB** | Undefined behavior: code whose result the language does not define |
| **SDL** | Cross-platform library for windows, input, audio and threads; wrapped by `eng_platform` |
| **Fixed step** | Gameplay always advances by the same `dt`, regardless of frame rate |
| **Interpolation alpha** | 0..1 fraction between two simulation steps, used for smooth drawing |

---

## Where to go next

- `docs/architecture.md` — why the layers and rules are what they are.
- `docs/performance.md` — the numbers the engine must hit, and how they are measured.
- `docs/conventions.md` — the definition of done for any change.
- `docs/status.md` — what is finished, what is next, what is blocked.
