#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <memory>
#include "task.hpp"

namespace co_async {

struct EOFException {};

template <class Reader>
struct IStreamBase {
    explicit IStreamBase(std::size_t bufferSize = 8192)
        : mBuffer(std::make_unique<char[]>(bufferSize)),
          mBufSize(bufferSize) {}

    IStreamBase(IStreamBase &&) = default;
    IStreamBase &operator=(IStreamBase &&) = default;

    Task<char> getchar() {
        if (bufferEmpty()) {
            co_await fillBuffer();
        }
        char c = mBuffer[mIndex];
        ++mIndex;
        co_return c;
    }

    Task<std::string> getline(char eol = '\n') {
        std::string s;
        while (true) {
            char c = co_await getchar();
            if (c == eol) {
                break;
            }
            s.push_back(c);
        }
        co_return s;
    }

    Task<std::string> getline(std::string_view eol) {
        std::string s;
        while (true) {
            char c = co_await getchar();
            if (c == eol[0]) {
                std::size_t i;
                for (i = 1; i < eol.size(); ++i) {
                    char c = co_await getchar();
                    if (c != eol[i]) {
                        break;
                    }
                }
                if (i == eol.size()) {
                    break;
                }
                for (std::size_t j = 0; j < i; ++j) {
                    s.push_back(eol[j]);
                }
                continue;
            }
            s.push_back(c);
        }
        co_return s;
    }

    Task<std::string> getn(std::size_t n) {
        std::string s;
        for (std::size_t i = 0; i < n; i++) {
            char c = co_await getchar();
            s.push_back(c);
        }
        co_return s;
    }

private:
    bool bufferEmpty() const noexcept {
        return mIndex == mEnd;
    }

    Task<> fillBuffer() {
        auto *that = static_cast<Reader *>(this);
        mEnd = co_await that->read(std::span(mBuffer.get(), mBufSize));
        mIndex = 0;
        if (mEnd == 0) [[unlikely]] {
            throw EOFException();
        }
    }

    std::unique_ptr<char[]> mBuffer;
    std::size_t mIndex = 0;
    std::size_t mEnd = 0;
    std::size_t mBufSize = 0;
};

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
        for (char c: s) {
            co_await putchar(c);
        }
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
                throw EOFException();
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

template <class StreamBuf>
struct [[nodiscard]] IOStream : IOStreamBase<IOStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IOStream(Args &&...args)
        : IOStreamBase<IOStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    IOStream() = default;
};

template <class StreamBuf>
struct [[nodiscard]] OStream : OStreamBase<OStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit OStream(Args &&...args)
        : OStreamBase<OStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    OStream() = default;
};

template <class StreamBuf>
struct [[nodiscard]] IStream : IStreamBase<OStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IStream(Args &&...args)
        : IStreamBase<IStream<StreamBuf>>(),
          StreamBuf(std::forward<Args>(args)...) {}

    IStream() = default;
};

} // namespace co_async
