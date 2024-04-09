/*{module;}*/

#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#endif

#pragma once/*{export module co_async:system.fs;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

#ifdef __linux__

#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/system/system_loop.hpp>/*{import :system.system_loop;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/system/process.hpp>/*{import :system.process;}*/
#include <co_async/iostream/file_stream.hpp>/*{import :iostream.file_stream;}*/
#include <co_async/system/error_handling.hpp>/*{import :system.error_handling;}*/

namespace co_async {

/*[export]*/ struct BasicFileNotifier : FileIStream {
    enum FileEvent : std::uint32_t {
        OnAccessed = IN_ACCESS,
        OnOpened = IN_OPEN,
        OnAttributeChanged = IN_ATTRIB,
        OnModified = IN_MODIFY,
        OnDeleted = IN_DELETE_SELF,
        OnMoved = IN_MOVE_SELF,
        OnChildCreated = IN_CREATE,
        OnChildDeleted = IN_DELETE,
        OnChildMovedAway = IN_MOVED_FROM,
        OnChildMovedInto = IN_MOVED_TO,
        OnWriteFinished = IN_CLOSE_WRITE,
        OnReadFinished = IN_CLOSE_NOWRITE,
    };


    BasicFileNotifier() : FileIStream(FileHandle(checkError(inotify_init1(0)))) {
    }

    int watch(std::filesystem::path path, FileEvent event) {
        return checkError(inotify_add_watch(get().fileNo(), path.c_str(), event));
    }

    void unwatch(int watch) {
        checkError(inotify_rm_watch(get().fileNo(), watch));
    }

    struct WaitFileResult {
        int watch;
        FileEvent event;
        std::string name;
    };

    Task<WaitFileResult> wait() {
        if (!co_await getstruct(*mEventBuffer)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        std::string name;
        name.reserve(mEventBuffer->len);
        co_await getn(name, mEventBuffer->len);
        co_return {
            .watch = mEventBuffer->wd,
            .event = (FileEvent)mEventBuffer->mask,
            .name = name.c_str(),
        };
    }

private:
    std::unique_ptr<struct inotify_event> mEventBuffer = std::make_unique<struct inotify_event>();
};

/*[export]*/ struct FileNotifier : private BasicFileNotifier {
    using BasicFileNotifier::BasicFileNotifier;
    using enum BasicFileNotifier::FileEvent;

    FileNotifier &watch(std::filesystem::path path, FileEvent event, bool recursive = false) {
        int wd = BasicFileNotifier::watch(path, event);
        mWatches.emplace(wd, path);
        if (recursive && std::filesystem::is_directory(path)) {
            for (auto const &entry: std::filesystem::recursive_directory_iterator(path)) {
                watch(entry.path(), event, false);
            }
        }
        return *this;
    }

    struct WaitFileResult {
        std::filesystem::path path;
        FileEvent event;
    };

    Task<WaitFileResult> wait() {
        auto basicRes = co_await BasicFileNotifier::wait();
        auto path = mWatches.at(basicRes.watch);
        if (!basicRes.name.empty()) {
            path /= make_path(basicRes.name);
        }
        co_return {
            .path = std::move(path),
            .event = (FileEvent)basicRes.event,
        };
    }

private:
    std::map<int, std::filesystem::path> mWatches;
};

} // namespace co_async
#endif
