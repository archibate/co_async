#pragma once

#include <unistd.h>
#include <termios.h>
#include <co_async/task.hpp>
#include <co_async/epoll_loop.hpp>

namespace co_async {

inline void termiosDisableCanon(int fileNo) {
    struct termios tc;
    tcgetattr(fileNo, &tc);
    tc.c_lflag &= ~ICANON;
    tc.c_lflag &= ~ECHO;
    tcsetattr(fileNo, TCSANOW, &tc);
}

inline AsyncFile asyncStdFile(int fileNo) {
    AsyncFile file(checkError(dup(fileNo)));
    file.setNonblock();
    return file;
}

inline AsyncFile async_stdin() {
    AsyncFile file = asyncStdFile(STDIN_FILENO);
    if (isatty(file.fileNo())) {
        termiosDisableCanon(file.fileNo());
    }
    return file;
}

inline AsyncFile async_stdout() {
    return asyncStdFile(STDOUT_FILENO);
}

inline AsyncFile async_stderr() {
    return asyncStdFile(STDERR_FILENO);
}

}
