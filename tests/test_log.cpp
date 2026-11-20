#include "framework.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "peri/core/log.hpp"

namespace {

struct Capture {
    static constexpr int k_capacity = 8;

    int count = 0;
    peri::LogLevel levels[k_capacity] = {};
    char category_text[k_capacity][64] = {};
    char messages[k_capacity][peri::k_max_message_length] = {};
    double seconds[k_capacity] = {};
};

void capture_sink(const peri::LogRecord& record, void* user_data) {
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
    ~SinkGuard() { peri::set_log_sink(nullptr, nullptr); }
};

}  // namespace

TEST_CASE(log_records_level_category_and_message) {
    Capture capture;
    peri::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    PERI_LOG_WARN("render", "sprite %d missing atlas %s", 7, "hero");

    REQUIRE(capture.count == 1);
    CHECK(capture.levels[0] == peri::LogLevel::warn);
    CHECK(std::string(capture.category_text[0]) == "render");
    CHECK(std::string(capture.messages[0]) == "sprite 7 missing atlas hero");
    CHECK(capture.seconds[0] >= 0.0);
}

TEST_CASE(log_multiple_records_are_ordered) {
    Capture capture;
    peri::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    PERI_LOG_INFO("core", "first");
    PERI_LOG_ERROR("core", "second");
    PERI_LOG_TRACE("core", "third");

    REQUIRE(capture.count == 3);
    CHECK(std::string(capture.messages[0]) == "first");
    CHECK(std::string(capture.messages[1]) == "second");
    CHECK(std::string(capture.messages[2]) == "third");
    CHECK(capture.levels[1] == peri::LogLevel::error);
    CHECK(capture.levels[2] == peri::LogLevel::trace);
}

TEST_CASE(log_truncates_long_messages_with_marker) {
    Capture capture;
    peri::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    const std::string long_text(2000, 'x');
    PERI_LOG_INFO("core", "%s", long_text.c_str());

    REQUIRE(capture.count == 1);
    const std::size_t length = std::strlen(capture.messages[0]);
    CHECK(length < peri::k_max_message_length);
    CHECK(length == peri::k_max_message_length - 1);
    CHECK(std::strstr(capture.messages[0], "...[truncated]") != nullptr);
}

TEST_CASE(log_handles_null_category) {
    Capture capture;
    peri::set_log_sink(capture_sink, &capture);
    SinkGuard guard;

    peri::log_message(peri::LogLevel::info, nullptr, "no category");

    REQUIRE(capture.count == 1);
    CHECK(std::string(capture.category_text[0]).empty());
    CHECK(std::string(capture.messages[0]) == "no category");
}

TEST_CASE(log_level_names) {
    CHECK(std::string(peri::log_level_name(peri::LogLevel::trace)) == "trace");
    CHECK(std::string(peri::log_level_name(peri::LogLevel::debug)) == "debug");
    CHECK(std::string(peri::log_level_name(peri::LogLevel::info)) == "info");
    CHECK(std::string(peri::log_level_name(peri::LogLevel::warn)) == "warn");
    CHECK(std::string(peri::log_level_name(peri::LogLevel::error)) == "error");
    CHECK(std::string(peri::log_level_name(peri::LogLevel::fatal)) == "fatal");
}

TEST_CASE(log_restores_default_sink) {
    Capture capture;
    peri::set_log_sink(capture_sink, &capture);
    CHECK(capture.count == 0);

    peri::set_log_sink(nullptr, nullptr);
    // With the default sink, nothing should reach our capture.
    PERI_LOG_INFO("core", "to stderr");
    CHECK(capture.count == 0);
}
