#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest);
Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest);
} // namespace co_async
