#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/just.hpp>
#include <co_async/utils/expected.hpp>

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

    Task<Expected<char>> getchar() {
        if (bufempty()) {
            co_await co_await fillbuf();
        }
        char c = mBuffer[mIndex];
        ++mIndex;
        co_return c;
    }

    Task<Expected<>> getline(std::string &s, char eol) {
        std::size_t start = mIndex;
        while (true) {
            for (std::size_t i = start; i < mEnd; ++i) {
                if (mBuffer[i] == eol) {
                    s.append(mBuffer.get() + start, i - start);
                    mIndex = i + 1;
                    co_return {};
                }
            }
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropline(char eol) {
        std::size_t start = mIndex;
        while (true) {
            for (std::size_t i = start; i < mEnd; ++i) {
                if (mBuffer[i] == eol) {
                    mIndex = i + 1;
                    co_return {};
                }
            }
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getline(std::string &s, std::string_view eol) {
        again:
            co_await co_await getline(s, eol.front());
            for (std::size_t i = 1; i < eol.size(); ++i) {
                if (bufempty()) {
                    co_await co_await fillbuf();
                }
                char c = mBuffer[mIndex];
                if (eol[i] == c) [[likely]] {
                    ++mIndex;
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
                co_await co_await fillbuf();
            }
            char c = mBuffer[mIndex];
            if (eol[i] == c) [[likely]] {
                ++mIndex;
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
        std::size_t start = mIndex;
        while (true) {
            auto end = start + n;
            if (end <= mEnd) {
                p = std::copy(mBuffer.get() + start, mBuffer.get() + end, p);
                mIndex = end;
                co_return {};
            }
            p = std::copy(mBuffer.get() + start, mBuffer.get() + mEnd, p);
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropn(std::size_t n) {
        std::size_t start = mIndex;
        while (true) {
            auto end = start + n;
            if (end <= mEnd) {
                mIndex = end;
                co_return {};
            }
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getn(std::string &s, std::size_t n) {
        while (true) {
            if (mIndex + n <= mEnd) {
                s.append(mBuffer.get() + mIndex, n);
                mIndex += n;
                co_return {};
            }
            s.append(mBuffer.get() + mIndex, mEnd - mIndex);
            n -= mEnd - mIndex;
            co_await co_await fillbuf();
        }
    }

    Task<Expected<std::string>> getn(std::size_t n) {
        std::string s;
        s.reserve(n);
        co_await co_await getn(s, n);
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

    std::span<char const> rdbuf() const noexcept {
        return {mBuffer.get() + mIndex, mEnd - mIndex};
    }

    void rdbufadvance(std::size_t n) noexcept {
        mIndex += n;
    }

    Task<Expected<>> peekn(std::string &s, std::size_t n) {
        while (mEnd - mIndex < n) {
            co_await co_await fillbuf();
        }
        s.append(mBuffer.get(), n);
        co_return {};
    }

    Task<Expected<std::string>> peekn(std::size_t n) {
        std::string s;
        co_await co_await peekn(s, n);
        co_return s;
    }

    Task<Expected<>> fillbuf() {
        mIndex = 0;
        mEnd = co_await raw_read(std::span(mBuffer.get(), mBufSize));
        if (mEnd == 0) [[unlikely]] {
            co_return Unexpected{};
        }
        co_return {};
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

    Task<Expected<>> putchar(char c) {
        if (buffull()) {
            co_await co_await flush();
        }
        mBuffer[mIndex] = c;
        ++mIndex;
        co_return {};
    }

    Task<Expected<>> putspan(std::span<char const> s) {
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
            co_await co_await flush();
            mIndex = 0;
            goto again;
        }
        co_return {};
    }

    Task<Expected<>> puts(std::string_view s) {
        return putspan(std::span<char const>(s.data(), s.size()));
    }

    template <class T>
    Task<Expected<>> putstruct(T const &s) {
        return putspan(
            std::span<char const>((char const *)std::addressof(s), sizeof(T)));
    }

    Task<Expected<>> putline(std::string_view s) {
        co_await co_await puts(s);
        co_await co_await putchar('\n');
        co_return co_await flush();
    }

    std::span<char> wrbuf() const noexcept {
        return {mBuffer.get() + mIndex, mBufSize - mIndex};
    }

    void wrbufadvance(std::size_t n) noexcept {
        mIndex += n;
    }

    Task<Expected<>> flush() {
        if (mIndex) [[likely]] {
            auto buf = std::span(mBuffer.get(), mIndex);
            auto len = co_await raw_write(buf);
            while (len > 0 && len != buf.size()) [[unlikely]] {
                buf = buf.subspan(len);
                len = co_await raw_write(buf);
            }
            if (len == 0) [[unlikely]] {
                co_return Unexpected{};
            }
            mIndex = 0;
            co_await raw_flush();
        }
        co_return {};
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

template <std::derived_from<IOStreamRaw> StreamRaw>
struct [[nodiscard]] IOStreamImpl : IOStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit IOStreamImpl(Args &&...args)
        : IOStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    IOStreamImpl() = default;
};

template <std::derived_from<OStreamRaw> StreamRaw>
struct [[nodiscard]] OStreamImpl : OStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit OStreamImpl(Args &&...args)
        : OStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    OStreamImpl() = default;
};

template <std::derived_from<IStreamRaw> StreamRaw>
struct [[nodiscard]] IStreamImpl : IStream, StreamRaw {
    template <class... Args>
        requires std::constructible_from<StreamRaw, Args...>
    explicit IStreamImpl(Args &&...args)
        : IStream(),
          StreamRaw(std::forward<Args>(args)...) {}

    IStreamImpl() = default;
};

} // namespace co_async
