#pragma once

#include <coroutine>
#include <chrono>
#include <cstdint>
#include <utility>
#include <optional>
#include <string>
#include <string_view>
#include <span>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>
#include <co_async/task.hpp>
#include <co_async/epoll_loop.hpp>
#include <co_async/error_handling.hpp>

namespace co_async {

enum class OpenMode : int {
    Read = O_RDONLY,
    Write = O_WRONLY | O_CREAT,
    ReadWrite = O_RDWR | O_CREAT,
    Append = O_WRONLY | O_APPEND | O_CREAT,
    ReadAppend = O_RDWR | O_APPEND | O_CREAT,
};

inline Task<AsyncFile> open_fs_file(EpollLoop &loop, std::filesystem::path path, OpenMode mode, mode_t access = 0644) {
    int oflags = (int)mode;
    oflags |= O_NONBLOCK;
    int res = checkError(open(path.c_str(), oflags, access));
    AsyncFile file(res);
    co_return file;
}

}
