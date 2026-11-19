# Status and handoff

This is the source of truth for current implementation and verification.

Last updated: 2026-11-19

## Current position

- **M0 is complete on the evidence recorded below.** clang-tidy is blocking in
  CI, the OpenGL loader is pinned (glad 2.0.8, ADR-0010), and the Windows CRT
  leak check now fails the test run instead of being skipped. The only caveat is
  what that check measures; see the limitation list.
- **M1 is in progress:** SDL now has an explicit, reference-counted lifecycle
  (`eng::SdlSession`), so shutdown is deterministic. Window, input, loop and
  file-system work is done; the remaining M1 gates are the hardware and
  visible-window checks, and a real CPU-time measurement.
- Version `0.1.0-dev`; implementation: core and SDL3 platform, tests and
  `bench_blank`. No game renderer or audio.
- `main` is pushed to `https://github.com/atlasnail/engine.git`. Linux CI has
  run and passes; it uses SDL's dummy video driver, so no real display, GPU or
  gamepad has been exercised on Linux.

## Latest session: SDL lifecycle and the CRT leak gate

- **`eng::SdlSession`** (`engine/platform/include/eng/platform/sdl.hpp`) owns the
  process-wide SDL lifetime explicitly. The first `acquire()` initialises video
  and the gamepad subsystem; the last release closes the selected gamepad and
  calls `SDL_Quit`. It is move-only and reference counted, so an `App` and a test
  harness can both hold one without shutting SDL down early.
- `App` owns a session, so `App::create` still works on its own, and the session
  is declared before the window so it is destroyed after it. `Window::create`
  now fails with a clear error if no session is active instead of initialising
  SDL lazily and hoping nobody noticed.
- The lazy `std::call_once` initialisation and the `ErrorCode` global that went
  with it are gone: initialisation failure is returned to the caller that
  acquired the session.
- The test binaries now have separate entry points: `tests/main_core.cpp` for the
  core suite and `tests/platform/main_platform.cpp`, which holds one session for
  the whole process and releases it before the leak check reports.
- **Windows CRT leak gate** (`tests/leak_check.hpp`): both test executables
  compare CRT heap snapshots at the start and end of the run and fail with a
  non-zero exit code when blocks allocated during the run are still live. It is
  Debug-only and Windows-only; elsewhere it compiles to a no-op.
- Verified by hand: a deliberate 64-byte `malloc` in a temporary test case made
  the platform suite exit 1 and dump the block; removing it returned both
  suites to exit 0.

## Previous session: input delivery and stats display

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

- **CI on `15b900c` is green** (run `44591972552`; the `CI` workflow badge for
  `main` reports passing, and the Linux job page shows the blocking
  `Static analysis` step). Windows (MSVC): format, Debug build/tests, Release
  build/tests. Linux (GCC 13, `ubuntu-24.04`): format, Debug build/tests,
  ASan+UBSan build/tests, Release build/tests, blocking clang-tidy. The vendored
  glad loader compiles in both jobs. Linux tests use `SDL_VIDEODRIVER=dummy`;
  window tests that need a real driver still skip.
- Per-step log detail is not readable without repository admin rights, and the
  unauthenticated API hit its hourly limit mid-session; the gate above was read
  from the workflow badge and the public job page.
- Windows MSVC Debug locally: build clean and CTest passes, including the new
  input, stats and file-system tests. `tools/check_format.py` and
  `tools/dev/ci_clang_tidy.sh build/windows-debug` both pass at tool version
  22.1.8 (installed from PyPI, same as CI).
- Not verified: real Linux desktop (X11/Wayland) behavior, real controller
  hardware, a current local Clang build, visible-window appearance of the stats
  readout, and any Tier A performance gate.
- Release benchmark from the previous session (overlay, file log and actions
  disabled): `dev-01` paced p99 17.491 ms, 58.9 FPS, 0 dropped steps, 4.5 MB
  exit private bytes. One short run, not a gate; see
  `benchmarks/results/2026-11-19-bench-blank-review-win-x64-dev01.md`.
- `bench_blank` measures elapsed frame-start intervals, not isolated CPU time,
  and its unlimited run samples only the first 200,000 frames. F1 is unverified.

## Open gates and known limitations

1. The Windows leak check compares CRT heap snapshots taken at the start and end
   of the test run and fails on blocks allocated during it that were never
   freed. It therefore covers everything the tests allocate, including SDL and
   the standard library, but it does not flag state that is alive for the whole
   process by design (the harness registries, CRT stdio buffers). The engine's
   tracking allocator still fails on engine allocations that are never released.
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
   are process-global, but their lifetime is now explicit: `eng::SdlSession`
   initialises on the first acquire and calls `SDL_Quit` on the last release.
6. Per-frame tracked-allocation claims do not cover ordinary STL/SDL allocation;
   no comprehensive allocation audit was done. CSV logging uses a stream.
7. Atomic replacement is not power-loss durable: file and directory sync are
   not implemented. A single writer per path is assumed.
8. Tier A hardware and the 60-second gate are unavailable. Rework CPU timing and
   sampling before claiming F1.
9. `Result<T>` error messages are non-owning. Existing code may still allow
   allocation/standard-library exceptions across boundaries.

## Next steps

- Remaining M1 gates: visible-window checks on both platforms, real gamepad,
  minimized behavior, a real CPU-time measurement, and the GL clear/VSync pass
  through a real context.
- Do not start the renderer (M2) or add store SDKs until sequencing is agreed.

## Gates closed this session

- **M0 is closed.** With the CRT leak check wired into both test entry points,
  every M0 exit criterion now has evidence on Windows locally and on Linux in
  CI, except the caveat about what the Windows check covers (limitation 1).
- **clang-tidy is blocking.** The advisory step is gone: CI installs clang-tidy
  22.1.8 from PyPI (same version as clang-format, so local and CI findings
  match) and runs `tools/dev/ci_clang_tidy.sh` over all engine sources with
  `--warnings-as-errors=*`. The header filter now covers `engine/.*`, previously
  excluding `engine/platform` headers. It is clean today; findings are annotated.
- **OpenGL loader pinned.** glad 2.0.8, GL 3.3 core, no extensions, generated
  once and vendored in `third_party/glad/` (ADR-0010). Builds as
  `eng::gl_loader`; it needs no Python or network. The root project now enables
  the C language because this loader and SDL are C.

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
