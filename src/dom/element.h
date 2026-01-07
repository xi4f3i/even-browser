#pragma once

#include "dom/attr.h"
#include "dom/node.h"
#include <string>
#include <vector>

/// @brief DOM Element
///
/// https://dom.spec.whatwg.org/#interface-element
class Element : public Node {
protected:
    /// @brief local name
    /// A non-empty string.
    /// https://dom.spec.whatwg.org/#concept-element-local-name
    std::string local_name_;
    std::vector<Attr> attributes_;

public:
    Element(std::string_view local_name)
        : Node(Node::Type::ELEMENT_NODE)
        , local_name_(std::move(local_name))
    {
    }
};