#pragma once /*{export module co_async:system.system_loop;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/system/uring_loop.hpp>/*{import :system.uring_loop;}*/
#include <co_async/threading/basic_loop.hpp>/*{import :threading.basic_loop;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/

namespace co_async {

/*[export]*/ inline UringLoop loop;

template <class T>
/*[export]*/ inline Future<T> make_future() {
    return Future<T>(loop);
}

template <class T, class P>
/*[export]*/ inline void co_spawn(Task<T, P> &&task) {
    return loop_enqueue_detach(loop, std::move(task));
}

template <class T, class P>
/*[export]*/ inline Future<T> co_future(Task<T, P> &&task) {
    return loop_enqueue_future(loop, std::move(task));
}

template <class T, class P>
/*[export]*/ inline auto co_synchronize(Task<T, P> const &task) {
    return loop_enqueue_and_wait(loop, task);
}

} // namespace co_async
