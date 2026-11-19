# Engineering conventions

These rules apply from M0 onward. Changing a rule requires a short note in this file and, if it affects architecture or compatibility, an ADR.

## 1. Language and build

- **C++20**, no compiler extensions. MSVC (`/std:c++20`), GCC and Clang must all accept the code.
- Build with **CMake + presets**. Configurations: `debug`, `release`, `relwithdebinfo`, `asan` (Linux), `asan-win` where practical.
- Warnings are errors: MSVC `/W4 /permissive-`; GCC/Clang `-Wall -Wextra -Wpedantic -Wconversion` (conversion warnings may start as non-fatal in M0, then become fatal).
- `Release`/`RelWithDebInfo` settings are pinned in the preset (optimization level, LTO on/off, float behavior). Never rely on a developer's local defaults.
- No platform headers outside `engine/platform/`. Public headers must compile on both targets.
- Dependencies are pinned to an exact version with a recorded license and source. No floating branches, no auto-updating package manager by default.

## 2. Code style

- `.clang-format` is authoritative and pinned by version; CI verifies formatting.
- `.clang-tidy` runs on changed files; warnings that matter are errors.
- Naming: `snake_case` for functions, variables and files; `PascalCase` for types; `kPascalCase` for constants; trailing underscore avoided; namespaces `eng::` (public) and `eng::detail::` (internal).
- Headers: `#pragma once`; include what you use; forward-declare in public headers where possible.
- Components are small and single-purpose; prefer free functions and plain structs to deep hierarchies.
- Comments explain **why**, constraints and invariants — not what the code does. Every public declaration carries a doc comment with ownership and error behavior.
- No `using namespace` in headers. No `std::endl`. No hidden statics with mutable state.
- Prefer `std::span`, `std::string_view` and fixed-width integers at boundaries.

## 3. Error handling

| Situation | Mechanism |
| --- | --- |
| Programmer bug (invariant violated) | `ENG_ASSERT` in Debug; never rely on it in Release |
| Expected runtime failure (missing file, bad asset, device loss) | `Result<T>` / status enum, logged with context |
| Unrecoverable startup failure | Log fatal, attempt dump, exit with documented code |
| Script error (Lua) | Catch at the script boundary, log, disable that script, game continues |
| Audio callback failure | Degrade (drop voices / silence), never block or allocate |

- No exceptions cross module boundaries or the public API. Tools and tests may use them.
- Every public fallible function documents what it returns on failure.
- Never swallow errors; "best effort" is only acceptable when documented.

## 4. Memory rules

- Allocations go through engine allocators, tagged by subsystem.
- No heap allocation in the frame loop after warm-up; use frame arenas and pools.
- No allocation, locks or file I/O on the audio thread.
- Asset ownership is by handle; raw owning pointers are not exposed in the public API.
- Long-lived resources are released explicitly and verified by the leak checks.
- Debug builds track bytes, peak and call sites; shipping builds compile tracking out but keep counters where cheap.

## 5. Threading rules

- Engine subsystems are not thread-safe unless documented.
- Worker threads only run engine-owned jobs; they never call into game code.
- Audio and main thread communicate through lock-free queues.
- Every thread has a name and is registered with the profiler/log for diagnostics.
- New concurrency requires tests and a soak run; races found in review block the change.

## 6. Testing

| Level | Scope | Runs |
| --- | --- | --- |
| Unit | math, containers, allocators, parsers, collision primitives | every build |
| Integration | asset load → render command, save/load round-trip, audio graph, input mapping | every build |
| Determinism | fixed-step world hash across platforms | CI, both OSes |
| Fuzz | atlas/save/tilemap parsers with malformed input | nightly, 1M iterations |
| Soak | 24 h example game, audio + memory monitoring | milestone/release |
| Visual | fixed scene screenshot comparison, documented tolerance | CI where stable |

- Tests live under `tests/`, mirroring module layout, and use the single documented command.
- Every bug fix adds a regression test where feasible.
- Flaky tests are fixed or quarantined with an issue; never silently ignored.
- Sanitizers (ASan/UBSan) run in CI on Linux; MSVC leak checks on Windows.

## 7. Compatibility and versioning

- **Engine versions:** SemVer. `0.x` may break with changelog notice; from `1.0` breaking changes require a major version.
- **API tiers:** stable (`engine/public`, `engine/c_api`) follows SemVer; experimental may change in a minor release; internal has no guarantees.
- **Deprecation:** announce in changelog, keep working for at least one minor release, emit a warning, document the replacement.
- **Save files:** integer format version + magic; readers support a documented range; migrations are ordered, tested and never lossy by default.
- **Asset formats** (atlas, animation, tilemap, packed archive): same rules as save files.
- **C ABI:** append-only structs with `size` fields, opaque handles, explicit ABI version check at load; no C++ types crossing the boundary.
- **Changelog:** `CHANGELOG.md` entries grouped Added/Changed/Fixed/Removed/Deprecated/Security, with migration notes for anything touching formats or stable API.

## 8. Portability

- Assume case-sensitive filesystems in CI even on Windows (test on Linux); asset paths are lower-case with `/`.
- All paths are UTF-8 internally; convert at the OS boundary only.
- Do not depend on `wchar_t` sizes, endianness, or `long` width; use fixed-width types.
- Simulation code must not use unordered iteration, locale-dependent parsing, or platform RNGs.
- Document intentional platform differences (audio backends, DPI behavior) in one place.
- Test on both OSes for any change touching platform, files, input, audio or rendering.

## 9. Performance discipline

- Record a baseline before optimizing; cite `benchmarks/results/` in the change.
- No speculative optimization, no custom allocator for a structure whose cost was never measured.
- New features that run per frame get a budget line in `docs/performance.md`.
- Prefer simpler code until a measurement says otherwise.

## 10. Documentation obligations

- `docs/status.md`: updated every work session (milestone, next tasks, blockers, benchmark snapshot).
- ADR: any cross-cutting or hard-to-reverse decision.
- `docs/roadmap.md`: when scope, ordering or exit criteria change.
- `docs/architecture.md`: when module boundaries or core concepts change.
- `CHANGELOG.md`: every user-visible change.
- Public headers: doc comments that match behavior.

## 11. Definition of done

A change is done when all apply:

- [ ] Builds on Windows and Linux, zero warnings; architecture matches `docs/architecture.md`.
- [ ] Unit/integration tests added or updated and passing; determinism suite unaffected.
- [ ] No new per-frame allocations; tracking allocator clean; sanitizer run clean where applicable.
- [ ] Benchmarks unaffected, or the difference is measured and explained.
- [ ] Public API/format changes versioned, documented, migrated, changelogged.
- [ ] `docs/status.md` updated; related docs consistent.
- [ ] No new dependency without an ADR and license record.
- [ ] Verification reported honestly: what ran, what did not, on which platforms.

## 12. Non-negotiables

- Never commit build output, dumps, large binaries, secrets, or assets without clear rights.
- Never claim cross-platform support from a single-platform build.
- Never silence a failing test or warning to make CI green.
- Never rewrite history to hide a regression.
