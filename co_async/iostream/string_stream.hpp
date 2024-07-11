#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>

namespace co_async {
struct IStringStream : Stream {
    IStringStream() noexcept : mPosition(0) {}

    IStringStream(std::string_view strView)
        : mStringView(strView),
          mPosition(0) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        std::size_t size =
            std::min(buffer.size(), mStringView.size() - mPosition);
        std::copy_n(mStringView.begin() + mPosition, size, buffer.begin());
        mPosition += size;
        co_return size;
    }

    std::string_view str() const noexcept {
        return mStringView;
    }

    std::string_view unread_str() const noexcept {
        return mStringView.substr(mPosition);
    }

private:
    std::string_view mStringView;
    std::size_t mPosition;
};

struct OStringStream : Stream {
    OStringStream(String &output) noexcept : mOutput(output) {}

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        mOutput.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }

    String &str() const noexcept {
        return mOutput;
    }

    String release() noexcept {
        return std::move(mOutput);
    }

private:
    String &mOutput;
};
} // namespace co_async
