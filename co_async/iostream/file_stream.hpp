#pragma once

#include <span>
#include <utility>
#include <co_async/system/fs.hpp>
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

    FileHandle release() noexcept {
        return std::move(mFile);
    }
};

using FileIStream = IStream<FileBuf>;
using FileOStream = OStream<FileBuf>;
using FileStream = IOStream<FileBuf>;

} // namespace co_async
