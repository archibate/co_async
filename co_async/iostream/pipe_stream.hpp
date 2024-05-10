#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/threading/condition_variable.hpp>
#include <co_async/utils/concurrent_queue.hpp>

namespace co_async {

struct PipeStreamBuffer {
    ConcurrentQueue<std::string, (1 << 8) - 1> mChunks;
    ConditionList mNonEmpty;
    ConditionList mNonFull;
};

struct IPipeStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        while (true) {
            if (auto chunk = mPipe->mChunks.pop()) {
                mPipe->mNonFull.notify_one();
                auto n = std::min(buffer.size(), chunk->size());
                std::memcpy(buffer.data(), chunk->data(), n);
                co_return n;
            }
            co_await mPipe->mNonEmpty;
        }
    }

    Task<> raw_close() override {
        mPipe.reset();
        co_return;
    }

    explicit IPipeStream(std::shared_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::shared_ptr<PipeStreamBuffer> mPipe;
};

struct OPipeStream : Stream {
    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        if (auto p = mPipe.lock()) [[likely]] {
            if (buffer.empty()) [[unlikely]]
                co_return 0;
            while (!p->mChunks.push(std::string(buffer.data(), buffer.size())))
                [[unlikely]] {
                co_await p->mNonFull;
            }
            p->mNonEmpty.notify_one();
            co_return buffer.size();
        } else {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
        }
    }

    Task<> raw_close() override {
        if (auto p = mPipe.lock()) [[likely]] {
            while (!p->mChunks.push(std::string())) [[unlikely]] {
                co_await p->mNonFull;
            }
            p->mNonEmpty.notify_one();
        }
        mPipe.reset();
        co_return;
    }

    ~OPipeStream() override {
        if (auto p = mPipe.lock()) {
            (void)p->mChunks.push(std::string());
            p->mNonEmpty.notify_one();
        }
    }

    explicit OPipeStream(std::weak_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::weak_ptr<PipeStreamBuffer> mPipe;
};

inline Task<Expected<std::array<OwningStream, 2>>> pipe_stream() {
    auto pipePtr = std::make_shared<PipeStreamBuffer>();
    auto pipeWeakPtr = std::weak_ptr(pipePtr);
    co_return std::array{make_stream<IPipeStream>(std::move(pipePtr)),
                         make_stream<OPipeStream>(std::move(pipeWeakPtr))};
}

inline Task<Expected<>> pipe_forward(BorrowedStream &in, BorrowedStream &out) {
    while (true) {
        if (in.bufempty()) {
            if (!co_await in.fillbuf()) {
                break;
            }
        }
        auto n = co_await co_await out.write(in.peekbuf());
        if (n == 0) [[unlikely]] {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
        }
        in.seenbuf(n);
    }
    co_return {};
}

/* inline Task<Expected<std::string>> */
/* pipe_invoke_string(std::invocable<BorrowedStream &> auto func) { */
/*     auto [r, w] = co_await co_await pipe_stream(); */
/*     auto [a, b] = co_await when_all( */
/*         co_bind([func = std::move(func), */
/*                  w = std::move(w)]() mutable -> Task<Expected<>> { */
/*             co_await co_await std::invoke(func, w); */
/*             co_await co_await w.flush(); */
/*             co_await w.close(); */
/*             co_return {}; */
/*         }), */
/*         co_bind([r = std::move(r)]() mutable -> Task<Expected<std::string>> { */
/*             auto out = co_await r.getall(); */
/*             co_return std::move(out); */
/*         })); */
/*     co_await std::move(a); */
/*     co_return co_await std::move(b); */
/* } */
/*  */
/* inline Task<Expected<std::string>> */
/* pipe_invoke_string(std::invocable<BorrowedStream &, BorrowedStream &> auto func, */
/*                    std::string_view in) { */
/*     auto [r1, w1] = co_await co_await pipe_stream(); */
/*     auto [r2, w2] = co_await co_await pipe_stream(); */
/*     auto [a, b, c] = co_await when_all( */
/*         co_bind([in, w1 = std::move(w1)]() mutable -> Task<Expected<>> { */
/*             co_await co_await w1.puts(in); */
/*             co_await co_await w1.flush(); */
/*             co_await w1.close(); */
/*             co_return {}; */
/*         }), */
/*         co_bind([func = std::move(func), r1 = std::move(r1), */
/*                  w2 = std::move(w2)]() mutable -> Task<Expected<>> { */
/*             co_await co_await std::invoke(func, r1, w2); */
/*             co_await co_await w2.flush(); */
/*             co_await w2.close(); */
/*             co_return {}; */
/*         }), */
/*         co_bind([r2 = std::move(r2)]() mutable -> Task<Expected<std::string>> { */
/*             auto out = co_await r2.getall(); */
/*             co_return std::move(out); */
/*         })); */
/*     co_await std::move(a); */
/*     co_await std::move(b); */
/*     co_return co_await std::move(c); */
/* } */
/*  */
/* inline Task<Expected<OwningStream>> */
/* pipe_invoke(std::invocable<BorrowedStream &> auto func) { */
/*     auto [r, w] = co_await co_await pipe_stream(); */
/*     co_spawn([func = std::move(func), */
/*               w = std::move(w)]() mutable -> Task<Expected<>> { */
/*         co_await co_await std::invoke(func, w); */
/*         co_await co_await w.flush(); */
/*         co_await w.close(); */
/*         co_return {}; */
/*     }); */
/*     co_return {std::move(r)}; */
/* } */
/*  */
/* inline Task<Expected<OwningStream>> */
/* pipe_invoke(std::invocable<BorrowedStream &, BorrowedStream &> auto func, */
/*             BorrowedStream &in) { */
/*     return pipe_invoke( */
/*         std::bind(std::move(func), std::ref(in), std::placeholders::_1)); */
/* } */

template <class Func, class... Args>
    requires std::invocable<Func, Args..., OwningStream &>
inline Task<Expected<>> pipe_bind(OwningStream w, Func &&func, Args &&...args) {
    return co_bind(
        [func = std::forward<decltype(func)>(func),
         w = std::move(w)](auto &&...args) mutable -> Task<Expected<>> {
            auto e1 =
                co_await std::invoke(std::forward<decltype(func)>(func),
                                     std::forward<decltype(args)>(args)..., w);
            auto e2 = co_await w.flush();
            co_await w.close();
            co_await e1;
            co_await e2;
            co_return {};
        },
        std::forward<decltype(args)>(args)...);
}

} // namespace co_async
