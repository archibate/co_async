#pragma once
#include <co_async/std.hpp>

namespace co_async {
template <class T>
struct PImplMethod {};

template <class T>
struct PImplConstruct {
    static std::shared_ptr<T> construct();
};

#define StructPImpl(T) \
    struct T; \
    template <> \
    struct PImplMethod<T>
#define DefinePImpl(T) \
    template <> \
    std::shared_ptr<T> PImplConstruct<T>::construct() { \
        return std::make_shared<T>(); \
    }
#define ForwardPImplMethod(T, func, args, ...) \
    PImplMethod<T>::func args { \
        return pimpl(this).func(__VA_ARGS__); \
    }

template <class T>
struct PImpl : PImplMethod<T> {
    PImpl()
        requires(requires {
            {
                PImplConstruct<T>::construct()
            } -> std::convertible_to<std::shared_ptr<T>>;
        })
        : mImpl(PImplConstruct<T>::construct()) {}

    template <class... Args>
        requires(sizeof...(Args) != 0 &&
                 requires(Args &&...args) {
                     {
                         PImplConstruct<T>::construct(
                             std::forward<Args>(args)...)
                     } -> std::convertible_to<std::shared_ptr<T>>;
                 })
    explicit PImpl(Args &&...args)
        : mImpl(PImplConstruct<T>::construct(std::forward<Args>(args)...)) {}

    PImpl(std::nullptr_t) : mImpl(nullptr) {}

    T &operator*() const noexcept {
        return *mImpl;
    }

    T *operator->() const noexcept {
        return mImpl.get();
    }

    T *impl() const noexcept {
        return mImpl.get();
    }

    T *operator&() const noexcept {
        return mImpl.get();
    }

    operator T &() const noexcept {
        return *mImpl;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(mImpl);
    }

private:
    std::shared_ptr<T> mImpl;
};

template <class T>
T &pimpl(PImplMethod<T> const *that) {
    return **static_cast<PImpl<T> const *>(that);
}
} // namespace co_async
