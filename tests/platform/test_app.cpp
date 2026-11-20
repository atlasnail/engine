#include "framework.hpp"

#include <atomic>
#include <chrono>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <SDL3/SDL.h>

#include "peri/platform/app.hpp"

TEST_CASE(app_rejects_invalid_timing) {
    peri::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.fixed_step_seconds = 0.0;

    const peri::Result<std::unique_ptr<peri::App>> app = peri::App::create(config);
    CHECK(app.is_error());
    CHECK(app.error().code == peri::ErrorCode::invalid_argument);

    config.fixed_step_seconds = std::numeric_limits<double>::quiet_NaN();
    CHECK(peri::App::create(config).is_error());
    config.fixed_step_seconds = 1.0 / 60.0;
    config.target_fps = std::numeric_limits<double>::infinity();
    CHECK(peri::App::create(config).is_error());
    config.target_fps = std::numeric_limits<double>::denorm_min();
    CHECK(peri::App::create(config).is_error());
}

TEST_CASE(app_runs_frames_until_quit) {
    peri::AppConfig config;
    config.window.title = "engine app test";
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.window.resizable = false;
    config.limit_frames = false;

    peri::Result<std::unique_ptr<peri::App>> result = peri::App::create(config);
    if (result.is_error()) {
        SKIP_TEST("video driver unavailable");
    }

    std::unique_ptr<peri::App> app = std::move(result.value());
    peri::App* app_ptr = app.get();

    int updates = 0;
    int frames = 0;
    const int exit_code = app->run([&updates](double) { ++updates; }, [](double) {},
                                   [&frames, app_ptr](const peri::FrameInfo& info) {
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

TEST_CASE(app_frame_cpu_excludes_sleep) {
    peri::AppConfig config;
    config.window.hidden = true;
    config.limit_frames = false;
    config.measure_frame_cpu = true;
    auto result = peri::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    double cpu_seconds = -1.0;
    const auto start = std::chrono::steady_clock::now();
    const int code = app->run([](double) {},
                              [](double) {
                                  std::this_thread::sleep_for(std::chrono::milliseconds(150));
                                  const auto until = std::chrono::steady_clock::now() +
                                                     std::chrono::milliseconds(120);
                                  while (std::chrono::steady_clock::now() < until) {
                                  }
                              },
                              [&app, &cpu_seconds](const peri::FrameInfo& info) {
                                  cpu_seconds = info.cpu_seconds;
                                  app->quit();
                              });
    const double wall_seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    CHECK(code == 0);
    CHECK(cpu_seconds > 0.01);
    CHECK(cpu_seconds < wall_seconds - 0.08);
}

TEST_CASE(app_pauses_zero_size_until_restored) {
    peri::AppConfig config;
    config.window.hidden = true;
    config.limit_frames = false;
    auto result = peri::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    REQUIRE(app->actions().bind("jump", peri::ActionBinding::key(peri::Key::space)).is_ok());
    auto* handle = static_cast<SDL_Window*>(app->window().native_handle());
    const SDL_WindowID id = SDL_GetWindowID(handle);

    SDL_Event event{};
    event.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    event.window.windowID = id;
    event.window.data1 = 0;
    event.window.data2 = 0;
    REQUIRE(SDL_PushEvent(&event));

    std::mutex restore_mutex;
    bool restore_published = false;
    int callbacks_before_restore = 0;
    std::atomic<bool> input_pushed{true};
    std::atomic<bool> restore_pushed{false};
    std::thread restore([id, &restore_mutex, &restore_published, &restore_pushed, &input_pushed] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for (int i = 0; i < 301; ++i) {
            SDL_Event key{};
            key.type = (i % 2 == 0) ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
            key.key.windowID = id;
            key.key.scancode = SDL_SCANCODE_SPACE;
            if (!SDL_PushEvent(&key)) {
                input_pushed.store(false);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        SDL_Event restored{};
        restored.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
        restored.window.windowID = id;
        restored.window.data1 = 160;
        restored.window.data2 = 120;
        {
            std::lock_guard<std::mutex> lock(restore_mutex);
            restore_pushed.store(SDL_PushEvent(&restored));
            restore_published = restore_pushed.load();
        }
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
    const auto check_callback = [&restore_mutex, &restore_published, &callbacks_before_restore] {
        std::lock_guard<std::mutex> lock(restore_mutex);
        if (!restore_published) {
            ++callbacks_before_restore;
        }
    };
    const int code = app->run(
        [&updates, &check_callback](double) {
            ++updates;
            check_callback();
        },
        [&renders, &check_callback](double) {
            ++renders;
            check_callback();
        },
        [&app, &frame_seconds, &check_callback](const peri::FrameInfo& info) {
            check_callback();
            frame_seconds = info.frame_seconds;
            app->quit();
        });
    const auto run_finished = std::chrono::steady_clock::now();
    restore.join();
    CHECK(restore_pushed.load());
    CHECK(input_pushed.load());
    CHECK(run_finished - started >= std::chrono::milliseconds(150));
    CHECK(callbacks_before_restore == 0);
    CHECK(code == 0);
    CHECK(renders == 1);
    CHECK(updates == 0);
    CHECK(frame_seconds < 0.01);
    CHECK(app->dropped_steps() == 0);
    app->actions().begin_step();
    CHECK(app->actions().down("jump"));
    CHECK(app->actions().pressed("jump"));
    app->actions().begin_step();
    CHECK(app->actions().down("jump"));
    CHECK(!app->actions().pressed("jump"));
}

TEST_CASE(app_frame_stats_overlay_runs) {
    peri::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.limit_frames = false;
    config.frame_stats_overlay = true;
    auto result = peri::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    const int code =
        app->run([](double) {}, [](double) {}, [&app](const peri::FrameInfo&) { app->quit(); });
    CHECK(code == 0);
    CHECK(app->frame_count() == 1);
}

TEST_CASE(app_stats_callback_shares_game_rendering) {
    peri::AppConfig config;
    config.window.width = 160;
    config.window.height = 120;
    config.window.hidden = true;
    config.limit_frames = false;
    config.frame_stats_overlay = true;
    auto result = peri::App::create(config);
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
                     return peri::Status::ok();
                 });
    CHECK(code == 0);
    CHECK(stage == 2);
}

TEST_CASE(app_stats_callback_error_stops_loop) {
    peri::AppConfig config;
    config.window.hidden = true;
    config.frame_stats_overlay = true;
    config.limit_frames = false;
    auto result = peri::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    const int code = app->run([](double) {}, [](double) {}, {},
                              [](const char*) {
                                  return peri::Status::failure(peri::ErrorCode::internal_error,
                                                               "display failed");
                              });
    CHECK(code != 0);
}

TEST_CASE(app_stats_callback_enables_readout_without_flag) {
    peri::AppConfig config;
    config.window.hidden = true;
    config.limit_frames = false;
    REQUIRE(!config.frame_stats_overlay);
    auto result = peri::App::create(config);
    REQUIRE(result.is_ok());
    auto app = std::move(result.value());
    int calls = 0;
    const int code =
        app->run([](double) {}, [](double) {}, {},
                 [&calls, &app](const char* text) {
                     ++calls;
                     CHECK(std::string_view(text).find("FPS") != std::string_view::npos);
                     app->quit();
                     return peri::Status::ok();
                 });
    CHECK(code == 0);
    CHECK(calls == 1);
}
