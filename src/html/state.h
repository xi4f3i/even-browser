#pragma once

enum class State {
    Data,
    TagOpen,
    EndTagOpen,
    TagName,
    BeforeAttributeName,
    AttributeName,
    AfterAttributeName,
    BeforeAttributeValue,
    AttributeValueUnquoted,
    AttributeValueDoubleQuoted,
    AttributeValueSingleQuoted,
    AfterAttributeValueQuoted,
    SelfClosingStartTag,
    Comment,
};