#pragma once

// Tracked allocation primitives.
//
// Every engine allocation goes through this API so that per-subsystem memory
// use and leaks are visible. It is deliberately plain: a tag, a size, and an
// alignment. Higher-level allocators (arenas, pools) are built on top in M6.
//
// Contract:
//   - allocate(0, ...) returns nullptr and changes no counters.
//   - deallocate(nullptr) is a no-op.
//   - `alignment` must be a power of two and at least 1.
//   - A pointer returned by allocate() must be released exactly once with
//     deallocate(). Mixing with free()/delete is undefined behavior.
//   - Calls are thread-safe.
//
// The allocation header adds a small, constant overhead per allocation. Stats
// count the requested bytes only; header overhead is not included, and will be
// reported separately by debug tooling later.

#include <cstddef>

#include "eng/core/types.hpp"

namespace eng::mem {

enum class Tag : u8 {
    general = 0,
    engine,
    platform,
    render,
    audio,
    assets,
    scene,
    script,
    ui,
    tools,
    test,
    count,
};

inline constexpr usize k_default_alignment = alignof(std::max_align_t);

struct Stats {
    usize current_bytes = 0;
    usize peak_bytes = 0;
    u64 allocations = 0;
    u64 frees = 0;
};

// Returns nullptr on failure or when bytes == 0. An invalid alignment asserts
// in debug builds and returns nullptr.
void* allocate(usize bytes, usize alignment = k_default_alignment, Tag tag = Tag::general) noexcept;

void deallocate(void* pointer) noexcept;

// Statistics for a single tag, or for all tags combined.
[[nodiscard]] Stats stats(Tag tag) noexcept;
[[nodiscard]] Stats total_stats() noexcept;

// Number of allocations that have not been deallocated. Used by shutdown and
// leak checks; a nonzero value after tests is a bug.
[[nodiscard]] usize live_allocations() noexcept;

[[nodiscard]] const char* tag_name(Tag tag) noexcept;

}  // namespace eng::mem
