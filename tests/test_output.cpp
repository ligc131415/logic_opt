#include <gtest/gtest.h>
#include "../src/minimizer/qmc.h"
#include "../src/minimizer/output.h"

namespace logic {

// ========== Test OutputFormatter: SOP Format ==========

TEST(OutputFormatterTest, FormatSopEmpty) {
    std::vector<uint64_t> minterms = {};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_sop(minterms, vars);
    EXPECT_EQ(sop, "0");
}

TEST(OutputFormatterTest, FormatSopAll) {
    // All 4 minterms for 2 variables
    std::vector<uint64_t> minterms = {0, 1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_sop(minterms, vars);
    EXPECT_EQ(sop, "1");
}

TEST(OutputFormatterTest, FormatSopSingle) {
    std::vector<uint64_t> minterms = {3};  // A & B
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_sop(minterms, vars);
    // Should contain A and B
    EXPECT_NE(sop.find("A"), std::string::npos);
    EXPECT_NE(sop.find("B"), std::string::npos);
}

TEST(OutputFormatterTest, FormatSopOr) {
    std::vector<uint64_t> minterms = {1, 2, 3};  // A | B
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_sop(minterms, vars);
    EXPECT_NE(sop.find("|"), std::string::npos);
}

// ========== Test OutputFormatter: POS Format ==========

TEST(OutputFormatterTest, FormatPosEmpty) {
    std::vector<uint64_t> maxterms = {};
    std::vector<std::string> vars = {"A", "B"};

    std::string pos = OutputFormatter::format_pos(maxterms, vars);
    EXPECT_EQ(pos, "1");
}

TEST(OutputFormatterTest, FormatPosAll) {
    // All 4 maxterms for 2 variables
    std::vector<uint64_t> maxterms = {0, 1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    std::string pos = OutputFormatter::format_pos(maxterms, vars);
    EXPECT_EQ(pos, "0");
}

TEST(OutputFormatterTest, FormatPosAnd) {
    std::vector<uint64_t> maxterms = {0, 1, 2};  // A & B
    std::vector<std::string> vars = {"A", "B"};

    std::string pos = OutputFormatter::format_pos(maxterms, vars);
    // Should contain & for AND
    EXPECT_NE(pos.find("&"), std::string::npos);
}

// ========== Test OutputFormatter: From Minterms ==========

TEST(OutputFormatterTest, GetSopFromMintermsSimple) {
    std::vector<uint64_t> minterms = {2, 3};  // B (10, 11)
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::get_sop_from_minterms(minterms, vars);
    // Should simplify to just B
    EXPECT_EQ(sop, "B");
}

TEST(OutputFormatterTest, GetSopFromMintermsOr) {
    std::vector<uint64_t> minterms = {1, 2, 3};  // A | B
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::get_sop_from_minterms(minterms, vars);
    // Should contain A and B with OR
    EXPECT_NE(sop.find("A"), std::string::npos);
    EXPECT_NE(sop.find("B"), std::string::npos);
}

// ========== Test OutputFormatter: From Maxterms ==========

TEST(OutputFormatterTest, GetPosFromMaxtermsSimple) {
    std::vector<uint64_t> maxterms = {0, 1, 2};  // A & B
    std::vector<std::string> vars = {"A", "B"};

    std::string pos = OutputFormatter::get_pos_from_maxterms(maxterms, vars);
    // Should contain A and B
    EXPECT_NE(pos.find("A"), std::string::npos);
    EXPECT_NE(pos.find("B"), std::string::npos);
}

// ========== Test OutputFormatter: Implicant Formatting ==========

TEST(OutputFormatterTest, FormatImplicantsAsSopEmpty) {
    std::vector<QMCImplicant> implicants = {};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_implicants_as_sop(implicants, vars);
    EXPECT_EQ(sop, "0");
}

TEST(OutputFormatterTest, FormatImplicantsAsSopSingle) {
    std::vector<int> bits = {1, -1};  // A only
    QMCImplicant imp(0, bits);
    std::vector<QMCImplicant> implicants = {imp};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_implicants_as_sop(implicants, vars);
    EXPECT_EQ(sop, "A");
}

TEST(OutputFormatterTest, FormatImplicantsAsSopAnd) {
    std::vector<int> bits = {1, 0};  // A & ~B
    QMCImplicant imp(0, bits);
    std::vector<QMCImplicant> implicants = {imp};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_implicants_as_sop(implicants, vars);
    EXPECT_NE(sop.find("A"), std::string::npos);
    EXPECT_NE(sop.find("~B"), std::string::npos);
}

TEST(OutputFormatterTest, FormatImplicantsAsSopOr) {
    std::vector<int> bits1 = {1, -1};  // A
    std::vector<int> bits2 = {0, 1};   // ~B
    QMCImplicant imp1(0, bits1);
    QMCImplicant imp2(0, bits2);
    std::vector<QMCImplicant> implicants = {imp1, imp2};
    std::vector<std::string> vars = {"A", "B"};

    std::string sop = OutputFormatter::format_implicants_as_sop(implicants, vars);
    EXPECT_NE(sop.find("|"), std::string::npos);
    EXPECT_NE(sop.find("A"), std::string::npos);
    EXPECT_NE(sop.find("B"), std::string::npos);
}

// ========== Test OutputFormatter: Product Term Formatting ==========

TEST(OutputFormatterTest, FormatProductTermAllSpecified) {
    std::vector<int> bits = {0, 1};  // ~A & B
    std::vector<std::string> vars = {"A", "B"};

    std::string term = OutputFormatter::format_product_term(bits, vars, true);
    EXPECT_NE(term.find("~A"), std::string::npos);
    EXPECT_NE(term.find("B"), std::string::npos);
}

TEST(OutputFormatterTest, FormatProductTermWildcard) {
    std::vector<int> bits = {0, -1};  // ~A
    std::vector<std::string> vars = {"A", "B"};

    std::string term = OutputFormatter::format_product_term(bits, vars, true);
    EXPECT_EQ(term, "~A");
}

TEST(OutputFormatterTest, FormatProductTermEmpty) {
    std::vector<int> bits = {-1, -1};  // All wildcards = 1
    std::vector<std::string> vars = {"A", "B"};

    std::string term = OutputFormatter::format_product_term(bits, vars, true);
    EXPECT_EQ(term, "1");
}

// ========== Test OutputFormatter: Variable Naming ==========

TEST(OutputFormatterTest, GetVariableName) {
    EXPECT_EQ(OutputFormatter::get_variable_name(0), "A");
    EXPECT_EQ(OutputFormatter::get_variable_name(1), "B");
    EXPECT_EQ(OutputFormatter::get_variable_name(25), "Z");
    EXPECT_EQ(OutputFormatter::get_variable_name(26), "X26");
}

// ========== Test OutputFormatter: Minimization Output ==========

TEST(OutputFormatterTest, PrintMinimizationResult) {
    std::vector<uint64_t> minterms = {2, 3};  // A
    std::vector<uint64_t> maxterms = {0, 1};
    std::vector<std::string> vars = {"A", "B"};

    // Should not crash
    OutputFormatter::print_minimization_result(minterms, maxterms, vars);
}

TEST(OutputFormatterTest, PrintMinimizationResultAll) {
    std::vector<uint64_t> minterms = {0, 1, 2, 3};  // 1
    std::vector<uint64_t> maxterms = {};
    std::vector<std::string> vars = {"A", "B"};

    // Should not crash
    OutputFormatter::print_minimization_result(minterms, maxterms, vars);
}

TEST(OutputFormatterTest, PrintMinimizationResultNone) {
    std::vector<uint64_t> minterms = {};  // 0
    std::vector<uint64_t> maxterms = {0, 1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    // Should not crash
    OutputFormatter::print_minimization_result(minterms, maxterms, vars);
}

// ========== Test OutputFormatter: Edge Cases ==========

TEST(OutputFormatterTest, SingleVariableSop) {
    std::vector<uint64_t> minterms = {1};  // A
    std::vector<std::string> vars = {"A"};

    std::string sop = OutputFormatter::get_sop_from_minterms(minterms, vars);
    EXPECT_EQ(sop, "A");
}

TEST(OutputFormatterTest, SingleVariablePos) {
    std::vector<uint64_t> maxterms = {0};  // A
    std::vector<std::string> vars = {"A"};

    std::string pos = OutputFormatter::get_pos_from_maxterms(maxterms, vars);
    EXPECT_EQ(pos, "A");
}

// ========== Test OutputFormatter: Three Variables ==========

TEST(OutputFormatterTest, ThreeVarMajority) {
    // Majority function: 011, 101, 110, 111 (3, 5, 6, 7)
    std::vector<uint64_t> minterms = {3, 5, 6, 7};
    std::vector<std::string> vars = {"A", "B", "C"};

    std::string sop = OutputFormatter::get_sop_from_minterms(minterms, vars);
    // Should have 3 terms
    size_t or_count = 0;
    for (char c : sop) {
        if (c == '|') or_count++;
    }
    EXPECT_EQ(or_count, 2);  // 3 terms = 2 ORs
}

} // namespace logic
