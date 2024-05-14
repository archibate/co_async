#pragma once
#include <co_async/std.hpp>
namespace co_async {
template <class K, class V>
struct SimpleMap {
    SimpleMap() = default;
    SimpleMap(std::initializer_list<std::pair<K, V>> init)
        : mData(init.begin(), init.end()) {}
    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    V *at(Key &&key) noexcept {
        auto it = mData.find(std::forward<Key>(key));
        if (it == mData.end()) {
            return nullptr;
        }
        return std::addressof(it->second);
    }
    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    V const *at(Key const &key) const noexcept {
        auto it = mData.find(key);
        if (it == mData.end()) {
            return nullptr;
        }
        return std::addressof(it->second);
    }
    template <class Key, class F = std::identity>
        requires(requires(F f, V const &v, K const &k, Key const &key) {
            std::invoke(f, v);
            k < key;
            key < k;
        })
    decltype(std::optional(std::declval<std::invoke_result_t<F, V const &>>()))
    get(Key const &key, F &&func = {}) const noexcept {
        auto it = mData.find(key);
        if (it == mData.end()) {
            return std::nullopt;
        }
        return std::invoke(func, it->second);
    }
    V &insert_or_assign(K key, V value) {
        return mData.insert_or_assign(std::move(key), std::move(value))
            .first->second;
    }
    V &insert(K key, V value) {
        return mData.emplace(std::move(key), std::move(value)).first->second;
    }
    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    bool contains(Key &&key) const noexcept {
        return mData.find(std::forward<Key>(key)) != mData.end();
    }
    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    bool erase(Key &&key) {
        auto it = mData.find(std::forward<Key>(key));
        if (it == mData.end()) {
            return false;
        }
        mData.erase(it);
        return true;
    }
    auto begin() const noexcept {
        return mData.begin();
    }
    auto end() const noexcept {
        return mData.end();
    }
    auto begin() noexcept {
        return mData.begin();
    }
    auto end() noexcept {
        return mData.end();
    }
    bool empty() const noexcept {
        return mData.empty();
    }
    std::size_t size() const noexcept {
        return mData.size();
    }

private:
    std::map<K, V, std::less<>> mData;
};
} // namespace co_async
