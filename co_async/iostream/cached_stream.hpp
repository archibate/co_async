#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct CachedStreamRaw : virtual IStreamRaw {
    Task<std::size_t> raw_read(std::span<char> buffer) override {
        if (mPos != mCache.size()) {
            auto n = std::min(mCache.size() - mPos, buffer.size());
            std::memcpy(buffer.data(), mCache.data() + mPos, n);
            mPos += n;
            co_return n;
        }
        auto n = co_await mStream->raw_read(buffer);
        mCache.append(buffer.data(), n);
        co_return n;
    }

    void raw_timeout(std::chrono::nanoseconds timeout) override {
        mStream->raw_timeout(timeout);
    }

    IStreamRaw *get() noexcept {
        return mStream;
    }

    explicit CachedStreamRaw(IStream *stream) : mStream(stream) {}

    Task<Expected<>> raw_seek(std::uint64_t pos) override {
        if (pos <= mCache.size()) {
            mPos = pos;
            co_return {};
        } else {
            co_return Unexpected{};
        }
    }

private:
    IStream *mStream;
    std::string mCache;
    std::size_t mPos = 0;
};

struct CachedStream : IStreamImpl<CachedStreamRaw> {
    using IStreamImpl<CachedStreamRaw>::IStreamImpl;
};

} // namespace co_async
