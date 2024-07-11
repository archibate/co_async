#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>
#include <dirent.h>

namespace co_async {
struct DirectoryWalker {
    explicit DirectoryWalker(FileHandle file);
    DirectoryWalker(DirectoryWalker &&) = default;
    DirectoryWalker &operator=(DirectoryWalker &&) = default;
    ~DirectoryWalker();
    Task<Expected<String>> next();

private:
    OwningStream mStream;
};

Task<Expected<DirectoryWalker>> dir_open(std::filesystem::path path);
} // namespace co_async
