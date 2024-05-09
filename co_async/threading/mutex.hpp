#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/threading/condition_variable.hpp>

namespace co_async {

struct Mutex {
    ConditionVariableTimed ready;

    Task<Expected<>> lock() {
        co_return {};
    }
};

} // namespace co_async
