#pragma once /*{export module co_async:iostream.stream_base;}*/

#include <co_async/std.hpp>         /*{import std;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/awaiter/just.hpp>/*{import :awaiter.just;}*/
// #include <co_async/utils/expected.hpp>/*{import :utils.expected;}*/

namespace co_async {

struct IStreamRaw {
protected:
    virtual Task<std::size_t> raw_read(std::span<char> buffer) = 0;
};

struct OStreamRaw {
protected:
    virtual Task<std::size_t> raw_write(std::span<char const> buffer) = 0;

    virtual Task<> raw_flush() {
        return just_void();
    }
};

struct IOStreamRaw : virtual IStreamRaw, virtual OStreamRaw {};

struct IStream : virtual IStreamRaw {
    explicit IStream(std::size_t bufferSize = 8192)
        : mBuffer(std::make_unique<char[]>(bufferSize)),
          mBufSize(bufferSize) {}

    virtual ~IStream() = default;

    IStream(IStream &&) = default;
    IStream &operator=(IStream &&) = default;

    Task<std::optional<char>> getchar() {
        if (bufempty()) {
            if (!co_await fillbuf()) {
                co_return std::nullopt;
            }
        }
        char c = mBuffer[mIndex];
        ++mIndex;
        co_return c;
    }

    Task<bool> getline(std::string &s, char eol) {
        std::size_t start = mIndex;
        while (true) {
            for (std::size_t i = start; i < mEnd; ++i) {
                if (mBuffer[i] == eol) {
                    s.append(mBuffer.get() + start, i - start);
                    mIndex = i + 1;
                    co_return true;
                }
            }
            if (!co_await fillbuf()) {
                co_return false;
            }
            start = 0;
        }
    }

    Task<bool> dropline(char eol) {
        std::size_t start = mIndex;
        while (true) {
            for (std::size_t i = start; i < mEnd; ++i) {
                if (mBuffer[i] == eol) {
                    mIndex = i + 1;
                    co_return true;
                }
            }
            if (!co_await fillbuf()) {
                co_return false;
            }
            start = 0;
        }
    }

    Task<bool> getline(std::string &s, std::string_view eol) {
        while (true) {
        again:
            if (!co_await getline(s, eol.front())) {
                co_return false;
            }
            for (std::size_t i = 1; i < eol.size(); ++i) {
                if (bufempty()) {
                    if (!co_await fillbuf()) {
                        co_return false;
                    }
                }
                char c = mBuffer[mIndex];
                if (eol[i] == c) [[likely]] {
                    ++mIndex;
                } else {
                    s.append(eol.data(), i);
                    goto again;
                }
            }
            co_return true;
        }
    }

    Task<bool> dropline(std::string_view eol) {
        while (true) {
        again:
            if (!co_await dropline(eol.front())) {
                co_return false;
            }
            for (std::size_t i = 1; i < eol.size(); ++i) {
                if (bufempty()) {
                    if (!co_await fillbuf()) {
                        co_return false;
                    }
                }
                char c = mBuffer[mIndex];
                if (eol[i] == c) [[likely]] {
                    ++mIndex;
                } else {
                    goto again;
                }
            }
            co_return true;
        }
    }

    Task<std::string> getline(char eol) {
        std::string s;
        co_await getline(s, eol);
        co_return s;
    }

    Task<std::string> getline(std::string_view eol) {
        std::string s;
        co_await getline(s, eol);
        co_return s;
    }

    Task<bool> getspan(std::span<char> s) {
        auto p = s.data();
        auto n = s.size();
        std::size_t start = mIndex;
        while (true) {
            auto end = start + n;
            if (end <= mEnd) {
                p = std::copy(mBuffer.get() + start, mBuffer.get() + end, p);
                mIndex = end;
                co_return true;
            }
            p = std::copy(mBuffer.get() + start, mBuffer.get() + mEnd, p);
            if (!co_await fillbuf()) {
                co_return false;
            }
            start = 0;
        }
    }

    Task<bool> dropn(std::size_t n) {
        if (!n)
            co_return true;
        std::size_t start = mIndex;
        while (true) {
            auto end = start + n;
            if (end <= mEnd) {
                mIndex = end;
                co_return true;
            }
            if (!co_await fillbuf()) {
                co_return false;
            }
            start = 0;
        }
    }

    Task<bool> getn(std::string &s, std::size_t n) {
        while (true) {
            if (mIndex + n <= mEnd) {
                s.append(mBuffer.get() + mIndex, n);
                mIndex += n;
                co_return true;
            }
            s.append(mBuffer.get() + mIndex, mEnd - mIndex);
            n -= mEnd - mIndex;
            if (!co_await fillbuf()) {
                co_return false;
            }
        }
    }

    Task<std::string> getn(std::size_t n) {
        std::string s;
        s.reserve(n);
        co_await getn(s, n);
        co_return s;
    }

