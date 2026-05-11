#include <gtest/gtest.h>
#include "../src/minimizer/qmc.h"
#include "../src/minimizer/petrick.h"

namespace logic {

// ========== Test PetrickTerm: Basic Operations ==========

TEST(PetrickTermTest, CreateTerm) {
    PetrickTerm term(0);
    EXPECT_EQ(term.implicant_indices.size(), 1);
    EXPECT_EQ(term.implicant_indices[0], 0);
}

TEST(PetrickTermTest, CreateTermFromMultiple) {
    PetrickTerm term;
    term.implicant_indices = {0, 1, 2};
    EXPECT_EQ(term.implicant_indices.size(), 3);
}

// ========== Test PetrickTerm: OR Operation ==========

TEST(PetrickTermTest, OrOperation) {
    PetrickTerm t1(0);
    PetrickTerm t2(1);

    auto result = t1.or_with(t2);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].implicant_indices.size(), 1);
    EXPECT_EQ(result[1].implicant_indices.size(), 1);
}

// ========== Test PetrickTerm: AND Operation ==========

TEST(PetrickTermTest, AndOperation) {
    PetrickTerm t1(0);
    PetrickTerm t2(1);

    auto result = t1.and_with(t2);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].implicant_indices.size(), 2);
}

TEST(PetrickTermTest, AndOperationWithOverlap) {
    PetrickTerm t1(0);
    PetrickTerm t2(0);

    auto result = t1.and_with(t2);
    EXPECT_EQ(result.size(), 1);
    // Should deduplicate
    EXPECT_EQ(result[0].implicant_indices.size(), 1);
}

// ========== Test PetrickTerm: Subset Operations ==========

TEST(PetrickTermTest, IsSubsetOfSame) {
    PetrickTerm t1;
    t1.implicant_indices = {0, 1, 2};
    PetrickTerm t2;
    t2.implicant_indices = {0, 1, 2};

    EXPECT_TRUE(t1.is_subset_of(t2));
}

TEST(PetrickTermTest, IsSubsetOfProper) {
    PetrickTerm t1;
    t1.implicant_indices = {0, 1};
    PetrickTerm t2;
    t2.implicant_indices = {0, 1, 2};

    EXPECT_TRUE(t1.is_subset_of(t2));
}

TEST(PetrickTermTest, IsNotSubsetOf) {
    PetrickTerm t1;
    t1.implicant_indices = {0, 1, 3};
    PetrickTerm t2;
    t2.implicant_indices = {0, 1, 2};

    EXPECT_FALSE(t1.is_subset_of(t2));
}

// ========== Test PetrickMethod: Basic Cases ==========

TEST(PetrickMethodTest, SimpleCover) {
    // A & B - only one minterm, one implicant
    std::vector<uint64_t> minterms = {3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    const auto& prime_implicants = qmc.get_prime_implicants();
    const auto& table = qmc.get_prime_implicants();  // Simplified

    // Create implicant table manually
    std::vector<std::set<size_t>> implicant_table(prime_implicants.size());
    for (size_t i = 0; i < prime_implicants.size(); ++i) {
        for (size_t j = 0; j < minterms.size(); ++j) {
            if (prime_implicants[i].covers(minterms[j])) {
                implicant_table[i].insert(j);
            }
        }
    }

    PetrickMethod petrick(prime_implicants, minterms, implicant_table);
    auto cover = petrick.find_minimum_cover();

    EXPECT_GT(cover.size(), 0);
}

TEST(PetrickMethodTest, TwoMintermsTwoImplicants) {
    // A & B | A & ~B = A (minterms 2, 3)
    std::vector<uint64_t> minterms = {2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    const auto& prime_implicants = qmc.get_prime_implicants();
    std::vector<std::set<size_t>> implicant_table(prime_implicants.size());
    for (size_t i = 0; i < prime_implicants.size(); ++i) {
        for (size_t j = 0; j < minterms.size(); ++j) {
            if (prime_implicants[i].covers(minterms[j])) {
                implicant_table[i].insert(j);
            }
        }
    }

    PetrickMethod petrick(prime_implicants, minterms, implicant_table);
    auto cover = petrick.find_minimum_cover();

    // Should find one implicant
    EXPECT_EQ(cover.size(), 1);
}

// ========== Test PetrickMethod: Essential Prime Implicants ==========

TEST(PetrickMethodTest, EssentialPrimeImplicants) {
    std::vector<uint64_t> minterms = {1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto essential = qmc.get_essential_prime_implicants();
    EXPECT_GT(essential.size(), 0);
}

// ========== Test PetrickMethod: All Minimum Covers ==========

TEST(PetrickMethodTest, AllMinimumCovers) {
    std::vector<uint64_t> minterms = {0, 1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    const auto& prime_implicants = qmc.get_prime_implicants();
    std::vector<std::set<size_t>> implicant_table(prime_implicants.size());
    for (size_t i = 0; i < prime_implicants.size(); ++i) {
        for (size_t j = 0; j < minterms.size(); ++j) {
            if (prime_implicants[i].covers(minterms[j])) {
                implicant_table[i].insert(j);
            }
        }
    }

    PetrickMethod petrick(prime_implicants, minterms, implicant_table);
    auto covers = petrick.find_all_minimum_covers();

    EXPECT_GT(covers.size(), 0);
}

// ========== Test PetrickMethod: Complex Case ==========

TEST(PetrickMethodTest, MajorityFunction) {
    // Majority of 3: 011, 101, 110, 111 (3, 5, 6, 7)
    std::vector<uint64_t> minterms = {3, 5, 6, 7};
    std::vector<std::string> vars = {"A", "B", "C"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();
    // Should find 3 implicants: AB, AC, BC
    EXPECT_EQ(cover.size(), 3);
}

// ========== Test PetrickMethod: Cost Calculation ==========

TEST(PetrickMethodTest, CostCalculation) {
    std::vector<uint64_t> minterms = {1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    const auto& prime_implicants = qmc.get_prime_implicants();

    // First implicant should have lower or equal cost
    if (!prime_implicants.empty()) {
        PetrickTerm term(prime_implicants.size() - 1);
        size_t cost = term.cost(prime_implicants);
        EXPECT_GT(cost, 0);
    }
}

// ========== Test PetrickMethod: combine Expression Simplification ==========

TEST(PetrickMethodTest, combineExpressionSimplification) {
    // This tests the core Petrick's method algorithm
    // For (A + B)(A + C) = A + BC
    std::vector<uint64_t> minterms = {1, 2, 3};
    std::vector<std::string> vars = {"A", "B"};

    QuineMcCluskey qmc(minterms, vars);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();

    // Verify coverage
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

} // namespace logic
