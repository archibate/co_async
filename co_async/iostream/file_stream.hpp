#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>

namespace co_async {
Task<Expected<OwningStream>> file_open(std::filesystem::path path,
                                       OpenMode mode);
OwningStream file_from_handle(FileHandle handle);
Task<Expected<String>> file_read(std::filesystem::path path);
Task<Expected<>> file_write(std::filesystem::path path,
                            std::string_view content);
Task<Expected<>> file_append(std::filesystem::path path,
                             std::string_view content);
} // namespace co_async
