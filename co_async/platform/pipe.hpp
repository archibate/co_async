
#ifdef __linux__
# include <unistd.h>
#endif
#pragma once
#include <co_async/std.hpp>
#ifdef __linux__
# include <co_async/awaiter/task.hpp>
# include <co_async/iostream/file_stream.hpp>
# include <co_async/platform/error_handling.hpp>
# include <co_async/platform/fs.hpp>

namespace co_async {
struct FSPipeHandlePair {
    FileHandle mReader;
    FileHandle mWriter;

    std::array<OwningStream, 2> stream() {
        return {file_from_handle(reader()), file_from_handle(writer())};
    }

    FileHandle reader() {
# if CO_ASYNC_DEBUG
        if (!mReader) [[unlikely]] {
            throw std::logic_error(
                "PipeHandlePair::reader() can only be called once");
        }
# endif
        return std::move(mReader);
    }

    FileHandle writer() {
# if CO_ASYNC_DEBUG
        if (!mWriter) [[unlikely]] {
            throw std::logic_error(
                "PipeHandlePair::writer() can only be called once");
        }
# endif
        return std::move(mWriter);
    }
};

inline Task<Expected<FSPipeHandlePair>> fs_pipe() {
    int p[2];
    int res = pipe2(p, 0);
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_await expectError(res);
    co_return FSPipeHandlePair{FileHandle(p[0]), FileHandle(p[1])};
}

inline Task<Expected<>> send_file(FileHandle &sock, FileHandle &&file) {
    auto [readPipe, writePipe] = co_await co_await fs_pipe();
    while (auto n = co_await co_await fs_splice(file, writePipe, 65536)) {
        std::size_t m;
        while ((m = co_await co_await fs_splice(readPipe, sock, n)) < n)
            [[unlikely]] {
            n -= m;
        }
    }
    co_await co_await fs_close(std::move(file));
    co_await co_await fs_close(std::move(readPipe));
    co_await co_await fs_close(std::move(writePipe));
    co_return {};
}

inline Task<Expected<>> recv_file(FileHandle &sock, FileHandle &&file) {
    auto [readPipe, writePipe] = co_await co_await fs_pipe();
    while (auto n = co_await co_await fs_splice(sock, writePipe, 65536)) {
        std::size_t m;
        while ((m = co_await co_await fs_splice(readPipe, file, n)) < n)
            [[unlikely]] {
            n -= m;
        }
    }
    co_await co_await fs_close(std::move(file));
    co_await co_await fs_close(std::move(readPipe));
    co_await co_await fs_close(std::move(writePipe));
    co_return {};
}
} // namespace co_async
#endif
