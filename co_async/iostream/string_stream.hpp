#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct IStringStreamRaw : StreamRaw {
    IStringStreamRaw() noexcept : mPosition(0) {}

    IStringStreamRaw(std::string_view strView)
        : mStringView(strView),
          mPosition(0) {}

    Task<Expected<std::size_t, std::errc>>
    raw_read(std::span<char> buffer) override {
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

struct OStringStreamRaw : StreamRaw {
    OStringStreamRaw(std::string &output) noexcept : mOutput(output) {}

    Task<Expected<std::size_t, std::errc>>
    raw_write(std::span<char const> buffer) override {
        mOutput.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }

    std::string &str() const noexcept {
        return mOutput;
    }

    std::string release() noexcept {
        return std::move(mOutput);
    }

private:
    std::string &mOutput;
};

} // namespace co_async
