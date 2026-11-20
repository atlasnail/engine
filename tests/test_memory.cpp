#include "framework.hpp"

#include <cstdint>
#include <string>

#include "peri/core/memory.hpp"

namespace {

bool is_aligned(const void* pointer, peri::usize alignment) {
    return (reinterpret_cast<peri::usize>(pointer) % alignment) == 0;
}

}  // namespace

TEST_CASE(memory_allocate_and_free_updates_stats) {
    using namespace peri;

    const mem::Stats before = mem::stats(mem::Tag::test);

    void* block = mem::allocate(128, mem::k_default_alignment, mem::Tag::test);
    REQUIRE(block != nullptr);
    CHECK(is_aligned(block, mem::k_default_alignment));

    const mem::Stats during = mem::stats(mem::Tag::test);
    CHECK(during.current_bytes == before.current_bytes + 128);
    CHECK(during.allocations == before.allocations + 1);

    mem::deallocate(block);

    const mem::Stats after = mem::stats(mem::Tag::test);
    CHECK(after.current_bytes == before.current_bytes);
    CHECK(after.frees == before.frees + 1);
}

TEST_CASE(memory_zero_and_null_are_noops) {
    using namespace peri;

    const mem::Stats before = mem::total_stats();

    CHECK(mem::allocate(0, mem::k_default_alignment, mem::Tag::test) == nullptr);
    mem::deallocate(nullptr);

    const mem::Stats after = mem::total_stats();
    CHECK(after.allocations == before.allocations);
    CHECK(after.frees == before.frees);
    CHECK(after.current_bytes == before.current_bytes);
}

TEST_CASE(memory_honors_requested_alignment) {
    using namespace peri;

    constexpr usize alignments[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};

    for (const usize alignment : alignments) {
        void* block = mem::allocate(64, alignment, mem::Tag::test);
        REQUIRE(block != nullptr);
        CHECK(is_aligned(block, alignment));

        // The payload must be writable for its full size.
        auto* bytes = static_cast<unsigned char*>(block);
        for (usize i = 0; i < 64; ++i) {
            bytes[i] = static_cast<unsigned char>(i);
        }
        CHECK(bytes[0] == 0);
        CHECK(bytes[63] == 63);

        mem::deallocate(block);
    }
}

TEST_CASE(memory_tags_are_independent) {
    using namespace peri;

    const mem::Stats scene_before = mem::stats(mem::Tag::scene);
    const mem::Stats audio_before = mem::stats(mem::Tag::audio);

    void* scene_block = mem::allocate(32, mem::k_default_alignment, mem::Tag::scene);
    REQUIRE(scene_block != nullptr);

    CHECK(mem::stats(mem::Tag::scene).current_bytes == scene_before.current_bytes + 32);
    CHECK(mem::stats(mem::Tag::audio).current_bytes == audio_before.current_bytes);

    mem::deallocate(scene_block);

    CHECK(mem::stats(mem::Tag::scene).current_bytes == scene_before.current_bytes);
}

TEST_CASE(memory_live_allocation_count_returns_to_baseline) {
    using namespace peri;

    const usize baseline = mem::live_allocations();

    void* a = mem::allocate(10, mem::k_default_alignment, mem::Tag::test);
    void* b = mem::allocate(1000, 64, mem::Tag::test);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(mem::live_allocations() == baseline + 2);

    mem::deallocate(a);
    mem::deallocate(b);
    CHECK(mem::live_allocations() == baseline);
}

TEST_CASE(memory_tag_names_are_stable) {
    CHECK(peri::mem::tag_name(peri::mem::Tag::render) != nullptr);
    CHECK(std::string(peri::mem::tag_name(peri::mem::Tag::audio)) == "audio");
    CHECK(std::string(peri::mem::tag_name(peri::mem::Tag::general)) == "general");
}
