#include "framework.hpp"

#include <cmath>

#include "peri/core/math.hpp"

TEST_CASE(math_vec2_arithmetic) {
    using peri::Vec2;

    const Vec2 a{3.0f, 4.0f};
    const Vec2 b{1.0f, 2.0f};

    CHECK((a + b) == (Vec2{4.0f, 6.0f}));
    CHECK((a - b) == (Vec2{2.0f, 2.0f}));
    CHECK((a * 2.0f) == (Vec2{6.0f, 8.0f}));
    CHECK((2.0f * a) == (Vec2{6.0f, 8.0f}));
    CHECK((a / 2.0f) == (Vec2{1.5f, 2.0f}));
    CHECK((-a) == (Vec2{-3.0f, -4.0f}));

    Vec2 sum = a;
    sum += b;
    CHECK(sum == (Vec2{4.0f, 6.0f}));
    sum -= b;
    CHECK(sum == a);
    sum *= 0.5f;
    CHECK(sum == (Vec2{1.5f, 2.0f}));
}

TEST_CASE(math_vec2_length_and_normalize) {
    using peri::Vec2;

    const Vec2 v{3.0f, 4.0f};
    CHECK(peri::dot(v, v) == 25.0f);
    CHECK(peri::length_squared(v) == 25.0f);
    CHECK(std::fabs(peri::length(v) - 5.0f) < 1e-5f);
    CHECK(std::fabs(peri::distance(v, Vec2{0.0f, 0.0f}) - 5.0f) < 1e-5f);

    const Vec2 unit = peri::normalized(v);
    CHECK(std::fabs(peri::length(unit) - 1.0f) < 1e-5f);

    // Normalizing zero must not produce NaNs.
    const Vec2 zero = peri::normalized(Vec2{0.0f, 0.0f});
    CHECK(zero == (Vec2{0.0f, 0.0f}));
    CHECK(!std::isnan(zero.x));
    CHECK(!std::isnan(zero.y));

    const Vec2 midpoint = peri::lerp(Vec2{0.0f, 0.0f}, Vec2{10.0f, 20.0f}, 0.5f);
    CHECK(midpoint == (Vec2{5.0f, 10.0f}));
}

TEST_CASE(math_rect_half_open_semantics) {
    using peri::Rect;
    using peri::Vec2;

    const Rect r{10.0f, 20.0f, 30.0f, 40.0f};
    CHECK(r.right() == 40.0f);
    CHECK(r.bottom() == 60.0f);
    CHECK(r.center() == (Vec2{25.0f, 40.0f}));

    CHECK(r.contains(Vec2{10.0f, 20.0f}));
    CHECK(r.contains(Vec2{39.9f, 59.9f}));

    // Half-open: the right and bottom edges are outside.
    CHECK(!r.contains(Vec2{40.0f, 40.0f}));
    CHECK(!r.contains(Vec2{10.0f, 60.0f}));
    CHECK(!r.contains(Vec2{9.9f, 20.0f}));
}

TEST_CASE(math_rect_intersection) {
    using peri::Rect;

    const Rect a{0.0f, 0.0f, 10.0f, 10.0f};
    const Rect b{5.0f, 5.0f, 10.0f, 10.0f};
    const Rect touching{10.0f, 0.0f, 5.0f, 5.0f};
    const Rect apart{20.0f, 20.0f, 1.0f, 1.0f};

    CHECK(a.intersects(b));
    CHECK(!a.intersects(touching));  // touching edges have zero area
    CHECK(!a.intersects(apart));

    const Rect overlap = a.intersection(b);
    CHECK(overlap.x == 5.0f);
    CHECK(overlap.y == 5.0f);
    CHECK(overlap.w == 5.0f);
    CHECK(overlap.h == 5.0f);

    // A rect intersects itself and its own intersection is itself.
    const Rect self = a.intersection(a);
    CHECK(self.w == a.w);
    CHECK(self.h == a.h);
}

TEST_CASE(math_color) {
    const peri::Color opaque{1, 2, 3};
    CHECK(opaque.r == 1);
    CHECK(opaque.g == 2);
    CHECK(opaque.b == 3);
    CHECK(opaque.a == 255);

    const peri::Color translucent{1, 2, 3, 4};
    CHECK(translucent != opaque);
    CHECK(translucent == (peri::Color{1, 2, 3, 4}));
}
