#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/iostream/bytes_buffer.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
inline constexpr std::size_t kStreamBufferSize = 8192;

inline std::error_code eofError() {
    static struct : public std::error_category {
        const char *name() const noexcept override {
            return "eof";
        }
        std::string message(int) const override {
            return "End of file";
        }
    } category;
    return std::error_code(1, category);
}

struct Stream {
    virtual void raw_timeout(std::chrono::steady_clock::duration timeout) {}

    virtual Task<Expected<>> raw_seek(std::uint64_t pos) {
        co_return std::errc::invalid_seek;
    }

    virtual Task<Expected<>> raw_flush() {
        co_return {};
    }

    virtual Task<> raw_close() {
        co_return;
    }

    virtual Task<Expected<std::size_t>> raw_read(std::span<char> buffer) {
        co_return std::errc::not_supported;
    }

    virtual Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) {
        co_return std::errc::not_supported;
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
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        char c = mInBuffer[mInIndex];
        ++mInIndex;
        co_return c;
    }

    Task<Expected<>> getline(String &s, char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    s.append(mInBuffer.data() + start, i - start);
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            s.append(mInBuffer.data() + start, mInEnd - start);
            mInEnd = mInIndex = 0;
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
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getline(String &s, std::string_view eol) {
    again:
        co_await co_await getline(s, eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInEnd = mInIndex = 0;
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
                mInEnd = mInIndex = 0;
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

    Task<Expected<String>> getline(char eol) {
        String s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<String>> getline(std::string_view eol) {
        String s;
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
                p = std::copy(mInBuffer.data() + start, mInBuffer.data() + end,
                              p);
                mInIndex = end;
                co_return {};
            }
            p = std::copy(mInBuffer.data() + start, mInBuffer.data() + mInEnd,
                          p);
            mInEnd = mInIndex = 0;
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
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getn(String &s, std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                s.append(mInBuffer.data() + mInIndex, n);
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            s.append(mInBuffer.data() + mInIndex, m);
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<String>> getn(std::size_t n) {
        String s;
        s.reserve(n);
        co_await co_await getn(s, n);
        co_return s;
    }

    Task<Expected<>> dropall() {
        do {
            mInEnd = mInIndex = 0;
        } while (co_await (co_await fillbuf()).transform([] { return true; }).or_else(eofError(), [] { return false; }));
        co_return {};
    }

    Task<Expected<>> getall(String &s) {
        std::size_t start = mInIndex;
        do {
            s.append(mInBuffer.data() + start, mInEnd - start);
            start = 0;
            mInEnd = mInIndex = 0;
        } while (co_await (co_await fillbuf()).transform([] { return true; }).or_else(eofError(), [] { return false; }));
        co_return {};
    }

    Task<Expected<String>> getall() {
        String s;
        co_await co_await getall(s);
        co_return s;
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<>> getstruct(T &ret) {
        return getspan(
            std::span<char>(reinterpret_cast<char *>(&ret), sizeof(T)));
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<T>> getstruct() {
        T ret;
        co_await co_await getstruct(ret);
        co_return ret;
    }

    std::span<char const> peekbuf() const noexcept {
        return {mInBuffer.data() + mInIndex, mInEnd - mInIndex};
    }

    void seenbuf(std::size_t n) noexcept {
        mInIndex += n;
    }

    Task<Expected<String>> getchunk() noexcept {
        if (bufempty()) {
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        auto buf = peekbuf();
        String ret(buf.data(), buf.size());
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
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        co_return mInBuffer[mInIndex];
    }

    Task<Expected<>> peekn(String &s, std::size_t n) {
        if (mInBuffer.size() - mInIndex < n) {
            if (mInBuffer.size() < n) [[unlikely]] {
                co_return std::errc::value_too_large;
            }
            std::memmove(mInBuffer.data(), mInBuffer.data() + mInIndex,
                         mInEnd - mInIndex);
            mInEnd -= mInIndex;
            mInIndex = 0;
        }
        while (mInEnd - mInIndex < n) {
            co_await co_await fillbuf();
        }
        s.append(mInBuffer.data() + mInIndex, n);
        co_return {};
    }

    Task<Expected<String>> peekn(std::size_t n) {
        String s;
        co_await co_await peekn(s, n);
        co_return s;
    }

    void allocinbuf(std::size_t size) {
        if (!mInBuffer) [[likely]] {
            mInBuffer.allocate(size);
            mInIndex = 0;
            mInEnd = 0;
        }
    }

    Task<Expected<>> fillbuf() {
        if (!mInBuffer) {
            allocinbuf(kStreamBufferSize);
        }
        // #if CO_ASYNC_DEBUG
        //         if (!bufempty()) [[unlikely]] {
        //             throw std::logic_error("buf must be empty before
        //             fillbuf");
        //         }
        // #endif
        auto n = co_await co_await mRaw->raw_read(std::span(
            mInBuffer.data() + mInIndex, mInBuffer.size() - mInIndex));
        // auto n = co_await co_await mRaw->raw_read(mInBuffer);
        if (n == 0) [[unlikely]] {
            co_return eofError();
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
        if (std::size_t(pe - p) <= mOutBuffer.size() - mOutIndex) {
            auto b = mOutBuffer.data() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
        } else {
            auto b = mOutBuffer.data() + mOutIndex;
            auto const be = mOutBuffer.data() + mOutBuffer.size();
            mOutIndex = mOutBuffer.size();
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
            allocoutbuf(kStreamBufferSize);
        }
        auto p = s.data();
        auto const pe = s.data() + s.size();
        auto nMax = mOutBuffer.size() - mOutIndex;
        auto n = std::size_t(pe - p);
        if (n <= nMax) {
            auto b = mOutBuffer.data() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
            return n;
        } else {
            auto b = mOutBuffer.data() + mOutIndex;
            auto const be = mOutBuffer.data() + mOutBuffer.size();
            mOutIndex = mOutBuffer.size();
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
        return putspan(std::span<char const>(
            reinterpret_cast<char const *>(std::addressof(s)), sizeof(T)));
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

    void allocoutbuf(std::size_t size) {
        if (!mOutBuffer) [[likely]] {
            mOutBuffer.allocate(size);
            mOutIndex = 0;
        }
    }

    Task<Expected<>> flush() {
        if (!mOutBuffer) {
            allocoutbuf(kStreamBufferSize);
            co_return {};
        }
        if (mOutIndex) [[likely]] {
            auto buf = std::span(mOutBuffer.data(), mOutIndex);
            auto len = co_await mRaw->raw_write(buf);
            while (len.has_value() && *len > 0 && *len != buf.size()) {
                buf = buf.subspan(*len);
                len = co_await mRaw->raw_write(buf);
            }
            if (len.has_error()) [[unlikely]] {
#if CO_ASYNC_DEBUG
                co_return {len.error(), len.mErrorLocation};
#else
                co_return len.error();
#endif
            }
            if (*len == 0) [[unlikely]] {
                co_return eofError();
            }
            mOutIndex = 0;
            co_await co_await mRaw->raw_flush();
        }
        co_return {};
    }

    bool buffull() const noexcept {
        return mOutIndex == mOutBuffer.size();
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
            std::memcpy(buffer.data(), mInBuffer.data() + mInIndex, n);
            mInIndex += n;
            co_return n;
        }
        co_return co_await mRaw->raw_read(buffer);
    }

    Task<Expected<std::size_t>> read(void *buffer, std::size_t len) {
        return read(std::span<char>(static_cast<char *>(buffer), len));
    }

    std::size_t tryread(void *buffer, std::size_t len) {
        return tryread(std::span<char>(static_cast<char *>(buffer), len));
    }

    Task<Expected<std::size_t>> write(std::span<char const> buffer) {
        if (!buffull()) {
            auto n = std::min(mInBuffer.size() - mInIndex, buffer.size());
            co_await co_await putspan(buffer.subspan(0, n));
            co_return n;
        }
        co_return co_await mRaw->raw_write(buffer);
    }

    Task<Expected<std::size_t>> write(void const *buffer, std::size_t len) {
        return write(
            std::span<char const>(static_cast<char const *>(buffer), len));
    }

    Task<Expected<>> putspan(void const *buffer, std::size_t len) {
        return putspan(
            std::span<char const>(static_cast<char const *>(buffer), len));
    }

    std::size_t trywrite(void const *buffer, std::size_t len) {
        return trywrite(
            std::span<char const>(static_cast<char const *>(buffer), len));
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
    BytesBuffer mInBuffer;
    std::size_t mInIndex = 0;
    std::size_t mInEnd = 0;
    BytesBuffer mOutBuffer;
    std::size_t mOutIndex = 0;
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
