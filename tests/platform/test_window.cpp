#include "framework.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "eng/platform/window.hpp"

namespace {

// SDL_VIDEODRIVER=dummy is used on headless CI. getenv is deprecated by MSVC's
// CRT, so use its safer variant on Windows.
bool using_dummy_driver() {
#if defined(_WIN32)
    char* value = nullptr;
    size_t length = 0;
    if (_dupenv_s(&value, &length, "SDL_VIDEODRIVER") != 0 || value == nullptr) {
        return false;
    }
    const bool dummy = std::strcmp(value, "dummy") == 0;
    std::free(value);
    return dummy;
#else
    const char* value = std::getenv("SDL_VIDEODRIVER");
    return value != nullptr && std::strcmp(value, "dummy") == 0;
#endif
}

}  // namespace

TEST_CASE(window_rejects_invalid_size) {
    eng::WindowConfig config;
    config.width = 0;
    config.height = 100;

    const eng::Result<std::unique_ptr<eng::Window>> window = eng::Window::create(config);
    CHECK(window.is_error());
    CHECK(window.error().code == eng::ErrorCode::invalid_argument);
}

TEST_CASE(window_create_pump_and_close) {
    eng::WindowConfig config;
    config.title = "engine window test";
    config.width = 320;
    config.height = 240;
    config.hidden = true;
    config.resizable = false;
    config.vsync = false;

    eng::Result<std::unique_ptr<eng::Window>> result = eng::Window::create(config);
    if (result.is_error()) {
        SKIP_TEST("video driver unavailable");
    }

    std::unique_ptr<eng::Window> window = std::move(result.value());
    REQUIRE(window != nullptr);

    eng::InputState input;
    for (int frame = 0; frame < 3; ++frame) {
        window->pump_events(input);
    }

    CHECK(!window->should_close());

    // The dummy driver used on headless CI does not guarantee real sizes.
    const bool dummy_driver = using_dummy_driver();

    const eng::IVec2 size_points = window->size_points();
    const eng::IVec2 size_pixels = window->size_pixels();
    if (!dummy_driver) {
        CHECK(size_points.x > 0);
        CHECK(size_points.y > 0);
        CHECK(size_pixels.x > 0);
        CHECK(size_pixels.y > 0);
        CHECK(window->display_scale() > 0.0f);
    }

    // No keys are held in a headless test, so repeated pumping must not leave
    // pressed edges behind.
    window->pump_events(input);
    CHECK(!input.key_pressed(eng::Key::space));

    window->set_title("engine window test (renamed)");
    window->request_close();
    CHECK(window->should_close());
}
