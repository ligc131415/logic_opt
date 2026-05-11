#include <gtest/gtest.h>
#include "../src/parser/lexer.h"

namespace logic {

// ========== Test Token Creation ==========

TEST(LexerTest, TokenCreation) {
    Token t(TokenType::VARIABLE, "A", 1, 1);
    EXPECT_EQ(t.type, TokenType::VARIABLE);
    EXPECT_EQ(t.value, "A");
    EXPECT_EQ(t.line, 1);
    EXPECT_EQ(t.column, 1);
}

TEST(LexerTest, TokenToString) {
    Token t(TokenType::AND, "", 1, 5);
    std::string s = t.to_string();
    EXPECT_EQ(s, "AND(1:5)");
}

// ========== Test Lexer: Basic Operators ==========

TEST(LexerTest, LexAndOperator) {
    Lexer lexer("a & b");
    auto tokens = lexer.tokenize();

    // Should have: VARIABLE, AND, VARIABLE, EOF
    EXPECT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[0].value, "a");
    EXPECT_EQ(tokens[1].type, TokenType::AND);
    EXPECT_EQ(tokens[2].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[2].value, "b");
    EXPECT_EQ(tokens[3].type, TokenType::EOF_TOKEN);
}

TEST(LexerTest, LexOrOperator) {
    Lexer lexer("a | b");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[1].type, TokenType::OR);
}

TEST(LexerTest, LexNotOperator) {
    Lexer lexer("~a");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::NOT);
    EXPECT_EQ(tokens[1].type, TokenType::VARIABLE);
}

TEST(LexerTest, LexXorOperator) {
    Lexer lexer("a ^ b");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[1].type, TokenType::XOR);
}

// ========== Test Lexer: Parentheses ==========

TEST(LexerTest, LexLeftParen) {
    Lexer lexer("(a");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::LPAREN);
}

TEST(LexerTest, LexRightParen) {
    Lexer lexer("a)");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[1].type, TokenType::RPAREN);
}

// ========== Test Lexer: Variables ==========

TEST(LexerTest, LexSingleLetterVariable) {
    Lexer lexer("A");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[0].value, "A");
}

TEST(LexerTest, LexMultiLetterVariable) {
    Lexer lexer("ABC");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[0].value, "ABC");
}

TEST(LexerTest, LexVariableWithNumber) {
    Lexer lexer("A1B2");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[0].value, "A1B2");
}

// ========== Test Lexer: Whitespace Handling ==========

TEST(LexerTest, LexWithSpaces) {
    Lexer lexer("  A   &   B  ");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens.size(), 4);  // VARIABLE, AND, VARIABLE, EOF
    EXPECT_EQ(tokens[0].value, "A");
    EXPECT_EQ(tokens[2].value, "B");
}

TEST(LexerTest, LexWithTabsAndNewlines) {
    Lexer lexer("A\t&B\nC");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens.size(), 5);  // VARIABLE, AND, VARIABLE, VARIABLE, EOF
}

// ========== Test Lexer: Complex Expressions ==========

TEST(LexerTest, LexComplexExpression) {
    Lexer lexer("(A & B) | (~C ^ D)");
    auto tokens = lexer.tokenize();

    // 输入去除空格后: (A&B)|(~C^D)
    // Actual: LPAREN, VARIABLE, AND, VARIABLE, RPAREN, OR, LPAREN, NOT, VARIABLE, XOR, VARIABLE, RPAREN, EOF
    EXPECT_EQ(tokens.size(), 13);
    EXPECT_EQ(tokens[0].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[1].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[1].value, "A");
    EXPECT_EQ(tokens[2].type, TokenType::AND);
    EXPECT_EQ(tokens[3].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[3].value, "B");
    EXPECT_EQ(tokens[4].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[5].type, TokenType::OR);
    EXPECT_EQ(tokens[6].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[7].type, TokenType::NOT);
    EXPECT_EQ(tokens[8].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[8].value, "C");
    EXPECT_EQ(tokens[9].type, TokenType::XOR);
    EXPECT_EQ(tokens[10].type, TokenType::VARIABLE);
    EXPECT_EQ(tokens[10].value, "D");
    EXPECT_EQ(tokens[11].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[12].type, TokenType::EOF_TOKEN);
}

// ========== Test Lexer: Invalid Characters ==========

TEST(LexerTest, LexInvalidCharacter) {
    Lexer lexer("A $ B");
    auto tokens = lexer.tokenize();

    // Should have: VARIABLE, INVALID, VARIABLE, EOF
    EXPECT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[1].type, TokenType::INVALID);
}

} // namespace logic