    Task<> getall(std::string &s) {
        std::size_t start = mIndex;
        do {
            s.append(mBuffer.get() + start, mEnd - start);
            start = 0;
        } while (co_await fillbuf());
    }

    Task<std::string> getall() {
        std::string s;
        co_await getall(s);
        co_return s;
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<bool> getstruct(T &ret) {
        return getspan(std::span<char>((char *)&ret, sizeof(T)));
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<T> getstruct() {
        T ret;
        if (!co_await getstruct(ret)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        co_return ret;
    }

    std::span<char const> rdbuf() const noexcept {
        return {mBuffer.get() + mIndex, mEnd - mIndex};
    }

    void rdbufadvance(std::size_t n) noexcept {
        mIndex += n;
    }

    Task<bool> fillbuf() {
        mEnd = co_await raw_read(std::span(mBuffer.get(), mBufSize));
        mIndex = 0;
        co_return mEnd != 0;
    }

    bool bufempty() const noexcept {
        return mIndex == mEnd;
    }

    std::unique_ptr<char[]> mBuffer;
    std::size_t mIndex = 0;
    std::size_t mEnd = 0;
    std::size_t mBufSize = 0;
};

struct OStream : virtual OStreamRaw {
    explicit OStream(std::size_t bufferSize = 8192)
        : mBuffer(std::make_unique<char[]>(bufferSize)),
          mBufSize(bufferSize) {}

    virtual ~OStream() = default;

    OStream(OStream &&) = default;
    OStream &operator=(OStream &&) = default;

    Task<bool> putchar(char c) {
        if (buffull()) {
            if (!co_await flush()) [[unlikely]] {
                co_return false;
            }
        }
        mBuffer[mIndex] = c;
        ++mIndex;
        co_return true;
    }

    Task<bool> putspan(std::span<char const> s) {
        auto p = s.data();
        auto const pe = s.data() + s.size();
    again:
        if (std::size_t(pe - p) <= mBufSize - mIndex) {
            auto b = mBuffer.get() + mIndex;
            mIndex += pe - p;
            while (p < pe) {
                *b++ = *p++;
            }
        } else {
            auto b = mBuffer.get() + mIndex;
            auto const be = mBuffer.get() + mBufSize;
            mIndex = mBufSize;
            while (b < be) {
                *b++ = *p++;
            }
            if (!co_await flush()) [[unlikely]] {
                co_return false;
            }
            mIndex = 0;
            goto again;
        }
        co_return true;
    }

    Task<bool> puts(std::string_view s) {
        return putspan(std::span<char const>(s.data(), s.size()));
    }

    template <class T>
    Task<bool> putstruct(T const &s) {
        return putspan(
            std::span<char const>((char const *)std::addressof(s), sizeof(T)));
    }

    Task<bool> putline(std::string_view s) {
        if (!co_await puts(s)) [[unlikely]] co_return false;
        if (!co_await putchar('\n')) [[unlikely]] co_return false;
        co_return co_await flush();
    }

    std::span<char> wrbuf() const noexcept {
        return {mBuffer.get() + mIndex, mBufSize - mIndex};
    }

    void wrbufadvance(std::size_t n) noexcept {
        mIndex += n;
    }

    Task<bool> flush() {
        if (mIndex) [[likely]] {
            auto buf = std::span(mBuffer.get(), mIndex);
            auto len = co_await raw_write(buf);
            while (len > 0 && len != buf.size()) [[unlikely]] {
                buf = buf.subspan(len);
                len = co_await raw_write(buf);
            }
            if (len == 0) [[unlikely]] {
                /* throw std::runtime_error("ostream shutdown while writing"); */
                co_return false;
            }
            mIndex = 0;
            co_await raw_flush();
        }
        co_return true;
    }

    bool buffull() const noexcept {
        return mIndex == mBufSize;
    }

private:
    std::unique_ptr<char[]> mBuffer;
    std::size_t mIndex = 0;
    std::size_t mBufSize = 0;
};

struct IOStream : IStream, OStream {
    explicit IOStream(std::size_t bufferSize = 8192)
        : IStream(bufferSize),
          OStream(bufferSize) {}
};

/*[export]*/ template <std::derived_from<IOStreamRaw> StreamRaw>
struct [[nodiscard]] IOStreamImpl : IOStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit IOStreamImpl(Args &&...args)
        : IOStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    IOStreamImpl() = default;
};

/*[export]*/ template <std::derived_from<OStreamRaw> StreamRaw>
struct [[nodiscard]] OStreamImpl : OStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit OStreamImpl(Args &&...args)
        : OStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    OStreamImpl() = default;
};

/*[export]*/ template <std::derived_from<IStreamRaw> StreamRaw>
struct [[nodiscard]] IStreamImpl : IStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit IStreamImpl(Args &&...args)
        : IStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    IStreamImpl() = default;
};

} // namespace co_async
