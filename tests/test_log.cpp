#include "framework.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "eng/core/log.hpp"

namespace {

struct Capture {
    static constexpr int k_capacity = 8;

    int count = 0;
    eng::LogLevel levels[k_capacity] = {};
    char category_text[k_capacity][64] = {};
    char messages[k_capacity][eng::k_max_message_length] = {};
    double seconds[k_capacity] = {};
};

void capture_sink(const eng::LogRecord& record, void* user_data) {
    auto* capture = static_cast<Capture*>(user_data);
    if (capture->count >= Capture::k_capacity) {
        return;
    }

    const int index = capture->count;
    capture->levels[index] = record.level;
    std::snprintf(capture->category_text[index], sizeof(capture->category_text[index]), "%s",
                  record.category);
    std::snprintf(capture->messages[index], sizeof(capture->messages[index]), "%s", record.message);
    capture->seconds[index] = record.seconds;
    ++capture->count;
}

struct SinkGuard {
    ~SinkGuard() { eng::set_log_sink(nullptr, nullptr); }
};

}  // namespace

TEST_CASE(log_records_level_category_and_message) {
    Capture capture;
    eng::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    ENG_LOG_WARN("render", "sprite %d missing atlas %s", 7, "hero");

    REQUIRE(capture.count == 1);
    CHECK(capture.levels[0] == eng::LogLevel::warn);
    CHECK(std::string(capture.category_text[0]) == "render");
    CHECK(std::string(capture.messages[0]) == "sprite 7 missing atlas hero");
    CHECK(capture.seconds[0] >= 0.0);
}

TEST_CASE(log_multiple_records_are_ordered) {
    Capture capture;
    eng::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    ENG_LOG_INFO("core", "first");
    ENG_LOG_ERROR("core", "second");
    ENG_LOG_TRACE("core", "third");

    REQUIRE(capture.count == 3);
    CHECK(std::string(capture.messages[0]) == "first");
    CHECK(std::string(capture.messages[1]) == "second");
    CHECK(std::string(capture.messages[2]) == "third");
    CHECK(capture.levels[1] == eng::LogLevel::error);
    CHECK(capture.levels[2] == eng::LogLevel::trace);
}

TEST_CASE(log_truncates_long_messages_with_marker) {
    Capture capture;
    eng::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    const std::string long_text(2000, 'x');
    ENG_LOG_INFO("core", "%s", long_text.c_str());

    REQUIRE(capture.count == 1);
    const std::size_t length = std::strlen(capture.messages[0]);
    CHECK(length < eng::k_max_message_length);
    CHECK(length == eng::k_max_message_length - 1);
    CHECK(std::strstr(capture.messages[0], "...[truncated]") != nullptr);
}

TEST_CASE(log_handles_null_category) {
    Capture capture;
    eng::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    eng::log_message(eng::LogLevel::info, nullptr, "no category");

    REQUIRE(capture.count == 1);
    CHECK(std::string(capture.category_text[0]).empty());
    CHECK(std::string(capture.messages[0]) == "no category");
}

TEST_CASE(log_level_names) {
    CHECK(std::string(eng::log_level_name(eng::LogLevel::trace)) == "trace");
    CHECK(std::string(eng::log_level_name(eng::LogLevel::debug)) == "debug");
    CHECK(std::string(eng::log_level_name(eng::LogLevel::info)) == "info");
    CHECK(std::string(eng::log_level_name(eng::LogLevel::warn)) == "warn");
    CHECK(std::string(eng::log_level_name(eng::LogLevel::error)) == "error");
    CHECK(std::string(eng::log_level_name(eng::LogLevel::fatal)) == "fatal");
}

TEST_CASE(log_restores_default_sink) {
    Capture capture;
    eng::set_log_sink(capture_sink, &capture);
    CHECK(capture.count == 0);

    eng::set_log_sink(nullptr, nullptr);
    // With the default sink, nothing should reach our capture.
    ENG_LOG_INFO("core", "to stderr");
    CHECK(capture.count == 0);
}
