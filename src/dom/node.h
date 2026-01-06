#pragma once

enum class NodeType {
  ELEMENT_NODE = 1,
  TEXT_NODE = 3,
  DOCUMENT_NODE = 9,
};

/// @brief DOM Node
///
/// https://dom.spec.whatwg.org/#node
class Node {
 protected:
  NodeType node_type_;
  Node* parent_node_ = nullptr;
  Node* first_child_ = nullptr;
  Node* last_child_ = nullptr;
  Node* previous_sibling_ = nullptr;
  Node* next_sibling_ = nullptr;

  Node(NodeType type) : node_type_(type) {}

 public:
};