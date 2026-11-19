#pragma once

// Logging.
//
// Design constraints:
//   - Allocation-free on the calling thread in normal use (fixed buffer).
//   - Safe to call from any thread; a mutex serializes records.
//   - The sink is replaceable so tests can capture output. The record passed to
//     a sink is only valid for the duration of the call.
//   - Messages longer than k_max_message_length are truncated with a marker.
//
// Formatting uses printf-style syntax. The compiler checks the format string
// where supported (GCC/Clang attribute).

#include "eng/core/types.hpp"

namespace eng {

enum class LogLevel : u8 {
    trace = 0,
    debug,
    info,
    warn,
    error,
    fatal,
};

struct LogRecord {
    LogLevel level = LogLevel::info;
    const char* category = "";
    const char* message = "";
    double seconds = 0.0;  // monotonic seconds since engine start
};

// Receives every formatted record. `user_data` is whatever was passed to
// set_log_sink(). Sinks must not throw and must not call log_message().
using LogSink = void (*)(const LogRecord& record, void* user_data);

inline constexpr usize k_max_message_length = 1024;

// Installs a sink. Passing nullptr restores the default stderr sink.
void set_log_sink(LogSink sink, void* user_data) noexcept;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
void log_message(LogLevel level, const char* category, const char* format, ...) noexcept;

[[nodiscard]] const char* log_level_name(LogLevel level) noexcept;

}  // namespace eng

// Convenience macros. The category is a short, stable string such as "core",
// "render" or "assets"; it is not localized and should not be built at runtime.
#define ENG_LOG_TRACE(category, ...)                                                               \
    ::eng::log_message(::eng::LogLevel::trace, category, __VA_ARGS__)
#define ENG_LOG_DEBUG(category, ...)                                                               \
    ::eng::log_message(::eng::LogLevel::debug, category, __VA_ARGS__)
#define ENG_LOG_INFO(category, ...) ::eng::log_message(::eng::LogLevel::info, category, __VA_ARGS__)
#define ENG_LOG_WARN(category, ...) ::eng::log_message(::eng::LogLevel::warn, category, __VA_ARGS__)
#define ENG_LOG_ERROR(category, ...)                                                               \
    ::eng::log_message(::eng::LogLevel::error, category, __VA_ARGS__)
#define ENG_LOG_FATAL(category, ...)                                                               \
    ::eng::log_message(::eng::LogLevel::fatal, category, __VA_ARGS__)
