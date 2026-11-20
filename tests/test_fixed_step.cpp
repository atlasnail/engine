#include "framework.hpp"

#include "peri/core/fixed_step.hpp"

TEST_CASE(fixed_step_basic_accumulation) {
    peri::FixedTimestep timer(1.0 / 60.0);

    CHECK(timer.advance(0.0) == 0);
    CHECK(timer.advance(-1.0) == 0);

    // A frame shorter than one step runs nothing but reports progress.
    const int short_steps = timer.advance(0.5 / 60.0);
    CHECK(short_steps == 0);
    CHECK(timer.alpha() > 0.4);
    CHECK(timer.alpha() < 0.6);

    // The other half completes the step.
    CHECK(timer.advance(0.5 / 60.0) == 1);
    CHECK(timer.alpha() < 0.01);
}

TEST_CASE(fixed_step_multiple_steps_in_one_frame) {
    peri::FixedTimestep timer(0.01);
    CHECK(timer.advance(0.025) == 2);
    CHECK(timer.alpha() > 0.49);
    CHECK(timer.alpha() < 0.51);
}

TEST_CASE(fixed_step_drops_steps_instead_of_spiralling) {
    peri::FixedTimestep timer(1.0 / 60.0, 4);

    // 100 ms is six steps; only four may run and the rest are dropped.
    const int steps = timer.advance(0.1);
    CHECK(steps == 4);
    CHECK(timer.dropped_steps() >= 2);
    CHECK(timer.alpha() < 1.0);

    // The next normal frame must not try to catch up the dropped work.
    CHECK(timer.advance(1.0 / 60.0) == 1);
}

TEST_CASE(fixed_step_clamps_long_frames) {
    peri::FixedTimestep timer(1.0 / 60.0, 100);

    // 10 seconds is clamped to 250 ms = 15 steps, not 600.
    CHECK(timer.advance(10.0) == 15);
}

TEST_CASE(fixed_step_reset_clears_progress) {
    peri::FixedTimestep timer(0.01);
    CHECK(timer.advance(0.015) == 1);
    timer.reset();
    CHECK(timer.alpha() == 0.0);
    CHECK(timer.advance(0.005) == 0);
}
