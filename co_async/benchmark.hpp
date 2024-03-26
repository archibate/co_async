
#pragma once

#include <co_async/task.hpp>
#include <chrono>
#include <iostream>

namespace co_async {

template <class T>
Task<T> operator*(Task<T> task) {
    auto t0 = std::chrono::high_resolution_clock::now();
    T ret = co_await task;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = t1 - t0;
    std::cout << dt << '\n';
    co_return ret;
}

}
