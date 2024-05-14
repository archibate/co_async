#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
inline constexpr std::size_t kStreamBufferSize = 8192;

struct Stream {
    virtual void raw_timeout(std::chrono::steady_clock::duration timeout) {}

    virtual Task<Expected<>> raw_seek(std::uint64_t pos) {
        co_return Unexpected{std::make_error_code(std::errc::invalid_seek)};
    }

    virtual Task<Expected<>> raw_flush() {
        co_return {};
    }

    virtual Task<> raw_close() {
        co_return;
    }

    virtual Task<Expected<std::size_t>> raw_read(std::span<char> buffer) {
        co_return Unexpected{std::make_error_code(std::errc::not_supported)};
    }

    virtual Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) {
        co_return Unexpected{std::make_error_code(std::errc::not_supported)};
    }

    Stream &operator=(Stream &&) = delete;
    virtual ~Stream() = default;
};

struct BorrowedStream {
    BorrowedStream() : mRaw() {}

    explicit BorrowedStream(Stream *raw) : mRaw(raw) {}

    virtual ~BorrowedStream() = default;
    BorrowedStream(BorrowedStream &&) = default;
    BorrowedStream &operator=(BorrowedStream &&) = default;

    Task<Expected<char>> getchar() {
        if (bufempty()) {
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        char c = mInBuffer[mInIndex];
        ++mInIndex;
        co_return c;
    }

    Task<Expected<>> getline(std::string &s, char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    s.append(mInBuffer.get() + start, i - start);
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            s.append(mInBuffer.get() + start, mInEnd - start);
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropline(char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getline(std::string &s, std::string_view eol) {
    again:
        co_await co_await getline(s, eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInIndex = 0;
                co_await co_await fillbuf();
            }
            char c = mInBuffer[mInIndex];
            if (eol[i] == c) [[likely]] {
                ++mInIndex;
            } else {
                s.append(eol.data(), i);
                goto again;
            }
        }
        co_return {};
    }

    Task<Expected<>> dropline(std::string_view eol) {
    again:
        co_await co_await dropline(eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInIndex = 0;
                co_await co_await fillbuf();
            }
            char c = mInBuffer[mInIndex];
            if (eol[i] == c) [[likely]] {
                ++mInIndex;
            } else {
                goto again;
            }
        }
        co_return {};
    }

    Task<Expected<std::string>> getline(char eol) {
        std::string s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<std::string>> getline(std::string_view eol) {
        std::string s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<>> getspan(std::span<char> s) {
        auto p = s.data();
        auto n = s.size();
        std::size_t start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                p = std::copy(mInBuffer.get() + start, mInBuffer.get() + end,
                              p);
                mInIndex = end;
                co_return {};
            }
            p = std::copy(mInBuffer.get() + start, mInBuffer.get() + mInEnd, p);
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropn(std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getn(std::string &s, std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                s.append(mInBuffer.get() + mInIndex, n);
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            s.append(mInBuffer.get() + mInIndex, m);
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<std::string>> getn(std::size_t n) {
        std::string s;
        s.reserve(n);
        co_await co_await getn(s, n);
        co_return s;
    }

    Task<> dropall() {
        do {
            mInIndex = 0;
        } while (co_await fillbuf());
    }

    Task<> getall(std::string &s) {
        std::size_t start = mInIndex;
        do {
            s.append(mInBuffer.get() + start, mInEnd - start);
            start = 0;
            mInIndex = 0;
        } while (co_await fillbuf());
    }

    Task<std::string> getall() {
        std::string s;
        co_await getall(s);
        co_return s;
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<>> getstruct(T &ret) {
        return getspan(std::span<char>((char *)&ret, sizeof(T)));
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<T>> getstruct() {
        T ret;
        co_await co_await getstruct(ret);
        co_return ret;
    }

    std::span<char const> peekbuf() const noexcept {
        return {mInBuffer.get() + mInIndex, mInEnd - mInIndex};
    }

    void seenbuf(std::size_t n) noexcept {
        mInIndex += n;
    }

    Task<Expected<std::string>> getchunk() noexcept {
        if (bufempty()) {
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        auto buf = peekbuf();
        std::string ret(buf.data(), buf.size());
        seenbuf(buf.size());
        co_return std::move(ret);
    }

    std::size_t tryread(std::span<char> buffer) {
        auto peekBuf = peekbuf();
        std::size_t n = std::min(buffer.size(), peekBuf.size());
        std::memcpy(buffer.data(), peekBuf.data(), n);
        seenbuf(n);
        return n;
    }

    Task<Expected<char>> peekchar() {
        if (bufempty()) {
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        co_return mInBuffer[mInIndex];
    }

    Task<Expected<>> peekn(std::string &s, std::size_t n) {
        while (mInEnd - mInIndex < n) {
            co_await co_await fillbuf();
        }
        s.append(mInBuffer.get() + mInIndex, n);
        co_return {};
    }

    Task<Expected<std::string>> peekn(std::size_t n) {
        std::string s;
        co_await co_await peekn(s, n);
        co_return s;
    }

    void allocinbuf(std::size_t size = kStreamBufferSize) {
        if (!mInBuffer) [[likely]] {
            mInBuffer = std::make_unique<char[]>(size);
            mInBufSize = size;
            mInIndex = 0;
            mInEnd = 0;
        }
    }

    Task<Expected<>> fillbuf() {
        if (!mInBuffer) {
            allocinbuf();
        }
        auto n = co_await co_await mRaw->raw_read(
            std::span(mInBuffer.get() + mInIndex, mInBufSize - mInIndex));
        if (n == 0) [[unlikely]] {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
        }
        mInEnd = mInIndex + n;
        co_return {};
    }

    bool bufempty() const noexcept {
        return mInIndex == mInEnd;
    }

    Task<Expected<>> putchar(char c) {
        if (buffull()) {
            co_await co_await flush();
        }
        mOutBuffer[mOutIndex] = c;
        ++mOutIndex;
        co_return {};
    }

    Task<Expected<>> putspan(std::span<char const> s) {
        auto p = s.data();
        auto const pe = s.data() + s.size();
    again:
        if (std::size_t(pe - p) <= mOutBufSize - mOutIndex) {
            auto b = mOutBuffer.get() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
        } else {
            auto b = mOutBuffer.get() + mOutIndex;
            auto const be = mOutBuffer.get() + mOutBufSize;
            mOutIndex = mOutBufSize;
            while (b < be) {
                *b++ = *p++;
            }
            co_await co_await flush();
            mOutIndex = 0;
            goto again;
        }
        co_return {};
    }

    std::size_t trywrite(std::span<char const> s) {
        if (!mOutBuffer) {
            allocoutbuf();
        }
        auto p = s.data();
        auto const pe = s.data() + s.size();
        auto nMax = mOutBufSize - mOutIndex;
        auto n = std::size_t(pe - p);
        if (n <= nMax) {
            auto b = mOutBuffer.get() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
            return n;
        } else {
            auto b = mOutBuffer.get() + mOutIndex;
            auto const be = mOutBuffer.get() + mOutBufSize;
            mOutIndex = mOutBufSize;
            while (b < be) {
                *b++ = *p++;
            }
            return nMax;
        }
    }

    Task<Expected<>> puts(std::string_view s) {
        return putspan(std::span<char const>(s.data(), s.size()));
    }

    template <class T>
    Task<Expected<>> putstruct(T const &s) {
        return putspan(
            std::span<char const>((char const *)std::addressof(s), sizeof(T)));
    }

    Task<Expected<>> putchunk(std::string_view s) {
        co_await co_await puts(s);
        co_return co_await flush();
    }

    Task<Expected<>> putline(std::string_view s) {
        co_await co_await puts(s);
        co_await co_await putchar('\n');
        co_return co_await flush();
    }

    void allocoutbuf(std::size_t size = kStreamBufferSize) {
        if (!mOutBuffer) [[likely]] {
            mOutBuffer = std::make_unique<char[]>(size);
            mOutBufSize = size;
            mOutIndex = 0;
        }
    }

    Task<Expected<>> flush() {
        if (!mOutBuffer) {
            allocoutbuf();
            co_return {};
        }
        if (mOutIndex) [[likely]] {
            auto buf = std::span(mOutBuffer.get(), mOutIndex);
            auto len = co_await mRaw->raw_write(buf);
            while (len.has_value() && *len > 0 && *len != buf.size()) {
                buf = buf.subspan(*len);
                len = co_await mRaw->raw_write(buf);
            }
            if (len.has_error()) [[unlikely]] {
                co_return Unexpected{len.error()};
            }
            if (*len == 0) [[unlikely]] {
                co_return Unexpected{
                    std::make_error_code(std::errc::broken_pipe)};
            }
            mOutIndex = 0;
            co_await co_await mRaw->raw_flush();
        }
        co_return {};
    }

    bool buffull() const noexcept {
        return mOutIndex == mOutBufSize;
    }

    Stream &raw() const noexcept {
        return *mRaw;
    }

    template <std::derived_from<Stream> Derived>
    Derived &raw() const {
        return dynamic_cast<Derived &>(*mRaw);
    }

    Task<> close() {
#if CO_ASYNC_DEBUG
        if (mOutIndex) [[unlikely]] {
            std::cerr << "WARNING: stream closed with buffer not flushed\n";
        }
#endif
        return mRaw->raw_close();
    }

    Task<Expected<std::size_t>> read(std::span<char> buffer) {
        if (!bufempty()) {
            auto n = std::min(mInEnd - mInIndex, buffer.size());
            std::memcpy(buffer.data(), mInBuffer.get() + mInIndex, n);
            mInIndex += n;
            co_return n;
        }
        co_return co_await mRaw->raw_read(buffer);
    }

    Task<Expected<std::size_t>> read(void *buffer, std::size_t len) {
        return read(std::span<char>((char *)buffer, len));
    }

    std::size_t tryread(void *buffer, std::size_t len) {
        return tryread(std::span<char>((char *)buffer, len));
    }

    Task<Expected<std::size_t>> write(std::span<char const> buffer) {
        if (!buffull()) {
            auto n = std::min(mInBufSize - mInIndex, buffer.size());
            co_await co_await putspan(buffer.subspan(0, n));
            co_return n;
        }
        co_return co_await mRaw->raw_write(buffer);
    }

    Task<Expected<std::size_t>> write(void const *buffer, std::size_t len) {
        return write(std::span<char const>((char const *)buffer, len));
    }

    Task<Expected<>> putspan(void const *buffer, std::size_t len) {
        return putspan(std::span<char const>((char const *)buffer, len));
    }

    std::size_t trywrite(void const *buffer, std::size_t len) {
        return trywrite(std::span<char const>((char const *)buffer, len));
    }

    void timeout(std::chrono::steady_clock::duration timeout) {
        mRaw->raw_timeout(timeout);
    }

    Task<Expected<>> seek(std::uint64_t pos) {
        co_await co_await mRaw->raw_seek(pos);
        mInIndex = 0;
        mInEnd = 0;
        mOutIndex = 0;
        co_return {};
    }

private:
    std::unique_ptr<char[]> mInBuffer;
    std::size_t mInIndex = 0;
    std::size_t mInEnd = 0;
    std::size_t mInBufSize = 0;
    std::unique_ptr<char[]> mOutBuffer;
    std::size_t mOutIndex = 0;
    std::size_t mOutBufSize = 0;
    Stream *mRaw;
};

struct OwningStream : BorrowedStream {
    explicit OwningStream() : BorrowedStream(), mRawUnique() {}

    explicit OwningStream(std::unique_ptr<Stream> raw)
        : BorrowedStream(raw.get()),
          mRawUnique(std::move(raw)) {}

    std::unique_ptr<Stream> releaseraw() noexcept {
        return std::move(mRawUnique);
    }

private:
    std::unique_ptr<Stream> mRawUnique;
};

template <std::derived_from<Stream> Stream, class... Args>
OwningStream make_stream(Args &&...args) {
    return OwningStream(std::make_unique<Stream>(std::forward<Args>(args)...));
}
} // namespace co_async
