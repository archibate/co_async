#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/platform_io.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace co_async {
struct [[nodiscard]] FileHandle {
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
        if (mFileNo != -1) {
            close(mFileNo);
        }
    }

protected:
    int mFileNo;
};

struct FileStat {
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

    unsigned int uid() const noexcept {
        return mStatx.stx_uid;
    }

    unsigned int gid() const noexcept {
        return mStatx.stx_gid;
    }

    bool is_directory() const noexcept {
        return (mStatx.stx_mode & S_IFDIR) != 0;
    }

    bool is_regular_file() const noexcept {
        return (mStatx.stx_mode & S_IFREG) != 0;
    }

    bool is_symlink() const noexcept {
        return (mStatx.stx_mode & S_IFLNK) != 0;
    }

    bool is_readable() const noexcept {
        return (mStatx.stx_mode & S_IRUSR) != 0;
    }

    bool is_writable() const noexcept {
        return (mStatx.stx_mode & S_IWUSR) != 0;
    }

    bool is_executable() const noexcept {
        return (mStatx.stx_mode & S_IXUSR) != 0;
    }

    std::chrono::system_clock::time_point accessed_time() const {
        return statTimestampToTimePoint(mStatx.stx_atime);
    }

    std::chrono::system_clock::time_point attribute_changed_time() const {
        return statTimestampToTimePoint(mStatx.stx_ctime);
    }

    std::chrono::system_clock::time_point created_time() const {
        return statTimestampToTimePoint(mStatx.stx_btime);
    }

    std::chrono::system_clock::time_point modified_time() const {
        return statTimestampToTimePoint(mStatx.stx_mtime);
    }

private:
    struct statx mStatx;

    static std::chrono::system_clock::time_point
    statTimestampToTimePoint(struct statx_timestamp const &time) {
        return std::chrono::system_clock::time_point(
            std::chrono::seconds(time.tv_sec) +
            std::chrono::nanoseconds(time.tv_nsec));
    }
};

#if CO_ASYNC_DIRECT
static constexpr size_t kOpenModeDefaultFlags =
    O_LARGEFILE | O_CLOEXEC | O_DIRECT;
#else
static constexpr size_t kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC;
#endif

enum class OpenMode : int {
    Read = O_RDONLY | kOpenModeDefaultFlags,
    Write = O_WRONLY | O_TRUNC | O_CREAT | kOpenModeDefaultFlags,
    ReadWrite = O_RDWR | O_CREAT | kOpenModeDefaultFlags,
    Append = O_WRONLY | O_APPEND | O_CREAT | kOpenModeDefaultFlags,
    Directory = O_RDONLY | O_DIRECTORY | kOpenModeDefaultFlags,
};

inline std::filesystem::path make_path(std::string_view path) {
    return std::filesystem::path(
        reinterpret_cast<char8_t const *>(std::string(path).c_str()));
}

template <std::convertible_to<std::string_view>... Ts>
    requires(sizeof...(Ts) >= 2)
inline std::filesystem::path make_path(Ts &&...chunks) {
    return (make_path(chunks) / ...);
}

inline Task<Expected<FileHandle>> fs_open(std::filesystem::path path, OpenMode mode,
                                          mode_t access = 0644) {
    int oflags = static_cast<int>(mode);
    int fd = co_await expectError(co_await UringOp().prep_openat(
        AT_FDCWD, path.c_str(), oflags, access))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] { return expectError(open(path.c_str(), oflags, access)); })
#endif
        ;
    FileHandle file(fd);
    co_return file;
}

inline Task<Expected<FileHandle>> fs_openat(FileHandle dir,
                                            std::filesystem::path path,
                                            OpenMode mode,
                                            mode_t access = 0644) {
    int oflags = static_cast<int>(mode);
    int fd = co_await expectError(co_await UringOp().prep_openat(
        dir.fileNo(), path.c_str(), oflags, access))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] { return expectError(openat(dir.fileNo(), path.c_str(), oflags, access)); })
#endif
        ;
    FileHandle file(fd);
    co_return file;
}

