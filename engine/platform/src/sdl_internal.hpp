#pragma once

// Internal hooks shared by the platform layer's translation units. Not installed
// and not part of the public surface.

namespace peri::platform::detail {

// Closes SDL devices owned by the platform layer (the selected gamepad) before
// SDL_Quit. Called by SdlSession when the last reference is released, because
// those handles must not outlive the library's own shutdown.
void shutdown_devices() noexcept;

}  // namespace peri::platform::detail
