#pragma once

// Application loop.
//
// Responsibilities: pump OS events, run fixed-step game updates, render with an
// interpolation factor, optionally limit the frame rate, and record one CSV row
// per frame. It owns the window and input state; game code supplies callbacks.
//
// The loop is deliberately small. Anything that runs per frame must already be
// allocated: callbacks and the optional observer are set up before run().

#include <functional>
#include <memory>

#include "peri/core/result.hpp"
#include "peri/core/types.hpp"
#include "peri/platform/input.hpp"
#include "peri/platform/window.hpp"

namespace peri {

class SdlSession;

struct AppConfig {
    WindowConfig window{};
    double fixed_step_seconds = 1.0 / 60.0;
    int max_steps_per_frame = 8;
    bool limit_frames = true;
    double target_fps = 60.0;
    // Optional CSV path: frame index, frame ms, update ms, fixed steps.
    const char* frame_log_path = nullptr;
    // Draw frame stats with a standalone software renderer when no display
    // callback is passed to run(). Passing a StatsDisplay callback enables the
    // readout by itself, so this flag never has to agree with it. Off by default.
    bool frame_stats_overlay = false;
    // Opt in to main-thread CPU accounting around event, update and render work.
    bool measure_frame_cpu = false;
};

struct FrameInfo {
    double frame_seconds = 0.0;
    double update_seconds = 0.0;
    int fixed_steps = 0;
    // Main-thread CPU time for this frame; zero when accounting is disabled.
    double cpu_seconds = 0.0;
};

class App {
public:
    using UpdateFn = std::function<void(double dt)>;
    using RenderFn = std::function<void(double interpolation_alpha)>;
    using FrameObserver = std::function<void(const FrameInfo& info)>;
    // Called after game rendering with short-lived text. Draw it using the game
    // renderer and present that renderer here. Return an error on failure.
    using StatsDisplay = std::function<Status(const char* text)>;

    static Result<std::unique_ptr<App>> create(const AppConfig& config);
    // Shuts SDL down when this is the last object holding a session.
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    [[nodiscard]] Window& window() noexcept { return *window_; }
    // Raw input is frame-scoped; use actions() in fixed updates.
    [[nodiscard]] InputState& input() noexcept { return input_; }
    [[nodiscard]] ActionMap& actions() noexcept { return actions_; }
    [[nodiscard]] u64 frame_count() const noexcept { return frame_count_; }
    [[nodiscard]] u64 dropped_steps() const noexcept;

    // Stops after the current frame. The window's close request also stops it.
    void quit() noexcept { quit_requested_ = true; }

    // Runs until the window closes or quit() is called. Can only be called once;
    // returns 0 on a normal exit, nonzero on input, display or CPU clock failure.
    // Passing a display callback enables the frame readout even when
    // frame_stats_overlay is false. In that mode the render callback must draw
    // without presenting; the display callback draws the text and presents.
    // Without a callback and with the flag set, a software renderer owns the window.
    // Minimized or zero-pixel windows pause updates and rendering while events
    // continue to be pumped; elapsed paused time and transient input edges are
    // discarded. On restore, actions reconcile to the latest held button state.
    int run(const UpdateFn& fixed_update, const RenderFn& render,
            const FrameObserver& observer = {}, const StatsDisplay& display = {});

private:
    App(std::unique_ptr<SdlSession> session, std::unique_ptr<Window> window,
        const AppConfig& config) noexcept;

    // Declared before the window so it is destroyed after it: SDL must outlive
    // every window, gamepad and renderer it owns.
    std::unique_ptr<SdlSession> session_;
    std::unique_ptr<Window> window_;
    InputState input_;
    ActionMap actions_;
    void* overlay_renderer_ = nullptr;  // SDL renderer, owned privately
    AppConfig config_;
    u64 frame_count_ = 0;
    double stats_seconds_ = 0.0;
    double stats_fps_ = 0.0;
    unsigned stats_frames_ = 0;
    bool quit_requested_ = false;
    bool has_run_ = false;
    u64 dropped_steps_ = 0;
};

}  // namespace peri
