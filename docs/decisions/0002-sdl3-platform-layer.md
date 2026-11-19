# ADR-0002: SDL3 for the platform layer

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

Window creation, input, gamepad hot-plug, clipboard, DPI, timers and audio devices differ across Windows, X11 and Wayland. Options:

1. **SDL3** behind a thin engine boundary.
2. Native Win32 + X11 + Wayland implementations, written and maintained by us.
3. GLFW for window/input plus separate audio/device code.
4. A heavier framework (Qt, SFML) that would blur the line between engine and dependency.

We are building an engine, not a platform abstraction library. Writing and maintaining three windowing/input/audio backends is weeks of work with little differentiation, and Wayland's input/IME rules are a moving target. GLFW lacks audio and gamepad database support. Heavier frameworks bring policy and dependencies we do not want.

## Decision

Use **SDL3** for windowing, input, gamepads, clipboard, high-resolution timers, threads and the audio device. Confine every SDL include and call to `engine/platform/`. The engine exposes its own types; no SDL type appears in public headers except as an opaque forward declaration if unavoidable.

Keep the boundary narrow enough that replacing SDL (or adding a native backend) is a bounded project rather than a rewrite.

## Consequences

- Fast path to correct cross-platform behavior, including Wayland, DPI and controller mappings.
- One well-maintained dependency with a permissive license (zlib); must be pinned and audited.
- SDL's audio callback model and device changes become our constraints; documented in the audio module.
- Slight binary size and startup cost; acceptable.
- We accept SDL's event and joystick semantics rather than inventing our own device model.

## Revisit when

SDL cannot deliver a required behavior (e.g. a needed audio backend or Wayland feature), SDL becomes unmaintained, or platform needs grow beyond what it covers.
