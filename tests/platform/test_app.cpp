#include "framework.hpp"

#include <limits>
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

    config.fixed_step_seconds = std::numeric_limits<double>::quiet_NaN();
    CHECK(eng::App::create(config).is_error());
    config.fixed_step_seconds = 1.0 / 60.0;
    config.target_fps = std::numeric_limits<double>::infinity();
    CHECK(eng::App::create(config).is_error());
    config.target_fps = std::numeric_limits<double>::denorm_min();
    CHECK(eng::App::create(config).is_error());
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

TEST_CASE(app_frame_stats_overlay_runs) {
    eng::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.limit_frames = false;
    config.frame_stats_overlay = true;
    auto result = eng::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    const int code =
        app->run([](double) {}, [](double) {}, [&app](const eng::FrameInfo&) { app->quit(); });
    CHECK(code == 0);
    CHECK(app->frame_count() == 1);
}

TEST_CASE(app_stats_callback_shares_game_rendering) {
    eng::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.limit_frames = false;
    config.frame_stats_overlay = true;
    auto result = eng::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    int stage = 0;
    const int code =
        app->run([](double) {}, [&stage](double) { stage = 1; }, {},
                 [&stage, &app](const char* text) {
                     CHECK(stage == 1);
                     CHECK(std::string_view(text).find("FRAME") != std::string_view::npos);
                     stage = 2;
                     app->quit();
                     return eng::Status::ok();
                 });
    CHECK(code == 0);
    CHECK(stage == 2);
}

TEST_CASE(app_stats_callback_error_stops_loop) {
    eng::AppConfig config;
    config.window.hidden = true;
    config.frame_stats_overlay = true;
    config.limit_frames = false;
    auto result = eng::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    const int code =
        app->run([](double) {}, [](double) {}, {},
                 [](const char*) {
                     return eng::Status::failure(eng::ErrorCode::internal_error, "display failed");
                 });
    CHECK(code != 0);
}
