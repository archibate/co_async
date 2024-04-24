/*{module;}*/

#include <dirent.h>

#pragma once/*{export module co_async:iostream.directory_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

struct DirectoryStreamRaw : virtual IStreamRaw {
    Task<std::size_t> raw_read(std::span<char> buffer) {
        return fs_getdents(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit DirectoryStreamRaw(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

/*[export]*/ struct DirectoryStream : IStreamImpl<DirectoryStreamRaw> {
    using IStreamImpl<DirectoryStreamRaw>::IStreamImpl;

    Task<std::optional<std::string>> getdirent() {
        struct LinuxDirent64 {
            int64_t		d_ino;    /* 64-bit inode number */
            int64_t		d_off;    /* 64-bit offset to next structure */
            unsigned short	d_reclen; /* Size of this dirent */
            unsigned char	d_type;   /* File type */
        } dent;
        if (!co_await getspan(std::span<char>((char *)&dent, 19))) {
            co_return std::nullopt;
        }
        std::string rest;
        rest.reserve(dent.d_reclen - 19);
        co_await getn(rest, dent.d_reclen - 19);
        co_return rest.data();
    }
};

} // namespace co_async
