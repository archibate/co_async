#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct IPipeStream : Stream {
    Task<Expected<std::size_t, std::errc>>
    raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFile, buffer);
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

    explicit IPipeStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

struct OPipeStream : Stream {
    Task<Expected<std::size_t, std::errc>>
    raw_write(std::span<char const> buffer) override {
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

    explicit OPipeStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

inline Task<Expected<std::array<OwningStream, 2>, std::errc>> pipe_stream() {
    auto [r, w] = co_await co_await fs_pipe();
    co_return std::array{make_stream<IPipeStream>(std::move(r)),
                         make_stream<OPipeStream>(std::move(w))};
}

} // namespace co_async
