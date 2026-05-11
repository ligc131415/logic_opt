#include <gtest/gtest.h>
#include "../src/parser/expression.h"
#include "../src/parser/lexer.h"
#include "../src/parser/parser.h"
#include "../src/evaluator/truth_table.h"

namespace logic {

// ========== Test TruthTable: Basic Construction ==========

TEST(TruthTableTest, SingleVariable) {
    Parser parser("A");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.get_variable_count(), 1);
    EXPECT_EQ(table.size(), 2);  // 2^1 = 2 rows
}

TEST(TruthTableTest, TwoVariables) {
    Parser parser("A & B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.get_variable_count(), 2);
    EXPECT_EQ(table.size(), 4);  // 2^2 = 4 rows
}

TEST(TruthTableTest, ThreeVariables) {
    Parser parser("A | B | C");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.get_variable_count(), 3);
    EXPECT_EQ(table.size(), 8);  // 2^3 = 8 rows
}

// ========== Test TruthTable: AND Function ==========

TEST(TruthTableTest, AndFunction) {
    Parser parser("A & B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // A=0, B=0: output = 0
    EXPECT_EQ(table.evaluate(0b00), false);
    // A=0, B=1: output = 0
    EXPECT_EQ(table.evaluate(0b01), false);
    // A=1, B=0: output = 0
    EXPECT_EQ(table.evaluate(0b10), false);
    // A=1, B=1: output = 1
    EXPECT_EQ(table.evaluate(0b11), true);

    auto minterms = table.get_minterms();
    EXPECT_EQ(minterms.size(), 1);
    EXPECT_EQ(minterms[0], 0b11);
}

// ========== Test TruthTable: OR Function ==========

TEST(TruthTableTest, OrFunction) {
    Parser parser("A | B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    auto minterms = table.get_minterms();
    EXPECT_EQ(minterms.size(), 3);  // 01, 10, 11
}

TEST(TruthTableTest, OrFunctionValues) {
    Parser parser("A | B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.evaluate(0b00), false);
    EXPECT_EQ(table.evaluate(0b01), true);
    EXPECT_EQ(table.evaluate(0b10), true);
    EXPECT_EQ(table.evaluate(0b11), true);
}

// ========== Test TruthTable: NOT Function ==========

TEST(TruthTableTest, NotFunction) {
    Parser parser("~A");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.evaluate(0b0), true);
    EXPECT_EQ(table.evaluate(0b1), false);
}

// ========== Test TruthTable: XOR Function ==========

TEST(TruthTableTest, XorFunction) {
    Parser parser("A ^ B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    EXPECT_EQ(table.evaluate(0b00), false);
    EXPECT_EQ(table.evaluate(0b01), true);
    EXPECT_EQ(table.evaluate(0b10), true);
    EXPECT_EQ(table.evaluate(0b11), false);

    auto minterms = table.get_minterms();
    EXPECT_EQ(minterms.size(), 2);
}

// ========== Test TruthTable: Complex Expressions ==========

TEST(TruthTableTest, expression) {
    Parser parser("(A & B) | (A & ~B)");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // This simplifies to A, so output should equal A's value
    // mask 0b00: A=0, B=0 -> false
    // mask 0b01: A=1, B=0 -> true
    // mask 0b10: A=0, B=1 -> false
    // mask 0b11: A=1, B=1 -> true
    EXPECT_EQ(table.evaluate(0b00), false);
    EXPECT_EQ(table.evaluate(0b01), true);
    EXPECT_EQ(table.evaluate(0b10), false);
    EXPECT_EQ(table.evaluate(0b11), true);
}

TEST(TruthTableTest, Implification) {
    Parser parser("(A | B) & (~A | C)");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // For (A | B) & (~A | C):
    // mask 0b000: A=0, B=0, C=0 -> (0|0)&(1|0) = 0&1 = 0
    // mask 0b110: A=0, B=1, C=1 -> (0|1)&(1|1) = 1&1 = 1
    EXPECT_EQ(table.evaluate(0b000), false);
    EXPECT_EQ(table.evaluate(0b110), true);
}

// ========== Test TruthTable: Minterms and Maxterms ==========

TEST(TruthTableTest, GetMinterms) {
    Parser parser("A & B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    auto minterms = table.get_minterms();
    EXPECT_EQ(minterms.size(), 1);
    EXPECT_EQ(minterms[0], 3);  // 11 binary
}

TEST(TruthTableTest, GetMaxterms) {
    Parser parser("A & B");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    auto maxterms = table.get_maxterms();
    EXPECT_EQ(maxterms.size(), 3);  // 0, 1, 2
}

// ========== Test TruthTable: Variable Extraction ==========

TEST(TruthTableTest, ExtractVariablesSimple) {
    Parser parser("A & B | C");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 3);
    // Should be sorted
    EXPECT_EQ(vars[0], "A");
    EXPECT_EQ(vars[1], "B");
    EXPECT_EQ(vars[2], "C");
}

TEST(TruthTableTest, ExtractVariablesDuplicates) {
    Parser parser("A & A | A");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0], "A");
}

TEST(TruthTableTest, ExtractVariablesMultiChar) {
    Parser parser("ABC & XYZ");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);

    EXPECT_EQ(vars.size(), 2);
}

// ========== Test TruthTable: Edge Cases ==========

TEST(TruthTableTest, ConstantTrue) {
    Parser parser("1");  // Using our hack for constants
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // Should always be true regardless of inputs
    for (size_t i = 0; i < table.size(); ++i) {
        EXPECT_EQ(table.evaluate(i), true);
    }
}

TEST(TruthTableTest, ConstantFalse) {
    Parser parser("0");  // Using our hack for constants
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // Should always be false
    for (size_t i = 0; i < table.size(); ++i) {
        EXPECT_EQ(table.evaluate(i), false);
    }
}

// ========== Test TruthTable: Output Formatting ==========

TEST(TruthTableTest, PrintDoesNotCrash) {
    Parser parser("A & B | C");
    auto ast = parser.parse();
    auto vars = extract_variables(ast);
    TruthTable table(ast, vars);

    // Just make sure it doesn't crash
    table.print();
}

} // namespace logic
