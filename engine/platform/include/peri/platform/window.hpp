#pragma once

// Window and OS event handling.
//
// SDL is an implementation detail: this header does not include any SDL type.
// A window is created once and destroyed with its owner; pump_events() must be
// called once per frame and feeds InputState.
//
// The OpenGL context is created by the renderer in M2, not here, so window
// creation works on machines (and CI) without a GPU.

#include <memory>

#include "peri/core/math.hpp"
#include "peri/core/result.hpp"
#include "peri/platform/input.hpp"

namespace peri {

struct WindowConfig {
    const char* title = "Peridot Engine";
    int width = 1280;
    int height = 720;
    bool resizable = true;
    bool hidden = false;
    bool high_density = true;
    // Stored for the renderer, which applies it when it creates the GL context.
    bool vsync = true;
};

class Window {
public:
    // Fails with an Error if the video subsystem or the window cannot be
    // created. On headless CI, set SDL_VIDEODRIVER=dummy to use the dummy
    // driver; the failure is reported, not fatal to the process.
    static Result<std::unique_ptr<Window>> create(const WindowConfig& config);

    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    // Clears per-frame input, pumps every pending OS event, updates window
    // state and the provided input. Call exactly once per frame.
    void pump_events(InputState& input);

    [[nodiscard]] bool should_close() const noexcept;
    void request_close() noexcept;

    void set_title(const char* title);
    void toggle_fullscreen();

    // Pixel size is what the renderer draws into; point size is what the OS
    // reports for UI/mouse coordinates.
    [[nodiscard]] IVec2 size_pixels() const noexcept;
    [[nodiscard]] IVec2 size_points() const noexcept;
    [[nodiscard]] float display_scale() const noexcept;

    [[nodiscard]] bool is_minimized() const noexcept;
    [[nodiscard]] bool has_focus() const noexcept;
    [[nodiscard]] bool is_fullscreen() const noexcept;

    // Opaque native handle for the renderer backend (SDL_Window* today).
    [[nodiscard]] void* native_handle() const noexcept;

private:
    struct Impl;
    explicit Window(std::unique_ptr<Impl> impl) noexcept;

    std::unique_ptr<Impl> impl_;
};

}  // namespace peri
