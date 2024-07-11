#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/directory_stream.hpp>
#include <co_async/iostream/file_stream.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>
#include <dirent.h>

namespace co_async {
namespace {
struct DirectoryStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_getdents(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit DirectoryStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};
} // namespace

DirectoryWalker::DirectoryWalker(FileHandle file)
    : mStream(make_stream<DirectoryStream>(std::move(file))) {}

DirectoryWalker::~DirectoryWalker() = default;

Task<Expected<String>> DirectoryWalker::DirectoryWalker::next() {
    struct LinuxDirent64 {
        int64_t d_ino;           /* 64-bit inode number */
        int64_t d_off;           /* 64-bit offset to next structure */
        unsigned short d_reclen; /* Size of this dirent */
        unsigned char d_type;    /* File type */
    } dent;

    co_await co_await mStream.getspan(
        std::span<char>(reinterpret_cast<char *>(&dent), 19));
    String rest;
    rest.reserve(dent.d_reclen - 19);
    co_await co_await mStream.getn(rest, dent.d_reclen - 19);
    co_return String(rest.data());
}

Task<Expected<DirectoryWalker>> dir_open(std::filesystem::path path) {
    auto handle = co_await co_await fs_open(path, OpenMode::Directory);
    co_return DirectoryWalker(std::move(handle));
}
} // namespace co_async
