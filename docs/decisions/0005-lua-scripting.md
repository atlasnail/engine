# ADR-0005: Lua 5.4 for gameplay scripting

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

The engine should support more than one language for 2D game development without multiplying engine maintenance. Options: no scripting (C++ only), Lua, JavaScript (quickjs/V8), Python, C#/Mono or embedded WASM.

Constraints: small footprint, fast iteration, easy embedding with a C API, deterministic enough for gameplay, no runtime download, permissive license, and low binding-maintenance cost for one person plus agents.

- **Lua 5.4** is tiny (~300 KB), has a clean C API, excellent embedding track record, a familiar game-scripting syntax, and permissive licensing. Its bytecode and GC are well understood; determinism is achievable with the engine providing RNG and time.
- **JavaScript engines** are heavier and introduce a large dependency and GC pressure.
- **Python/C#** require substantially more runtime surface and native interop work.
- **WASM** is capable but adds toolchain and ABI complexity far beyond scripting needs at this stage.

## Decision

Embed **Lua 5.4** (vendored, pinned) in M8 as the gameplay scripting language. Bindings go through an explicit, versioned script API; scripts receive a context table, never raw engine pointers, and every script error is isolated so the game keeps running. The C ABI (also M8) is the path for other languages.

## Consequences

- Fast gameplay iteration without recompiling; small footprint.
- One more dependency to audit and update; scripting adds a security/robustness surface (sandboxing, instruction limits).
- Two ways to write gameplay (C++ and Lua) must not diverge; document which layer owns what.
- Lua is not the engine's configuration language and not a replacement for C++ systems work.
- Hot reload of scripts is straightforward and expected; hot reload of native code is not in scope.

## Revisit when

Lua cannot express required gameplay patterns, performance of script-driven systems becomes a measured bottleneck with no mitigation, or binding maintenance outweighs the iteration benefit.
