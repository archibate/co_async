#pragma once
#include <co_async/std.hpp>

namespace co_async {

inline uint32_t getSeedByTime() {
    return static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
}

inline uint32_t getSecureSeed() {
    return std::random_device{}();
}

inline uint32_t wangsHash(uint32_t x) noexcept {
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

struct WangsHash {
    using result_type = uint32_t;

    result_type mSeed;

    WangsHash(result_type seed) noexcept : mSeed(seed) {
    }

    void seed(result_type seed) noexcept {
        mSeed = seed;
    }

    result_type operator()() noexcept {
        mSeed = wangsHash(mSeed);
        return mSeed;
        std::mt19937 mt;
        mt.discard(1);
    }

    static result_type max() noexcept {
        return std::numeric_limits<result_type>::max();
    }

    static result_type min() noexcept {
        return std::numeric_limits<result_type>::min();
    }
};

}
