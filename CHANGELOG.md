# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow
[Semantic Versioning](https://semver.org/). While the version is `0.x`, minor
releases may contain breaking changes, and they are always described below.

## [Unreleased]

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
- Project name and license are still open; see `QA.md`.
