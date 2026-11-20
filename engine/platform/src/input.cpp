#include "peri/platform/input.hpp"

#include "input_access.hpp"

namespace peri {
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
    return valid_gamepad_button(button) && gamepad_pressed_[static_cast<usize>(button)];
}

bool InputState::gamepad_button_released(GamepadButton button) const noexcept {
    return valid_gamepad_button(button) && gamepad_released_[static_cast<usize>(button)];
}

float InputState::gamepad_axis(GamepadAxis axis) const noexcept {
    return gamepad_connected_ && valid_gamepad_axis(axis) ? gamepad_axes_[static_cast<usize>(axis)]
                                                          : 0.0f;
}

void InputState::record(ButtonEvent::Device device, u16 code, bool down) noexcept {
    if (event_count_ == events_.size()) {
        overflow_ = true;
        return;
    }
    events_[event_count_++] = ButtonEvent{device, code, down};
}

namespace platform::detail {

void InputAccess::begin_frame(InputState& input) noexcept {
    input.event_count_ = 0;
    input.overflow_ = false;
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
    if (down != input.key_down_[index]) {
        input.record(ButtonEvent::Device::key, static_cast<u16>(key), down);
    }
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
            set_key(input, static_cast<Key>(i), false);
        }
    }
    for (usize i = 0; i < k_mouse_button_count; ++i) {
        if (input.mouse_down_[i]) {
            mouse_button(input, static_cast<MouseButton>(i), false);
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
    if (down != input.mouse_down_[index]) {
        input.record(ButtonEvent::Device::mouse, static_cast<u16>(button), down);
    }
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
        for (usize i = 0; i < k_gamepad_button_count; ++i) {
            if (input.gamepad_down_[i]) {
                set_gamepad_button(input, static_cast<GamepadButton>(i), false);
            }
        }
        input.gamepad_axes_.fill(0.0f);
    }
}

void InputAccess::set_gamepad_button(InputState& input, GamepadButton button, bool down) noexcept {
    if (!valid_gamepad_button(button)) {
        return;
    }
    const usize index = static_cast<usize>(button);
    if (down != input.gamepad_down_[index]) {
        input.record(ButtonEvent::Device::gamepad, static_cast<u16>(button), down);
    }
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
}  // namespace peri

namespace peri {
namespace {

// Compared via an explicit mapping: the two enums are independent types and
// must not silently depend on having identical numeric values.
bool same_device(ActionBinding::Device a, ButtonEvent::Device b) noexcept {
    switch (a) {
    case ActionBinding::Device::key:
        return b == ButtonEvent::Device::key;
    case ActionBinding::Device::mouse:
        return b == ButtonEvent::Device::mouse;
    case ActionBinding::Device::gamepad:
        return b == ButtonEvent::Device::gamepad;
    }
    return false;
}

bool valid_binding(ActionBinding binding) noexcept {
    switch (binding.device) {
    case ActionBinding::Device::key:
        return binding.code > 0 && binding.code < k_key_count;
    case ActionBinding::Device::mouse:
        return binding.code < k_mouse_button_count;
    case ActionBinding::Device::gamepad:
        return binding.code < k_gamepad_button_count;
    }
    return false;
}

}  // namespace

const ActionMap::Action* ActionMap::find(std::string_view name) const noexcept {
    for (usize i = 0; i < action_count_; ++i) {
        if (name == actions_[i].name.data()) {
            return &actions_[i];
        }
    }
    return nullptr;
}

Status ActionMap::bind(std::string_view name, ActionBinding binding) noexcept {
    if (name.empty() || name.size() > max_name_length || name.find('\0') != name.npos ||
        !valid_binding(binding)) {
        return Status::failure(ErrorCode::invalid_argument, "invalid action name or binding");
    }
    Action* action = nullptr;
    for (usize i = 0; i < action_count_; ++i) {
        if (name == actions_[i].name.data()) {
            action = &actions_[i];
            break;
        }
    }
    if (action == nullptr) {
        if (action_count_ == max_actions) {
            return Status::failure(ErrorCode::invalid_argument, "action capacity exceeded");
        }
        action = &actions_[action_count_++];
        name.copy(action->name.data(), name.size());
        action->name[name.size()] = '\0';
    }
    for (usize i = 0; i < action->binding_count; ++i) {
        if (action->bindings[i].device == binding.device &&
            action->bindings[i].code == binding.code) {
            return Status::failure(ErrorCode::invalid_argument, "duplicate action binding");
        }
    }
    if (action->binding_count == max_bindings_per_action) {
        return Status::failure(ErrorCode::invalid_argument, "action binding capacity exceeded");
    }
    action->bindings[action->binding_count++] = binding;
    return Status::ok();
}

Status ActionMap::update(const InputState& input) noexcept {
    if (input.overflow_) {
        return Status::failure(ErrorCode::internal_error, "input event buffer overflow");
    }
    for (usize event_index = 0; event_index < input.event_count_; ++event_index) {
        const ButtonEvent& event = input.events_[event_index];
        for (usize i = 0; i < action_count_; ++i) {
            Action& action = actions_[i];
            bool matched = false;
            for (usize j = 0; j < action.binding_count; ++j) {
                const ActionBinding binding = action.bindings[j];
                if (same_device(binding.device, event.device) && binding.code == event.code) {
                    action.binding_down[j] = event.down;
                    matched = true;
                }
            }
            if (!matched) {
                continue;
            }
            bool down = false;
            for (usize j = 0; j < action.binding_count; ++j) {
                down |= action.binding_down[j];
            }
            if (down != action.observed_down) {
                if (action.count == action.pending.size()) {
                    return Status::failure(ErrorCode::internal_error,
                                           "action event queue overflow");
                }
                action.pending[(action.head + action.count) % action.pending.size()] = down;
                ++action.count;
                action.observed_down = down;
            }
        }
    }
    return Status::ok();
}

void ActionMap::synchronize(const InputState& input) noexcept {
    for (usize i = 0; i < action_count_; ++i) {
        Action& action = actions_[i];
        bool held = false;
        for (usize j = 0; j < action.binding_count; ++j) {
            const ActionBinding binding = action.bindings[j];
            bool down = false;
            switch (binding.device) {
            case ActionBinding::Device::key:
                down = input.key_down(static_cast<Key>(binding.code));
                break;
            case ActionBinding::Device::mouse:
                down = input.mouse_down(static_cast<MouseButton>(binding.code));
                break;
            case ActionBinding::Device::gamepad:
                down = input.gamepad_button_down(static_cast<GamepadButton>(binding.code));
                break;
            }
            action.binding_down[j] = down;
            held |= down;
        }
        action.head = 0;
        action.count = 0;
        action.observed_down = held;
        action.pressed = false;
        action.released = false;
        if (held != action.down) {
            action.pending[0] = held;
            action.count = 1;
        }
    }
}

void ActionMap::begin_step() noexcept {
    for (usize i = 0; i < action_count_; ++i) {
        Action& action = actions_[i];
        action.pressed = false;
        action.released = false;
        if (action.count != 0) {
            action.down = action.pending[action.head];
            action.head = (action.head + 1) % action.pending.size();
            --action.count;
            action.pressed = action.down;
            action.released = !action.down;
        }
    }
}

bool ActionMap::down(std::string_view name) const noexcept {
    const Action* action = find(name);
    return action != nullptr && action->down;
}

bool ActionMap::pressed(std::string_view name) const noexcept {
    const Action* action = find(name);
    return action != nullptr && action->pressed;
}

bool ActionMap::released(std::string_view name) const noexcept {
    const Action* action = find(name);
    return action != nullptr && action->released;
}
}  // namespace peri
