#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct FileStreamRaw : StreamRaw {
    Task<Expected<std::size_t, std::errc>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFile, buffer);
    }

    Task<Expected<std::size_t, std::errc>> raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFile, buffer);
    }

    Task<> raw_close() override {
        (co_await fs_close(std::move(mFile))).value_or();
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit FileStreamRaw(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

static Task<Expected<OwningStream, std::errc>> file_open(std::filesystem::path path, OpenMode mode) {
    co_return make_stream<FileStreamRaw>(co_await co_await fs_open(path, mode));
}

inline Task<Expected<std::string, std::errc>> file_read(std::filesystem::path path) {
    auto file = co_await co_await file_open(path, OpenMode::Read);
    co_return co_await file.getall();
}

inline Task<Expected<void, std::errc>> file_write(std::filesystem::path path, std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Write);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

inline Task<Expected<void, std::errc>> file_append(std::filesystem::path path, std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Append);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

} // namespace co_async
