#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct FileStreamRaw : virtual IOStreamRaw {
    Task<std::size_t> raw_read(std::span<char> buffer) override {
        co_return (co_await fs_read(mFile, buffer)).value_or(0);
    }

    Task<std::size_t> raw_write(std::span<char const> buffer) override {
        co_return (co_await fs_write(mFile, buffer)).value_or(0);
    }

    Task<> close() {
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

struct FileIStream : IStreamImpl<FileStreamRaw> {
    using IStreamImpl<FileStreamRaw>::IStreamImpl;

    static Task<Expected<FileIStream>> open(DirFilePath path) {
        co_return FileIStream(co_await co_await fs_open(path, OpenMode::Read));
    }
};

struct FileOStream : OStreamImpl<FileStreamRaw> {
    using OStreamImpl<FileStreamRaw>::OStreamImpl;

    static Task<Expected<FileOStream>> open(DirFilePath path, bool append = false) {
        co_return FileOStream(co_await co_await fs_open(path, append ? OpenMode::Append
                                                            : OpenMode::Write));
    }
};

struct FileStream : IOStreamImpl<FileStreamRaw> {
    using IOStreamImpl<FileStreamRaw>::IOStreamImpl;

    static Task<Expected<FileStream>> open(DirFilePath path) {
        co_return FileStream(co_await co_await fs_open(path, OpenMode::ReadWrite));
    }
};

inline Task<Expected<std::string>> file_read(DirFilePath path) {
    auto file = co_await co_await FileIStream::open(path);
    co_return co_await file.getall();
}

inline Task<Expected<>> file_write(DirFilePath path, std::string_view content) {
    auto file = co_await co_await FileOStream::open(path, false);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

inline Task<Expected<>> file_append(DirFilePath path, std::string_view content) {
    auto file = co_await co_await FileOStream::open(path, true);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

} // namespace co_async
