#include "tokenizer.h"

#include <fmt/base.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "../util/char_util.h"
#include "state.h"
#include "token.h"

void print_parse_error(std::string_view msg) {
  fmt::println("[HTML Tokenizer] parser error: {}", msg);
}

Tokenizer::Tokenizer(std::string_view input)
    : input_(input),
      pos_(0),
      reconsume_(false),
      state_(State::Data),
      cur_tag_kind_(TokenTag::Kind::Start),
      cur_tag_self_closing_(false) {}

Tokenizer::~Tokenizer() {}

std::optional<char> Tokenizer::peek() {
  if (reconsume_) {
    reconsume_ = false;
    if (pos_ <= 0) {
      return std::nullopt;
    }

    pos_--;
  }

  if (pos_ >= input_.size()) {
    return std::nullopt;
  }

  auto c = input_[pos_];

  pos_++;

  return c;
}

Token Tokenizer::next() {
  if (!pending_tokens_.empty()) {
    auto last = std::move(pending_tokens_.back());
    pending_tokens_.pop_back();
    return last;
  }

  while (true) {
    auto c = peek();

    switch (state_) {
      case State::Data:
        // https://html.spec.whatwg.org/multipage/parsing.html#data-state
        if (c.has_value()) {
          auto ch = c.value();

          // TODO: U+0026 AMPERSAND (&)
          // TODO: U+0000 NULL

          if (ch == '<') {
            // U+003C LESS-THAN SIGN (<) - Switch to the tag open state.
            state_ = State::TagOpen;
          } else {
            // Anything else
            // Emit the current input character as a character token.
            return emit_char(ch);
          }
        } else {
          // EOF - Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::TagOpen:
        // https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
        if (c.has_value()) {
          auto ch = c.value();
          if (ch == '!') {
            // TODO: U+0021 EXCLAMATION MARK (!) - Switch to the markup
            // declaration open state.
            state_ = State::Comment;
          } else if (ch == '/') {
            // U+002F SOLIDUS (/) - Switch to the end tag open state.
            state_ = State::EndTagOpen;
          } else if (CharUtil::is_ascii_alpha(ch)) {
            // ASCII alpha
            // Create a new start tag token,
            // set its tag name to the empty string.
            create_start_tag();
            // Reconsume in the tag name state.
            reconsume_ = true;
            state_ = State::TagName;
          } else if (ch == '?') {
            // TODO: U+003F QUESTION MARK (?)
            // This is an unexpected-question-mark-instead-of-tag-name
            // parse error.
            print_parse_error("unexpected-question-mark-instead-of-tag-name");
            // Create a comment token whose data is the empty string.
            // Reconsume in the bogus comment state.
            reconsume_ = true;
            state_ = State::Comment;
          } else {
            // Anything else
            // This is an invalid-first-character-of-tag-name parse error.
            print_parse_error("invalid-first-character-of-tag-name");
            // Emit a U+003C LESS-THAN SIGN character token.
            // Reconsume in the data state.
            reconsume_ = true;
            state_ = State::Data;
            return emit_char('<');
          }
        } else {
          // EOF
          // This is an eof-before-tag-name parse error.
          print_parse_error("eof-before-tag-name");
          // Emit a U+003C LESS-THAN SIGN character token
          // and an end-of-file token.
          pending_tokens_.push_back(emit_eof());
          return emit_char('<');
        }
        break;
      case State::EndTagOpen:
        // https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
        if (c.has_value()) {
          auto ch = c.value();

          if (CharUtil::is_ascii_alpha(ch)) {
            // ASCII alpha
            // Create a new end tag token, set its tag name to the empty string.
            create_end_tag();
            // Reconsume in the tag name state.
            reconsume_ = true;
            state_ = State::TagName;
          } else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // This is a missing-end-tag-name parse error.
            print_parse_error("missing-end-tag-name");
            // Switch to the data state.
            state_ = State::Data;
          } else {
            // Anything else
            // This is an invalid-first-character-of-tag-name parse error.
            print_parse_error("invalid-first-character-of-tag-name");
            // TODO: Create a comment token whose data is the empty string.
            // Reconsume in the bogus comment state.
            reconsume_ = true;
            state_ = State::Comment;
          }
        } else {
          // This is an eof-before-tag-name parse error.
          print_parse_error("eof-before-tag-name");
          // Emit a U+003C LESS-THAN SIGN character token, a U+002F SOLIDUS
          // character token and an end-of-file token.
          pending_tokens_.push_back(emit_eof());
          pending_tokens_.push_back(emit_char('/'));
          return emit_char('<');
        }
        break;
      case State::TagName:
        // https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Switch to the before
            // attribute name state.
            // Switch to the before attribute name state.
            state_ = State::BeforeAttributeName;
          } else if (ch == '/') {
            // U+002F SOLIDUS (/) - Switch to the self-closing start tag state.
            state_ = State::SelfClosingStartTag;
          } else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // Switch to the data state.
            // Emit the current tag token.
            state_ = State::Data;
            return emit_cur_tag();
          } /* TODO: U+0000 NULL*/ else {
            // Anything else
            // ASCII upper alpha - Append the lowercase version of the current
            // input character (add 0x0020 to the character's code point) to the
            // current tag token's tag name. Append the current input character
            // to the current tag token's tag name.
            cur_tag_name_.push_back(CharUtil::to_ascii_lower(ch));
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::BeforeAttributeName:
        // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-name-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Ignore the character.
            continue;
          } else if (ch == '/' || ch == '>') {
            // U+002F SOLIDUS (/) | U+003E GREATER-THAN SIGN (>)
            // Reconsume in the after attribute name state.
            reconsume_ = true;
            state_ = State::AfterAttributeName;
          } else if (ch == '=') {
            // U+003D EQUALS SIGN (=)
            // This is an unexpected-equals-sign-before-attribute-name parse
            // error.
            print_parse_error("unexpected-equals-sign-before-attribute-name");
            // Start a new attribute in the current tag token.
            create_attr();
            // Set that attribute's name to the current input character, and its
            // value to the empty string.
            cur_attr_name_.push_back(ch);
            // Switch to the attribute name state.
            state_ = State::AttributeName;
          } else {
            // Anything else
            // Start a new attribute in the current tag token.
            create_attr();
            // Set that attribute name and value to the empty string.
            // Reconsume in the attribute name state.
            reconsume_ = true;
            state_ = State::AttributeName;
          }
        } else {
          // EOF - Reconsume in the after attribute name state.
          reconsume_ = true;
          state_ = State::AfterAttributeName;
        }
        break;
      case State::AttributeName:
        // https://html.spec.whatwg.org/multipage/parsing.html#attribute-name-state
        // TODO: When the user agent leaves the attribute name state (and before
        // emitting the tag token, if appropriate), the complete attribute's
        // name must be compared to the other attributes on the same token; if
        // there is already an attribute on the token with the exact same name,
        // then this is a duplicate-attribute parse error and the new attribute
        // must be removed from the token. If an attribute is so removed from a
        // token, it, and the value that gets associated with it, if any, are
        // never subsequently used by the parser, and are therefore effectively
        // discarded. Removing the attribute in this way does not change its
        // status as the "current attribute" for the purposes of the tokenizer,
        // however.
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ' ||
              ch == '/' || ch == '>') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE | U+002F SOLIDUS (/) |
            // U+003E GREATER-THAN SIGN (>)
            // Reconsume in the after attribute name state.
            reconsume_ = true;
            state_ = State::AfterAttributeName;
          } else if (ch == '=') {
            // U+003D EQUALS SIGN (=)
            // Switch to the before attribute value state.
            state_ = State::BeforeAttributeValue;
          } /* TODO: U+0000 NULL */ else if (ch == '"' || ch == '\'' ||
                                             ch == '<') {
            // U+0022 QUOTATION MARK (") | U+0027 APOSTROPHE (') | U+003C
            // LESS-THAN SIGN (<).
            // This is an unexpected-character-in-attribute-name parse error.
            print_parse_error("unexpected-character-in-attribute-name");
            // Treat it as per the "anything else" entry below.
            cur_attr_name_.push_back(CharUtil::to_ascii_lower(ch));
          } else {
            // Anything else
            // Append the current input character to the current attribute's
            // name.
            // ASCII upper alpha - Append the lowercase version of the current
            // input character (add 0x0020 to the character's code point) to the
            // current attribute's name.
            cur_attr_name_.push_back(CharUtil::to_ascii_lower(ch));
          }
        } else {
          // EOF - Reconsume in the after attribute name state.
          reconsume_ = true;
          state_ = State::AfterAttributeName;
        }
        break;
      case State::AfterAttributeName:
        // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-name-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Ignore the character.
            continue;
          } else if (ch == '/') {
            // U+002F SOLIDUS (/) - Switch to the self-closing start tag state.
            state_ = State::SelfClosingStartTag;
          } else if (ch == '=') {
            // U+003D EQUALS SIGN (=)
            // Switch to the before attribute value state.
            state_ = State::BeforeAttributeValue;
          } else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // Switch to the data state. Emit the current tag token.
            state_ = State::Data;
            return emit_cur_tag();
          } else {
            // Start a new attribute in the current tag token.
            // Set that attribute name and value to the empty string.
            create_attr();
            // Reconsume in the attribute name state.
            reconsume_ = true;
            state_ = State::AttributeName;
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::BeforeAttributeValue:
        // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-value-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Ignore the character.
            continue;
          } else if (ch == '"') {
            // U+0022 QUOTATION MARK (")
            // Switch to the attribute value (double-quoted) state.
            state_ = State::AttributeValueDoubleQuoted;
          } else if (ch == '\'') {
            // U+0027 APOSTROPHE (')
            // Switch to the attribute value (single-quoted) state.
            state_ = State::AttributeValueSingleQuoted;
          } else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // This is a missing-attribute-value parse error.
            print_parse_error("missing-attribute-value");
            // Switch to the data state.
            state_ = State::Data;
            // Emit the current tag token.
            return emit_cur_tag();
          } else {
            // Anything else
            // Reconsume in the attribute value (unquoted) state.
            reconsume_ = true;
            state_ = State::AttributeValueUnquoted;
          }
        } else {
          // Anything else - Reconsume in the attribute value (unquoted) state.
          reconsume_ = true;
          state_ = State::AttributeValueUnquoted;
        }
        break;
      case State::AttributeValueDoubleQuoted:
        // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(double-quoted)-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '"') {
            // U+0022 QUOTATION MARK (")
            // Switch to the after attribute value (quoted) state.
            state_ = State::AfterAttributeValueQuoted;
          } /* TODO: U+0026 AMPERSAND (&) U+0000 NULL */ else {
            // Anything else - Append the current input character to the current
            // attribute's value.
            cur_attr_value_.push_back(ch);
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag parse");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::AttributeValueSingleQuoted:
        // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(single-quoted)-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\'') {
            // U+0027 APOSTROPHE (')
            // Switch to the after attribute value (quoted) state.
            state_ = State::AfterAttributeValueQuoted;
          } /* TODO: U+0026 AMPERSAND (&) U+0000 NULL */ else {
            // Anything else - Append the current input character to the current
            // attribute's value.
            cur_attr_value_.push_back(ch);
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag parse");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::AttributeValueUnquoted:
        // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(unquoted)-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Switch to the before
            // attribute name state.
            state_ = State::BeforeAttributeName;
          } /* TODO: U+0026 AMPERSAND (&) */ else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // Switch to the data state.
            // Emit the current tag token.
            state_ = State::Data;
            return emit_cur_tag();
          } /* TODO: U+0000 NULL */ else if (ch == '"' || ch == '\'' ||
                                             ch == '<' || ch == '=' ||
                                             ch == '`') {
            // U+0022 QUOTATION MARK (") | U+0027 APOSTROPHE (') | U+003C
            // LESS-THAN SIGN (<) | U+003D EQUALS SIGN (=) | U+0060 GRAVE ACCENT
            // (`)
            // This is an unexpected-character-in-unquoted-attribute-value parse
            // error.
            print_parse_error(
                "unexpected-character-in-unquoted-attribute-value");
            // Treat it as per the "anything else" entry below.
            cur_attr_value_.push_back(ch);
          } else {
            // Anything else - Append the current input character to the current
            // attribute's value.
            cur_attr_value_.push_back(ch);
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag parse");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::AfterAttributeValueQuoted:
        // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-value-(quoted)-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '\t' || ch == '\n' || ch == '\f' || ch == ' ') {
            // U+0009 CHARACTER TABULATION (tab) | U+000A LINE FEED (LF) |
            // U+000C FORM FEED (FF) | U+0020 SPACE - Switch to the before
            // attribute name state.
            state_ = State::BeforeAttributeName;
          } else if (ch == '/') {
            // U+002F SOLIDUS (/) - Switch to the self-closing start tag state.
            state_ = State::SelfClosingStartTag;
          } else if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // Switch to the data state.
            // Emit the current tag token.
            state_ = State::Data;
            return emit_cur_tag();
          } else {
            // Anything else
            // This is a missing-whitespace-between-attributes parse error.
            print_parse_error("missing-whitespace-between-attributes");
            // Reconsume in the before attribute name state.
            reconsume_ = true;
            state_ = State::BeforeAttributeName;
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::SelfClosingStartTag:
        // https://html.spec.whatwg.org/multipage/parsing.html#self-closing-start-tag-state
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '>') {
            // U+003E GREATER-THAN SIGN (>)
            // Set the self-closing flag of the current tag token.
            cur_tag_self_closing_ = true;
            // Switch to the data state.
            state_ = State::Data;
            // Emit the current tag token.
            return emit_cur_tag();
          } else {
            // Anything else
            // This is an unexpected-solidus-in-tag parse error.
            print_parse_error("unexpected-solidus-in-tag");
            // Reconsume in the before attribute name state.
            reconsume_ = true;
            state_ = State::BeforeAttributeName;
          }
        } else {
          // This is an eof-in-tag parse error.
          print_parse_error("eof-in-tag");
          // Emit an end-of-file token.
          return emit_eof();
        }
        break;
      case State::Comment:
        // TODO: Comment & Bogus Comment
        if (c.has_value()) {
          auto ch = c.value();

          if (ch == '>') {
            state_ = State::Data;
          }
        } else {
          print_parse_error("eof-in-comment");
          return emit_eof();
        }
        break;
    }
  }
}

