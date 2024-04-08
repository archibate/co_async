#pragma once/*{export module co_async:iostream.string_stream;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

/*[export]*/ struct StringReadBuf {
    StringReadBuf() noexcept : mPosition(0) {}

    StringReadBuf(std::string_view strView)
        : mStringView(strView),
          mPosition(0) {}

    Task<std::size_t> read(std::span<char> buffer) {
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

/*[export]*/ struct StringWriteBuf {
    StringWriteBuf() noexcept {}

    StringWriteBuf(std::string &&str) noexcept : mString(std::move(str)) {}

    StringWriteBuf(std::string_view str) : mString(str) {}

    Task<std::size_t> write(std::span<char const> buffer) {
        mString.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }

    std::string_view str() const noexcept {
        return mString;
    }

    std::string release() noexcept {
        return std::move(mString);
    }

private:
    std::string mString;
};

/*[export]*/ using StringIStream = IStream<StringReadBuf>;
/*[export]*/ using StringOStream = OStream<StringWriteBuf>;

} // namespace co_async
