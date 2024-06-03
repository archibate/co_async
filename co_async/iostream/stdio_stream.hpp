#pragma once
#include <co_async/std.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {
OwningStream &stdio();
OwningStream &raw_stdio();
} // namespace co_async
