#include <gtest/gtest.h>
#include "../src/minimizer/qmc.h"

namespace logic {

// ========== Test QMCImplicant: Basic Operations ==========

TEST(QMCImplicantTest, CreateImplicant) {
    std::vector<int> bits = {0, 1, -1};  // A=0, B=1, C=don't care
    QMCImplicant imp(0b010, bits);

    EXPECT_EQ(imp.num_variables(), 3);
    EXPECT_EQ(imp.bits[0], 0);
    EXPECT_EQ(imp.bits[1], 1);
    EXPECT_EQ(imp.bits[2], -1);
}

TEST(QMCImplicantTest, CheckCoverage) {
    std::vector<int> bits = {0, -1};  // A=0, B=don't care
    QMCImplicant imp(0, bits);

    // Should cover both A=0, B=0 and A=0, B=1
    EXPECT_TRUE(imp.covers(0b00));  // A=0, B=0
    EXPECT_TRUE(imp.covers(0b10));  // A=0, B=1 (B is bit 1)
}

TEST(QMCImplicantTest, DoesNotCoverWrongValue) {
    std::vector<int> bits = {0, -1};  // A=0, B=don't care
    QMCImplicant imp(0, bits);

    // Should NOT cover A=1
    EXPECT_FALSE(imp.covers(0b01));  // A=1, B=0
    EXPECT_FALSE(imp.covers(0b11));  // A=1, B=1
}

// ========== Test QMCImplicant: Merging ==========

TEST(QMCImplicantTest, CanMerge) {
    std::vector<int> bits1 = {0, 0};  // A=0, B=0
    std::vector<int> bits2 = {0, 1};  // A=0, B=1
    QMCImplicant imp1(1, bits1);
    QMCImplicant imp2(2, bits2);

    int merge_pos = imp1.can_merge_with(imp2);
    EXPECT_EQ(merge_pos, 1);  // Can merge at position 1 (B)
}

TEST(QMCImplicantTest, CannotMerge) {
    std::vector<int> bits1 = {0, 0};  // A=0, B=0
    std::vector<int> bits2 = {1, 1};  // A=1, B=1
    QMCImplicant imp1(1, bits1);
    QMCImplicant imp2(3, bits2);

    int merge_pos = imp1.can_merge_with(imp2);
    EXPECT_EQ(merge_pos, -1);  // Cannot merge
}

TEST(QMCImplicantTest, MergeResults) {
    std::vector<int> bits1 = {0, 0};
    std::vector<int> bits2 = {0, 1};
    QMCImplicant imp1(1, bits1);
    QMCImplicant imp2(2, bits2);

    QMCImplicant merged = imp1.merge(imp2);

    EXPECT_EQ(merged.bits[0], 0);
    EXPECT_EQ(merged.bits[1], -1);  // Don't care
    EXPECT_EQ(merged.terms, 3);      // 1 | 2 = 3
}

// ========== Test QMCImplicant: String Representation ==========

TEST(QMCImplicantTest, ToStringSimple) {
    std::vector<int> bits = {0, 1};  // A=0, B=1
    QMCImplicant imp(0, bits);
    std::vector<std::string> vars = {"A", "B"};

    std::string s = imp.to_string(vars);
    // Expected: ~A&B (A=0 gives ~A, B=1 gives B)
    EXPECT_EQ(s, "~A&B");
}

TEST(QMCImplicantTest, ToStringWithWildcard) {
    std::vector<int> bits = {0, -1};  // A=0, B=don't care
    QMCImplicant imp(0, bits);
    std::vector<std::string> vars = {"A", "B"};

    std::string s = imp.to_string(vars);
    EXPECT_EQ(s, "~A");
}

// ========== Test QuineMcCluskey: Basic ==========

TEST(QuineMcCluskeyTest, SingleMinterm) {
    std::vector<uint64_t> minterms = {3};  // A=1, B=1
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    EXPECT_TRUE(qmc.is_minimized());
    EXPECT_EQ(qmc.get_prime_implicants().size(), 1);
}

TEST(QuineMcCluskeyTest, TwoMintermsAdjacent) {
    std::vector<uint64_t> minterms = {1, 3};  // A=1,B=0 and A=1,B=1
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    // Should combine to just A
    auto implicants = qmc.get_prime_implicants();
    EXPECT_EQ(implicants.size(), 1);
    EXPECT_EQ(implicants[0].bits[0], 1);  // A=1
    EXPECT_EQ(implicants[0].bits[1], -1);  // B is wildcard
}

TEST(QuineMcCluskeyTest, TwoMintermsNonAdjacent) {
    std::vector<uint64_t> minterms = {1, 4};  // 001 and 100
    std::vector<std::string> vars = {"A", "B", "C"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    // Cannot combine, should have two implicants
    EXPECT_EQ(qmc.get_prime_implicants().size(), 2);
}

// ========== Test QuineMcCluskey: Classic Cases ==========

TEST(QuineMcCluskeyTest, expression) {
    // (A & B) | (A & ~B) = A
    std::vector<uint64_t> minterms = {2, 3};  // A=1 (01, 11)
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();
    EXPECT_EQ(cover.size(), 1);
}

TEST(QuineMcCluskeyTest, MajorityFunction) {
    // Majority of 3 variables: 110, 101, 011, 111
    std::vector<uint64_t> minterms = {3, 5, 6, 7};
    std::vector<std::string> vars = {"A", "B", "C"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    // Should simplify to AB + AC + BC
    auto implicants = qmc.get_prime_implicants();
    EXPECT_EQ(implicants.size(), 3);
}

TEST(QuineMcCluskeyTest, XOR3) {
    // A ^ B ^ C = 001, 010, 100, 111
    std::vector<uint64_t> minterms = {1, 2, 4, 7};
    std::vector<std::string> vars = {"A", "B", "C"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    // XOR cannot be simplified
    auto implicants = qmc.get_prime_implicants();
    EXPECT_EQ(implicants.size(), 4);
}

// ========== Test QuineMcCluskey: Prime Implicant Table ==========

TEST(QuineMcCluskeyTest, EssentialPrimeImplicants) {
    // A simple case where all implicants are essential
    std::vector<uint64_t> minterms = {1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto essential = qmc.get_essential_prime_implicants();
    EXPECT_GT(essential.size(), 0);
}

TEST(QuineMcCluskeyTest, MinimumCover) {
    std::vector<uint64_t> minterms = {0, 1, 2, 5, 6, 7};
    std::vector<std::string> vars = {"A", "B", "C"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();
    EXPECT_GT(cover.size(), 0);

    // Verify all minterms are covered
    for (uint64_t m : minterms) {
        bool covered = false;
        for (const auto& imp : cover) {
            if (imp.covers(m)) {
                covered = true;
                break;
            }
        }
        EXPECT_TRUE(covered) << "Minterm " << m << " not covered";
    }
}

// ========== Test QuineMcCluskey: Edge Cases ==========

TEST(QuineMcCluskeyTest, AllMinterms) {
    // All 4 combinations for 2 variables
    std::vector<uint64_t> minterms = {0, 1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();
    // Should simplify to 1 (constant true)
    EXPECT_GT(cover.size(), 0);
}

TEST(QuineMcCluskeyTest, SingleMintermMultipleVars) {
    std::vector<uint64_t> minterms = {15};  // 1111
    std::vector<std::string> vars = {"A", "B", "C", "D"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto implicants = qmc.get_prime_implicants();
    EXPECT_EQ(implicants.size(), 1);
    // All bits should be specified (no wildcards)
    for (int b : implicants[0].bits) {
        EXPECT_NE(b, -1);
    }
}

// ========== Test QuineMcCluskey: Larger Scale ==========

TEST(QuineMcCluskeyTest, EighthMinterms8Vars) {
    std::vector<uint64_t> minterms = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<std::string> vars = {"A", "B", "C", "D", "E", "F", "G", "H"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    // Should simplify to ~A & ~B & ~C & ~D
    auto cover = qmc.get_minimum_cover();
    EXPECT_GT(cover.size(), 0);
}

} // namespace logic
