#include "eng/platform/app.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>

#include <SDL3/SDL.h>

#include "eng/core/assert.hpp"
#include "eng/core/fixed_step.hpp"
#include "eng/core/log.hpp"
#include "eng/core/time.hpp"
#include "eng/platform/sdl.hpp"
#include "input_access.hpp"

namespace eng {

App::App(std::unique_ptr<SdlSession> session, std::unique_ptr<Window> window,
         const AppConfig& config) noexcept
    : session_(std::move(session)), window_(std::move(window)), config_(config) {}

App::~App() {
    if (overlay_renderer_ != nullptr) {
        SDL_DestroyRenderer(static_cast<SDL_Renderer*>(overlay_renderer_));
    }
}

Result<std::unique_ptr<App>> App::create(const AppConfig& config) {
    if (!std::isfinite(config.fixed_step_seconds) || config.fixed_step_seconds <= 0.0) {
        return Error{ErrorCode::invalid_argument, "fixed step must be positive"};
    }
    if (config.max_steps_per_frame <= 0) {
        return Error{ErrorCode::invalid_argument, "max steps per frame must be positive"};
    }
    // The limiter converts the target period to integer nanoseconds. Reject
    // periods too large for that conversion, including positive subnormals.
    constexpr double minimum_fps =
        1'000'000'000.0 / static_cast<double>(std::numeric_limits<Uint64>::max());
    if (config.limit_frames &&
        (!std::isfinite(config.target_fps) || config.target_fps <= minimum_fps)) {
        return Error{ErrorCode::invalid_argument, "target fps must be positive when limiting"};
    }

    // The App owns the SDL session so a plain `App::create` works on its own;
    // callers that need SDL before or after the App acquire their own session
    // and the reference count keeps the library alive for the union.
    Result<SdlSession> session = SdlSession::acquire();
    if (session.is_error()) {
        return session.error();
    }
    auto held_session = std::make_unique<SdlSession>(std::move(session).value());

    Result<std::unique_ptr<Window>> window = Window::create(config.window);
    if (window.is_error()) {
        return window.error();
    }

    auto app =
        std::unique_ptr<App>(new App(std::move(held_session), std::move(window.value()), config));
    return app;
}

u64 App::dropped_steps() const noexcept {
    return dropped_steps_;
}

int App::run(const UpdateFn& fixed_update, const RenderFn& render, const FrameObserver& observer,
             const StatsDisplay& display) {
    ENG_ASSERT(!has_run_);
    if (has_run_) {
        return 1;
    }
    has_run_ = true;
    // A callback is itself a request for the readout; the flag only selects the
    // standalone software renderer when no callback can draw it.
    const bool show_stats = config_.frame_stats_overlay || static_cast<bool>(display);
    if (show_stats && !display) {
        overlay_renderer_ =
            SDL_CreateRenderer(static_cast<SDL_Window*>(window_->native_handle()), "software");
        if (overlay_renderer_ == nullptr) {
            ENG_LOG_ERROR("app", "could not create frame stats renderer: %s", SDL_GetError());
            return 1;
        }
    }

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
    bool paused_for_minimize = false;

    while (!window_->should_close() && !quit_requested_) {
        window_->pump_events(input_);
        const bool minimized = window_->is_minimized();
        if (minimized || paused_for_minimize) {
            // A pause has no fixed updates to drain the bounded action queues.
            // Retain final held state, but discard transient edges and deltas,
            // including input received in the restore pump.
            actions_.synchronize(input_);
            platform::detail::InputAccess::begin_frame(input_);
        } else {
            const Status input_status = actions_.update(input_);
            if (input_status.is_error()) {
                ENG_LOG_ERROR("app", "input delivery failed: %s", input_status.error().message);
                return 1;
            }
        }

        if (window_->should_close() || quit_requested_) {
            break;
        }
        if (minimized) {
            paused_for_minimize = true;
            // Continue pumping events while paused without busy-spinning, even
            // when the normal frame limiter is disabled.
            SDL_Delay(50);
            continue;
        }

        const std::chrono::steady_clock::time_point now = Clock::now();
        if (paused_for_minimize) {
            timestep.reset();
            previous = now;
            paused_for_minimize = false;
        }
        const std::chrono::duration<double> delta = now - previous;
        previous = now;

        const double frame_seconds = delta.count();

        eng::time::Stopwatch update_watch;
        const int steps = timestep.advance(frame_seconds);
        for (int step = 0; step < steps; ++step) {
            actions_.begin_step();
            fixed_update(config_.fixed_step_seconds);
        }
        const double update_seconds = update_watch.elapsed_seconds();

        render(timestep.alpha());

        dropped_steps_ = timestep.dropped_steps();

        if (show_stats) {
            // Average over a short window so the number is readable; the raw
            // per-frame time below still shows individual frames.
            stats_seconds_ += frame_seconds;
            ++stats_frames_;
            if (stats_seconds_ >= 0.5) {
                stats_fps_ = static_cast<double>(stats_frames_) / stats_seconds_;
                stats_seconds_ = 0.0;
                stats_frames_ = 0;
            }
            char text[256];
            std::snprintf(text, sizeof(text),
                          "FPS %.1f\nFRAME %.3f MS\nUPDATE %.3f MS\nSTEPS %d  DROPPED %llu",
                          stats_fps_, frame_seconds * 1000.0, update_seconds * 1000.0, steps,
                          static_cast<unsigned long long>(dropped_steps_));
            if (display) {
                const Status status = display(text);
                if (status.is_error()) {
                    ENG_LOG_ERROR("app", "frame stats display failed: %s", status.error().message);
                    return 1;
                }
            } else {
                auto* renderer = static_cast<SDL_Renderer*>(overlay_renderer_);
                if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) || !SDL_RenderClear(renderer) ||
                    !SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) ||
                    !SDL_RenderDebugText(renderer, 8.0f, 8.0f, text) ||
                    !SDL_RenderPresent(renderer)) {
                    ENG_LOG_ERROR("app", "frame stats overlay failed: %s", SDL_GetError());
                    return 1;
                }
            }
        }

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
