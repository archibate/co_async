#pragma once
#include <co_async/std.hpp>

namespace co_async {
template <class Value, class Compare = std::less<>>
struct RbTree : private Compare {
private:
    enum RbColor {
        RED,
        BLACK
    };

protected:
    struct RbNode {
        RbNode() noexcept
            : rbLeft(nullptr),
              rbRight(nullptr),
              rbParent(nullptr),
              rbTree(nullptr),
              rbColor(RED) {}
        friend struct RbTree;

    private:
        RbNode *rbLeft;
        RbNode *rbRight;
        RbNode *rbParent;

    protected:
        RbTree *rbTree;

    private:
        RbColor rbColor;
    };

public:
    struct NodeType : RbNode {
        NodeType() = default;
        NodeType(NodeType &&) = delete;

        ~NodeType() noexcept {
            erase_from_parent();
        }

    protected:
        void erase_from_parent() {
            static_assert(
                std::is_base_of_v<NodeType, Value>,
                "Value type must be derived from RbTree<Value>::NodeType");
            if (this->rbTree) {
                this->rbTree->doErase(this);
                this->rbTree = nullptr;
            }
        }
    };

private:
    RbNode *root;

    bool compare(RbNode *left, RbNode *right) const noexcept {
        return static_cast<Compare const &>(*this)(
            static_cast<Value &>(*left), static_cast<Value &>(*right));
    }

    void rotateLeft(RbNode *node) noexcept {
        RbNode *rightChild = node->rbRight;
        node->rbRight = rightChild->rbLeft;
        if (rightChild->rbLeft != nullptr) {
            rightChild->rbLeft->rbParent = node;
        }
        rightChild->rbParent = node->rbParent;
        if (node->rbParent == nullptr) {
            root = rightChild;
        } else if (node == node->rbParent->rbLeft) {
            node->rbParent->rbLeft = rightChild;
        } else {
            node->rbParent->rbRight = rightChild;
        }
        rightChild->rbLeft = node;
        node->rbParent = rightChild;
    }

    void rotateRight(RbNode *node) noexcept {
        RbNode *leftChild = node->rbLeft;
        node->rbLeft = leftChild->rbRight;
        if (leftChild->rbRight != nullptr) {
            leftChild->rbRight->rbParent = node;
        }
        leftChild->rbParent = node->rbParent;
        if (node->rbParent == nullptr) {
            root = leftChild;
        } else if (node == node->rbParent->rbRight) {
            node->rbParent->rbRight = leftChild;
        } else {
            node->rbParent->rbLeft = leftChild;
        }
        leftChild->rbRight = node;
        node->rbParent = leftChild;
    }

    void fixViolation(RbNode *node) noexcept {
        RbNode *parent = nullptr;
        RbNode *grandParent = nullptr;
        while (node != root && node->rbColor != BLACK &&
               node->rbParent->rbColor == RED) {
            parent = node->rbParent;
            grandParent = parent->rbParent;
            if (parent == grandParent->rbLeft) {
                RbNode *uncle = grandParent->rbRight;
                if (uncle != nullptr && uncle->rbColor == RED) {
                    grandParent->rbColor = RED;
                    parent->rbColor = BLACK;
                    uncle->rbColor = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->rbRight) {
                        rotateLeft(parent);
                        node = parent;
                        parent = node->rbParent;
                    }
                    rotateRight(grandParent);
                    std::swap(parent->rbColor, grandParent->rbColor);
                    node = parent;
                }
            } else {
                RbNode *uncle = grandParent->rbLeft;
                if (uncle != nullptr && uncle->rbColor == RED) {
                    grandParent->rbColor = RED;
                    parent->rbColor = BLACK;
                    uncle->rbColor = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->rbLeft) {
                        rotateRight(parent);
                        node = parent;
                        parent = node->rbParent;
                    }
                    rotateLeft(grandParent);
                    std::swap(parent->rbColor, grandParent->rbColor);
                    node = parent;
                }
            }
        }
        root->rbColor = BLACK;
    }

    void doInsert(RbNode *node) noexcept {
        node->rbLeft = nullptr;
        node->rbRight = nullptr;
        node->rbTree = this;
        node->rbColor = RED;
        RbNode *parent = nullptr;
        RbNode *current = root;
        while (current != nullptr) {
            parent = current;
            if (compare(node, current)) {
                current = current->rbLeft;
            } else {
                current = current->rbRight;
            }
        }
        node->rbParent = parent;
        if (parent == nullptr) {
            root = node;
        } else if (compare(node, parent)) {
            parent->rbLeft = node;
        } else {
            parent->rbRight = node;
        }
        fixViolation(node);
    }

    void doErase(RbNode *current) noexcept {
        current->rbTree = nullptr;
        RbNode *node = nullptr;
        RbNode *child = nullptr;
        RbColor color = RED;
        if (current->rbLeft != nullptr && current->rbRight != nullptr) {
            RbNode *replace = current;
            replace = replace->rbRight;
            while (replace->rbLeft != nullptr) {
                replace = replace->rbLeft;
            }
            if (current != replace->rbParent) {
                current->rbParent->rbLeft = replace->rbRight;
                replace->rbRight = current->rbRight;
                current->rbRight->rbParent = replace;
            } else {
                replace->rbParent = current;
            }
            if (current == root) {
                root = replace;
            } else if (current->rbParent->rbLeft == current) {
                current->rbParent->rbLeft = replace;
            } else {
                current->rbParent->rbRight = replace;
            }
            replace->rbLeft = current->rbLeft;
            current->rbLeft->rbParent = replace;
            node = replace;
            color = node->rbColor;
            child = node->rbRight;
        } else {
            node = current;
            color = node->rbColor;
            child = (node->rbLeft != nullptr) ? node->rbLeft : node->rbRight;
        }
        if (child != nullptr) {
            child->rbParent = node->rbParent;
        }
        if (node == root) {
            root = child;
        } else if (node->rbParent->rbLeft == node) {
            node->rbParent->rbLeft = child;
        } else {
            node->rbParent->rbRight = child;
        }
        if (color == BLACK && root) {
            fixViolation(child ? child : node->rbParent);
        }
    }

    RbNode *getFront() const noexcept {
        RbNode *current = root;
        while (current->rbLeft != nullptr) {
            current = current->rbLeft;
        }
        return current;
    }

    RbNode *getBack() const noexcept {
        RbNode *current = root;
        while (current->rbRight != nullptr) {
            current = current->rbRight;
        }
        return current;
    }

    template <class Visitor>
    void doTraverseInorder(RbNode *node, Visitor &&visitor) {
        if (node == nullptr) {
            return;
        }
        doTraverseInorder(node->rbLeft, visitor);
        visitor(node);
        doTraverseInorder(node->rbRight, visitor);
    }

    void doClear(RbNode *node) {
        if (node == nullptr) {
            return;
        }
        doClear(node->rbLeft);
        node->rbTree = nullptr;
        doClear(node->rbRight);
    }

    void doClear() {
        doClear(root);
        root = nullptr;
    }

