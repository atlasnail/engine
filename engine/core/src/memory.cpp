#include "eng/core/memory.hpp"

#include <atomic>
#include <cstdlib>
#include <memory>
#include <new>

#include "eng/core/assert.hpp"

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace eng::mem {
namespace {

constexpr u32 k_magic = 0x454E4741u;  // 'ENGA'

// Stored immediately before every returned pointer. `raw` is what the system
// allocator returned; for over-aligned allocations the returned pointer sits
// after it, so freeing must use `raw`.
struct alignas(std::max_align_t) Header {
    void* raw = nullptr;
    usize size = 0;
    u32 magic = 0;
    Tag tag = Tag::general;
};

static_assert(sizeof(Header) % alignof(std::max_align_t) == 0,
              "Header size must preserve alignment of the payload");

constexpr usize k_tag_count = static_cast<usize>(Tag::count);

std::atomic<usize> g_current[k_tag_count];
std::atomic<usize> g_peak[k_tag_count];
std::atomic<u64> g_allocations[k_tag_count];
std::atomic<u64> g_frees[k_tag_count];

std::atomic<usize> g_total_current{0};
std::atomic<usize> g_total_peak{0};
std::atomic<u64> g_total_allocations{0};
std::atomic<u64> g_total_frees{0};

usize tag_index(Tag tag) noexcept {
    const usize index = static_cast<usize>(tag);
    ENG_ASSERT(index < k_tag_count);
    return index < k_tag_count ? index : 0;
}

void update_peak(std::atomic<usize>& peak, usize value) noexcept {
    usize previous = peak.load(std::memory_order_relaxed);
    while (value > previous &&
           !peak.compare_exchange_weak(previous, value, std::memory_order_relaxed,
                                       std::memory_order_relaxed)) {
        // `previous` is refreshed by compare_exchange_weak on failure.
    }
}

void* system_aligned_allocate(usize size, usize alignment) noexcept {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    // aligned_alloc requires the size to be a multiple of the alignment.
    const usize rounded = ((size + alignment - 1) / alignment) * alignment;
    return std::aligned_alloc(alignment, rounded);
#endif
}

void system_aligned_free(void* pointer) noexcept {
#if defined(_MSC_VER)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

bool is_power_of_two(usize value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

}  // namespace

void* allocate(usize bytes, usize alignment, Tag tag) noexcept {
    if (bytes == 0) {
        return nullptr;
    }

    ENG_ASSERT(is_power_of_two(alignment));
    if (!is_power_of_two(alignment)) {
        return nullptr;
    }

    void* raw = nullptr;
    void* payload = nullptr;

    if (alignment <= k_default_alignment) {
        const usize total = sizeof(Header) + bytes;
        raw = std::malloc(total);
        if (raw == nullptr) {
            return nullptr;
        }
        auto* header = ::new (raw) Header{};
        header->raw = raw;
        payload = static_cast<char*>(raw) + sizeof(Header);
    } else {
        // Reserve room for the header plus alignment slack, then place the
        // payload on the requested boundary. std::align keeps this as pointer
        // arithmetic instead of integer casts, which optimize better.
        const usize total = sizeof(Header) + bytes + alignment;
        raw = system_aligned_allocate(total, alignment);
        if (raw == nullptr) {
            return nullptr;
        }

        void* search = static_cast<char*>(raw) + sizeof(Header);
        usize space = total - sizeof(Header);
        void* aligned = std::align(alignment, bytes, search, space);
        ENG_ASSERT(aligned != nullptr);
        if (aligned == nullptr) {
            system_aligned_free(raw);
            return nullptr;
        }

        auto* header = reinterpret_cast<Header*>(static_cast<char*>(aligned) - sizeof(Header));
        header->raw = raw;
        payload = aligned;
    }

    auto* header = static_cast<Header*>(payload) - 1;
    header->size = bytes;
    header->magic = k_magic;
    header->tag = tag;

    const usize index = tag_index(tag);
    const usize current = g_current[index].fetch_add(bytes, std::memory_order_relaxed) + bytes;
    update_peak(g_peak[index], current);
    g_allocations[index].fetch_add(1, std::memory_order_relaxed);

    const usize total_current = g_total_current.fetch_add(bytes, std::memory_order_relaxed) + bytes;
    update_peak(g_total_peak, total_current);
    g_total_allocations.fetch_add(1, std::memory_order_relaxed);

    return payload;
}

void deallocate(void* pointer) noexcept {
    if (pointer == nullptr) {
        return;
    }

    auto* header = static_cast<Header*>(pointer) - 1;
    ENG_ASSERT(header->magic == k_magic);
    if (header->magic != k_magic) {
        // Corrupt or foreign pointer. Do not attempt to free it; the bug is
        // reported by the assertion above in debug builds.
        return;
    }

    const usize index = tag_index(header->tag);
    g_current[index].fetch_sub(header->size, std::memory_order_relaxed);
    g_frees[index].fetch_add(1, std::memory_order_relaxed);
    g_total_current.fetch_sub(header->size, std::memory_order_relaxed);
    g_total_frees.fetch_add(1, std::memory_order_relaxed);

    header->magic = 0;

    void* raw = header->raw;
    header->~Header();
    if (raw == static_cast<void*>(header)) {
        std::free(raw);
    } else {
        system_aligned_free(raw);
    }
}

Stats stats(Tag tag) noexcept {
    const usize index = tag_index(tag);
    Stats result;
    result.current_bytes = g_current[index].load(std::memory_order_relaxed);
    result.peak_bytes = g_peak[index].load(std::memory_order_relaxed);
    result.allocations = g_allocations[index].load(std::memory_order_relaxed);
    result.frees = g_frees[index].load(std::memory_order_relaxed);
    return result;
}

Stats total_stats() noexcept {
    Stats result;
    result.current_bytes = g_total_current.load(std::memory_order_relaxed);
    result.peak_bytes = g_total_peak.load(std::memory_order_relaxed);
    result.allocations = g_total_allocations.load(std::memory_order_relaxed);
    result.frees = g_total_frees.load(std::memory_order_relaxed);
    return result;
}

usize live_allocations() noexcept {
    const u64 allocated = g_total_allocations.load(std::memory_order_relaxed);
    const u64 freed = g_total_frees.load(std::memory_order_relaxed);
    return static_cast<usize>(allocated - freed);
}

const char* tag_name(Tag tag) noexcept {
    switch (tag) {
    case Tag::general:
        return "general";
    case Tag::engine:
        return "engine";
    case Tag::platform:
        return "platform";
    case Tag::render:
        return "render";
    case Tag::audio:
        return "audio";
    case Tag::assets:
        return "assets";
    case Tag::scene:
        return "scene";
    case Tag::script:
        return "script";
    case Tag::ui:
        return "ui";
    case Tag::tools:
        return "tools";
    case Tag::test:
        return "test";
    case Tag::count:
        break;
    }
    return "unknown";
}

}  // namespace eng::mem
