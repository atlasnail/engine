#pragma once

#include <memory>
#include <string_view>

#include "peri/core/log.hpp"
#include "peri/core/result.hpp"
#include "peri/core/types.hpp"

namespace peri {

// An opt-in sink for set_log_sink(). It also writes each record to stderr.
// The file is flushed after every record, but this is not an fsync or a crash
// handler and does not guarantee persistence through power loss.
// Install with set_log_sink(&FileLog::sink, log.get()). Before destruction,
// restore the default sink and join all threads that might still be logging:
// set_log_sink does not wait for previously dispatched sink calls.
class FileLog {
public:
    // path is UTF-8. Up to backups old files are kept as path.1, path.2, ...;
    // maximum_bytes must be positive, backups must be 1..99. A single record
    // larger than the limit is stored intact in an otherwise empty file.
    [[nodiscard]] static Result<std::unique_ptr<FileLog>>
    open(std::string_view path, usize maximum_bytes, unsigned backups);
    ~FileLog();
    FileLog(const FileLog&) = delete;
    FileLog& operator=(const FileLog&) = delete;

    // A write or rotation failure is reported on stderr and retained here.
    [[nodiscard]] Status status() const noexcept;
    static void sink(const LogRecord& record, void* user) noexcept;

private:
    struct Impl;
    explicit FileLog(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> impl_;
};

}  // namespace peri
