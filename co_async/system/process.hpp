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
#include <co_async/system/system_loop.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/awaiter/task.hpp>
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

inline Task<Expected<void, std::errc>> kill_process(Pid pid,
                                                    int sig = SIGKILL) {
    co_await expectError(kill(pid, sig));
    co_return {};
}

inline Task<Expected<WaitProcessResult, std::errc>>
wait_process(Pid pid, int options = WEXITED) {
    siginfo_t info{};
    co_await expectError(co_await uring_waitid(P_PID, pid, &info, options, 0));
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
    };
}

inline Task<Expected<WaitProcessResult, std::errc>>
wait_process(Pid pid, std::chrono::nanoseconds timeout, int options = WEXITED) {
    siginfo_t info{};
    auto ts = durationToKernelTimespec(timeout);
    auto ret = expectError(
        co_await uring_join(uring_waitid(P_PID, pid, &info, options, 0),
                            uring_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    if (ret == std::errc::operation_canceled) {
        co_return Unexpected{std::errc::timed_out};
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
        throwingError(posix_spawnattr_init(&mAttr));
        throwingError(posix_spawn_file_actions_init(&mFileActions));
    }

    ProcessBuilder(ProcessBuilder &&) = delete;

    ~ProcessBuilder() {
        posix_spawnattr_destroy(&mAttr);
        posix_spawn_file_actions_destroy(&mFileActions);
    }

    ProcessBuilder &chdir(std::filesystem::path path) {
        throwingError(
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
        throwingError(
            posix_spawn_file_actions_adddup2(&mFileActions, ourFd, fd));
        return *this;
    }

    ProcessBuilder &close(int fd) {
        throwingError(posix_spawn_file_actions_addclose(&mFileActions, fd));
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

    Task<Expected<Pid, std::errc>> spawn() {
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
            co_return Unexpected{std::errc(errno)};
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

} // namespace co_async
#endif
