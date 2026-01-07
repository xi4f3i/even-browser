#pragma once

#include "dom/node.h"

/// @brief DOM Document
///
/// https://dom.spec.whatwg.org/#interface-document
class Document : public Node {
protected:
public:
    Document()
        : Node(Node::Type::DOCUMENT_NODE)
    {
    }
};