#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/threading/future.hpp>
#include <co_async/threading/future_group.hpp>
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

inline Task<Expected<void, std::errc>> pipe_forward(BorrowedStream &in, BorrowedStream &out) {
    while (true) {
        if (in.bufempty()) {
            if (!co_await in.fillbuf()) {
                break;
            }
        }
        auto n = co_await co_await out.write(in.peekbuf());
        if (n == 0) [[unlikely]] {
            co_return Unexpected{std::errc::broken_pipe};
        }
        in.seenbuf(n);
    }
    co_return {};
}

inline Task<Expected<std::string, std::errc>>
pipe_invoke(std::string_view in,
     std::invocable<OwningStream &, OwningStream &> auto func) {
    auto [r1, w1] = co_await co_await pipe_stream();
    auto [r2, w2] = co_await co_await pipe_stream();
    auto [a, b, c] = co_await when_all(
        co_bind([in, w1 = std::move(
                         w1)]() mutable -> Task<Expected<void, std::errc>> {
            co_await co_await w1.puts(in);
            co_await co_await w1.flush();
            co_await w1.close();
            co_return {};
        }),
        co_bind([func = std::move(func), r1 = std::move(r1), w2 = std::move(w2)]() mutable
                -> Task<Expected<void, std::errc>> {
            co_await co_await std::invoke(func, r1, w2);
            co_await co_await w2.flush();
            co_await w2.close();
            co_return {};
        }),
        co_bind([r2 = std::move(
                     r2)]() mutable -> Task<Expected<std::string, std::errc>> {
            auto out = co_await r2.getall();
            co_return std::move(out);
        }));
    co_await std::move(a);
    co_await std::move(b);
    co_return co_await std::move(c);
}

inline Task<Expected<OwningStream, std::errc>>
pipe_invoke(FutureGroup &group, BorrowedStream &in,
     std::invocable<OwningStream &, OwningStream &> auto func) {
    auto [r, w] = co_await co_await pipe_stream();
    group.add([func = std::move(func), &in, w = std::move(w)]() mutable
                     -> Task<Expected<void, std::errc>> {
        co_await co_await std::invoke(func, in, w);
        co_await co_await w.flush();
        co_await w.close();
        co_return {};
    });
    co_return {std::move(r)};
}

inline Task<Expected<OwningStream, std::errc>>
pipe_invoke(FutureGroup &group, std::invocable<OwningStream &> auto func) {
    auto [r, w] = co_await co_await pipe_stream();
    group.add([func = std::move(func), w = std::move(w)]() mutable
                     -> Task<Expected<void, std::errc>> {
        co_await co_await std::invoke(func, w);
        co_await co_await w.flush();
        co_await w.close();
        co_return {};
    });
    co_return {std::move(r)};
}

} // namespace co_async
