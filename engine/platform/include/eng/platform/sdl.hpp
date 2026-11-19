#pragma once

// Process-wide SDL lifetime.
//
// SDL keeps global state: drivers must be initialised once, and SDL_Quit must
// run after the last window, gamepad and renderer is gone. This type makes that
// explicit and deterministic instead of relying on lazy initialisation that is
// never undone, which is what the platform layer did before M1 close-out.
//
// Sessions are reference counted because more than one owner can exist: an App
// owns one, and a test harness typically holds another for the whole process.
// The first acquire() initialises SDL; the last release quits it. Move-only.
// Acquire and release on the main thread.

#include "eng/core/result.hpp"

namespace eng {

class SdlSession {
public:
    // Initialises the video subsystem, which is required, and the gamepad
    // subsystem, which degrades to a warning when unavailable. Fails with an
    // Error if video initialisation fails, for example on a machine with no
    // display and no SDL_VIDEODRIVER override.
    static Result<SdlSession> acquire();

    // Releases this session; the last one shuts SDL down.
    ~SdlSession();

    SdlSession(const SdlSession&) = delete;
    SdlSession& operator=(const SdlSession&) = delete;
    SdlSession(SdlSession&& other) noexcept;
    SdlSession& operator=(SdlSession&& other) noexcept;

private:
    explicit SdlSession(bool holds_reference) noexcept : active_(holds_reference) {}

    void release() noexcept;

    bool active_ = false;
};

// True while at least one session holds a reference. Window creation requires
// it; it is exposed so callers can check their own lifetime ordering.
[[nodiscard]] bool sdl_session_active() noexcept;

}  // namespace eng
