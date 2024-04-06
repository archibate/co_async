export module co_async:system.system_loop;

import std;
import :system.uring_loop;

namespace co_async {

export inline UringLoop loop;

export inline void co_spawn(auto task) {
    loop_enqueue(loop, std::move(task));
}

export inline auto co_spawn_and_wait(auto task) {
    return loop_run(loop, std::move(task));
}

} // namespace co_async
