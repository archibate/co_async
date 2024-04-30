

#ifdef __linux__
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#endif

#pragma once

#include <co_async/std.hpp>

#ifdef __linux__

#include <co_async/awaiter/task.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/process.hpp>
#include <co_async/iostream/file_stream.hpp>
#include <co_async/system/error_handling.hpp>

namespace co_async {

struct FileWatch : FileIStream {
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

    FileWatch() : FileIStream(FileHandle(throwingError(inotify_init1(0)))) {}

    FileWatch &watch(std::filesystem::path path, FileEvent event,
                     bool recursive = false) {
        int wd =
            throwingError(inotify_add_watch(get().fileNo(), path.c_str(), event));
        mWatches.emplace(wd, path);
        if (recursive && std::filesystem::is_directory(path)) {
            for (auto const &entry:
                 std::filesystem::recursive_directory_iterator(path)) {
                watch(entry.path(), event, false);
            }
        }
        return *this;
    }

    /* FileWatch &unwatch(std::filesystem::path path) { */
    /* checkError(inotify_rm_watch(get().fileNo(), wd)); */
    /* } */

    struct WaitFileResult {
        std::filesystem::path path;
        FileEvent event;
    };

    Task<Expected<WaitFileResult>> wait() {
        if (!co_await getstruct(*mEventBuffer)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        std::string name;
        name.reserve(mEventBuffer->len);
        co_await co_await getn(name, mEventBuffer->len);
        name = name.c_str();
        auto path = mWatches.at(mEventBuffer->wd);
        if (!name.empty()) {
            path /= make_path(name);
        }
        co_return WaitFileResult{
            .path = std::move(path),
            .event = (FileEvent)mEventBuffer->mask,
        };
    }

private:
    std::unique_ptr<struct inotify_event> mEventBuffer =
        std::make_unique<struct inotify_event>();
    std::map<int, std::filesystem::path> mWatches;
};

} // namespace co_async
#endif
