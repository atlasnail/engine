#include "framework.hpp"

#include <string>

#include "../../engine/platform/src/input_access.hpp"
#include "peri/platform/input.hpp"

using peri::platform::detail::InputAccess;

TEST_CASE(actions_queue_edges_across_frames_and_steps) {
    peri::ActionMap actions;
    peri::InputState input;
    REQUIRE(actions.bind("jump", peri::ActionBinding::key(peri::Key::space)).is_ok());
    REQUIRE(actions.bind("jump", peri::ActionBinding::mouse(peri::MouseButton::left)).is_ok());
    REQUIRE(actions.bind("jump", peri::ActionBinding::gamepad(peri::GamepadButton::south)).is_ok());
    CHECK(actions.bind("jump", peri::ActionBinding::key(peri::Key::space)).is_error());
    CHECK(actions.bind("", peri::ActionBinding::key(peri::Key::space)).is_error());
    CHECK(actions.bind("bad", peri::ActionBinding::key(peri::Key::unknown)).is_error());

    // Several pumps without a simulation update, including two complete taps.
    InputAccess::set_key(input, peri::Key::space, true);
    InputAccess::set_key(input, peri::Key::space, false);
    REQUIRE(actions.update(input).is_ok());
    InputAccess::begin_frame(input);
    InputAccess::set_key(input, peri::Key::space, true);
    InputAccess::set_key(input, peri::Key::space, false);
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
    peri::ActionMap actions;
    peri::InputState input;
    REQUIRE(actions.bind("fire", peri::ActionBinding::key(peri::Key::space)).is_ok());
    REQUIRE(actions.bind("fire", peri::ActionBinding::gamepad(peri::GamepadButton::south)).is_ok());
    InputAccess::set_gamepad_connected(input, true);
    InputAccess::set_key(input, peri::Key::space, true);
    InputAccess::set_gamepad_button(input, peri::GamepadButton::south, true);
    InputAccess::set_key(input, peri::Key::space, false);
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
    CHECK(input.gamepad_button_released(peri::GamepadButton::south));
    actions.begin_step();
    CHECK(actions.released("fire"));
    CHECK(!actions.down("fire"));
    actions.begin_step();
    CHECK(!actions.released("fire"));
}

TEST_CASE(actions_capacity_and_names) {
    peri::ActionMap actions;
    for (peri::usize i = 0; i < peri::ActionMap::max_actions; ++i) {
        REQUIRE(
            actions.bind(std::to_string(i), peri::ActionBinding::key(peri::Key::key_a)).is_ok());
    }
    CHECK(actions.bind("overflow", peri::ActionBinding::key(peri::Key::key_a)).is_error());
    CHECK(
        actions.bind(std::string(64, 'a'), peri::ActionBinding::key(peri::Key::key_a)).is_error());
    CHECK(actions.bind(std::string("a\0b", 3), peri::ActionBinding::key(peri::Key::key_a))
              .is_error());
}

TEST_CASE(actions_pause_synchronizes_without_stale_edges_or_overflow) {
    peri::ActionMap actions;
    peri::InputState input;
    REQUIRE(actions.bind("jump", peri::ActionBinding::key(peri::Key::space)).is_ok());
    REQUIRE(actions.bind("jump", peri::ActionBinding::mouse(peri::MouseButton::left)).is_ok());

    // More than one action queue's worth of transitions per pump and many
    // pumps: the last held state, not the event buffer, determines recovery.
    for (int frame = 0; frame < 140; ++frame) {
        InputAccess::begin_frame(input);
        for (int i = 0; i < 300; ++i) {
            InputAccess::set_key(input, peri::Key::space, (i % 2) == 0);
        }
        actions.synchronize(input);
    }
    actions.begin_step();
    CHECK(!actions.down("jump"));
    CHECK(!actions.pressed("jump"));
    CHECK(!actions.released("jump"));

    InputAccess::begin_frame(input);
    InputAccess::set_key(input, peri::Key::space, true);
    InputAccess::mouse_button(input, peri::MouseButton::left, true);
    actions.synchronize(input);
    actions.begin_step();
    CHECK(actions.down("jump"));
    CHECK(actions.pressed("jump"));
    InputAccess::begin_frame(input);
    InputAccess::set_key(input, peri::Key::space, false);
    actions.synchronize(input);
    actions.begin_step();
    CHECK(actions.down("jump"));
    CHECK(!actions.released("jump"));
    InputAccess::begin_frame(input);
    InputAccess::mouse_button(input, peri::MouseButton::left, false);
    actions.synchronize(input);
    actions.begin_step();
    CHECK(!actions.down("jump"));
    CHECK(actions.released("jump"));
    actions.begin_step();
    CHECK(!actions.released("jump"));

    // A pre-pause queued tap should not be replayed after synchronization.
    InputAccess::begin_frame(input);
    InputAccess::set_key(input, peri::Key::space, true);
    InputAccess::set_key(input, peri::Key::space, false);
    REQUIRE(actions.update(input).is_ok());
    actions.synchronize(input);
    actions.begin_step();
    CHECK(!actions.pressed("jump"));
    CHECK(!actions.released("jump"));
}

TEST_CASE(actions_report_both_buffer_overflows) {
    peri::ActionMap actions;
    peri::InputState input;
    REQUIRE(actions.bind("jump", peri::ActionBinding::key(peri::Key::space)).is_ok());
    for (int i = 0; i < 129; ++i) {
        InputAccess::begin_frame(input);
        InputAccess::set_key(input, peri::Key::space, (i % 2) == 0);
        const auto status = actions.update(input);
        if (i < 128) {
            REQUIRE(status.is_ok());
        } else {
            CHECK(status.is_error());
        }
    }
    peri::InputState burst;
    for (int i = 0; i < 257; ++i) {
        InputAccess::set_key(burst, peri::Key::space, (i % 2) == 0);
    }
    peri::ActionMap another;
    CHECK(another.update(burst).is_error());
}
