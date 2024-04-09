/*{module;}*/

#ifdef __linux__
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#pragma once/*{export module co_async:system.fs;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

#ifdef __linux__

#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/system/system_loop.hpp>/*{import :system.system_loop;}*/
#include <co_async/system/error_handling.hpp>/*{import :system.error_handling;}*/

namespace co_async {

/*[export]*/ struct [[nodiscard]] FileHandle {
    FileHandle() noexcept : mFileNo(-1) {}

    explicit FileHandle(int fileNo) noexcept : mFileNo(fileNo) {}

    int fileNo() const noexcept {
        return mFileNo;
    }

    int releaseFile() noexcept {
        int ret = mFileNo;
        mFileNo = -1;
        return ret;
    }

    explicit operator bool() noexcept {
        return mFileNo != -1;
    }

    FileHandle(FileHandle &&that) noexcept : mFileNo(that.releaseFile()) {}

    FileHandle &operator=(FileHandle &&that) noexcept {
        std::swap(mFileNo, that.mFileNo);
        return *this;
    }

    ~FileHandle() {
        if (mFileNo != -1)
            close(mFileNo);
    }

protected:
    int mFileNo;
};

/*[export]*/ struct [[nodiscard]] DirFilePath {
    DirFilePath(std::filesystem::path path) : mPath(path), mDirFd(AT_FDCWD) {}

    explicit DirFilePath(std::filesystem::path path, FileHandle const &dir)
        : mPath(path),
          mDirFd(dir.fileNo()) {}

    char const *c_str() const noexcept {
        return mPath.c_str();
    }

    std::filesystem::path path() const {
        return mPath;
    }

    int dir_file() const noexcept {
        return mDirFd;
    }

private:
    std::filesystem::path mPath;
    int mDirFd;
};

/*[export]*/ struct FileStat {
    struct statx *getNativeStatx() {
        return &mStatx;
    }

    std::uint64_t size() const noexcept {
        return mStatx.stx_size;
    }

    std::uint64_t num_blocks() const noexcept {
        return mStatx.stx_blocks;
    }

    mode_t mode() const noexcept {
        return mStatx.stx_mode;
    }

    std::chrono::system_clock::time_point accessed_time() const {
        return statTimestampToTimePoint(mStatx.stx_atime);
    }

    std::chrono::system_clock::time_point created_time() const {
        return statTimestampToTimePoint(mStatx.stx_btime);
    }

    std::chrono::system_clock::time_point modified_time() const {
        return statTimestampToTimePoint(mStatx.stx_mtime);
    }

private:
    struct statx mStatx;

    static std::chrono::system_clock::time_point statTimestampToTimePoint(struct statx_timestamp const &time) {
        return std::chrono::system_clock::time_point(std::chrono::seconds(time.tv_sec) + std::chrono::nanoseconds(time.tv_nsec));
    }
};

/*[export]*/ enum class OpenMode : int {
    Read = O_RDONLY,
    Write = O_WRONLY | O_TRUNC | O_CREAT,
    ReadWrite = O_RDWR | O_CREAT,
    Append = O_WRONLY | O_APPEND | O_CREAT,
};

/*[export]*/ inline std::filesystem::path make_path(std::string path) {
    return std::filesystem::path((char8_t const *)path.c_str());
}

template <std::convertible_to<std::string>... Ts>
    requires(sizeof...(Ts) >= 2)
/*[export]*/ inline std::filesystem::path make_path(Ts &&...chunks) {
    return (make_path(chunks) / ...);
}

/*[export]*/ inline Task<FileHandle> fs_open(DirFilePath path, OpenMode mode,
                                             mode_t access = 0644) {
    int oflags = (int)mode;
    int fd = co_await uring_openat(loop, path.dir_file(), path.c_str(), oflags,
                                   access);
    FileHandle file(fd);
    co_return file;
}

/*[export]*/ inline Task<> fs_close(FileHandle &&file) {
    co_await uring_close(loop, file.fileNo());
    file.releaseFile();
}

/*[export]*/ inline Task<> fs_mkdir(DirFilePath path, mode_t access = 0755) {
    co_await uring_mkdirat(loop, path.dir_file(), path.c_str(), access);
}

/*[export]*/ inline Task<> fs_link(DirFilePath oldpath, DirFilePath newpath) {
    co_await uring_linkat(loop, oldpath.dir_file(), oldpath.c_str(),
                          newpath.dir_file(), newpath.c_str(), 0);
}

/*[export]*/ inline Task<> fs_symlink(DirFilePath target,
                                      DirFilePath linkpath) {
    co_await uring_symlinkat(loop, target.c_str(), linkpath.dir_file(),
                             linkpath.c_str());
}

/*[export]*/ inline Task<> fs_unlink(DirFilePath path) {
    co_await uring_unlinkat(loop, path.dir_file(), path.c_str(), 0);
}

/*[export]*/ inline Task<> fs_rmdir(DirFilePath path) {
    co_await uring_unlinkat(loop, path.dir_file(), path.c_str(), AT_REMOVEDIR);
}

/*[export]*/ inline Task<std::optional<FileStat>>
fs_stat(DirFilePath path, int mask = STATX_BASIC_STATS | STATX_BTIME) {
    FileStat ret;
    int res = co_await uring_statx(loop, path.dir_file(), path.c_str(), 0, mask,
                                   ret.getNativeStatx());
    int const allowed[] = {ENOENT, ENOTDIR, ENAMETOOLONG, ELOOP, EACCES};
    if (res < 0) {
        res = -res;
        for (auto e: allowed) {
            if (res == e) {
                co_return std::nullopt;
            }
        }
    }
    co_return ret;
}

/*[export]*/ inline Task<std::uint64_t> fs_stat_size(DirFilePath path) {
    FileStat ret;
    checkErrorReturn(co_await uring_statx(loop, path.dir_file(), path.c_str(),
                                          0, STATX_SIZE, ret.getNativeStatx()));
    co_return ret.size();
}

/*[export]*/ inline Task<std::size_t>
fs_read(FileHandle &file, std::span<char> buffer, std::uint64_t offset = -1) {
    return uring_read(loop, file.fileNo(), buffer, offset);
}

/*[export]*/ inline Task<std::size_t> fs_write(FileHandle &file,
                                               std::span<char const> buffer,
                                               std::uint64_t offset = -1) {
    return uring_write(loop, file.fileNo(), buffer, offset);
}

/*[export]*/ inline Task<> fs_truncate(FileHandle &file,
                                       std::uint64_t size = 0) {
    co_await uring_ftruncate(loop, file.fileNo(), size);
}

} // namespace co_async
#endif
