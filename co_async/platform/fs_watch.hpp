#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/file_stream.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/platform/process.hpp>
#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace co_async {
struct FileWatch {
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

    FileWatch()
        : mFile(throwingErrorErrno(inotify_init1(0))),
          mStream(file_from_handle(FileHandle(mFile))) {}

    int add(std::filesystem::path const &path, FileEvent event) {
        int wd =
            throwingErrorErrno(inotify_add_watch(mFile, path.c_str(), event));
        mWatches.emplace(wd, path);
        return wd;
    }

    FileWatch &watch(std::filesystem::path const &path, FileEvent event,
                     bool recursive = false) {
        add(path, event);
        if (recursive && std::filesystem::is_directory(path)) {
            for (auto const &entry:
                 std::filesystem::recursive_directory_iterator(path)) {
                add(entry.path(), event);
            }
        }
        return *this;
    }

    FileWatch &remove(int wd) {
        throwingErrorErrno(inotify_rm_watch(mFile, wd));
        mWatches.erase(wd);
        return *this;
    }

    struct WaitFileResult {
        std::filesystem::path path;
        FileEvent event;
    };

    Task<Expected<WaitFileResult>> wait() {
        if (!co_await mStream.getstruct(*mEventBuffer)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        String name;
        name.reserve(mEventBuffer->len);
        co_await co_await mStream.getn(name, mEventBuffer->len);
        name = name.c_str();
        auto path = mWatches.at(mEventBuffer->wd);
        if (!name.empty()) {
            path /= make_path(name);
        }
        co_return WaitFileResult{
            .path = std::move(path),
            .event = static_cast<FileEvent>(mEventBuffer->mask),
        };
    }

private:
    int mFile;
    OwningStream mStream;
    std::unique_ptr<struct inotify_event> mEventBuffer =
        std::make_unique<struct inotify_event>();
    std::map<int, std::filesystem::path> mWatches;
};
} // namespace co_async
