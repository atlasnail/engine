# Status and handoff

This is the source of truth for current implementation and verification.

Last updated: 2026-11-19

## Current position

- **M0 is close but not signed off:** Windows and Linux now build and test in
  CI (Debug, Release, ASan/UBSan on Linux), but the pinned GL loader gate and
  required (non-advisory) clang-tidy enforcement are still open.
- **M1 is in progress:** platform work was started before M0's gates were met.
  This is not an approval to declare M0 or M1 complete, or to start M2.
- Version `0.1.0-dev`; implementation: core and SDL3 platform, tests and
  `bench_blank`. No game renderer or audio.
- `main` is pushed to `https://github.com/atlasnail/engine.git`. Linux CI has
  run and passes; it uses SDL's dummy video driver, so no real display, GPU or
  gamepad has been exercised on Linux.

## Latest session: input delivery and stats display

- Action edges are now queued from ordered button events and delivered one per
  fixed update, so a frame with no fixed step no longer loses a tap and a frame
  with several steps no longer repeats one. Keyboard, mouse and gamepad share
  the path; two taps in one pump produce two press/release pairs.
- Gamepad buttons now also arrive as events instead of only polling, so a quick
  tap between pumps is captured. Disconnect still releases held buttons.
- Queues are fixed size and report overflow instead of dropping events. `App::run`
  stops with a nonzero code if input or the stats display fails.
- `frame_stats_overlay` shows FPS, frame and update times, steps and dropped
  steps. A `StatsDisplay` callback lets a game renderer draw that text and
  present, instead of a second renderer clearing the window; without a callback
  the old standalone software renderer is used.
- Tests added for ordering, taps with zero or several steps, overlapping
  bindings, queue overflow, the callback after game rendering, and callback
  failure.

## Earlier M1 work in this branch

- MIT license (`LICENSE`, copyright Dylan Alexander); name stays `engine`/`eng`.
- Fixed-capacity named digital actions; opt-in rotating file log sink; standalone
  frame stats display; gamepad disconnect fix; non-finite timing rejected;
  directory creation on a file rejected; temporary file cleanup on write failure;
  corrected M0/benchmark claims; storefront integration planning in the backlog.
- `QA.md` holds the user's answers and is kept as-is.

## Verification

- **CI run 44588973457 on `0423ad0` is green.** Windows (MSVC): format, Debug
  build/tests, Release build/tests. Linux (GCC 13, `ubuntu-24.04`): format,
  Debug build/tests, ASan+UBSan build/tests, Release build/tests, advisory tidy.
  Linux tests run with `SDL_VIDEODRIVER=dummy`; window tests that need a real
  driver still skip.
- Windows MSVC Debug locally: build clean and CTest passes, including the new
  input, stats and file-system tests.
- Not verified: real Linux desktop (X11/Wayland) behavior, real controller
  hardware, a current local Clang build, CRT leak gate, visible-window
  appearance of the stats readout, and any Tier A performance gate.
- Release benchmark from the previous session (overlay, file log and actions
  disabled): `dev-01` paced p99 17.491 ms, 58.9 FPS, 0 dropped steps, 4.5 MB
  exit private bytes. One short run, not a gate; see
  `benchmarks/results/2026-11-19-bench-blank-review-win-x64-dev01.md`.
- `bench_blank` measures elapsed frame-start intervals, not isolated CPU time,
  and its unlimited run samples only the first 200,000 frames. F1 is unverified.

## Open gates and known limitations

1. M0 exit criteria remain unmet: complete CI enforcement (clang-tidy is still
   advisory) and the GL loader pin.
2. Linux is verified only headless. X11/Wayland window behavior, DPI, fullscreen
   and focus handling on a real desktop are untested.
3. M1 parity and hardware tests are incomplete: real gamepad, visible window,
   fullscreen, focus, DPI/minimize behavior, Linux display drivers, shutdown.
   Window tests skip when no video driver is available.
4. M1's GL context, clear and VSync need the later renderer; no approved
   roadmap exception is recorded. Minimized windows may spin if frame limiting
   is disabled.
5. Fixed-step input keeps at most 128 pending changes per action and 256 raw
   button changes per pump, then fails explicitly. Dropped fixed steps mean
   those simulation steps do not consume input; the queue is not a replay log.
   Mouse movement and wheel remain frame-scoped. SDL and the selected gamepad
   are process-global; no SDL_Quit lifecycle is implemented.
6. Per-frame tracked-allocation claims do not cover ordinary STL/SDL allocation;
   no comprehensive allocation audit was done. CSV logging uses a stream.
7. Atomic replacement is not power-loss durable: file and directory sync are
   not implemented. A single writer per path is assumed.
8. Tier A hardware and the 60-second gate are unavailable. Rework CPU timing and
   sampling before claiming F1.
9. `Result<T>` error messages are non-owning. Existing code may still allow
   allocation/standard-library exceptions across boundaries.

## Next steps

- Finish M0's quality gates: make clang-tidy blocking once it has stayed green,
  and pin the OpenGL loader before M2 needs it.
- Remaining M1 gates: visible-window checks on both platforms, real gamepad,
  minimized behavior, and a real CPU-time measurement.
- Do not start the renderer (M2) or add store SDKs until sequencing is agreed.

## CI fixes found by the first Linux run

- SDL 3.4.20 treats XTest as required with X11; `libxtst-dev` was missing, so
  configure failed. The dependency list also gained pkg-config, xkbcommon-x11,
  libdecor, libdrm, libgbm and libudev.
- Linux Release failed with a GCC 13 `-Wnull-dereference` false positive inside
  libstdc++ triggered by `istreambuf_iterator`. `read_text_file` now sizes and
  reads the file directly instead of suppressing the warning.
- `tools/dev/ci_configure.sh` and `tools/dev/ci_build.sh` turn configure and
  compiler failures into annotations, because job logs need admin access.

## Historical context

Prior Windows tests covered 30 core / 186 checks and 10 platform / 43 checks.
SDL 3.4.20 is pinned by URL and SHA-256. The earlier developer-machine baseline
is `benchmarks/results/2026-11-19-bench_blank-win-x64-dev01.md`, corrected to
identify its elapsed-time limitation. No Linux or Tier A result is implied.
