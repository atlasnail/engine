# ADR-0001: C++20 as the core language

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

The engine needs predictable performance, manual control of memory and layout, direct access to platform APIs, and long-term maintainability by a very small team (often one person plus AI agents). Candidate languages were C++20, C17, Rust, Odin and Zig.

- C17 is simple and stable but lacks the abstractions this codebase wants (RAII, spans, concepts, constexpr, generics without macros).
- Rust offers memory safety but raises the learning and agent-iteration cost, and interop with C/C++ middleware and graphics APIs adds friction.
- Odin/Zig have smaller ecosystems, fewer guarantees for MSVC support, and long-term risk for a multi-year project.
- C++20 is supported by MSVC, GCC and Clang on both targets, has mature tooling (clang-format/tidy, sanitizers), and is the lingua franca for game engines and graphics APIs.

## Decision

Use **C++20** for the engine, game code and tools. No compiler extensions. Prefer the standard library where it is not a performance or ABI hazard; use engine containers/allocators in hot paths.

## Consequences

- Broad tooling and hiring/agent familiarity; simplest integration with SDL, OpenGL and Lua.
- Manual memory safety remains our responsibility: invariants, handles, sanitizers and clear ownership rules compensate.
- Compile times and template complexity must be watched; keep headers lean and the public API narrow.
- A future language binding layer (C ABI, M8) is the answer for non-C++ users, not a rewrite.

## Revisit when

The engine cannot meet correctness or maintenance goals despite tooling, or C++20 support on a target platform becomes a practical blocker.
