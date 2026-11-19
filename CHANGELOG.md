# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow
[Semantic Versioning](https://semver.org/). While the version is `0.x`, minor
releases may contain breaking changes, and they are always described below.

## [Unreleased]

### Added

- MIT license, selected in `QA.md`.
- Fixed-capacity named digital action bindings for keyboard, mouse and gamepad;
  opt-in rotating file/console log sink and a frame stats display.
- `StatsDisplay` callback so the frame readout can be drawn by the game renderer
  instead of a separate software renderer.
- Planning notes for optional Steam and GOG integrations without introducing SDKs.
- SDL 3.4.20 pinned by URL and SHA-256 and built statically inside `engine/platform`.
- `eng_platform`:
  - window creation, OS event pump, resize/focus/minimize/fullscreen handling;
  - engine-owned keyboard, mouse and gamepad state with pressed/released edges;
  - fixed-step application loop with catch-up cap, interpolation alpha, optional
    FPS limiter and per-frame CSV logging;
  - UTF-8 file helpers with atomic replace, directory creation and executable
    directory lookup.
- Fixed-step timing utilities in `eng_core`.
- `bench_blank` benchmark driver with warm-up, sample cap, pacing/unlimited modes
  and process-memory reporting.
- Platform test suite (window, file system, app loop).

### Changed

- The first CMake configure now downloads SDL; offline builds reuse
  `build/<preset>/_deps/`.

### Fixed

- Action edges were lost when a frame ran no fixed update and repeated when a
  frame ran several. Button changes are now queued and delivered one per fixed
  update, in order. Gamepad buttons are event-driven, so quick taps are no
  longer missed between polls.
- Gamepad disconnect now reports held-button releases; stick readings are clamped.
- Reject non-finite app timing values and correct milestone/benchmark claims.

### Notes

- OpenGL context creation moved to M2 with the renderer so headless window tests
  do not require a GPU.
- Input queues are fixed size (256 raw button changes per pump, 128 pending
  changes per action) and stop the loop with an error on overflow rather than
  dropping events.
- Baseline numbers are from `dev-01`, not Tier A hardware; see
  `benchmarks/results/2026-11-19-bench_blank-win-x64-dev01.md`.

## [0.1.0-dev] - 2026-11-19

First implementation milestone (M0: foundations). No rendering, input or audio
yet; this is the dependency-free core the rest of the engine builds on.

### Added

- CMake project with per-platform presets (`windows-*`, `linux-*`, `linux-asan`,
  `windows-clang-debug`), warnings-as-errors, and `check` target.
- `eng_core` static library with:
  - fixed-width integer aliases and 2D math (`Vec2`, `IVec2`, `Rect`, `Color`);
  - `Result<T>` / `Status` error values, with no exceptions across boundaries;
  - thread-safe logging with replaceable sinks, fixed-buffer formatting and
    truncation markers;
  - tracked allocations with per-subsystem tags, peak/current counters and leak
    counting;
  - monotonic time and a `Stopwatch`.
- Self-contained test harness and 25 test cases covering math, results, logging,
  memory tracking, time and versioning.
- Windows developer environment helper (`tools/dev/msvc_env.bat`) that locates
  Visual Studio, enters the MSVC x64 environment and adds CMake/Ninja/LLVM.
- Formatting check (`tools/check_format.py`) and clang-tidy configuration.
- GitHub Actions CI for Windows (MSVC Debug/Release) and Linux (GCC
  Debug/Release, ASan + UBSan, formatting, advisory clang-tidy).

### Fixed

- `time::now_seconds()` could return a small negative value on its first call
  because the epoch and the current time were read in one expression with
  unspecified evaluation order.
- Over-aligned allocations now use `std::align` pointer arithmetic instead of
  integer-to-pointer casts.

### Notes

- Linux builds are configured in CI but have not been run locally on this
  machine (no WSL/Docker/native Linux toolchain available). See
  `docs/status.md`.
- At that point the project name and license were open; see the Unreleased entry for the later decision.
