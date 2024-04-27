#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct PipeIStreamRaw : virtual IStreamRaw {
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

struct PipeOStreamRaw : virtual OStreamRaw {
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

struct PipeIStream : IStreamImpl<PipeIStreamRaw> {
    using IStreamImpl<PipeIStreamRaw>::IStreamImpl;
};

struct PipeOStream : OStreamImpl<PipeOStreamRaw> {
    using OStreamImpl<PipeOStreamRaw>::OStreamImpl;
};

inline Task<std::tuple<PipeIStream, PipeOStream>> pipe_stream() {
    auto [r, w] = co_await fs_pipe();
    co_return {PipeIStream(std::move(r)), PipeOStream(std::move(w))};
}

} // namespace co_async
