#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {
struct CachedStream : Stream {
    explicit CachedStream(BorrowedStream &stream) : mStream(stream) {}

    BorrowedStream &base() const noexcept {
        return mStream;
    }

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        if (mPos != mCache.size()) {
            auto n = std::min(mCache.size() - mPos, buffer.size());
            std::memcpy(buffer.data(), mCache.data() + mPos, n);
            mPos += n;
            co_return n;
        }
        auto n = co_await co_await mStream.read(buffer);
        mCache.append(buffer.data(), n);
        co_return n;
    }

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        mStream.timeout(timeout);
    }

    Task<> raw_close() override {
        return mStream.close();
    }

    Task<Expected<>> raw_flush() override {
        return mStream.flush();
    }

    Task<Expected<>> raw_seek(std::uint64_t pos) override {
        if (pos <= mCache.size()) {
            mPos = pos;
            co_return {};
        } else {
            co_return std::errc::invalid_seek;
        }
    }

private:
    BorrowedStream &mStream;
    std::string mCache;
    std::size_t mPos = 0;
};
} // namespace co_async
