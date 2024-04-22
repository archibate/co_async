#pragma once/*{export module co_async:iostream.file_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

struct PipeIStreamRaw : virtual IOStreamRaw {
    Task<std::size_t> raw_read(std::span<char> buffer) override {
        return fs_read(mFile, buffer);
    }

    Task<> close() {
        co_await fs_close(std::move(mFile));
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit PipeIStreamRaw(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

struct PipeOStreamRaw : virtual IOStreamRaw {
    Task<std::size_t> raw_write(std::span<char const> buffer) override {
        return fs_write(mFile, buffer);
    }

    Task<> close() {
        co_await fs_close(std::move(mFile));
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit PipeOStreamRaw(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

/*[export]*/ struct PipeIStream : IStreamImpl<PipeIStreamRaw> {
    using IStreamImpl<PipeIStreamRaw>::IStreamImpl;

    static Task<PipeIStream> open(DirFilePath path) {
        co_return PipeIStream(co_await fs_open(path, OpenMode::ReadWrite));
    }
};

} // namespace co_async
