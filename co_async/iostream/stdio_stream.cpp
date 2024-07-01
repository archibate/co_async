#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stdio_stream.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>
#include <termios.h>
#include <unistd.h>

namespace co_async {
namespace {
struct StdioStream : Stream {
    explicit StdioStream(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFileIn, buffer, co_await co_cancel);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFileOut, buffer, co_await co_cancel);
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

int rawStdinFileHandleImpl() {
    if (isatty(STDIN_FILENO)) {
        struct termios tc;
        tcgetattr(STDIN_FILENO, &tc);
        tc.c_lflag &= ~static_cast<tcflag_t>(ICANON);
        tc.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &tc);
    }
    return STDIN_FILENO;
}

FileHandle &rawStdinFileHandle() {
    static FileHandle h(rawStdinFileHandleImpl());
    return h;
}

} // namespace

OwningStream &stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        stdFileHandle<STDIN_FILENO>(), stdFileHandle<STDOUT_FILENO>());
    return s;
}

OwningStream &raw_stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        rawStdinFileHandle(), stdFileHandle<STDOUT_FILENO>());
    return s;
}
} // namespace co_async
