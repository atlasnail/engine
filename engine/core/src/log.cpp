#include "peri/core/log.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>

#include "peri/core/time.hpp"

namespace peri {
namespace {

struct SinkState {
    LogSink fn = nullptr;
    void* user = nullptr;
};

std::mutex& sink_mutex() {
    static std::mutex mutex;
    return mutex;
}

SinkState& sink_state() {
    static SinkState state;
    return state;
}

void stderr_sink(const LogRecord& record, void* /*user*/) {
    std::fprintf(stderr, "[%9.3f] %-5s %s: %s\n", record.seconds, log_level_name(record.level),
                 record.category, record.message);
}

}  // namespace

void set_log_sink(LogSink sink, void* user_data) noexcept {
    std::lock_guard<std::mutex> lock(sink_mutex());
    sink_state().fn = sink;
    sink_state().user = user_data;
}

void log_message(LogLevel level, const char* category, const char* format, ...) noexcept {
    char buffer[k_max_message_length];
    buffer[0] = '\0';

    if (format != nullptr) {
        va_list args;
        va_start(args, format);
        const int needed = std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        if (needed < 0) {
            std::snprintf(buffer, sizeof(buffer), "<log format error>");
        } else if (static_cast<usize>(needed) >= sizeof(buffer)) {
            // Truncated. Mark it so nobody mistakes a partial message for the
            // whole thing. The marker always fits because we shorten the text
            // first; keep the buffer NUL-terminated.
            constexpr char kMarker[] = "...[truncated]";
            constexpr usize kMarkerLength = sizeof(kMarker) - 1;
            usize used = std::strlen(buffer);
            if (used > sizeof(buffer) - 1 - kMarkerLength) {
                used = sizeof(buffer) - 1 - kMarkerLength;
            }
            std::memcpy(buffer + used, kMarker, kMarkerLength);
            buffer[used + kMarkerLength] = '\0';
        }
    }

    const LogRecord record{
        level,
        category != nullptr ? category : "",
        buffer,
        time::now_seconds(),
    };

    // Copy the sink under the lock, then call it outside so a sink that logs
    // (or replaces itself) cannot deadlock. Record order may interleave between
    // threads, which is acceptable for diagnostics.
    SinkState state;
    {
        std::lock_guard<std::mutex> lock(sink_mutex());
        state = sink_state();
    }

    if (state.fn != nullptr) {
        state.fn(record, state.user);
    } else {
        stderr_sink(record, nullptr);
    }
}

const char* log_level_name(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::trace:
        return "trace";
    case LogLevel::debug:
        return "debug";
    case LogLevel::info:
        return "info";
    case LogLevel::warn:
        return "warn";
    case LogLevel::error:
        return "error";
    case LogLevel::fatal:
        return "fatal";
    }
    return "info";
}

}  // namespace peri
