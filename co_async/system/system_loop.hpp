#pragma once /*{export module co_async:system.system_loop;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/system/uring_loop.hpp>/*{import :system.uring_loop;}*/

namespace co_async {

/*[export]*/ inline UringLoop loop;

/*[export]*/ inline void co_spawn(auto task) {
    loop_enqueue(loop, std::move(task));
}

/*[export]*/ inline auto co_spawn_and_wait(auto task) {
    return loop_run(loop, std::move(task));
}

} // namespace co_async
