

#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#endif

#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

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

struct StdioStreamRaw : virtual IOStreamRaw {
    explicit StdioStreamRaw(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<std::size_t> raw_read(std::span<char> buffer) override {
        co_return (co_await fs_read(mFileIn, buffer)).value_or(0);
    }

    Task<std::size_t> raw_write(std::span<char const> buffer) override {
        co_return (co_await fs_write(mFileOut, buffer)).value_or(0);
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

using StdioStream = IOStreamImpl<StdioStreamRaw>;

template <int fileNo>
inline FileHandle &stdHandle() {
    static FileHandle h(fileNo);
    return h;
}

inline StdioStream &stdio() {
    static thread_local StdioStream s(stdHandle<STDIN_FILENO>(),
                                      stdHandle<STDOUT_FILENO>());
    return s;
}

} // namespace co_async
