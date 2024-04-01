module;

#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#endif

export module co_async:iostream.stdio_stream;

import std;
import :system.fs;
import :awaiter.task;
import :iostream.stream_base;

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

export struct StdioBuf {
    FileHandle &mFileIn;
    FileHandle &mFileOut;

    Task<std::size_t> read(std::span<char> buffer) {
        return fs_read(mFileIn, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return fs_write(mFileOut, buffer);
    }
};

export using StdioStream = IOStream<StdioBuf>;

template <int fileNo>
inline FileHandle &stdHandle() {
    static FileHandle h(fileNo);
    return h;
}

export inline StdioStream &ios() {
    static thread_local StdioStream s(stdHandle<STDIN_FILENO>(), stdHandle<STDOUT_FILENO>());
    return s;
}

} // namespace co_async
