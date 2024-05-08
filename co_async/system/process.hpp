#pragma once

#include <co_async/std.hpp>

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>
#include <signal.h>
#include <errno.h>
#include <co_async/system/error_handling.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/system/timer.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/threading/future_group.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {

using Pid = pid_t;

struct WaitProcessResult {
    Pid pid;
    int status;

    enum ExitType : int {
        Continued = CLD_CONTINUED,
        Stopped = CLD_STOPPED,
        Trapped = CLD_TRAPPED,
        Dumped = CLD_DUMPED,
        Killed = CLD_KILLED,
        Exited = CLD_EXITED,
        Timeout = -1,
    } exitType;
};

inline Task<Expected<>> kill_process(Pid pid, int sig = SIGKILL) {
    co_await expectError(kill(pid, sig));
    co_return {};
}

inline Task<Expected<WaitProcessResult>> wait_process(Pid pid,
                                                      int options = WEXITED) {
    siginfo_t info{};
    co_await expectError(co_await uring_waitid(P_PID, pid, &info, options, 0));
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
    };
}

inline Task<Expected<WaitProcessResult>>
wait_process(Pid pid, std::chrono::nanoseconds timeout, int options = WEXITED) {
    siginfo_t info{};
    auto ts = durationToKernelTimespec(timeout);
    auto ret = expectError(
        co_await uring_join(uring_waitid(P_PID, pid, &info, options, 0),
                            uring_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    if (ret == std::make_error_code(std::errc::operation_canceled)) {
        co_return Unexpected{std::make_error_code(std::errc::timed_out)};
    }
    co_await std::move(ret);
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
    };
}

struct ProcessBuilder {
    ProcessBuilder() {
        mAbsolutePath = false;
        mEnvInherited = false;
        throwingErrorErrno(posix_spawnattr_init(&mAttr));
        throwingErrorErrno(posix_spawn_file_actions_init(&mFileActions));
    }

    ProcessBuilder(ProcessBuilder &&) = delete;

    ~ProcessBuilder() {
        posix_spawnattr_destroy(&mAttr);
        posix_spawn_file_actions_destroy(&mFileActions);
    }

    ProcessBuilder &chdir(std::filesystem::path path) {
        throwingErrorErrno(
            posix_spawn_file_actions_addchdir_np(&mFileActions, path.c_str()));
        return *this;
    }

    ProcessBuilder &open(int fd, FileHandle &&file) {
        open(fd, file.fileNo());
        mFileStore.push_back(std::move(file));
        return *this;
    }

    ProcessBuilder &open(int fd, FileHandle const &file) {
        return open(fd, file.fileNo());
    }

    ProcessBuilder &open(int fd, int ourFd) {
        if (fd != ourFd) {
            throwingErrorErrno(
                posix_spawn_file_actions_adddup2(&mFileActions, ourFd, fd));
        }
        return *this;
    }

    ProcessBuilder &pipe_out(int fd, OwningStream &stream) {
        int p[2];
        throwingErrorErrno(pipe2(p, 0));
        open(fd, FileHandle(p[1]));
        stream = file_from_handle(FileHandle(p[0]));
        close(p[0]);
        close(p[1]);
        return *this;
    }

    ProcessBuilder &pipe_in(int fd, OwningStream &stream) {
        int p[2];
        throwingErrorErrno(pipe2(p, 0));
        open(fd, FileHandle(p[0]));
        stream = file_from_handle(FileHandle(p[1]));
        close(p[0]);
        close(p[1]);
        return *this;
    }

    ProcessBuilder &close(int fd) {
        throwingErrorErrno(
            posix_spawn_file_actions_addclose(&mFileActions, fd));
        return *this;
    }

    ProcessBuilder &path(std::filesystem::path path, bool isAbsolute = false) {
        mPath = path.string();
        mAbsolutePath = isAbsolute;
        return *this;
    }

    ProcessBuilder &arg(std::string_view arg) {
        mArgvStore.emplace_back(arg);
        return *this;
    }

    ProcessBuilder &inherit_env(bool inherit = true) {
        if (inherit) {
            for (char *const *e = environ; *e; ++e) {
                mEnvpStore.emplace_back(*e);
            }
        }
        mEnvInherited = true;
        return *this;
    }

    ProcessBuilder &env(std::string_view key, std::string_view val) {
        if (!mEnvInherited) {
            inherit_env();
        }
        std::string env(key);
        env.push_back('=');
        env.append(val);
        mEnvpStore.emplace_back(std::move(env));
        return *this;
    }

    Task<Expected<Pid>> spawn() {
        Pid pid;
        std::vector<char *> argv;
        std::vector<char *> envp;
        if (!mArgvStore.empty()) {
            argv.reserve(mArgvStore.size() + 2);
            argv.push_back(mPath.data());
            for (auto &s: mArgvStore) {
                argv.push_back(s.data());
            }
            argv.push_back(nullptr);
        } else {
            argv = {mPath.data(), nullptr};
        }
        if (!mEnvpStore.empty()) {
            envp.reserve(mEnvpStore.size() + 1);
            for (auto &s: mEnvpStore) {
                envp.push_back(s.data());
            }
            envp.push_back(nullptr);
        }
        int status = (mAbsolutePath ? posix_spawn : posix_spawnp)(
            &pid, mPath.c_str(), &mFileActions, &mAttr, argv.data(),
            mEnvpStore.empty() ? environ : envp.data());
        if (status != 0) [[unlikely]] {
            co_return Unexpected{std::make_error_code(std::errc(errno))};
        }
        mPath.clear();
        mArgvStore.clear();
        mEnvpStore.clear();
        mFileStore.clear();
        co_return pid;
    }

private:
    posix_spawn_file_actions_t mFileActions;
    posix_spawnattr_t mAttr;
    bool mAbsolutePath;
    bool mEnvInherited;
    std::string mPath;
    std::vector<std::string> mArgvStore;
    std::vector<std::string> mEnvpStore;
    std::vector<FileHandle> mFileStore;
};

//inline Task<Expected<OwningStream>>
//pipe_capture(FutureGroup &group, ProcessBuilder &process) {
//auto pipe = co_await co_await fs_pipe();
//process.open(1, pipe.writer());
//Pid pid = co_await co_await process.spawn();
//group.add([pid]() mutable -> Task<Expected<>> {
///* using namespace std::chrono_literals; */
///* co_await sleep_for(300ms); */
//co_return co_await wait_process(pid);
//});
//auto reader = file_from_handle(pipe.reader());
//co_return {std::move(reader)};
//}
//
//inline Task<Expected<OwningStream>>
//pipe_capture(FutureGroup &group, ProcessBuilder &process, BorrowedStream &in) {
//auto pipe = co_await co_await fs_pipe();
//process.open(0, pipe.reader());
//group.add([&in, writer = file_from_handle(pipe.writer())]() mutable -> Task<Expected<>> {
//while (true) {
//if (in.bufempty()) {
//if (!co_await in.fillbuf()) {
//break;
//}
//}
//co_await co_await writer.write(in.peekbuf());
//}
//co_return {};
//});
//co_return co_await pipe_capture(group, process);
//}
//
//inline Task<Expected<std::string>>
//pipe_capture_string(ProcessBuilder &process,
//std::string_view in) {
//FutureGroup group;
//auto pipe = co_await co_await fs_pipe();
//process.open(0, pipe.reader());
//group.add([in, writer = file_from_handle(pipe.writer())]() mutable -> Task<Expected<>> {
//co_await co_await writer.puts(in);
//co_await co_await writer.flush();
//co_await writer.close();
//co_return {};
//});
//auto out = co_await co_await pipe_capture(group, process);
//auto ret = co_await out.getall();
///* auto f = file_from_handle(pipe.reader()); */
///* auto ret = co_await f.getall(); */
//co_await co_await group.wait();
//co_return ret;
//}
//
//inline Task<Expected<std::string>>
//pipe_capture_string(ProcessBuilder &process) {
//FutureGroup group;
//auto out = co_await co_await pipe_capture(group, process);
//auto ret = co_await out.getall();
//co_await co_await group.wait();
//co_return ret;
//}

} // namespace co_async
#endif
