#pragma once

#include "dom/node.h"
#include <string>
#include <string_view>

/// @brief DOM Text
///
/// https://dom.spec.whatwg.org/#interface-text
class Text : public Node {
protected:
    std::string data_;

public:
    Text(std::string_view data)
        : Node(Node::Type::TEXT_NODE)
        , data_(std::move(data))
    {
    }
};