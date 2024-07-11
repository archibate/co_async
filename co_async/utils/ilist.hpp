#pragma once
#include <co_async/std.hpp>

namespace co_async {

struct ListHead {
    struct ListNode {
        ListNode() noexcept : listNext(nullptr), listPrev(nullptr) {}
        friend struct ListHead;

    private:
        ListNode *listNext;
        ListNode *listPrev;
    };

    struct NodeType : ListNode {
        NodeType() = default;
        NodeType(NodeType &&) = delete;

        ~NodeType() noexcept {
            erase_from_parent();
        }

    protected:
        void erase_from_parent() {
            if (this->listNext) {
                auto listPrev = this->listPrev;
                auto listNext = this->listNext;
                listPrev->listNext = listNext;
                listNext->listPrev = listPrev;
                this->listPrev = nullptr;
                this->listNext = nullptr;
            }
        }
    };

    void doPushFront(ListNode *node) noexcept {
        node->listNext = root.listNext;
        node->listPrev = &root;
        root.listNext = node;
        node->listNext->listPrev = node;
    }

    void doPushBack(ListNode *node) noexcept {
        node->listNext = &root;
        node->listPrev = root.listPrev;
        root.listPrev = node;
        node->listPrev->listNext = node;
    }

    void doInsertAfter(ListNode *pivot, ListNode *node) noexcept {
        node->listNext = pivot->listNext;
        node->listPrev = pivot;
        pivot->listNext = node;
        node->listNext->listPrev = node;
    }

    void doInsertBefore(ListNode *pivot, ListNode *node) noexcept {
        node->listNext = pivot;
        node->listPrev = pivot->listPrev;
        pivot->listPrev = node;
        node->listPrev->listNext = node;
    }

    void doErase(ListNode *node) noexcept {
        node->listNext->listPrev = node->listPrev;
        node->listPrev->listNext = node->listNext;
        node->listNext = nullptr;
        node->listPrev = nullptr;
    }

    ListNode *doFront() const noexcept {
        return root.listNext;
    }

    ListNode *doBack() const noexcept {
        return root.listPrev;
    }

    bool doEmpty() const noexcept {
        return root.listNext == nullptr;
    }

    ListNode *doPopFront() noexcept {
        auto node = root.listNext;
        if (node != &root) {
            node->listNext->listPrev = &root;
            root.listNext = node->listNext;
            node->listNext = nullptr;
            node->listPrev = nullptr;
        } else {
            node = nullptr;
        }
        return node;
    }

    ListNode *doPopBack() noexcept {
        auto node = root.listPrev;
        if (node != &root) {
            node->listNext->listPrev = node->listPrev;
            node->listPrev->listNext = node->listNext;
            node->listNext = nullptr;
            node->listPrev = nullptr;
        } else {
            node = nullptr;
        }
        return node;
    }

    void doClear() {
        for (ListNode *current = root.listNext, *next; current != &root;
             current = next) {
            next = current->listNext;
            current->listNext = nullptr;
            current->listPrev = nullptr;
        }
        root.listNext = root.listPrev = &root;
    }

    static void doIterNext(ListNode *&current) noexcept {
        current = current->listNext;
    }

    static void doIterPrev(ListNode *&current) noexcept {
        current = current->listPrev;
    }

    ListNode *doIterBegin() const noexcept {
        return root.listNext;
    }

    ListNode *doIterEnd() const noexcept {
        return const_cast<ListNode *>(&root);
    }

    ListHead() noexcept : root() {
        root.listNext = root.listPrev = &root;
    }

    ListHead(ListHead &&) = delete;