inline Task<Expected<>> fs_close(FileHandle file) {
    co_await expectError(co_await UringOp().prep_close(file.fileNo()));
    file.releaseFile();
    co_return {};
}

inline Task<Expected<>> fs_mkdir(std::filesystem::path path, mode_t access = 0755) {
    co_await expectError(
        co_await UringOp().prep_mkdirat(AT_FDCWD, path.c_str(), access));
    co_return {};
}

inline Task<Expected<>> fs_link(std::filesystem::path oldpath, std::filesystem::path newpath) {
    co_await expectError(
        co_await UringOp().prep_linkat(AT_FDCWD, oldpath.c_str(),
                                       AT_FDCWD, newpath.c_str(), 0));
    co_return {};
}

inline Task<Expected<>> fs_symlink(std::filesystem::path target, std::filesystem::path linkpath) {
    co_await expectError(co_await UringOp().prep_symlinkat(
        target.c_str(), AT_FDCWD, linkpath.c_str()));
    co_return {};
}

inline Task<Expected<>> fs_unlink(std::filesystem::path path) {
    co_await expectError(
        co_await UringOp().prep_unlinkat(AT_FDCWD, path.c_str(), 0));
    co_return {};
}

inline Task<Expected<>> fs_rmdir(std::filesystem::path path) {
    co_await expectError(co_await UringOp().prep_unlinkat(
        AT_FDCWD, path.c_str(), AT_REMOVEDIR));
    co_return {};
}

inline Task<Expected<FileStat>>
fs_stat(std::filesystem::path path, unsigned int mask = STATX_BASIC_STATS | STATX_BTIME, int flags = 0) {
    FileStat ret;
    co_await expectError(co_await UringOp().prep_statx(
        AT_FDCWD, path.c_str(), flags, mask, ret.getNativeStatx()))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::bad_file_descriptor, [&] { return expectError(statx(AT_FDCWD, path.c_str(), flags, mask, ret.getNativeStatx())); })
#endif
            ;
    co_return ret;
}

inline Task<Expected<std::size_t>>
fs_read(FileHandle &file, std::span<char> buffer,
        std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(
            co_await UringOp().prep_read(file.fileNo(), buffer, offset))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(read(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pread64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_write(FileHandle &file, std::span<char const> buffer,
         std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(
            co_await UringOp().prep_write(file.fileNo(), buffer, offset))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(write(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pwrite64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_read(FileHandle &file, std::span<char> buffer, CancelToken cancel,
        std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_read(file.fileNo(), buffer, offset)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(read(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pread64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_write(FileHandle &file, std::span<char const> buffer, CancelToken cancel,
         std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_write(file.fileNo(), buffer, offset)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(write(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pwrite64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<>> fs_truncate(FileHandle &file, std::uint64_t size = 0) {
    co_await expectError(co_await UringOp().prep_ftruncate(
        file.fileNo(), static_cast<loff_t>(size)));
    co_return {};
}

inline Task<Expected<std::size_t>>
fs_splice(FileHandle &fileIn, FileHandle &fileOut, std::size_t size,
          std::int64_t offsetIn = -1, std::int64_t offsetOut = -1) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp().prep_splice(
            fileIn.fileNo(), offsetIn, fileOut.fileNo(), offsetOut, size, 0)));
}

inline Task<Expected<std::size_t>> fs_getdents(FileHandle &dirFile,
                                               std::span<char> buffer) {
    int res = static_cast<int>(
        getdents64(dirFile.fileNo(), buffer.data(), buffer.size()));
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_return static_cast<std::size_t>(co_await expectError(res));
}

inline Task<int> fs_nop() {
    co_return co_await UringOp().prep_nop();
}

inline Task<Expected<>> fs_cancel_fd(FileHandle &file) {
    co_await expectError(co_await UringOp().prep_cancel_fd(
        file.fileNo(), IORING_ASYNC_CANCEL_FD | IORING_ASYNC_CANCEL_ALL));
    co_return {};
}
} // namespace co_async
