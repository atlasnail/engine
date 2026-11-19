#include "eng/platform/app.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>

#include <SDL3/SDL.h>

#include "eng/core/assert.hpp"
#include "eng/core/fixed_step.hpp"
#include "eng/core/log.hpp"
#include "eng/core/time.hpp"

namespace eng {

App::App(std::unique_ptr<Window> window, const AppConfig& config) noexcept
    : window_(std::move(window)), config_(config) {}

App::~App() = default;

Result<std::unique_ptr<App>> App::create(const AppConfig& config) {
    if (config.fixed_step_seconds <= 0.0) {
        return Error{ErrorCode::invalid_argument, "fixed step must be positive"};
    }
    if (config.max_steps_per_frame <= 0) {
        return Error{ErrorCode::invalid_argument, "max steps per frame must be positive"};
    }
    if (config.limit_frames && config.target_fps <= 0.0) {
        return Error{ErrorCode::invalid_argument, "target fps must be positive when limiting"};
    }

    Result<std::unique_ptr<Window>> window = Window::create(config.window);
    if (window.is_error()) {
        return window.error();
    }

    return std::unique_ptr<App>(new App(std::move(window.value()), config));
}

u64 App::dropped_steps() const noexcept {
    return dropped_steps_;
}

int App::run(const UpdateFn& fixed_update, const RenderFn& render, const FrameObserver& observer) {
    ENG_ASSERT(!has_run_);
    if (has_run_) {
        return 1;
    }
    has_run_ = true;

    FixedTimestep timestep(config_.fixed_step_seconds, config_.max_steps_per_frame);

    std::ofstream frame_log;
    if (config_.frame_log_path != nullptr) {
        frame_log.open(config_.frame_log_path, std::ios::trunc);
        if (frame_log) {
            frame_log << "frame,frame_ms,update_ms,fixed_steps\n";
        } else {
            ENG_LOG_WARN("app", "could not open frame log %s", config_.frame_log_path);
        }
    }

    using Clock = std::chrono::steady_clock;
    const std::chrono::duration<double> target_period{
        config_.target_fps > 0.0 ? 1.0 / config_.target_fps : 0.0};

    std::chrono::steady_clock::time_point previous = Clock::now();

    while (!window_->should_close() && !quit_requested_) {
        window_->pump_events(input_);

        const std::chrono::steady_clock::time_point now = Clock::now();
        const std::chrono::duration<double> delta = now - previous;
        previous = now;

        const double frame_seconds = delta.count();

        eng::time::Stopwatch update_watch;
        const int steps = timestep.advance(frame_seconds);
        for (int step = 0; step < steps; ++step) {
            fixed_update(config_.fixed_step_seconds);
        }
        const double update_seconds = update_watch.elapsed_seconds();

        render(timestep.alpha());

        dropped_steps_ = timestep.dropped_steps();

        if (observer) {
            observer(FrameInfo{frame_seconds, update_seconds, steps});
        }

        if (frame_log) {
            frame_log << frame_count_ << ',' << frame_seconds * 1000.0 << ','
                      << update_seconds * 1000.0 << ',' << steps << '\n';
        }

        ++frame_count_;

        if (config_.limit_frames) {
            const std::chrono::steady_clock::time_point after_work = Clock::now();
            const std::chrono::duration<double> work = after_work - now;
            if (work < target_period) {
                // SDL_DelayNS uses a high-resolution timer on Windows and a
                // normal sleep elsewhere, which paces better than sleep_for
                // with the default Windows timer resolution.
                const double remaining_seconds = (target_period - work).count();
                if (remaining_seconds > 0.0) {
                    SDL_DelayNS(static_cast<Uint64>(remaining_seconds * 1'000'000'000.0));
                }
            }
        }
    }

    return 0;
}

}  // namespace eng