    ~ListHead() noexcept {
        doClear();
    }

private:
    ListNode root;
};

// struct ConcurrentListHead {
//     struct ListNode {
//         ListNode() noexcept
//             : listNext(nullptr),
//               listPrev(nullptr),
//               listHead(nullptr) {}
//         friend struct ConcurrentListHead;
//
//     private:
//         ListNode *listNext;
//         ListNode *listPrev;
//
//     protected:
//         ConcurrentListHead *listHead;
//     };
//
//     struct NodeType : ListNode {
//         NodeType() = default;
//         NodeType(NodeType &&) = delete;
//
//         ~NodeType() noexcept {
//             erase_from_parent();
//         }
//
//     protected:
//         void erase_from_parent() {
//             if (this->listHead) {
//                 this->listHead->doErase(this);
//                 this->listHead = nullptr;
//             }
//         }
//     };
//
//     void doPushFront(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = root.listNext;
//         node->listPrev = &root;
//         root.listNext = node;
//         if (node->listNext) {
//             node->listNext->listPrev = node;
//         }
//     }
//
//     void doPushBack(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = &root;
//         node->listPrev = root.listPrev;
//         root.listPrev = node;
//         if (node->listPrev) {
//             node->listPrev->listNext = node;
//         }
//     }
//
//     void doInsertAfter(ListNode *pivot,
//                        ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = pivot->listNext;
//         node->listPrev = pivot;
//         pivot->listNext = node;
//         if (node->listNext) {
//             node->listNext->listPrev = node;
//         }
//     }
//
//     void doInsertBefore(ListNode *pivot,
//                         ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = pivot;
//         node->listPrev = pivot->listPrev;
//         pivot->listPrev = node;
//         if (node->listPrev) {
//             node->listPrev->listNext = node;
//         }
//     }
//
//     void doErase(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = nullptr;
//         node->listNext->listPrev = node->listPrev;
//         node->listPrev->listNext = node->listNext;
//         node->listNext = nullptr;
//         node->listPrev = nullptr;
//     }
//
//     ListNode *doFront() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listNext;
//     }
//
//     ListNode *doBack() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listPrev;
//     }
//
//     bool doEmpty() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listNext == nullptr;
//     }
//
//     ListNode *doPopFront() noexcept {
//         std::lock_guard lock(listLock);
//         auto node = root.listNext;
//         if (node) {
//             node->listHead = nullptr;
//             node->listNext->listPrev = &root;
//             root.listNext = node->listNext;
//             node->listNext = nullptr;
//             node->listPrev = nullptr;
//         }
//         return node;
//     }
//
//     ListNode *doPopBack() noexcept {
//         std::lock_guard lock(listLock);
//         auto node = root.listPrev;
//         if (node) {
//             node->listHead = nullptr;
//             node->listNext->listPrev = node->listPrev;
//             node->listPrev->listNext = node->listNext;
//             node->listNext = nullptr;
//             node->listPrev = nullptr;
//         }
//         return node;
//     }
//
//     void doClear() {
//         std::lock_guard lock(listLock);
//         for (ListNode *current = root.listNext, *next;
//              current != &root; current = next) {
//             next = current->listNext;
//             current->listHead = nullptr;
//             current->listNext = nullptr;
//             current->listPrev = nullptr;
//         }
//     }
//
//     ConcurrentListHead() noexcept : root() {}
//
//     ConcurrentListHead(ConcurrentListHead &&) = delete;
//
//     ~ConcurrentListHead() noexcept {
//         doClear();
//     }
//
// private:
//     ListNode root;
//     mutable std::mutex listLock;
// };

template <class Value>
struct IntrusiveList : private ListHead {
    using ListHead::NodeType;

    IntrusiveList() noexcept {
        static_assert(
            std::is_base_of_v<NodeType, Value>,
            "Value type must be derived from IntrusiveList<Value>::NodeType");
    }

    struct iterator {
    private:
        ListNode *node;

        explicit iterator(ListNode *node) noexcept : node(node) {}

        friend IntrusiveList;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = Value *;
        using reference = Value &;

        iterator() noexcept : node(nullptr) {}

        explicit iterator(Value &value) noexcept
            : node(&static_cast<ListNode &>(value)) {}

        Value &operator*() const noexcept {
            return *static_cast<Value *>(node);
        }

