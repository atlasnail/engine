#include "eng/platform/sdl.hpp"

#include <mutex>

#include <SDL3/SDL.h>

#include "eng/core/log.hpp"
#include "sdl_internal.hpp"

namespace eng {
namespace {

// SDL exposes one global library state, so the reference count that guards it is
// also global. The mutex only serialises acquire/release; every other platform
// call is main-thread-only by contract.
std::mutex g_session_mutex;
int g_session_count = 0;

void initialise() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ENG_LOG_ERROR("platform", "SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
        return;
    }
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        // A machine without a controller subsystem is still usable; input falls
        // back to keyboard and mouse.
        ENG_LOG_WARN("platform", "gamepad subsystem unavailable: %s", SDL_GetError());
    }
}

}  // namespace

Result<SdlSession> SdlSession::acquire() {
    std::lock_guard<std::mutex> lock(g_session_mutex);
    if (g_session_count == 0) {
        initialise();
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
            // Returns the failure to the caller; the reason is already logged
            // with SDL's own message, which is more specific than any literal.
            return Error{ErrorCode::internal_error, "SDL video initialization failed"};
        }
    }
    ++g_session_count;
    return SdlSession(true);
}

SdlSession::SdlSession(SdlSession&& other) noexcept : active_(other.active_) {
    other.active_ = false;
}

SdlSession& SdlSession::operator=(SdlSession&& other) noexcept {
    if (this != &other) {
        release();
        active_ = other.active_;
        other.active_ = false;
    }
    return *this;
}

SdlSession::~SdlSession() {
    release();
}

void SdlSession::release() noexcept {
    if (!active_) {
        return;
    }
    active_ = false;

    std::lock_guard<std::mutex> lock(g_session_mutex);
    if (g_session_count == 0) {
        return;
    }
    --g_session_count;
    if (g_session_count != 0) {
        return;
    }
    // Device handles must go before the library they belong to.
    platform::detail::shutdown_devices();
    SDL_Quit();
}

bool sdl_session_active() noexcept {
    std::lock_guard<std::mutex> lock(g_session_mutex);
    return g_session_count > 0;
}

}  // namespace eng
