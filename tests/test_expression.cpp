#include <gtest/gtest.h>
#include "../src/parser/expression.h"

namespace logic {

// ========== Test ExpressionNode Creation ==========

TEST(ExpressionNodeTest, CreateVariable) {
    auto var = make_var("A");
    EXPECT_EQ(var->op, OpType::VARIABLE);
    EXPECT_EQ(var->get_var_name(), "A");
    EXPECT_TRUE(var->is_variable());
    EXPECT_TRUE(var->is_leaf());
}

TEST(ExpressionNodeTest, CreateConstantTrue) {
    auto c = make_true();
    EXPECT_EQ(c->op, OpType::TRUE);
    EXPECT_TRUE(c->is_constant());
}

TEST(ExpressionNodeTest, CreateConstantFalse) {
    auto c = make_false();
    EXPECT_EQ(c->op, OpType::FALSE);
    EXPECT_TRUE(c->is_constant());
}

TEST(ExpressionNodeTest, CreateNot) {
    auto child = make_var("A");
    auto not_node = make_not(std::move(child));
    EXPECT_EQ(not_node->op, OpType::NOT);
    EXPECT_EQ(not_node->children.size(), 1);
}

TEST(ExpressionNodeTest, CreateAnd) {
    auto left = make_var("A");
    auto right = make_var("B");
    auto and_node = make_and(std::move(left), std::move(right));
    EXPECT_EQ(and_node->op, OpType::AND);
    EXPECT_EQ(and_node->children.size(), 2);
}

TEST(ExpressionNodeTest, CreateOr) {
    auto left = make_var("A");
    auto right = make_var("B");
    auto or_node = make_or(std::move(left), std::move(right));
    EXPECT_EQ(or_node->op, OpType::OR);
    EXPECT_EQ(or_node->children.size(), 2);
}

TEST(ExpressionNodeTest, CreateXor) {
    auto left = make_var("A");
    auto right = make_var("B");
    auto xor_node = make_xor(std::move(left), std::move(right));
    EXPECT_EQ(xor_node->op, OpType::XOR);
    EXPECT_EQ(xor_node->children.size(), 2);
}

// ========== Test Operator Precedence ==========

TEST(ExpressionNodeTest, PrecedenceOrder) {
    EXPECT_GT(get_precedence(OpType::NOT), get_precedence(OpType::AND));
    EXPECT_GT(get_precedence(OpType::AND), get_precedence(OpType::XOR));
    EXPECT_GT(get_precedence(OpType::XOR), get_precedence(OpType::OR));
    EXPECT_EQ(get_precedence(OpType::OR), 1);
    EXPECT_EQ(get_precedence(OpType::XOR), 2);
    EXPECT_EQ(get_precedence(OpType::AND), 3);
    EXPECT_EQ(get_precedence(OpType::NOT), 4);
}

// ========== Test Right Associativity ==========

TEST(ExpressionNodeTest, RightAssociativeNot) {
    EXPECT_TRUE(is_right_associative(OpType::NOT));
    EXPECT_FALSE(is_right_associative(OpType::AND));
    EXPECT_FALSE(is_right_associative(OpType::OR));
}

// ========== Test to_string (AST Visualization) ==========

TEST(ExpressionNodeTest, ToStringVariable) {
    auto var = make_var("X");
    std::string result = var->to_string();
    EXPECT_NE(result.find("VARIABLE"), std::string::npos);
    EXPECT_NE(result.find("X"), std::string::npos);
}

TEST(ExpressionNodeTest, ToStringAnd) {
    auto left = make_var("A");
    auto right = make_var("B");
    auto node = make_and(std::move(left), std::move(right));
    std::string result = node->to_string();
    EXPECT_NE(result.find("AND"), std::string::npos);
}

TEST(ExpressionNodeTest, ToStringNested) {
    auto a = make_var("A");
    auto b = make_var("B");
    auto c = make_var("C");
    auto and1 = make_and(std::move(a), std::move(b));
    auto or_node = make_or(std::move(and1), std::move(c));
    std::string result = or_node->to_string();
    EXPECT_NE(result.find("OR"), std::string::npos);
    EXPECT_NE(result.find("AND"), std::string::npos);
    EXPECT_NE(result.find("A"), std::string::npos);
    EXPECT_NE(result.find("B"), std::string::npos);
    EXPECT_NE(result.find("C"), std::string::npos);
}

} // namespace logic
