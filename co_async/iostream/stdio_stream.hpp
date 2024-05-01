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

struct StdioStreamRaw : StreamRaw {
    void disableTTYCanonAndEcho() {
        if (isatty(mFileIn.fileNo())) {
            struct termios tc;
            tcgetattr(mFileIn.fileNo(), &tc);
            tc.c_lflag &= ~ICANON;
            tc.c_lflag &= ~ECHO;
            tcsetattr(mFileIn.fileNo(), TCSANOW, &tc);
        }
    }

    explicit StdioStreamRaw(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<Expected<std::size_t, std::errc>>
    raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFileIn, buffer);
    }

    Task<Expected<std::size_t, std::errc>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFileOut, buffer);
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

template <int fileNo>
inline FileHandle &stdFileHandle() {
    static FileHandle h(fileNo);
    return h;
}

inline OwningStream &stdio() {
    static thread_local OwningStream s = make_stream<StdioStreamRaw>(
        stdFileHandle<STDIN_FILENO>(), stdFileHandle<STDOUT_FILENO>());
    return s;
}

} // namespace co_async
