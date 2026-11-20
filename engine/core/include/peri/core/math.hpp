#pragma once

// Small, dependency-free 2D math types.
//
// Conventions (to be confirmed at M2, see docs/architecture.md D3):
//   - World and screen definitions use y-down coordinates, matching how 2D art
//     and UI are authored.
//   - Rectangles are half-open: a rect contains x in [x, x+w) and y in [y, y+h).
//     Two rects "intersect" only when the overlap has positive area.
//   - Vec2 is 32-bit float. Simulation code must not depend on exact float
//     results across compilers; determinism tests hash state with documented
//     tolerances.

#include <cmath>

#include "peri/core/types.hpp"

namespace peri {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x_in, float y_in) : x(x_in), y(y_in) {}
};

constexpr Vec2 operator+(Vec2 a, Vec2 b) {
    return {a.x + b.x, a.y + b.y};
}
constexpr Vec2 operator-(Vec2 a, Vec2 b) {
    return {a.x - b.x, a.y - b.y};
}
constexpr Vec2 operator-(Vec2 v) {
    return {-v.x, -v.y};
}
constexpr Vec2 operator*(Vec2 v, float s) {
    return {v.x * s, v.y * s};
}
constexpr Vec2 operator*(float s, Vec2 v) {
    return v * s;
}
constexpr Vec2 operator/(Vec2 v, float s) {
    return {v.x / s, v.y / s};
}

constexpr Vec2& operator+=(Vec2& a, Vec2 b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}
constexpr Vec2& operator-=(Vec2& a, Vec2 b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
constexpr Vec2& operator*=(Vec2& v, float s) {
    v.x *= s;
    v.y *= s;
    return v;
}

constexpr bool operator==(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}
constexpr bool operator!=(Vec2 a, Vec2 b) {
    return !(a == b);
}

constexpr float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}
constexpr float length_squared(Vec2 v) {
    return dot(v, v);
}

inline float length(Vec2 v) {
    return std::sqrt(length_squared(v));
}
inline float distance(Vec2 a, Vec2 b) {
    return length(a - b);
}

// Returns {0, 0} for a zero-length vector instead of producing NaNs.
inline Vec2 normalized(Vec2 v) {
    const float len_sq = length_squared(v);
    if (len_sq <= 0.0f) {
        return {0.0f, 0.0f};
    }
    return v / std::sqrt(len_sq);
}

constexpr Vec2 lerp(Vec2 a, Vec2 b, float t) {
    return a + (b - a) * t;
}

struct IVec2 {
    i32 x = 0;
    i32 y = 0;

    constexpr IVec2() = default;
    constexpr IVec2(i32 x_in, i32 y_in) : x(x_in), y(y_in) {}

    constexpr bool operator==(const IVec2&) const = default;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x_in, float y_in, float w_in, float h_in)
        : x(x_in), y(y_in), w(w_in), h(h_in) {}

    [[nodiscard]] constexpr float right() const { return x + w; }
    [[nodiscard]] constexpr float bottom() const { return y + h; }
    [[nodiscard]] constexpr Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }

    [[nodiscard]] constexpr bool contains(Vec2 p) const {
        return p.x >= x && p.x < right() && p.y >= y && p.y < bottom();
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return x < other.right() && other.x < right() && y < other.bottom() && other.y < bottom();
    }

    // Only valid when intersects(other) is true.
    [[nodiscard]] constexpr Rect intersection(const Rect& other) const {
        const float nx = x > other.x ? x : other.x;
        const float ny = y > other.y ? y : other.y;
        const float nr = right() < other.right() ? right() : other.right();
        const float nb = bottom() < other.bottom() ? bottom() : other.bottom();
        return {nx, ny, nr - nx, nb - ny};
    }
};

struct Color {
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    constexpr Color() = default;
    constexpr Color(u8 r_in, u8 g_in, u8 b_in, u8 a_in = 255)
        : r(r_in), g(g_in), b(b_in), a(a_in) {}

    constexpr bool operator==(const Color&) const = default;
};

}  // namespace peri
