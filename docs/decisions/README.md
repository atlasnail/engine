# Architecture decision records

Short records of decisions that are hard to reverse, cross-cutting, or likely to be questioned later. One file per decision, numbered and never renumbered or deleted. A superseded ADR stays and points to its replacement.

## Format

```
# ADR-000N: Title
Status: Proposed | Accepted | Rejected | Superseded by ADR-000M
Date: YYYY-MM-DD (date of the current status)
Context: the forcing problem, constraints, alternatives
Decision: what we will do
Consequences: benefits, costs, what becomes harder, follow-ups
Revisit when: the trigger that should reopen this
```

## Rules

- Proposed ADRs are **not** policy; do not build on them as if settled. Accepted
ADRs are binding until superseded.
- Accepted ADRs are binding until superseded.
- Changing an Accepted decision means a new ADR, not an edit of history (minor factual fixes are fine).
- Any new dependency, public format, public API tier, or platform boundary needs an ADR.
- Keep them under a page. Detail belongs in the architecture or spec docs.

## Index

| ADR | Title | Status |
| --- | --- | --- |
| [0001](0001-cpp20-core-language.md) | C++20 as the core language | Accepted |
| [0002](0002-sdl3-platform-layer.md) | SDL3 for the platform layer | Accepted |
| [0003](0003-opengl-33-renderer.md) | OpenGL 3.3 core as the first renderer | Accepted |
| [0004](0004-cmake-and-pinned-dependencies.md) | CMake presets and pinned dependencies | Accepted |
| [0005](0005-lua-scripting.md) | Lua 5.4 for gameplay scripting | Accepted |
| [0006](0006-save-format.md) | Versioned JSON saves with atomic writes | Accepted |
| [0007](0007-api-stability-and-deprecation.md) | API stability tiers and deprecation policy | Accepted |
| [0008](0008-error-model.md) | Error values at boundaries, no exceptions in public API | Accepted |
| [0009](0009-test-harness.md) | A minimal in-repository test harness | Accepted |
| [0010](0010-opengl-loader.md) | glad 2.0.8 as the pinned OpenGL 3.3 core loader | Accepted |
