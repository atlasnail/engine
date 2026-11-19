#include "framework.hpp"

#include <memory>
#include <utility>

#include "eng/platform/app.hpp"

TEST_CASE(app_rejects_invalid_timing) {
    eng::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.fixed_step_seconds = 0.0;

    const eng::Result<std::unique_ptr<eng::App>> app = eng::App::create(config);
    CHECK(app.is_error());
    CHECK(app.error().code == eng::ErrorCode::invalid_argument);
}

TEST_CASE(app_runs_frames_until_quit) {
    eng::AppConfig config;
    config.window.title = "engine app test";
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.window.resizable = false;
    config.limit_frames = false;

    eng::Result<std::unique_ptr<eng::App>> result = eng::App::create(config);
    if (result.is_error()) {
        SKIP_TEST("video driver unavailable");
    }

    std::unique_ptr<eng::App> app = std::move(result.value());
    eng::App* app_ptr = app.get();

    int updates = 0;
    int frames = 0;
    const int exit_code = app->run([&updates](double) { ++updates; }, [](double) {},
                                   [&frames, app_ptr](const eng::FrameInfo& info) {
                                       CHECK(info.frame_seconds >= 0.0);
                                       CHECK(info.fixed_steps >= 0);
                                       ++frames;
                                       if (frames >= 3) {
                                           app_ptr->quit();
                                       }
                                   });

    CHECK(exit_code == 0);
    CHECK(frames >= 3);
    CHECK(app->frame_count() >= 3);
    (void)updates;
}
