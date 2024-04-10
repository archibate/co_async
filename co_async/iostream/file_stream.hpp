#pragma once /*{export module co_async:iostream.file_stream;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>                /*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>             /*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

/*[export]*/ struct FileBuf {
    Task<std::size_t> read(std::span<char> buffer) {
        return fs_read(mFile, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return fs_write(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle const &get() const noexcept {
        return mFile;
    }

    explicit FileBuf(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

/*[export]*/ using FileIStream = IStream<FileBuf>;
/*[export]*/ using FileOStream = OStream<FileBuf>;
/*[export]*/ using FileStream = IOStream<FileBuf>;

} // namespace co_async
