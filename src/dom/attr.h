#pragma once

#include <string>
#include <string_view>

/// @brief DOM Attr
///
/// https://dom.spec.whatwg.org/#interface-attr
class Attr {
protected:
    std::string name_;
    std::string value_;

public:
    Attr(std::string_view name, std::string_view value)
        : name_(std::move(name))
        , value_(std::move(value))
    {
    }
};