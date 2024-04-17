#pragma once/*{export module co_async:iostream.file_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

struct FileBuf {
    Task<std::size_t> raw_read(std::span<char> buffer) {
        return fs_read(mFile, buffer);
    }

    Task<std::size_t> raw_write(std::span<char const> buffer) {
        return fs_write(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit FileBuf(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

/*[export]*/ struct FileIStream : IStream<FileBuf> {
    using IStream<FileBuf>::IStream;

    static Task<FileIStream> open(DirFilePath path) {
        co_return FileIStream(co_await fs_open(path, OpenMode::Read));
    }
};

/*[export]*/ struct FileOStream : OStream<FileBuf> {
    using OStream<FileBuf>::OStream;

    static Task<FileOStream> open(DirFilePath path, bool append = false) {
        co_return FileOStream(co_await fs_open(path, append ? OpenMode::Append : OpenMode::Write));
    }
};

/*[export]*/ struct FileStream : IOStream<FileBuf> {
    using IOStream<FileBuf>::IOStream;

    static Task<FileStream> open(DirFilePath path) {
        co_return FileStream(co_await fs_open(path, OpenMode::ReadWrite));
    }
};

/*[export]*/ inline Task<std::string> file_read(DirFilePath path) {
    auto file = co_await FileIStream::open(path);
    co_return co_await file.getall();
}

/*[export]*/ inline Task<> file_write(DirFilePath path, std::string_view content) {
    auto file = co_await FileOStream::open(path, false);
    co_await file.puts(content);
    co_await file.flush();
}

/*[export]*/ inline Task<> file_append(DirFilePath path, std::string_view content) {
    auto file = co_await FileOStream::open(path, true);
    co_await file.puts(content);
    co_await file.flush();
}

} // namespace co_async
