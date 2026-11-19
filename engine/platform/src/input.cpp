#include "eng/platform/input.hpp"

#include "input_access.hpp"

namespace eng {
namespace {

bool valid_key(Key key) noexcept {
    return static_cast<usize>(key) < k_key_count;
}

bool valid_mouse(MouseButton button) noexcept {
    return static_cast<usize>(button) < k_mouse_button_count;
}

bool valid_gamepad_button(GamepadButton button) noexcept {
    return static_cast<usize>(button) < k_gamepad_button_count;
}

bool valid_gamepad_axis(GamepadAxis axis) noexcept {
    return static_cast<usize>(axis) < k_gamepad_axis_count;
}

}  // namespace

bool InputState::key_down(Key key) const noexcept {
    return valid_key(key) && key_down_[static_cast<usize>(key)];
}

bool InputState::key_pressed(Key key) const noexcept {
    return valid_key(key) && key_pressed_[static_cast<usize>(key)];
}

bool InputState::key_released(Key key) const noexcept {
    return valid_key(key) && key_released_[static_cast<usize>(key)];
}

bool InputState::mouse_down(MouseButton button) const noexcept {
    return valid_mouse(button) && mouse_down_[static_cast<usize>(button)];
}

bool InputState::mouse_pressed(MouseButton button) const noexcept {
    return valid_mouse(button) && mouse_pressed_[static_cast<usize>(button)];
}

bool InputState::mouse_released(MouseButton button) const noexcept {
    return valid_mouse(button) && mouse_released_[static_cast<usize>(button)];
}

bool InputState::gamepad_button_down(GamepadButton button) const noexcept {
    return gamepad_connected_ && valid_gamepad_button(button) &&
           gamepad_down_[static_cast<usize>(button)];
}

bool InputState::gamepad_button_pressed(GamepadButton button) const noexcept {
    return gamepad_connected_ && valid_gamepad_button(button) &&
           gamepad_pressed_[static_cast<usize>(button)];
}

bool InputState::gamepad_button_released(GamepadButton button) const noexcept {
    return gamepad_connected_ && valid_gamepad_button(button) &&
           gamepad_released_[static_cast<usize>(button)];
}

float InputState::gamepad_axis(GamepadAxis axis) const noexcept {
    return gamepad_connected_ && valid_gamepad_axis(axis) ? gamepad_axes_[static_cast<usize>(axis)]
                                                          : 0.0f;
}

namespace platform::detail {

void InputAccess::begin_frame(InputState& input) noexcept {
    input.key_pressed_.fill(false);
    input.key_released_.fill(false);
    input.mouse_pressed_.fill(false);
    input.mouse_released_.fill(false);
    input.gamepad_pressed_.fill(false);
    input.gamepad_released_.fill(false);
    input.mouse_delta_ = Vec2{};
    input.wheel_delta_ = 0.0f;
}

void InputAccess::set_key(InputState& input, Key key, bool down) noexcept {
    if (!valid_key(key)) {
        return;
    }
    const usize index = static_cast<usize>(key);
    if (down && !input.key_down_[index]) {
        input.key_pressed_[index] = true;
    }
    if (!down && input.key_down_[index]) {
        input.key_released_[index] = true;
    }
    input.key_down_[index] = down;
}

void InputAccess::release_all_keys(InputState& input) noexcept {
    for (usize i = 0; i < k_key_count; ++i) {
        if (input.key_down_[i]) {
            input.key_released_[i] = true;
            input.key_down_[i] = false;
        }
    }
    for (usize i = 0; i < k_mouse_button_count; ++i) {
        if (input.mouse_down_[i]) {
            input.mouse_released_[i] = true;
            input.mouse_down_[i] = false;
        }
    }
}

void InputAccess::mouse_motion(InputState& input, float x, float y) noexcept {
    const Vec2 position{x, y};
    input.mouse_delta_ += position - input.mouse_position_;
    input.mouse_position_ = position;
}

void InputAccess::mouse_button(InputState& input, MouseButton button, bool down) noexcept {
    if (!valid_mouse(button)) {
        return;
    }
    const usize index = static_cast<usize>(button);
    if (down && !input.mouse_down_[index]) {
        input.mouse_pressed_[index] = true;
    }
    if (!down && input.mouse_down_[index]) {
        input.mouse_released_[index] = true;
    }
    input.mouse_down_[index] = down;
}

void InputAccess::wheel(InputState& input, float delta) noexcept {
    input.wheel_delta_ += delta;
}

void InputAccess::set_gamepad_connected(InputState& input, bool connected) noexcept {
    if (input.gamepad_connected_ == connected) {
        return;
    }
    input.gamepad_connected_ = connected;
    if (!connected) {
        input.gamepad_down_.fill(false);
        input.gamepad_axes_.fill(0.0f);
        input.gamepad_released_.fill(false);
        input.gamepad_pressed_.fill(false);
    }
}

void InputAccess::set_gamepad_button(InputState& input, GamepadButton button, bool down) noexcept {
    if (!valid_gamepad_button(button)) {
        return;
    }
    const usize index = static_cast<usize>(button);
    if (down && !input.gamepad_down_[index]) {
        input.gamepad_pressed_[index] = true;
    }
    if (!down && input.gamepad_down_[index]) {
        input.gamepad_released_[index] = true;
    }
    input.gamepad_down_[index] = down;
}

void InputAccess::set_gamepad_axis(InputState& input, GamepadAxis axis, float value) noexcept {
    if (!valid_gamepad_axis(axis)) {
        return;
    }
    input.gamepad_axes_[static_cast<usize>(axis)] = value;
}

}  // namespace platform::detail
}  // namespace eng
