#pragma once

#include <unistd.h>
#include <termios.h>
#include <co_async/task.hpp>
#include <co_async/epoll_loop.hpp>

namespace co_async {

inline AsyncFile asyncStdFile(int fileNo) {
    AsyncFile file(checkError(dup(fileNo)));
    file.setNonblock();
    return file;
}

inline AsyncFile async_stdin(bool noCanon = false, bool noEcho = false) {
    AsyncFile file = asyncStdFile(STDIN_FILENO);
    if ((noCanon || noEcho) && isatty(file.fileNo())) {
        struct termios tc;
        tcgetattr(file.fileNo(), &tc);
        if (noCanon) tc.c_lflag &= ~ICANON;
        if (noEcho) tc.c_lflag &= ~ECHO;
        tcsetattr(file.fileNo(), TCSANOW, &tc);
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
