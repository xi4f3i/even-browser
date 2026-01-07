#include "node.h"

Node::~Node()
{
    // TODO: avoid stack overflow
    Node* child = first_child_;
    while (child) {
        Node* next = child->next_sibling_;
        delete child;
        child = next;
    }
}

void Node::append_child(std::unique_ptr<Node> node)
{
    if (!node) {
        return;
    }

    Node* child = node.release();

    child->parent_ = this;

    if (last_child_) {
        last_child_->next_sibling_ = child;
        child->previous_sibling_ = last_child_;
        last_child_ = child;
    } else {
        first_child_ = child;
        last_child_ = child;
    }

    child->next_sibling_ = nullptr;
}