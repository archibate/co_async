/*{module;}*/

#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#endif

#pragma once/*{export module co_async:iostream.stdio_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

inline void disableCanon(FileHandle &file) {
    if (isatty(file.fileNo())) {
        struct termios tc;
        tcgetattr(file.fileNo(), &tc);
        tc.c_lflag &= ~ICANON;
        tc.c_lflag &= ~ECHO;
        tcsetattr(file.fileNo(), TCSANOW, &tc);
    }
}

struct StdioBuf {
    explicit StdioBuf(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<std::size_t> raw_read(std::span<char> buffer) {
        return fs_read(mFileIn, buffer);
    }

    Task<std::size_t> raw_write(std::span<char const> buffer) {
        return fs_write(mFileOut, buffer);
    }

    FileHandle &in() const noexcept {
        return mFileIn;
    }

    FileHandle &out() const noexcept {
        return mFileOut;
    }

private:
    FileHandle &mFileIn;
    FileHandle &mFileOut;
};

/*[export]*/ using StdioStream = IOStream<StdioBuf>;

template <int fileNo>
inline FileHandle &stdHandle() {
    static FileHandle h(fileNo);
    return h;
}

/*[export]*/ inline StdioStream &stdio() {
    static thread_local StdioStream s(stdHandle<STDIN_FILENO>(),
                                      stdHandle<STDOUT_FILENO>());
    return s;
}

} // namespace co_async
