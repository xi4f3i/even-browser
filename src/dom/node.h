#pragma once

#include <memory>

/// @brief DOM Node
///
/// https://dom.spec.whatwg.org/#node
class Node {
protected:
    /// @brief Node Type
    /// https://dom.spec.whatwg.org/#dom-node-nodetype
    enum class Type {
        ELEMENT_NODE = 1,
        TEXT_NODE = 3,
        DOCUMENT_NODE = 9,
    } node_type_;
    /// @brief Tree Parent
    /// https://dom.spec.whatwg.org/#concept-tree-parent
    Node* parent_ = nullptr;
    Node* first_child_ = nullptr;
    Node* last_child_ = nullptr;
    Node* previous_sibling_ = nullptr;
    Node* next_sibling_ = nullptr;

public:
    Node(Node::Type type)
        : node_type_(type)
    {
    }

    virtual ~Node();

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&&) = delete;

    Node* parent_node() const { return parent_; }
    Node* first_child() const { return first_child_; }
    Node* last_child() const { return last_child_; }
    Node* next_sibling() const { return next_sibling_; }
    Node* previous_sibling() const { return previous_sibling_; }

    // void insert_before(std::unique_ptr<Node> node, Node* child);
    void append_child(std::unique_ptr<Node> node);
    // void replace_before(std::unique_ptr<Node> node, Node* child);
    // std::unique_ptr<Node> remove_child(Node* child);
};