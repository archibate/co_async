#pragma once

#include <co_async/std.hpp>

namespace co_async {

template <class F>
struct Finally {
private:
    F func;
    bool enable;

public:
    Finally(std::nullptr_t = nullptr) : enable(false) {}

    Finally(std::convertible_to<F> auto &&func)
        : func(std::forward<decltype(func)>(func)),
          enable(true) {}

    Finally(Finally &&that) : func(std::move(that.func)), enable(that.enable) {
        that.enable = false;
    }

    Finally &operator=(Finally &&that) {
        if (this != &that) {
            if (enable) {
                func();
            }
            func = std::move(that.func);
            enable = that.enable;
            that.enable = false;
        }
        return *this;
    }

    void reset() {
        if (enable) {
            func();
        }
        enable = false;
    }

    void release() {
        enable = false;
    }

    ~Finally() {
        if (enable) {
            func();
        }
    }
};

template <class F>
Finally(F &&) -> Finally<std::decay_t<F>>;

} // namespace co_async
