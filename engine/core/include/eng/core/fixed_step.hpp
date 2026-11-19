#pragma once

// Fixed-step simulation timing.
//
// Rendering can run at whatever rate the display allows; gameplay must see a
// constant step so physics, animation and determinism behave predictably. This
// class turns a variable frame duration into a number of fixed steps plus an
// interpolation factor for rendering.
//
// Behavior:
//   - Negative or zero frame times produce no steps.
//   - Frame times are clamped to k_max_frame_seconds to avoid a huge catch-up
//     after a breakpoint or a stalled window drag.
//   - At most `max_steps_per_frame` steps run per call. Steps beyond that are
//     dropped and counted, never allowed to spiral.
//   - alpha() is the fraction of the next step already elapsed, in [0, 1).

#include "eng/core/assert.hpp"
#include "eng/core/types.hpp"

namespace eng {

class FixedTimestep {
public:
    static constexpr double k_default_step_seconds = 1.0 / 60.0;
    static constexpr int k_default_max_steps_per_frame = 8;
    static constexpr double k_max_frame_seconds = 0.25;

    explicit FixedTimestep(double step_seconds = k_default_step_seconds,
                           int max_steps_per_frame = k_default_max_steps_per_frame) noexcept
        : step_seconds_(step_seconds), max_steps_per_frame_(max_steps_per_frame) {
        ENG_ASSERT(step_seconds_ > 0.0);
        ENG_ASSERT(max_steps_per_frame_ > 0);
        if (!(step_seconds_ > 0.0)) {
            step_seconds_ = k_default_step_seconds;
        }
        if (max_steps_per_frame_ <= 0) {
            max_steps_per_frame_ = k_default_max_steps_per_frame;
        }
    }

    // Adds one frame's duration and returns how many fixed steps to run.
    int advance(double frame_seconds) noexcept {
        if (frame_seconds <= 0.0) {
            return 0;
        }
        if (frame_seconds > k_max_frame_seconds) {
            frame_seconds = k_max_frame_seconds;
        }

        accumulator_ += frame_seconds;

        int steps = static_cast<int>(accumulator_ / step_seconds_);
        if (steps <= 0) {
            alpha_ = accumulator_ / step_seconds_;
            return 0;
        }

        if (steps > max_steps_per_frame_) {
            dropped_steps_ += static_cast<u64>(steps - max_steps_per_frame_);
            steps = max_steps_per_frame_;
        }

        accumulator_ -= static_cast<double>(steps) * step_seconds_;

        // If the clamp dropped whole steps, remove them from the accumulator
        // too, otherwise the next frames keep trying to catch up.
        if (accumulator_ >= step_seconds_) {
            const u64 extra = static_cast<u64>(accumulator_ / step_seconds_);
            dropped_steps_ += extra;
            accumulator_ -= static_cast<double>(extra) * step_seconds_;
        }

        alpha_ = accumulator_ / step_seconds_;
        return steps;
    }

    // Drops any partial time (e.g. after a pause or a scene load).
    void reset() noexcept {
        accumulator_ = 0.0;
        alpha_ = 0.0;
    }

    [[nodiscard]] double step_seconds() const noexcept { return step_seconds_; }
    [[nodiscard]] double alpha() const noexcept { return alpha_; }
    [[nodiscard]] u64 dropped_steps() const noexcept { return dropped_steps_; }

private:
    double step_seconds_;
    double accumulator_ = 0.0;
    double alpha_ = 0.0;
    int max_steps_per_frame_;
    u64 dropped_steps_ = 0;
};

}  // namespace eng
