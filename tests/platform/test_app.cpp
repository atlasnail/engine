#include "framework.hpp"

#include <atomic>
#include <chrono>
#include <limits>
#include <memory>
#include <thread>
#include <utility>

#include <SDL3/SDL.h>

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

TEST_CASE(app_pauses_zero_size_until_restored) {
    eng::AppConfig config;
    config.window.hidden = true;
    config.limit_frames = false;
    auto result = eng::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    auto* handle = static_cast<SDL_Window*>(app->window().native_handle());
    const SDL_WindowID id = SDL_GetWindowID(handle);

    SDL_Event event{};
    event.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    event.window.windowID = id;
    event.window.data1 = 0;
    event.window.data2 = 0;
    REQUIRE(SDL_PushEvent(&event));

    std::atomic<bool> restore_pushed{false};
    std::thread restore([id, &restore_pushed] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        SDL_Event restored{};
        restored.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
        restored.window.windowID = id;
        restored.window.data1 = 160;
        restored.window.data2 = 120;
        restore_pushed.store(SDL_PushEvent(&restored));
        if (!restore_pushed.load()) {
            SDL_Event quit{};
            quit.type = SDL_EVENT_QUIT;
            (void)SDL_PushEvent(&quit);
        }
    });

    int updates = 0;
    int renders = 0;
    double frame_seconds = -1.0;
    const auto started = std::chrono::steady_clock::now();
    const int code = app->run([&updates](double) { ++updates; }, [&renders](double) { ++renders; },
                              [&app, &frame_seconds](const eng::FrameInfo& info) {
                                  frame_seconds = info.frame_seconds;
                                  app->quit();
                              });
    restore.join();
    CHECK(restore_pushed.load());
    CHECK(std::chrono::steady_clock::now() - started >= std::chrono::milliseconds(150));
    CHECK(code == 0);
    CHECK(renders == 1);
    CHECK(updates == 0);
    CHECK(frame_seconds < 0.01);
    CHECK(app->dropped_steps() == 0);
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

TEST_CASE(app_stats_callback_enables_readout_without_flag) {
    eng::AppConfig config;
    config.window.hidden = true;
    config.limit_frames = false;
    REQUIRE(!config.frame_stats_overlay);
    auto result = eng::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    int calls = 0;
    const int code =
        app->run([](double) {}, [](double) {}, {},
                 [&calls, &app](const char* text) {
                     ++calls;
                     CHECK(std::string_view(text).find("FPS") != std::string_view::npos);
                     app->quit();
                     return eng::Status::ok();
                 });
    CHECK(code == 0);
    CHECK(calls == 1);
}
