#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "state.h"
#include "token.h"

/// @brief HTML Tokenizer
///
/// https://html.spec.whatwg.org/multipage/parsing.html#tokenization
class Tokenizer {
 private:
  std::string_view input_;
  std::size_t pos_;
  bool reconsume_;
  State state_;
  std::vector<Token> pending_tokens_;
  TokenTag::Kind cur_tag_kind_;
  std::string cur_tag_name_;
  bool cur_tag_self_closing_;
  std::vector<Attribute> cur_tag_attributes_;
  std::string cur_attr_name_;
  std::string cur_attr_value_;

  std::optional<char> peek();
  Token emit_eof();
  Token emit_char(char ch);
  Token emit_cur_tag();
  void create_start_tag();
  void create_end_tag();
  void create_tag();
  void clear_tag();
  void create_attr();
  void clear_attr();
  void append_cur_attr();

 public:
  explicit Tokenizer(std::string_view input);
  ~Tokenizer();
  Token next();
};