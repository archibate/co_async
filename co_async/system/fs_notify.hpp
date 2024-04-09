/*{module;}*/

#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fanotify.h>
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

/*[export]*/ struct FileNotifier : FileIStream {
    using FileIStream::FileIStream;

    enum FileAccessType : std::uint32_t {
        OnAccessed = FAN_ACCESS,
        OnOpened = FAN_OPEN,
        OnExecuted = FAN_OPEN_EXEC,
        OnAttributeChanged = FAN_ATTRIB,
        OnModified = FAN_MODIFY,
        OnDeleted = FAN_DELETE_SELF,
        OnMoved = FAN_MOVE_SELF,
        OnChildCreated = FAN_CREATE,
        OnChildDeleted = FAN_DELETE,
        OnChildRenamed = FAN_RENAME,
        OnChildMovedAway = FAN_MOVED_FROM,
        OnChildMovedInto = FAN_MOVED_TO,
        OnChildMovedIntoOrAway = FAN_MOVE,
        OnFilesystemError = FAN_FS_ERROR,
        OnWriteFinished = FAN_CLOSE_WRITE,
        OnReadFinished = FAN_CLOSE_NOWRITE,
        OnReadOrWriteFinished = FAN_CLOSE,
        OnDirectoryOnly = FAN_ONDIR,
        OnDirectoryChildrenOnly = FAN_EVENT_ON_CHILD,
    };


    FileNotifier() : FileIStream(FileHandle(checkError(fanotify_init(FAN_CLASS_NOTIF, O_RDONLY)))) {
    }

    FileNotifier &add(DirFilePath path, FileAccessType mask) {
        checkError(fanotify_mark(get().fileNo(), FAN_MARK_ADD, mask, path.dir_file(), path.c_str()));
        return *this;
    }

    FileNotifier &remove(DirFilePath path, FileAccessType mask) {
        checkError(fanotify_mark(get().fileNo(), FAN_MARK_REMOVE, mask, path.dir_file(), path.c_str()));
        return *this;
    }

    struct WaitFileResult {
        Pid pid;
        FileAccessType mask;
        std::string metadata;
    };

    Task<WaitFileResult> wait() {
        struct fanotify_event_metadata meta;
        if (!co_await getstruct(meta)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        if (meta.vers != FANOTIFY_METADATA_VERSION) [[unlikely]] {
            throw std::runtime_error("fanotify version mismatch");
        }
        std::string metadata;
        if (meta.event_len >= meta.metadata_len + sizeof(meta)) [[likely]] {
            metadata.reserve(meta.metadata_len);
            co_await getn(metadata, meta.metadata_len);
            co_await dropn(meta.event_len - meta.metadata_len - sizeof(meta));
        }
        co_return {
            .pid = meta.pid,
            .mask = (FileAccessType)meta.mask,
            .metadata = std::move(metadata),
        };
    }
};

} // namespace co_async
#endif
