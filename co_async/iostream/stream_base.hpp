#pragma once/*{export module co_async:iostream.stream_base;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/

namespace co_async {

template <class Reader>
struct IStreamBase {
    explicit IStreamBase(std::size_t bufferSize = 8192)
        : mBuffer(std::make_unique<char[]>(bufferSize)),
          mBufSize(bufferSize) {}

    IStreamBase(IStreamBase &&) = default;
    IStreamBase &operator=(IStreamBase &&) = default;

    Task<std::optional<char>> getchar() {
        if (bufferEmpty()) {
            if (!co_await fillBuffer()) {
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
            if (!co_await fillBuffer()) {
                co_return mEnd != start;
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
                if (bufferEmpty()) {
                    if (!co_await fillBuffer()) {
                        co_return true;
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
            co_return false;
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

    Task<bool> getn(std::string &s, std::size_t n) {
        std::size_t start = mIndex;
        while (true) {
            auto end = start + n;
            if (end <= mEnd) {
                s.append(mBuffer.get() + start, end - start);
                mIndex = end;
                co_return true;
            }
            s.append(mBuffer.get() + start, mEnd - start);
            if (!co_await fillBuffer()) {
                co_return mEnd != start;
            }
            start = 0;
        }
    }

    Task<std::string> getn(std::size_t n) {
        std::string s;
        co_await getn(s, n);
        co_return s;
    }

    Task<> getall(std::string &s) {
        std::size_t start = mIndex;
        do {
            s.append(mBuffer.get() + start, mEnd - start);
            start = 0;
        } while (co_await fillBuffer());
    }

private:
    bool bufferEmpty() const noexcept {
        return mIndex == mEnd;
    }

    Task<bool> fillBuffer() {
        auto *that = static_cast<Reader *>(this);
        mEnd = co_await that->read(std::span(mBuffer.get(), mBufSize));
        mIndex = 0;
        co_return mEnd != 0;
    }

    std::unique_ptr<char[]> mBuffer;
    std::size_t mIndex = 0;
    std::size_t mEnd = 0;
    std::size_t mBufSize = 0;
};

/*[export]*/ struct StreamShutdownException {};

template <class Writer>
struct OStreamBase {
    explicit OStreamBase(std::size_t bufferSize = 8192)
        : mBuffer(std::make_unique<char[]>(bufferSize)),
          mBufSize(bufferSize) {}

    OStreamBase(OStreamBase &&) = default;
    OStreamBase &operator=(OStreamBase &&) = default;

    Task<> putchar(char c) {
        if (bufferFull()) {
            co_await flush();
        }
        mBuffer[mIndex] = c;
        ++mIndex;
    }

    Task<> puts(std::string_view s) {
        auto p = s.data();
        auto const pe = s.data() + s.size();
    again:
        if (pe - p <= mBufSize - mIndex) {
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
            co_await flush();
            mIndex = 0;
            goto again;
        }
    }

    Task<> putline(std::string_view s) {
        co_await puts(s);
        co_await putchar('\n');
        co_await flush();
    }

    Task<> flush() {
        if (mIndex) [[likely]] {
            auto *that = static_cast<Writer *>(this);
            auto buf = std::span(mBuffer.get(), mIndex);
            auto len = co_await that->write(buf);
            while (len != buf.size()) [[unlikely]] {
                buf = buf.subspan(len);
                len = co_await that->write(buf);
            }
            if (len == 0) [[unlikely]] {
                throw StreamShutdownException();
            }
            mIndex = 0;
        }
    }

private:
    bool bufferFull() const noexcept {
        return mIndex == mBufSize;
    }

    std::unique_ptr<char[]> mBuffer;
    std::size_t mIndex = 0;
    std::size_t mBufSize = 0;
};

template <class StreamBuf>
struct IOStreamBase : IStreamBase<StreamBuf>, OStreamBase<StreamBuf> {
    explicit IOStreamBase(std::size_t bufferSize = 8192)
        : IStreamBase<StreamBuf>(bufferSize),
          OStreamBase<StreamBuf>(bufferSize) {}
};

/*[export]*/ template <class StreamBuf>
struct [[nodiscard]] IOStream : IOStreamBase<IOStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IOStream(Args &&...args)
        : IOStreamBase<IOStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    IOStream() = default;
};

/*[export]*/ template <class StreamBuf>
struct [[nodiscard]] OStream : OStreamBase<OStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit OStream(Args &&...args)
        : OStreamBase<OStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    OStream() = default;
};

/*[export]*/ template <class StreamBuf>
struct [[nodiscard]] IStream : IStreamBase<IStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IStream(Args &&...args)
        : IStreamBase<IStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    IStream() = default;
};

} // namespace co_async
