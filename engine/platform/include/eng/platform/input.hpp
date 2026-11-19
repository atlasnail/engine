#pragma once

// Input state for one frame.
//
// The engine exposes its own key/button enums; SDL scancodes and gamepad ids do
// not appear in this header, so a future backend change does not touch game
// code. State is updated by Window::pump_events() and read by the game:
//
//   window.pump_events(input);
//   if (input.key_pressed(Key::space)) { ... }
//
// `pressed` and `released` are true for exactly one pump, and mouse/wheel
// deltas reset each pump. `down` stays true until the key/button is released.

#include <array>

#include "eng/core/math.hpp"
#include "eng/core/types.hpp"

namespace eng::platform::detail {
struct InputAccess;
}

namespace eng {

enum class Key : u16 {
    unknown = 0,
    key_a,
    key_b,
    key_c,
    key_d,
    key_e,
    key_f,
    key_g,
    key_h,
    key_i,
    key_j,
    key_k,
    key_l,
    key_m,
    key_n,
    key_o,
    key_p,
    key_q,
    key_r,
    key_s,
    key_t,
    key_u,
    key_v,
    key_w,
    key_x,
    key_y,
    key_z,
    digit_0,
    digit_1,
    digit_2,
    digit_3,
    digit_4,
    digit_5,
    digit_6,
    digit_7,
    digit_8,
    digit_9,
    escape,
    space,
    enter,
    tab,
    backspace,
    left,
    right,
    up,
    down,
    left_shift,
    left_ctrl,
    left_alt,
    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,
    count,
};

enum class MouseButton : u8 {
    left = 0,
    right,
    middle,
    count,
};

enum class GamepadButton : u8 {
    south = 0,
    east,
    west,
    north,
    back,
    start,
    left_stick,
    right_stick,
    left_shoulder,
    right_shoulder,
    dpad_up,
    dpad_down,
    dpad_left,
    dpad_right,
    count,
};

enum class GamepadAxis : u8 {
    left_x = 0,
    left_y,
    right_x,
    right_y,
    left_trigger,
    right_trigger,
    count,
};

inline constexpr usize k_key_count = static_cast<usize>(Key::count);
inline constexpr usize k_mouse_button_count = static_cast<usize>(MouseButton::count);
inline constexpr usize k_gamepad_button_count = static_cast<usize>(GamepadButton::count);
inline constexpr usize k_gamepad_axis_count = static_cast<usize>(GamepadAxis::count);

class InputState {
public:
    [[nodiscard]] bool key_down(Key key) const noexcept;
    [[nodiscard]] bool key_pressed(Key key) const noexcept;
    [[nodiscard]] bool key_released(Key key) const noexcept;

    [[nodiscard]] bool mouse_down(MouseButton button) const noexcept;
    [[nodiscard]] bool mouse_pressed(MouseButton button) const noexcept;
    [[nodiscard]] bool mouse_released(MouseButton button) const noexcept;

    // Position in window points (DPI-independent), relative to the top-left.
    [[nodiscard]] Vec2 mouse_position() const noexcept { return mouse_position_; }
    // Movement since the previous pump.
    [[nodiscard]] Vec2 mouse_delta() const noexcept { return mouse_delta_; }
    // Vertical wheel movement since the previous pump; positive is away from the user.
    [[nodiscard]] float wheel_delta() const noexcept { return wheel_delta_; }

    [[nodiscard]] bool gamepad_connected() const noexcept { return gamepad_connected_; }
    [[nodiscard]] bool gamepad_button_down(GamepadButton button) const noexcept;
    [[nodiscard]] bool gamepad_button_pressed(GamepadButton button) const noexcept;
    [[nodiscard]] bool gamepad_button_released(GamepadButton button) const noexcept;
    // Sticks: -1..1. Triggers: 0..1. Zero when no gamepad is connected.
    [[nodiscard]] float gamepad_axis(GamepadAxis axis) const noexcept;

private:
    friend struct platform::detail::InputAccess;

    std::array<bool, k_key_count> key_down_{};
    std::array<bool, k_key_count> key_pressed_{};
    std::array<bool, k_key_count> key_released_{};
    std::array<bool, k_mouse_button_count> mouse_down_{};
    std::array<bool, k_mouse_button_count> mouse_pressed_{};
    std::array<bool, k_mouse_button_count> mouse_released_{};

    Vec2 mouse_position_{};
    Vec2 mouse_delta_{};
    float wheel_delta_ = 0.0f;

    bool gamepad_connected_ = false;
    std::array<bool, k_gamepad_button_count> gamepad_down_{};
    std::array<bool, k_gamepad_button_count> gamepad_pressed_{};
    std::array<bool, k_gamepad_button_count> gamepad_released_{};
    std::array<float, k_gamepad_axis_count> gamepad_axes_{};
};

}  // namespace eng
