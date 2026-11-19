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

#include "eng/core/result.hpp"
#include "eng/core/types.hpp"
#include "eng/platform/input.hpp"
#include "eng/platform/window.hpp"

namespace eng {

struct AppConfig {
    WindowConfig window{};
    double fixed_step_seconds = 1.0 / 60.0;
    int max_steps_per_frame = 8;
    bool limit_frames = true;
    double target_fps = 60.0;
    // Optional CSV path: frame index, frame ms, update ms, fixed steps.
    const char* frame_log_path = nullptr;
};

struct FrameInfo {
    double frame_seconds = 0.0;
    double update_seconds = 0.0;
    int fixed_steps = 0;
};

class App {
public:
    using UpdateFn = std::function<void(double dt)>;
    using RenderFn = std::function<void(double interpolation_alpha)>;
    using FrameObserver = std::function<void(const FrameInfo& info)>;

    static Result<std::unique_ptr<App>> create(const AppConfig& config);
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    [[nodiscard]] Window& window() noexcept { return *window_; }
    [[nodiscard]] InputState& input() noexcept { return input_; }
    [[nodiscard]] u64 frame_count() const noexcept { return frame_count_; }
    [[nodiscard]] u64 dropped_steps() const noexcept;

    // Stops after the current frame. The window's close request also stops it.
    void quit() noexcept { quit_requested_ = true; }

    // Runs until the window closes or quit() is called. Can only be called once;
    // returns 0 on a normal exit.
    int run(const UpdateFn& fixed_update, const RenderFn& render,
            const FrameObserver& observer = {});

private:
    App(std::unique_ptr<Window> window, const AppConfig& config) noexcept;

    std::unique_ptr<Window> window_;
    InputState input_;
    AppConfig config_;
    u64 frame_count_ = 0;
    bool quit_requested_ = false;
    bool has_run_ = false;
    u64 dropped_steps_ = 0;
};

}  // namespace eng
