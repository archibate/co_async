#pragma once

#include <utility>
#include <co_async/platform.hpp>
#if CO_ASYNC_PLAT_LINUX
#include <co_async/uring_loop.hpp>
#endif
#if CO_ASYNC_PLAT_WIN32
#include <co_async/iocp_loop.hpp>
#endif
#if CO_ASYNC_PLAT_APPLE
#include <co_async/apple_dispatch_loop.hpp>
#endif
#if CO_ASYNC_PLAT_UNKNOWN
#include <co_async/basic_loop.hpp>
#endif

namespace co_async {

#if CO_ASYNC_PLAT_LINUX
inline UringLoop loop;
#endif
#if CO_ASYNC_PLAT_WIN32
inline IocpLoop loop;
#endif
#if CO_ASYNC_PLAT_APPLE
inline AppleDispatchLoop loop;
#endif
#if CO_ASYNC_PLAT_UNKNOWN
inline QueueLoop loop;
#endif

inline void enqueue(auto task) {
    loop_enqueue(loop, std::move(task));
}

}