public:
    RbTree() noexcept(noexcept(Compare())) : Compare(), root(nullptr) {}

    explicit RbTree(Compare comp) noexcept(noexcept(Compare(comp)))
        : Compare(comp),
          root(nullptr) {}

    RbTree(RbTree &&) = delete;

    ~RbTree() noexcept {}

    void insert(Value &value) noexcept {
        doInsert(&static_cast<RbNode &>(value));
    }

    void erase(Value &value) noexcept {
        doErase(&static_cast<RbNode &>(value));
    }

    bool empty() const noexcept {
        return root == nullptr;
    }

    Value &front() const noexcept {
        return static_cast<Value &>(*getFront());
    }

    Value &back() const noexcept {
        return static_cast<Value &>(*getBack());
    }

    template <class Visitor, class V>
    std::pair<RbNode *, RbNode *> traverseEqualRange(Visitor &&visitor,
                                                     V &&value) {}

    template <class Visitor>
    void traverseInorder(Visitor &&visitor) {
        doTraverseInorder(root, [visitor = std::forward<Visitor>(visitor)](
                                    RbNode *node) mutable {
            visitor(static_cast<Value &>(*node));
        });
    }

    void clear() {
        doClear();
    }
};

// template <class Value, class Compare = std::less<>>
// struct ConcurrentRbTree : private RbTree<Value, Compare> {
// private:
//     using BaseTree = RbTree<Value, Compare>;
//
// public:
//     struct NodeType : BaseTree::RbNode {
//         NodeType() = default;
//         NodeType(NodeType &&) = delete;
//
//         ~NodeType() noexcept {
//             erase_from_parent();
//         }
//
//     protected:
//         void erase_from_parent() {
//             static_assert(
//                 std::is_base_of_v<NodeType, Value>,
//                 "Value type must be derived from RbTree<Value>::NodeType");
//             if (this->rbTree) {
//                 auto lock = static_cast<ConcurrentRbTree
//                 *>(this->rbTree)->lock(); lock->erase(static_cast<Value
//                 &>(*this)); this->rbTree = nullptr;
//             }
//         }
//     };
//
//     struct LockGuard {
//     private:
//         BaseTree *mThat;
//         std::unique_lock<std::mutex> mGuard;
//
//         explicit LockGuard(ConcurrentRbTree *that) noexcept
//             : mThat(that),
//               mGuard(that->mMutex) {}
//
//         friend ConcurrentRbTree;
//
//     public:
//         BaseTree &operator*() const noexcept {
//             return *mThat;
//         }
//
//         BaseTree *operator->() const noexcept {
//             return mThat;
//         }
//
//         void unlock() noexcept {
//             mGuard.unlock();
//             mThat = nullptr;
//         }
//     };
//
//     LockGuard lock() noexcept {
//         return LockGuard(this);
//     }
//
// private:
//     std::mutex mMutex;
// };
} // namespace co_async
