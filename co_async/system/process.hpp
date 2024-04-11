/*{module;}*/

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>
#include <signal.h>
#endif

#pragma once/*{export module co_async:system.socket;}*/

#include <co_async/std.hpp>/*{import std;}*/

#ifdef __linux__
#include <co_async/system/error_handling.hpp>/*{import :system.error_handling;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/system/system_loop.hpp>/*{import :system.system_loop;}*/
#include <co_async/utils/string_utils.hpp>/*{import :utils.string_utils;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/

namespace co_async {

/*[export]*/ using Pid = pid_t;

/*[export]*/ struct WaitProcessResult {
    Pid pid;
    int status;

    enum ExitType : int {
        Continued = CLD_CONTINUED,
        Stopped = CLD_STOPPED,
        Trapped = CLD_TRAPPED,
        Dumped = CLD_DUMPED,
        Killed = CLD_KILLED,
        Exited = CLD_EXITED,
    } exitType;
};

/*[export]*/ struct PipeHandlePair {
    FileHandle mReader;
    FileHandle mWriter;

    FileHandle reader() {
#if CO_ASYNC_DEBUG
        if (!mReader) [[unlikely]] {
            throw std::invalid_argument(
                "PipeHandlePair::reader() can only be called once");
        }
#endif
        return std::move(mReader);
    }

    FileHandle writer() {
#if CO_ASYNC_DEBUG
        if (!mWriter) [[unlikely]] {
            throw std::invalid_argument(
                "PipeHandlePair::writer() can only be called once");
        }
#endif
        return std::move(mWriter);
    }
};

/*[export]*/ inline Task<PipeHandlePair> make_pipe() {
    int p[2];
    checkError(pipe2(p, 0));
    co_return {FileHandle(p[0]), FileHandle(p[1])};
}

/*[export]*/ inline Task<> kill_process(Pid pid, int sig = SIGKILL) {
    checkError(kill(pid, sig));
    co_return;
}

/*[export]*/ inline Task<WaitProcessResult> wait_process(Pid pid) {
    siginfo_t info{};
    co_await uring_waitid(P_PID, pid, &info, WEXITED, 0);
    co_return {
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
    };
}

/*[export]*/ struct ProcessBuilder {
    ProcessBuilder() {
        mAbsolutePath = false;
        mEnvInherited = false;
        checkError(posix_spawnattr_init(&mAttr));
        checkError(posix_spawn_file_actions_init(&mFileActions));
    }

    ProcessBuilder(ProcessBuilder &&) = delete;

    ~ProcessBuilder() {
        posix_spawnattr_destroy(&mAttr);
        posix_spawn_file_actions_destroy(&mFileActions);
    }

    ProcessBuilder &chdir(std::filesystem::path path) {
        checkError(
            posix_spawn_file_actions_addchdir_np(&mFileActions, path.c_str()));
        return *this;
    }

    ProcessBuilder &open(int fd, FileHandle const &file) {
        return open(fd, file.fileNo());
    }

    ProcessBuilder &open(int fd, int ourFd) {
        checkError(posix_spawn_file_actions_adddup2(&mFileActions, ourFd, fd));
        return *this;
    }

    ProcessBuilder &close(int fd) {
        checkError(posix_spawn_file_actions_addclose(&mFileActions, fd));
        return *this;
    }

    ProcessBuilder &path(std::string_view path, bool isAbsolute = false) {
        mPath = path;
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

    Task<Pid> spawn() {
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
        checkErrorReturn((mAbsolutePath ? posix_spawn : posix_spawnp)(
            &pid, mPath.c_str(), &mFileActions, &mAttr, argv.data(),
            mEnvpStore.empty() ? environ : envp.data()));
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
};

} // namespace co_async
#endif
