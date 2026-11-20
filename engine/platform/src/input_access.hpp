#pragma once

// Internal bridge between the event pump and InputState. Not installed and not
// included outside engine/platform; game code uses the public accessors.

#include "peri/platform/input.hpp"

namespace peri::platform::detail {

struct InputAccess {
    static void begin_frame(InputState& input) noexcept;

    static void set_key(InputState& input, Key key, bool down) noexcept;
    static void release_all_keys(InputState& input) noexcept;

    static void mouse_motion(InputState& input, float x, float y) noexcept;
    static void mouse_button(InputState& input, MouseButton button, bool down) noexcept;
    static void wheel(InputState& input, float delta) noexcept;

    static void set_gamepad_connected(InputState& input, bool connected) noexcept;
    static void set_gamepad_button(InputState& input, GamepadButton button, bool down) noexcept;
    static void set_gamepad_axis(InputState& input, GamepadAxis axis, float value) noexcept;
};

}  // namespace peri::platform::detail
