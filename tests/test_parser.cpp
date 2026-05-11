#include <gtest/gtest.h>
#include "../src/parser/expression.h"
#include "../src/parser/lexer.h"
#include "../src/parser/parser.h"
#include "../src/evaluator/truth_table.h"

namespace logic {

// ========== Test Parser: Basic Expressions ==========

TEST(ParserTest, ParseSingleVariable) {
    Parser parser("A");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::VARIABLE);
    EXPECT_EQ(ast->get_var_name(), "A");
}

TEST(ParserTest, ParseVariableWithSpaces) {
    Parser parser("  A  ");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::VARIABLE);
}

// ========== Test Parser: AND Operator ==========

TEST(ParserTest, ParseAndExpression) {
    Parser parser("A & B");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::AND);
    EXPECT_EQ(ast->children.size(), 2);
    EXPECT_EQ(ast->children[0]->op, OpType::VARIABLE);
    EXPECT_EQ(ast->children[0]->get_var_name(), "A");
    EXPECT_EQ(ast->children[1]->op, OpType::VARIABLE);
    EXPECT_EQ(ast->children[1]->get_var_name(), "B");
}

TEST(ParserTest, ParseMultipleAnd) {
    Parser parser("A & B & C");
    auto ast = parser.parse();

    // Should be left-associative: ((A & B) & C)
    EXPECT_EQ(ast->op, OpType::AND);
    EXPECT_EQ(ast->children.size(), 2);
    EXPECT_EQ(ast->children[0]->op, OpType::AND);
}

// ========== Test Parser: OR Operator ==========

TEST(ParserTest, ParseOrExpression) {
    Parser parser("A | B");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::OR);
    EXPECT_EQ(ast->children.size(), 2);
}

TEST(ParserTest, ParseMultipleOr) {
    Parser parser("A | B | C");
    auto ast = parser.parse();

    EXPECT_EQ(ast->op, OpType::OR);
    EXPECT_EQ(ast->children.size(), 2);
}

// ========== Test Parser: XOR Operator ==========

TEST(ParserTest, ParseXorExpression) {
    Parser parser("A ^ B");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::XOR);
    EXPECT_EQ(ast->children.size(), 2);
}

// ========== Test Parser: NOT Operator ==========

TEST(ParserTest, ParseNotVariable) {
    Parser parser("~A");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::NOT);
    EXPECT_EQ(ast->children.size(), 1);
    EXPECT_EQ(ast->children[0]->op, OpType::VARIABLE);
}

TEST(ParserTest, ParseDoubleNot) {
    Parser parser("~~A");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::NOT);
    EXPECT_EQ(ast->children.size(), 1);
    EXPECT_EQ(ast->children[0]->op, OpType::NOT);
}

TEST(ParserTest, ParseNotComplex) {
    Parser parser("~(A & B)");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::NOT);
    EXPECT_EQ(ast->children[0]->op, OpType::AND);
}

// ========== Test Parser: Parentheses ==========

TEST(ParserTest, ParseParenthesizedExpression) {
    Parser parser("(A)");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::VARIABLE);
    EXPECT_EQ(ast->get_var_name(), "A");
}

TEST(ParserTest, ParseNestedParentheses) {
    Parser parser("((A))");
    auto ast = parser.parse();

    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->op, OpType::VARIABLE);
}

TEST(ParserTest, ParseParenthesizedAndOr) {
    Parser parser("(A | B) & C");
    auto ast = parser.parse();

    EXPECT_EQ(ast->op, OpType::AND);
    EXPECT_EQ(ast->children[0]->op, OpType::OR);
    EXPECT_EQ(ast->children[1]->op, OpType::VARIABLE);
}

// ========== Test Parser: Operator Precedence ==========

TEST(ParserTest, NotBeforeAnd) {
    Parser parser("~A & B");
    auto ast = parser.parse();

    // ~ should bind tighter: (~A) & B
    EXPECT_EQ(ast->op, OpType::AND);
    EXPECT_EQ(ast->children[0]->op, OpType::NOT);
}

TEST(ParserTest, AndBeforeOr) {
    Parser parser("A | B & C");
    auto ast = parser.parse();

    // & should bind tighter: A | (B & C)
    EXPECT_EQ(ast->op, OpType::OR);
    EXPECT_EQ(ast->children[1]->op, OpType::AND);
}

TEST(ParserTest, XorBeforeOr) {
    Parser parser("A | B ^ C");
    auto ast = parser.parse();

    // ^ should bind tighter: A | (B ^ C)
    EXPECT_EQ(ast->op, OpType::OR);
    EXPECT_EQ(ast->children[1]->op, OpType::XOR);
}

// ========== Test Parser: Complex Expressions ==========

TEST(ParserTest, Parseexpression) {
    Parser parser("(A & B) | (A & ~B)");
    auto ast = parser.parse();

    EXPECT_EQ(ast->op, OpType::OR);
    EXPECT_EQ(ast->children[0]->op, OpType::AND);
    EXPECT_EQ(ast->children[1]->op, OpType::AND);
    EXPECT_EQ(ast->children[1]->children[1]->op, OpType::NOT);
}

TEST(ParserTest, ParseXorChain) {
    Parser parser("A ^ B ^ C");
    auto ast = parser.parse();

    // Left-associative: (A ^ B) ^ C
    EXPECT_EQ(ast->op, OpType::XOR);
    EXPECT_EQ(ast->children[0]->op, OpType::XOR);
}

// ========== Test Parser: Error Cases ==========

TEST(ParserTest, ParseUnbalancedParenthesisLeft) {
    Parser parser("(A & B");
    try {
        auto ast = parser.parse();
        FAIL() << "Expected exception for unbalanced parentheses";
    } catch (const std::exception& e) {
        // Expected
    }
}

TEST(ParserTest, ParseUnbalancedParenthesisRight) {
    Parser parser("A & B)");
    try {
        auto ast = parser.parse();
        FAIL() << "Expected exception for unbalanced parentheses";
    } catch (const std::exception& e) {
        // Expected
    }
}

TEST(ParserTest, ParseEmpty) {
    Parser parser("");
    try {
        auto ast = parser.parse();
        // Empty input might just return nothing or throw
    } catch (const std::exception& e) {
        // Expected or not, depending on implementation
    }
}

// ========== Test Helper Functions ==========

TEST(ParserTest, ExtractVariablesSingle) {
    Parser parser("A");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0], "A");
}

TEST(ParserTest, ExtractVariablesMultiple) {
    Parser parser("A & B | C");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 3);
    EXPECT_EQ(vars[0], "A");
    EXPECT_EQ(vars[1], "B");
    EXPECT_EQ(vars[2], "C");
}

TEST(ParserTest, ExtractVariablesDuplicates) {
    Parser parser("A & A | A");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0], "A");
}

TEST(ParserTest, ExtractVariablesSorted) {
    Parser parser("C & A | B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    // Should be sorted
    EXPECT_EQ(vars[0], "A");
    EXPECT_EQ(vars[1], "B");
    EXPECT_EQ(vars[2], "C");
}

} // namespace logic
