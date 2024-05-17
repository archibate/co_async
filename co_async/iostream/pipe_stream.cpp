#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/utils/ring_queue.hpp>

namespace co_async {

namespace {

struct PipeStreamBuffer {
    InfinityQueue<std::string> mChunks;
    ConditionVariable mNonEmpty;
};

struct IPipeStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        while (true) {
            if (auto chunk = mPipe->mChunks.pop()) {
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
            if (buffer.empty()) [[unlikely]] {
                co_return std::size_t(0);
            }
            p->mChunks.push(std::string(buffer.data(), buffer.size()));
            p->mNonEmpty.notify_one();
            co_return buffer.size();
        } else {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
        }
    }

    Task<> raw_close() override {
        if (auto p = mPipe.lock()) {
            p->mChunks.push(std::string());
            p->mNonEmpty.notify_one();
        }
        co_return;
    }

    ~OPipeStream() override {
        if (auto p = mPipe.lock()) {
            p->mChunks.push(std::string());
            p->mNonEmpty.notify_one();
        }
    }

    explicit OPipeStream(std::weak_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::weak_ptr<PipeStreamBuffer> mPipe;
};

} // namespace

Task<Expected<std::array<OwningStream, 2>>> pipe_stream() {
    auto pipePtr = std::make_shared<PipeStreamBuffer>();
    auto pipeWeakPtr = std::weak_ptr(pipePtr);
    co_return std::array{make_stream<IPipeStream>(std::move(pipePtr)),
                         make_stream<OPipeStream>(std::move(pipeWeakPtr))};
}

Task<Expected<>> pipe_forward(BorrowedStream &in, BorrowedStream &out) {
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

} // namespace co_async
