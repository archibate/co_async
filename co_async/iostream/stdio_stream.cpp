#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stdio_stream.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>
#include <termios.h>
#include <unistd.h>

namespace co_async {
namespace {
struct StdioStream : Stream {
    void disableTTYCanonAndEcho() {
        if (isatty(mFileIn.fileNo())) {
            struct termios tc;
            tcgetattr(mFileIn.fileNo(), &tc);
            tc.c_lflag &= ~(tcflag_t)ICANON;
            tc.c_lflag &= ~(tcflag_t)ECHO;
            tcsetattr(mFileIn.fileNo(), TCSANOW, &tc);
        }
    }

    explicit StdioStream(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFileIn, buffer);
    }

    Task<Expected<std::size_t>>
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
FileHandle &stdFileHandle() {
    static FileHandle h(fileNo);
    return h;
}
} // namespace

OwningStream &stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        stdFileHandle<STDIN_FILENO>(), stdFileHandle<STDOUT_FILENO>());
    return s;
}
} // namespace co_async
