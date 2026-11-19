#include "framework.hpp"

#include <string>

#include "../../engine/platform/src/input_access.hpp"
#include "eng/platform/input.hpp"

using eng::platform::detail::InputAccess;

TEST_CASE(actions_queue_edges_across_frames_and_steps) {
    eng::ActionMap actions;
    eng::InputState input;
    REQUIRE(actions.bind("jump", eng::ActionBinding::key(eng::Key::space)).is_ok());
    REQUIRE(actions.bind("jump", eng::ActionBinding::mouse(eng::MouseButton::left)).is_ok());
    REQUIRE(actions.bind("jump", eng::ActionBinding::gamepad(eng::GamepadButton::south)).is_ok());
    CHECK(actions.bind("jump", eng::ActionBinding::key(eng::Key::space)).is_error());
    CHECK(actions.bind("", eng::ActionBinding::key(eng::Key::space)).is_error());
    CHECK(actions.bind("bad", eng::ActionBinding::key(eng::Key::unknown)).is_error());

    // Several pumps without a simulation update, including two complete taps.
    InputAccess::set_key(input, eng::Key::space, true);
    InputAccess::set_key(input, eng::Key::space, false);
    REQUIRE(actions.update(input).is_ok());
    InputAccess::begin_frame(input);
    InputAccess::set_key(input, eng::Key::space, true);
    InputAccess::set_key(input, eng::Key::space, false);
    REQUIRE(actions.update(input).is_ok());
    InputAccess::begin_frame(input);
    REQUIRE(actions.update(input).is_ok());
    CHECK(!actions.pressed("jump"));
    for (int i = 0; i < 2; ++i) {
        actions.begin_step();
        CHECK(actions.pressed("jump"));
        CHECK(actions.down("jump"));
        CHECK(!actions.released("jump"));
        actions.begin_step();
        CHECK(actions.released("jump"));
        CHECK(!actions.down("jump"));
        CHECK(!actions.pressed("jump"));
    }
    actions.begin_step();
    CHECK(!actions.pressed("jump"));
    CHECK(!actions.released("jump"));
    CHECK(!actions.down("missing"));
}

TEST_CASE(actions_binding_overlap_and_disconnect) {
    eng::ActionMap actions;
    eng::InputState input;
    REQUIRE(actions.bind("fire", eng::ActionBinding::key(eng::Key::space)).is_ok());
    REQUIRE(actions.bind("fire", eng::ActionBinding::gamepad(eng::GamepadButton::south)).is_ok());
    InputAccess::set_gamepad_connected(input, true);
    InputAccess::set_key(input, eng::Key::space, true);
    InputAccess::set_gamepad_button(input, eng::GamepadButton::south, true);
    InputAccess::set_key(input, eng::Key::space, false);
    REQUIRE(actions.update(input).is_ok());
    actions.begin_step();
    CHECK(actions.pressed("fire"));
    CHECK(actions.down("fire"));
    actions.begin_step();
    CHECK(!actions.pressed("fire"));
    CHECK(!actions.released("fire"));
    InputAccess::begin_frame(input);
    InputAccess::set_gamepad_connected(input, false);
    REQUIRE(actions.update(input).is_ok());
    CHECK(input.gamepad_button_released(eng::GamepadButton::south));
    actions.begin_step();
    CHECK(actions.released("fire"));
    CHECK(!actions.down("fire"));
    actions.begin_step();
    CHECK(!actions.released("fire"));
}

TEST_CASE(actions_capacity_and_names) {
    eng::ActionMap actions;
    for (eng::usize i = 0; i < eng::ActionMap::max_actions; ++i) {
        REQUIRE(actions.bind(std::to_string(i), eng::ActionBinding::key(eng::Key::key_a)).is_ok());
    }
    CHECK(actions.bind("overflow", eng::ActionBinding::key(eng::Key::key_a)).is_error());
    CHECK(actions.bind(std::string(64, 'a'), eng::ActionBinding::key(eng::Key::key_a)).is_error());
    CHECK(
        actions.bind(std::string("a\0b", 3), eng::ActionBinding::key(eng::Key::key_a)).is_error());
}

TEST_CASE(actions_report_both_buffer_overflows) {
    eng::ActionMap actions;
    eng::InputState input;
    REQUIRE(actions.bind("jump", eng::ActionBinding::key(eng::Key::space)).is_ok());
    for (int i = 0; i < 129; ++i) {
        InputAccess::begin_frame(input);
        InputAccess::set_key(input, eng::Key::space, (i % 2) == 0);
        const auto status = actions.update(input);
        if (i < 128) {
            REQUIRE(status.is_ok());
        } else {
            CHECK(status.is_error());
        }
    }
    eng::InputState burst;
    for (int i = 0; i < 257; ++i) {
        InputAccess::set_key(burst, eng::Key::space, (i % 2) == 0);
    }
    eng::ActionMap another;
    CHECK(another.update(burst).is_error());
}
