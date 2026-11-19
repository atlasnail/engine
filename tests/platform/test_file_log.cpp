#include "framework.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include "eng/core/log.hpp"
#include "eng/platform/file_log.hpp"
#include "eng/platform/fs.hpp"

TEST_CASE(file_log_rotates_and_flushes) {
    // Keep artifacts in the build/test working directory, not the system temp dir.
    const std::filesystem::path dir = "eng_file_log_test";
    std::error_code error;
    std::filesystem::remove_all(dir, error);
    REQUIRE(!error);
    std::filesystem::create_directory(dir, error);
    REQUIRE(!error);
    const std::string path = (dir / "engine.log").string();
    auto result = eng::FileLog::open(path, 80, 2);
    REQUIRE(result.is_ok());
    auto log = std::move(result.value());
    eng::set_log_sink(&eng::FileLog::sink, log.get());
    ENG_LOG_INFO("test", "first record with enough text to trigger rotation");
    ENG_LOG_INFO("test", "second record with enough text to trigger rotation");
    ENG_LOG_INFO("test", "third record with enough text to trigger rotation");
    eng::set_log_sink(nullptr, nullptr);
    CHECK(log->status().is_ok());
    log.reset();
    CHECK(std::filesystem::exists(dir / "engine.log"));
    CHECK(std::filesystem::exists(dir / "engine.log.1"));
    CHECK(std::filesystem::exists(dir / "engine.log.2"));
    const auto latest = eng::fs::read_text_file((dir / "engine.log").string());
    REQUIRE(latest.is_ok());
    CHECK(latest.value().find("third record") != std::string::npos);
    std::filesystem::remove_all(dir, error);
    CHECK(!error);
}

TEST_CASE(file_log_invalid_configuration) {
    CHECK(eng::FileLog::open("", 10, 1).is_error());
    CHECK(eng::FileLog::open("example.log", 0, 1).is_error());
    CHECK(eng::FileLog::open("example.log", 10, 0).is_error());
    CHECK(eng::FileLog::open("no-such-log-directory/engine.log", 10, 1).is_error());
}

TEST_CASE(file_log_reports_rotation_failure) {
    const std::filesystem::path dir = "eng_file_log_failure_test";
    std::error_code error;
    std::filesystem::remove_all(dir, error);
    REQUIRE(!error);
    std::filesystem::create_directories(dir / "engine.log.1", error);
    REQUIRE(!error);
    {
        std::ofstream blocker(dir / "engine.log.1" / "blocker");
        blocker << "do not remove";
    }
    auto result = eng::FileLog::open((dir / "engine.log").string(), 1, 1);
    REQUIRE(result.is_ok());
    auto log = std::move(result.value());
    eng::set_log_sink(&eng::FileLog::sink, log.get());
    ENG_LOG_INFO("test", "first");
    ENG_LOG_INFO("test", "rotation must fail");
    eng::set_log_sink(nullptr, nullptr);
    CHECK(log->status().is_error());
    log.reset();
    CHECK(std::filesystem::exists(dir / "engine.log.1" / "blocker"));
    std::filesystem::remove_all(dir, error);
    CHECK(!error);
}
