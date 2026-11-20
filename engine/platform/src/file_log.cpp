#include "peri/platform/file_log.hpp"

#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <system_error>
#include <utility>

namespace peri {

struct FileLog::Impl {
    std::filesystem::path path;
    std::ofstream file;
    usize maximum_bytes = 0;
    usize bytes = 0;
    unsigned backups = 0;
    bool failed = false;
    mutable std::mutex mutex;

    std::filesystem::path backup(unsigned index) const {
        auto result = path;
        result += "." + std::to_string(index);
        return result;
    }

    bool rotate() {
        file.close();
        if (file.fail()) {
            return false;
        }
        std::error_code error;
        std::filesystem::remove(backup(backups), error);
        if (error) {
            return false;
        }
        for (unsigned i = backups; i > 1; --i) {
            const auto source = backup(i - 1);
            const bool exists = std::filesystem::exists(source, error);
            if (error) {
                return false;
            }
            if (exists) {
                std::filesystem::rename(source, backup(i), error);
                if (error) {
                    return false;
                }
            }
        }
        std::filesystem::rename(path, backup(1), error);
        if (error) {
            return false;
        }
        file.open(path, std::ios::binary | std::ios::trunc);
        bytes = 0;
        return static_cast<bool>(file);
    }
};

FileLog::FileLog(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {}
FileLog::~FileLog() = default;

Result<std::unique_ptr<FileLog>> FileLog::open(std::string_view path, usize maximum_bytes,
                                               unsigned backups) {
    if (path.empty() || path.find('\0') != path.npos || maximum_bytes == 0 || backups == 0 ||
        backups > 99) {
        return Error{ErrorCode::invalid_argument, "invalid file log configuration"};
    }
    try {
        auto impl = std::make_unique<Impl>();
        std::u8string utf8;
        utf8.reserve(path.size());
        for (const char ch : path) {
            utf8.push_back(static_cast<char8_t>(static_cast<unsigned char>(ch)));
        }
        impl->path = std::filesystem::path(utf8);
        impl->maximum_bytes = maximum_bytes;
        impl->backups = backups;
        std::error_code error;
        const bool exists = std::filesystem::exists(impl->path, error);
        if (error) {
            std::fprintf(stderr, "file log: cannot inspect path\n");
            return Error{ErrorCode::io_failure, "cannot inspect log path"};
        }
        if (exists) {
            const auto size = std::filesystem::file_size(impl->path, error);
            if (error || size > static_cast<std::uintmax_t>(static_cast<usize>(-1))) {
                std::fprintf(stderr, "file log: cannot determine file size\n");
                return Error{ErrorCode::io_failure, "cannot determine log size"};
            }
            impl->bytes = static_cast<usize>(size);
        }
        impl->file.open(impl->path, std::ios::binary | std::ios::app);
        if (!impl->file) {
            std::fprintf(stderr, "file log: cannot open path\n");
            return Error{ErrorCode::io_failure, "cannot open log file"};
        }
        return std::unique_ptr<FileLog>(new FileLog(std::move(impl)));
    } catch (const std::bad_alloc&) {
        return Error{ErrorCode::out_of_memory, "cannot allocate file log"};
    } catch (const std::exception&) {
        std::fprintf(stderr, "file log: invalid path or file error\n");
        return Error{ErrorCode::io_failure, "invalid log path or file error"};
    }
}

Status FileLog::status() const noexcept {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->failed ? Status::failure(ErrorCode::io_failure, "file log write failed")
                         : Status::ok();
}

void FileLog::sink(const LogRecord& record, void* user) noexcept {
    auto& impl = *static_cast<FileLog*>(user)->impl_;
    std::lock_guard<std::mutex> lock(impl.mutex);
    std::fprintf(stderr, "[%9.3f] %-5s %s: %s\n", record.seconds, log_level_name(record.level),
                 record.category, record.message);
    if (impl.failed) {
        return;
    }
    // Categories are caller-owned and not necessarily bounded: bound the entire
    // file record and mark truncation instead of allocating during logging.
    char line[2048];
    const int length = std::snprintf(line, sizeof(line), "[%9.3f] %-5s %s: %s\n", record.seconds,
                                     log_level_name(record.level), record.category, record.message);
    if (length < 0) {
        impl.failed = true;
    } else {
        usize size = static_cast<usize>(length);
        if (size >= sizeof(line)) {
            constexpr char marker[] = "...[truncated]\n";
            size = sizeof(line) - 1;
            for (usize i = 0; i < sizeof(marker) - 1; ++i) {
                line[size - (sizeof(marker) - 1) + i] = marker[i];
            }
        }
        try {
            if (impl.bytes != 0 &&
                (impl.bytes >= impl.maximum_bytes || size > impl.maximum_bytes - impl.bytes) &&
                !impl.rotate()) {
                impl.failed = true;
            }
            if (!impl.failed) {
                impl.file.write(line, static_cast<std::streamsize>(size));
                impl.file.flush();
                if (!impl.file) {
                    impl.failed = true;
                } else {
                    impl.bytes += size;
                }
            }
        } catch (const std::exception&) {
            impl.failed = true;
        }
    }
    if (impl.failed) {
        std::fprintf(stderr, "file log: write or rotation failed; file logging disabled\n");
    }
}

}  // namespace peri
