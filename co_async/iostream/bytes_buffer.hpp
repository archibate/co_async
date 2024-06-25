#pragma once

#include <co_async/std.hpp>
#include <co_async/generic/allocator.hpp>

#if CO_ASYNC_ALLOC
struct StreamIOBuffer {
private:
    struct Deleter {
        std::size_t mSize;
        std::pmr::memory_resource *mResource;

        void operator()(char *p) noexcept {
            mResource->deallocate(p, mSize);
        }
    };

    std::unique_ptr<char[], Deleter> mData;

public:
    void allocate(std::size_t size, std::pmr::polymorphic_allocator<> alloc = {}) {
        std::pmr::memory_resource *resource = alloc.resource();
        mData.reset(reinterpret_cast<char *>(resource->allocate(size)));
        mData.get_deleter() = {size, resource};
    }

    char *data() const noexcept {
        return mData.get();
    }

    std::size_t size() const noexcept {
        return mData.get_deleter().mSize;
    }

    explicit operator bool() const noexcept {
        return (bool)mData;
    }

    char &operator[](std::size_t index) const noexcept {
        return mData[index];
    }
};
#else
struct StreamIOBuffer {
private:
    std::unique_ptr<char[]> mData;
    std::size_t mSize = 0;

public:
    void allocate(std::size_t size) {
        mData = std::make_unique<char[]>(size);
        mSize = size;
    }

    char *data() const noexcept {
        return mData.get();
    }

    std::size_t size() const noexcept {
        return mSize;
    }

    explicit operator bool() const noexcept {
        return (bool)mData;
    }

    char &operator[](std::size_t index) const noexcept {
        return mData[index];
    }
};
#endif