void Tokenizer::create_attr() { append_cur_attr(); }

void Tokenizer::clear_attr() {
  cur_attr_name_.clear();
  cur_attr_value_.clear();
}

void Tokenizer::create_start_tag() {
  cur_tag_kind_ = TokenTag::Kind::Start;
  create_tag();
}

void Tokenizer::create_end_tag() {
  cur_tag_kind_ = TokenTag::Kind::End;
  create_tag();
}

void Tokenizer::create_tag() {
  cur_tag_self_closing_ = false;
  clear_tag();
  clear_attr();
}

void Tokenizer::clear_tag() {
  cur_tag_name_.clear();
  cur_tag_attributes_.clear();
}

void Tokenizer::append_cur_attr() {
  if (cur_attr_name_.empty()) {
    return;
  }

  cur_tag_attributes_.emplace_back(std::move(cur_attr_name_),
                                   std::move(cur_attr_value_));

  clear_attr();
}

Token Tokenizer::emit_cur_tag() {
  append_cur_attr();

  TokenTag tag(cur_tag_kind_, std::move(cur_tag_name_), cur_tag_self_closing_,
               std::move(cur_tag_attributes_));
  clear_tag();

  if (tag.kind == TokenTag::Kind::Start) {
    return Token::new_start(std::move(tag));
  } else {
    return Token::new_end(std::move(tag));
  }
}

Token Tokenizer::emit_eof() { return Token::new_eof(); }

Token Tokenizer::emit_char(char ch) { return Token::new_char(ch); }