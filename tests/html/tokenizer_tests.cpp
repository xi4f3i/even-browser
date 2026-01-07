#include <gtest/gtest.h>

#include "../../src/html/tokenizer.h"
#include "html/token.h"

class TokenizerTest : public ::testing::Test {
protected:
    std::unique_ptr<Tokenizer> tokenizer;

    void SetUp() override { tokenizer = std::make_unique<Tokenizer>(""); }

    void TearDown() override { tokenizer.reset(); }
};

TEST_F(TokenizerTest, basic_text)
{
    tokenizer = std::make_unique<Tokenizer>("abc");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    auto& ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, 'a');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, 'b');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, 'c');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, basic_tags)
{
    tokenizer = std::make_unique<Tokenizer>("<div></div>");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::StartTag>(t.data));
    auto& startTag = std::get<Token::StartTag>(t.data);
    EXPECT_EQ(startTag.tag.kind, TokenTag::Kind::Start);
    EXPECT_EQ(startTag.tag.name, "div");
    EXPECT_FALSE(startTag.tag.self_closing);
    EXPECT_EQ(startTag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndTag>(t.data));
    auto& endTag = std::get<Token::EndTag>(t.data);
    EXPECT_EQ(endTag.tag.kind, TokenTag::Kind::End);
    EXPECT_EQ(endTag.tag.name, "div");
    EXPECT_EQ(endTag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, tag_case_insensitivity)
{
    tokenizer = std::make_unique<Tokenizer>("<DIV></div >");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::StartTag>(t.data));
    auto& startTag = std::get<Token::StartTag>(t.data);
    EXPECT_EQ(startTag.tag.kind, TokenTag::Kind::Start);
    EXPECT_EQ(startTag.tag.name, "div");
    EXPECT_FALSE(startTag.tag.self_closing);
    EXPECT_EQ(startTag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndTag>(t.data));
    auto& endTag = std::get<Token::EndTag>(t.data);
    EXPECT_EQ(endTag.tag.kind, TokenTag::Kind::End);
    EXPECT_EQ(endTag.tag.name, "div");
    EXPECT_EQ(endTag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, attributes_mixed)
{
    tokenizer = std::make_unique<Tokenizer>(
        "<div id=\"test\" v-data='v1' class=foo checked></div>");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::StartTag>(t.data));
    auto& startTag = std::get<Token::StartTag>(t.data);
    EXPECT_EQ(startTag.tag.kind, TokenTag::Kind::Start);
    EXPECT_EQ(startTag.tag.name, "div");
    EXPECT_FALSE(startTag.tag.self_closing);
    EXPECT_EQ(startTag.tag.attributes.size(), 4);
    EXPECT_EQ(startTag.tag.attributes[0].name, "id");
    EXPECT_EQ(startTag.tag.attributes[0].value, "test");
    EXPECT_EQ(startTag.tag.attributes[1].name, "v-data");
    EXPECT_EQ(startTag.tag.attributes[1].value, "v1");
    EXPECT_EQ(startTag.tag.attributes[2].name, "class");
    EXPECT_EQ(startTag.tag.attributes[2].value, "foo");
    EXPECT_EQ(startTag.tag.attributes[3].name, "checked");
    EXPECT_EQ(startTag.tag.attributes[3].value, "");

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndTag>(t.data));
    auto& endTag = std::get<Token::EndTag>(t.data);
    EXPECT_EQ(endTag.tag.kind, TokenTag::Kind::End);
    EXPECT_EQ(endTag.tag.name, "div");
    EXPECT_EQ(endTag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, self_closing_tag)
{
    tokenizer = std::make_unique<Tokenizer>("<br/>");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::StartTag>(t.data));
    auto& tag = std::get<Token::StartTag>(t.data);
    EXPECT_EQ(tag.tag.kind, TokenTag::Kind::Start);
    EXPECT_EQ(tag.tag.name, "br");
    EXPECT_TRUE(tag.tag.self_closing);
    EXPECT_EQ(tag.tag.attributes.size(), 0);

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, eof_in_tag)
{
    tokenizer = std::make_unique<Tokenizer>("</");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    auto& ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, '<');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, '/');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, invalid_tag_name_start)
{
    tokenizer = std::make_unique<Tokenizer>("<4");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    auto& ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, '<');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::Character>(t.data));
    ch = std::get<Token::Character>(t.data);
    EXPECT_EQ(ch.value, '4');

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}

TEST_F(TokenizerTest, attribute_value_with_illegal_chars)
{
    tokenizer = std::make_unique<Tokenizer>("<div data=foo\"bar>");

    Token t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::StartTag>(t.data));
    auto& tag = std::get<Token::StartTag>(t.data);
    EXPECT_EQ(tag.tag.kind, TokenTag::Kind::Start);
    EXPECT_EQ(tag.tag.name, "div");
    EXPECT_FALSE(tag.tag.self_closing);
    EXPECT_EQ(tag.tag.attributes.size(), 1);
    EXPECT_EQ(tag.tag.attributes[0].name, "data");
    EXPECT_EQ(tag.tag.attributes[0].value, "foo\"bar");

    t = tokenizer->next();
    ASSERT_TRUE(std::holds_alternative<Token::EndOfFile>(t.data));
}
