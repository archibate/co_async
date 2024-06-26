#pragma once

#include <cstddef>
#include <functional>
#include <utility>

namespace co_async {

template <class Value, class Compare = std::less<Value>>
struct RbTree {
    enum RbColor {
        RED,
        BLACK
    };

    struct RbNode {
        RbNode() noexcept
            : left(nullptr),
              right(nullptr),
              parent(nullptr),
              tree(nullptr),
              color(RED) {}

        RbNode(RbNode &&) = delete;

        ~RbNode() noexcept {
            if (tree) {
                tree->doErase(this);
            }
        }

        friend struct RbTree;

    private:
        RbNode *left;
        RbNode *right;
        RbNode *parent;
        RbTree *tree;
        RbColor color;
    };

private:
    RbNode *root;
    Compare comp;

    bool compare(RbNode *left, RbNode *right) const noexcept {
        return comp(static_cast<Value &>(*left), static_cast<Value &>(*right));
    }

    void rotateLeft(RbNode *node) noexcept {
        RbNode *rightChild = node->right;
        node->right = rightChild->left;
        if (rightChild->left != nullptr) {
            rightChild->left->parent = node;
        }
        rightChild->parent = node->parent;
        if (node->parent == nullptr) {
            root = rightChild;
        } else if (node == node->parent->left) {
            node->parent->left = rightChild;
        } else {
            node->parent->right = rightChild;
        }
        rightChild->left = node;
        node->parent = rightChild;
    }

    void rotateRight(RbNode *node) noexcept {
        RbNode *leftChild = node->left;
        node->left = leftChild->right;
        if (leftChild->right != nullptr) {
            leftChild->right->parent = node;
        }
        leftChild->parent = node->parent;
        if (node->parent == nullptr) {
            root = leftChild;
        } else if (node == node->parent->right) {
            node->parent->right = leftChild;
        } else {
            node->parent->left = leftChild;
        }
        leftChild->right = node;
        node->parent = leftChild;
    }

    void fixViolation(RbNode *node) noexcept {
        RbNode *parent = nullptr;
        RbNode *grandParent = nullptr;

        while (node != root && node->color != BLACK &&
               node->parent->color == RED) {
            parent = node->parent;
            grandParent = parent->parent;

            if (parent == grandParent->left) {
                RbNode *uncle = grandParent->right;

                if (uncle != nullptr && uncle->color == RED) {
                    grandParent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->right) {
                        rotateLeft(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotateRight(grandParent);
                    std::swap(parent->color, grandParent->color);
                    node = parent;
                }
            } else {
                RbNode *uncle = grandParent->left;

                if (uncle != nullptr && uncle->color == RED) {
                    grandParent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->left) {
                        rotateRight(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotateLeft(grandParent);
                    std::swap(parent->color, grandParent->color);
                    node = parent;
                }
            }
        }

        root->color = BLACK;
    }

    void doInsert(RbNode *node) noexcept {
        node->left = nullptr;
        node->right = nullptr;
        node->tree = this;
        node->color = RED;

        RbNode *parent = nullptr;
        RbNode *current = root;

        while (current != nullptr) {
            parent = current;
            if (compare(node, current)) {
                current = current->left;
            } else {
                current = current->right;
            }
        }

        node->parent = parent;
        if (parent == nullptr) {
            root = node;
        } else if (compare(node, parent)) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        fixViolation(node);
    }

    /* template <class Key> */
    /* RbNode* doFind(Key &&key) const { */
    /*     RbNode* current = root; */
    /*     // Find the node to delete */
    /*     while (current != nullptr) { */
    /*         if (key_compare(key, current)) { */
    /*             current = current->left; */
    /*         } else { */
    /*             current = current->right; */
    /*         } */
    /*     } */
    /*     return nullptr; */
    /* } */

    /* void doErase(RbNode* node) noexcept { */
    /*     RbNode* parent = nullptr; */
    /*  */
    /*     // Case 1: Node to delete has no children */
    /*     if (node->left == nullptr && node->right == nullptr) { */
    /*         if (node == root) { */
    /*             root = nullptr; */
    /*         } else { */
    /*             if (node == parent->left) { */
    /*                 parent->left = nullptr; */
    /*             } else { */
    /*                 parent->right = nullptr; */
    /*             } */
    /*         } */
    /*         delete node; */
    /*     } */
    /*     // Case 2: Node to delete has one child */
    /*     else if (node->left == nullptr || node->right == nullptr) { */
    /*         RbNode* child = (node->left != nullptr) ? node->left : node->right; */
    /*  */
    /*         if (node == root) { */
    /*             root = child; */
    /*         } else { */
    /*             if (node == parent->left) { */
    /*                 parent->left = child; */
    /*             } else { */
    /*                 parent->right = child; */
    /*             } */
    /*         } */
    /*         child->parent = parent; */
    /*         delete node; */
    /*     } */
    /*     // Case 3: Node to delete has two children */
    /*     else { */
    /*         RbNode* successor = node->right; */
    /*         while (successor->left != nullptr) { */
    /*             successor = successor->left; */
    /*         } */
    /*         doErase(successor); */
    /*     } */
    /* } */

    void doErase(RbNode *current) noexcept {
        current->tree = nullptr;

        RbNode *node = nullptr;
        RbNode *child = nullptr;
        RbColor color = RED;

        if (current->left != nullptr && current->right != nullptr) {
            RbNode *replace = current;
            replace = replace->right;
            while (replace->left != nullptr) {
                replace = replace->left;
            }

            if (current != replace->parent) {
                current->parent->left = replace->right;
                replace->right = current->right;
                current->right->parent = replace;
            } else {
                replace->parent = current;
            }

            if (current == root) {
                root = replace;
            } else if (current->parent->left == current) {
                current->parent->left = replace;
            } else {
                current->parent->right = replace;
            }

            replace->left = current->left;
            current->left->parent = replace;

            node = replace;
            color = node->color;
            child = node->right;
        } else {
            node = current;
            color = node->color;
            child = (node->left != nullptr) ? node->left : node->right;
        }

        if (child != nullptr) {
            child->parent = node->parent;
        }

        if (node == root) {
            root = child;
        } else if (node->parent->left == node) {
            node->parent->left = child;
        } else {
            node->parent->right = child;
        }

        if (color == BLACK && root) {
            fixViolation(child ? child : node->parent);
        }
    }

    RbNode *getFront() const noexcept {
        RbNode *current = root;
        while (current->left != nullptr) {
            current = current->left;
        }
        return current;
    }

    RbNode *getBack() const noexcept {
        RbNode *current = root;
        while (current->right != nullptr) {
            current = current->right;
        }
        return current;
    }

    template <class Visitor>
    void doTraversalInorder(RbNode *node, Visitor &&visitor) {
        if (node == nullptr) {
            return;
        }

        traversalInorder(node->left, visitor);
        visitor(node);
        doTraversalInorder(node->right, visitor);
    }

public:
    RbTree() noexcept : root(nullptr) {}

    explicit RbTree(Compare comp) noexcept(noexcept(Compare(comp)))
        : root(nullptr),
          comp(comp) {}

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

    template <class Visitor>
    void traversalInorder(Visitor &&visitor) {
        doTraversalInorder(root, std::forward<Visitor>(visitor));
    }
};

} // namespace co_async
