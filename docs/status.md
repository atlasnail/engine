# Status and handoff

**This file is the single source of truth for "where are we now".** Every work session updates it. Keep it short, factual and current; move detail into the linked documents.

Last updated: 2026-11-19

## Current position

| Field | Value |
| --- | --- |
| Phase | **M0 implemented; Windows verified; Linux gate pending CI** |
| Milestone | **M0 — Foundations and quality gates** |
| Engine version | `0.1.0-dev` |
| Code present | `eng_core` library + test suite + build system + CI config |
| Platforms verified | **Windows x64 only** (MSVC Debug/Release, Clang Debug) |
| Linux | Presets and CI written; **never compiled on this machine** (no WSL/Docker/native toolchain) |
| Benchmarks | None yet; no frame loop exists to measure. First gates at M1. |

## What works (verified 2026-11-19)

- Configure, build and test via CMake presets on Windows with MSVC 19.52 and Clang 22.1.3.
- **25 test cases / 168 checks pass** in Debug and Release (MSVC) and Debug (Clang).
- Zero compiler warnings under `/W4 /WX` (MSVC) and the Clang warning set.
- clang-format 22.1.3 check passes; clang-tidy 22 reports no warnings in project code.
- Engine-level leak check: test run ends with zero live tracked allocations.
- Release build starts from a clean configure in under a minute on the dev machine.

## Environment (verified)

| Tool | Version / path |
| --- | --- |
| Visual Studio | 2026 Community (`18`), MSVC toolset **14.52.37025** selected by vcvars (14.53 preview also installed) |
| CMake | **4.3.1-msvc1** (bundled with VS) |
| Ninja | **1.13.2** (bundled with VS) |
| Clang / clang-format / clang-tidy | **22.1.3** (bundled with VS) |
| Git / Python | 2.56.0 / 3.11 |
| Linux | none locally; CI targets `ubuntu-24.04` with GCC |

## Next tasks (in order)

1. Push to `https://github.com/atlasnail/engine.git` and confirm the Windows and
   Linux CI jobs pass. Linux passing is the remaining M0 exit criterion.
2. Resolve `QA.md` (license, name, Linux verification choice, review cadence).
3. Tag/record M0 complete; start M1:
   - pin SDL3 (exact version + integrity record) in `third_party/`;
   - window + GL 3.3 context + clean shutdown;
   - fixed-step loop, frame limiter, VSync toggle;
   - input (keyboard/mouse/gamepad) with data-driven actions;
   - file system basics (UTF-8 paths, atomic replace);
   - frame stats CSV and overlay;
   - `bench_blank` and the M1 performance gates (F1, F6, F7, M-1).
4. Set up `benchmarks/reference-hardware.md` with this machine as `tier-a-01`
   before recording any M1 number.

## Decisions made autonomously (2026-11-19)

| Decision | Why | Recorded |
| --- | --- | --- |
| Custom minimal test harness instead of Catch2/doctest | Zero dependencies, identical everywhere, readable in one minute | ADR-0009 |
| ADR-0001…0008 marked Accepted | They define the plan you approved and are needed to build | ADRs |
| Build presets: Ninja + per-platform compiler; extra Clang preset on Windows | Fast builds; second compiler catches portability issues early | `CMakePresets.json` |
| clang-tidy advisory in CI for now, blocking locally | Version differences between CI (18) and local (22) are unproven | `.github/workflows/ci.yml` |
| `Result<T>` copy operations compile only when `T` is copyable | Keeps move-only types usable without a second type | `result.hpp` |
| Allocation stats count requested bytes, not header overhead | Simpler, stable numbers; overhead reported later | `memory.hpp` |
| Working name/namespace stay `engine` / `eng`; license left open | Renaming is mechanical; license is your call | `QA.md` |

## Open questions

See `QA.md`. Until answered: no `LICENSE` file, working name `engine`, Linux
verified through GitHub Actions, autonomy continues per your instruction.

## Known issues and limitations

- Linux has never been built locally. Do not cite Linux support as verified
  until CI is green or a local toolchain runs the suite.
- No window/input/audio/render code exists; nothing visual to run yet.
- `Result<T>` error messages are non-owning `const char*`; dynamic context must
  be logged, not stored in the error. Revisit if call sites need more.
- Clang-tidy on CI uses Ubuntu's clang-tidy 18 while local is 22; the CI step is
  advisory to avoid version-driven false failures.
- The CI workflow itself has not run yet. The initial push to
  `https://github.com/atlasnail/engine.git` was attempted and blocked: no GitHub
  credentials are stored on this machine (HTTPS push cannot prompt here, no SSH
  key is configured). Commit `2f8b8ea` is ready on local `main`; run
  `git push -u origin main` once authenticated.

## Benchmark snapshot

None. Do not invent numbers. When `bench_blank` exists, record hardware ID,
commit, p50/p99 and memory here with a link to the result file.

## Session log (newest first)

| Date | Summary |
| --- | --- |
| 2026-11-19 | Implemented M0: CMake presets, warnings-as-errors, `eng_core` (math, Result/Status, logging, tracked memory, time), 25 tests, format/tidy tooling, CI, beginner guide. Verified Windows MSVC Debug/Release + Clang Debug. Found and fixed a first-call negative timestamp bug and two int-to-pointer cast warnings. |
| 2026-11-19 | Wrote planning set: README, AGENTS.md, roadmap, architecture, performance, conventions, ADRs, backlog, status. |
