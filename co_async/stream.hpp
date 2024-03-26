#pragma once

#include <span>
#include <utility>
#include <string>
#include <co_async/filesystem.hpp>
#include <co_async/stream_base.hpp>

namespace co_async {

struct FileBuf {
    FileHandle mFile;

    Task<std::size_t> read(std::span<char> buffer) {
        return fs_read(mFile, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return fs_write(mFile, buffer);
    }
};

using FileIStream = IStream<FileBuf>;
using FileOStream = OStream<FileBuf>;
using FileStream = IOStream<FileBuf>;

struct StringReadBuf {
    std::string_view mStringView;
    std::size_t mPosition;

    StringReadBuf() noexcept : mPosition(0) {}

    StringReadBuf(std::string_view strView)
        : mStringView(strView),
          mPosition(0) {}

    Task<std::size_t> read(std::span<char> buffer) {
        std::size_t size = std::min(buffer.size(), mStringView.size() - mPosition);
        std::copy_n(mStringView.begin() + mPosition, size, buffer.begin());
        mPosition += size;
        co_return size;
    }
};

struct StringWriteBuf {
    std::string mString;

    StringWriteBuf() noexcept {}

    StringWriteBuf(std::string &&str) : mString(std::move(str)) {}

    Task<std::size_t> write(std::span<char const> buffer) {
        mString.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }
};

using StringIStream = IStream<StringReadBuf>;
using StringOStream = OStream<StringWriteBuf>;

} // namespace co_async
