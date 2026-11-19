#pragma once

// Fixed-width aliases and small shared constants.
//
// Why fixed-width types: `int` and `long` differ between Windows and Linux
// (long is 32-bit on Windows and 64-bit on Linux). Using the fixed-width
// aliases everywhere below the platform layer keeps file formats, hashes and
// struct layouts identical on both targets.

#include <cstddef>
#include <cstdint>

namespace eng {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// Placeholder for "no index". Not a size, never used for arithmetic.
inline constexpr usize k_invalid_index = static_cast<usize>(-1);

}  // namespace eng
