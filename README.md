# 2D Engine (working title)

A from-scratch, native **2D-only game engine** for **Windows and Linux**, built for tight control over frame time and memory use, high-quality audio, a simple art-asset workflow, and a codebase that humans and AI agents can maintain over years.

**Status: M0 implemented (core foundation).** `eng_core` builds and its test
suite passes on Windows; Linux is configured in CI but not yet verified. There is
no window, renderer or audio yet — M1 is next. See
[`docs/status.md`](docs/status.md) for the exact state.

## Quick start (Windows)

Open **Developer PowerShell for VS 2026**, then:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

From Git Bash, prefix commands with
`cmd //c 'tools\dev\msvc_env.bat <command>'`. On Linux, use the
`linux-debug` preset. New to C++ or this codebase? Read
[`docs/guide/README.md`](docs/guide/README.md) first.

If something fails, `docs/status.md` lists verified commands, versions and known
limitations; do not assume a command works just because it is written here.

## Goals

| Goal | What that means in practice |
| --- | --- |
| Cross-platform | First-class Windows 10/11 (x64) and Linux (x64, X11 + Wayland via SDL) builds from one CMake project. No platform-specific code outside `engine/platform`. |
| High FPS | Measured CPU frame budgets, fixed-step simulation, batched 2D rendering, zero steady-state allocations in the frame loop. |
| Low memory | Explicit allocators (arenas/pools), asset handles instead of raw pointers, hard budgets per subsystem, leak checking in CI. |
| Good-quality sound | 48 kHz stereo mixing, streamed music, no glitches: measured underrun and mixing-latency targets. |
| Easy art import | Drop files into the game's asset folder, reference by name; atlas/sheet descriptors in JSON; hot reload in development. |
| Language support | C++20 engine and game code (primary), Lua 5.4 gameplay scripting, and a stable C ABI so other languages can be bound later without touching the core. |
| Agent-friendly | `AGENTS.md`, architecture notes, ADRs, explicit invariants, and a live [`docs/status.md`](docs/status.md) handoff file. |
| Safe persistence | Versioned save files, atomic writes, migrations, corruption detection. |
| Upgradeable | Semantic versioning, API stability tiers, deprecation policy, changelog, example game as a compatibility canary. |

## Non-goals for 1.0

3D rendering, mobile/console/web targets, a full visual editor, built-in networking, a general rigid-body physics engine, and native hot-reload of game code. See [`docs/backlog.md`](docs/backlog.md) for deferred ideas.

## Read in this order

| Document | Purpose | Read when |
| --- | --- | --- |
| [`AGENTS.md`](AGENTS.md) | Rules, invariants and workflow for any agent or new contributor. | First. Always. |
| [`docs/status.md`](docs/status.md) | Where the project actually is right now, and the next tasks. | Before starting any work. |
| [`docs/guide/README.md`](docs/guide/README.md) | Beginner-friendly tour: build, code layout, the C++ concepts used. | New to C++ or this repo. |
| [`docs/roadmap.md`](docs/roadmap.md) | Milestones M0–M9, priorities, exit criteria, performance gates. | Planning work. |
| [`docs/architecture.md`](docs/architecture.md) | Module layout, dependency rules, core design concepts, open questions. | Designing or reviewing code. |
| [`docs/performance.md`](docs/performance.md) | Budgets, benchmark suite, measurement protocol, regression policy. | Adding features that touch frame time or memory. |
| [`docs/conventions.md`](docs/conventions.md) | C++/build/style/versioning/testing standards and the definition of done. | Writing code or config. |
| [`docs/decisions/`](docs/decisions/README.md) | Accepted and proposed architecture decision records (ADRs). | Before changing a cross-cutting choice. |
| [`docs/backlog.md`](docs/backlog.md) | Deliberately deferred ideas and the reasons. | Tempted to add scope. |

## How to resume this project

1. Read `AGENTS.md` and `docs/status.md`.
2. Confirm the current milestone in `docs/roadmap.md` and pick one task from its list.
3. Follow `docs/conventions.md`; record any cross-cutting choice as an ADR.
4. Update `docs/status.md` and, if targets moved, the relevant docs in the same change.
