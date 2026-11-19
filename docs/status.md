# Status and handoff

**This file is the single source of truth for "where are we now".** Every work session updates it. Keep it short, factual and current; move detail into the linked documents.

Last updated: 2026-11-19

## Current position

| Field | Value |
| --- | --- |
| Phase | **M1 platform layer implemented; Windows verified; Linux gate pending CI** |
| Milestone | **M1 — Platform, window, loop and input** (partially complete, see below) |
| Engine version | `0.1.0-dev` |
| Code present | `eng_core`, `eng_platform` (SDL3), 40 test cases, `bench_blank` |
| Platforms verified | **Windows x64 only** (MSVC Debug/Release, Clang Debug) |
| Linux | Presets and CI written; **never compiled on this machine** (no WSL/Docker/native toolchain) |
| Benchmarks | `bench_blank` measured on `dev-01`; **not a Tier A gate run** |

## What works (verified 2026-11-19)

- CMake presets fetch and build SDL 3.4.20 (pinned URL + SHA-256) on Windows.
- **30 core tests / 186 checks** and **10 platform tests / 43 checks** pass in
  Debug and Release with MSVC; core also passes with Clang 22.1.3.
- Window creation (hidden or visible), event pump, resize/minimize/focus queries,
  fullscreen toggle, title changes, clean destruction.
- Keyboard, mouse (position, buttons, wheel) and gamepad state API. Gamepad code
  is compiled but has not been exercised with hardware.
- Fixed-step accumulator with catch-up cap and dropped-step accounting.
- Application loop: fixed updates, render callback with interpolation alpha,
  optional 60 FPS limiter (SDL high-resolution delay), optional per-frame CSV.
- Filesystem: UTF-8 paths, read/write, atomic replace, directory creation,
  executable directory.
- `bench_blank` (Release, hidden window, `dev-01`): empty-frame p99 **< 0.001 ms**
  unlimited, paced p99 **17.19 ms** at 60 FPS target (all frames within ±10 %),
  **0 dropped steps**, **4.4 MB** private memory at exit. Record:
  `benchmarks/results/2026-11-19-bench_blank-win-x64-dev01.md`.
- Formatting and clang-tidy clean on project code.

## Not done yet (M1 remainder and M2 boundary)

- Data-driven action mapping (keys → named actions); key state exists, actions do not.
- Rotating file logging sink (console sink only; `fs` primitives are ready).
- On-screen frame stats overlay (CSV logging exists).
- OpenGL context creation: deliberately moved to M2 with the renderer, so window
  creation works on headless CI.
- 60-second benchmark protocol on Tier A hardware; full F6/F7 measurement.
- Linux build/test (CI or local toolchain).
- Gamepad verification with real hardware.
- CI has never run; push is blocked on GitHub credentials (see below).

## Environment (verified)

| Tool | Version / path |
| --- | --- |
| Visual Studio | 2026 Community (`18`), MSVC toolset **14.52.37025** selected by vcvars |
| CMake / Ninja | **4.3.1-msvc1** / **1.13.2** (bundled with VS) |
| Clang / format / tidy | **22.1.3** (bundled with VS) |
| SDL | **3.4.20**, static, fetched by CMake into `build/<preset>/_deps/` |
| Git / Python | 2.56.0 / 3.11 |
| Linux | none locally; CI targets `ubuntu-24.04` |

## Next tasks (in order)

1. Push to `https://github.com/atlasnail/engine.git` (credentials needed) and get
   Windows + Linux CI green. Linux is the remaining platform gate.
2. Designate or borrow a **Tier A** machine (4c/8t, integrated GPU, 8 GB) and run
   the 60-second protocol; current numbers are a dev baseline only.
3. Finish M1: action mapping, file log sink, frame-stats overlay; test a real
   gamepad.
4. Start M2: OpenGL 3.3 context + sprite renderer, then `bench_sprites` (F2/F3),
   `bench_startup` (F9), and visual scaling tests.
5. Keep `benchmarks/reference-hardware.md` current before any gated run.

## Decisions made (2026-11-19, M1)

| Decision | Why | Recorded |
| --- | --- | --- |
| SDL 3.4.20, static, URL + SHA-256 pin | Exact, auditable, offline after first configure | `third_party/README.md` |
| GL context deferred from M1 to M2 | Window creation must work headless; renderer owns GL | this file, roadmap note |
| Own `Key`/`GamepadButton` enums, SDL types hidden | Keeps backend swappable and public headers clean | `input.hpp` |
| One gamepad at a time | Simplest useful behavior; multi-pad is backlog | `window.cpp` |
| Frame limiter uses `SDL_DelayNS` | Better pacing than `sleep_for` on Windows | `app.cpp` |
| Benchmark samples capped at 200k | Keeps memory small while p99 stays stable | `bench_blank.cpp` |
| `App` callbacks are `std::function`, set before run | No per-frame allocations or registration churn | `app.hpp` |

## Open questions

`QA.md` still needs: license, engine name, Linux verification route, review
cadence. Working name remains `engine`; no `LICENSE` file yet.

## Known issues and limitations

- **Push blocked**: no stored GitHub credentials (HTTPS cannot prompt in this
  harness, no SSH key). Local `main` has the commits; run
  `git push -u origin main` once authenticated.
- Linux has never been built locally; do not cite Linux support as verified.
- Numbers come from `dev-01` (Ryzen 9 9950X3D, RX 9070 XT), far above Tier A.
- Gamepad input compiles but is untested with a device.
- No action mapping, file log sink or stats overlay yet.
- `Result<T>` error messages remain non-owning `const char*`.
- Benchmark p99 prints 0.000 because the loop is faster than the printed 1 µs
  resolution; the raw CSV has microsecond values.

## Benchmark snapshot

| Metric | Target | `dev-01` | Record |
| --- | --- | --- | --- |
| F1 empty-frame p99 (unlimited) | ≤ 0.3 ms | < 0.001 ms | 2026-11-19 `bench_blank` |
| F6 paced frames within ±10 % | ≥ 99.5 % | 100 % of 296 sampled frames | same |
| F7 spikes > 2× target | 0 | 0 | same |
| M-1 private memory, empty app | ≤ 60 MB | 4.4 MB | same |

## Session log (newest first)

| Date | Summary |
| --- | --- |
| 2026-11-19 | M1 platform layer: pinned SDL 3.4.20; window/events, input (keyboard/mouse/gamepad), fixed step, app loop, filesystem, `bench_blank`; 40 tests pass on Windows; baseline measured on `dev-01`. |
| 2026-11-19 | M0: build system, core (math, Result, logging, tracked memory, time), 25 tests, formatting/tidy, CI, guide. |
| 2026-11-19 | Planning set: README, AGENTS, roadmap, architecture, performance, conventions, ADRs, backlog. |
