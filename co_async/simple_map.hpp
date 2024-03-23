#pragma once

#include <initializer_list>
#include <functional>
#include <map>

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

private:
    std::map<K, V, std::less<>> mData;
};

} // namespace co_async
