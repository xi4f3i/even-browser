#pragma once

#include <string>
#include <variant>
#include <vector>

struct Attribute {
  std::string name;
  std::string value;

  Attribute(std::string name, std::string value) : name(name), value(value) {}
};

struct TokenTag {
  enum class Kind { Start, End } kind;
  std::string name;
  bool self_closing = false;
  std::vector<Attribute> attributes;

  TokenTag() : kind(Kind::Start), self_closing(false) {}

  TokenTag(Kind kind, std::string name, bool self_closing,
           std::vector<Attribute> attributes)
      : kind(kind),
        name(name),
        self_closing(self_closing),
        attributes(attributes) {}
};

struct Token {
  enum class Kind {
    StartTag,
    EndTag,
    Character,
    EndOfFile,
  } kind;

  struct StartTag {
    TokenTag tag;
  };

  struct EndTag {
    TokenTag tag;
  };

  struct Character {
    char value;
  };

  struct EndOfFile {};

  using Data = std::variant<StartTag, EndTag, Character, EndOfFile>;
  Data data;

  static Token new_start(TokenTag t) {
    return {Token::Kind::StartTag, StartTag{std::move(t)}};
  }

  static Token new_end(TokenTag t) {
    return {Token::Kind::EndTag, EndTag{std::move(t)}};
  }

  static Token new_char(char c) {
    return {Token::Kind::Character, Character{c}};
  }

  static Token new_eof() { return {Token::Kind::EndOfFile, EndOfFile{}}; }
};