        Value *operator->() const noexcept {
            return static_cast<Value *>(node);
        }

        iterator &operator++() noexcept {
            ListHead::doIterNext(node);
            return *this;
        }

        iterator &operator--() noexcept {
            ListHead::doIterPrev(node);
            return *this;
        }

        iterator operator++(int) noexcept {
            auto copy = *this;
            ListHead::doIterNext(node);
            return copy;
        }

        iterator operator--(int) noexcept {
            auto copy = *this;
            ListHead::doIterPrev(node);
            return copy;
        }

        bool operator==(iterator const &other) const noexcept {
            return node == other.node;
        }

        bool operator!=(iterator const &other) const noexcept {
            return node != other.node;
        }
    };

    using const_iterator = iterator;

    iterator begin() const noexcept {
        return iterator(ListHead::doIterBegin());
    }

    iterator end() const noexcept {
        return iterator(ListHead::doIterEnd());
    }

    void push_front(Value &value) noexcept {
        doPushFront(&static_cast<ListNode &>(value));
    }

    void push_back(Value &value) noexcept {
        doPushBack(&static_cast<ListNode &>(value));
    }

    void insert_after(Value &pivot, Value &value) noexcept {
        doInsertAfter(&static_cast<ListNode &>(pivot),
                      &static_cast<ListNode &>(value));
    }

    void insert_before(Value &pivot, Value &value) noexcept {
        doInsertBefore(&static_cast<ListNode &>(pivot),
                       &static_cast<ListNode &>(value));
    }

    void erase(Value &value) noexcept {
        doErase(&static_cast<ListNode &>(value));
    }

    bool empty() const noexcept {
        return doEmpty();
    }

    Value &front() const noexcept {
        return static_cast<Value &>(*doFront());
    }

    Value &back() const noexcept {
        return static_cast<Value &>(*doBack());
    }

    Value *pop_front() noexcept {
        auto node = doPopFront();
        return node ? static_cast<Value *>(node) : nullptr;
    }

    Value *pop_back() noexcept {
        auto node = doPopBack();
        return node ? static_cast<Value *>(node) : nullptr;
    }

    void clear() {
        doClear();
    }
};

// template <class Value>
// struct ConcurrentIntrusiveList : private ConcurrentListHead {
//     using ConcurrentListHead::NodeType;
//
//     ConcurrentIntrusiveList() noexcept {
//         static_assert(
//             std::is_base_of_v<NodeType, Value>,
//             "Value type must be derived from
//             ConcurrentIntrusiveList<Value>::NodeType");
//     }
//
//     void push_front(Value &value) noexcept {
//         doPushFront(&static_cast<ListNode &>(value));
//     }
//
//     void push_back(Value &value) noexcept {
//         doPushBack(&static_cast<ListNode &>(value));
//     }
//
//     void insert_after(Value &pivot, Value &value) noexcept {
//         doInsertAfter(&static_cast<ListNode &>(pivot),
//                       &static_cast<ListNode &>(value));
//     }
//
//     void insert_before(Value &pivot, Value &value) noexcept {
//         doInsertBefore(&static_cast<ListNode &>(pivot),
//                        &static_cast<ListNode &>(value));
//     }
//
//     void erase(Value &value) noexcept {
//         doErase(&static_cast<ListNode &>(value));
//     }
//
//     bool empty() const noexcept {
//         return doEmpty();
//     }
//
//     Value &front() const noexcept {
//         return static_cast<Value &>(*doFront());
//     }
//
//     Value &back() const noexcept {
//         return static_cast<Value &>(*doBack());
//     }
//
//     Value *pop_front() noexcept {
//         auto node = doPopFront();
//         return node ? static_cast<Value *>(node) : nullptr;
//     }
//
//     Value *pop_back() noexcept {
//         auto node = doPopBack();
//         return node ? static_cast<Value *>(node) : nullptr;
//     }
//
//     void clear() {
//         doClear();
//     }
// };
} // namespace co_async
