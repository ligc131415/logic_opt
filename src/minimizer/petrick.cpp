#include "petrick.h"
#include <algorithm>
#include <set>

namespace logic {

// 计算项的代价
size_t PetrickTerm::cost(const std::vector<QMCImplicant>& prime_implicants) const {
    size_t var_count = prime_implicants.empty() ? 0 : prime_implicants[0].num_variables();
    size_t literals = 0;

    for (size_t idx : implicant_indices) {
        const QMCImplicant& imp = prime_implicants[idx];
        for (size_t i = 0; i < imp.num_variables(); ++i) {
            if (imp.bits[i] != -1) {
                literals++;
            }
        }
    }

    return implicant_indices.size() + literals / var_count;
}

// 合并两个Petrick项（OR操作）
std::vector<PetrickTerm> PetrickTerm::or_with(const PetrickTerm& other) const {
    std::vector<PetrickTerm> result;
    result.push_back(*this);
    result.push_back(other);
    return result;
}

// 与另一个Petrick项合并（AND操作）
std::vector<PetrickTerm> PetrickTerm::and_with(const PetrickTerm& other) const {
    std::vector<PetrickTerm> result;
    std::set<size_t> combined;

    // 合并两个项的索引
    for (size_t idx : implicant_indices) {
        combined.insert(idx);
    }
    for (size_t idx : other.implicant_indices) {
        combined.insert(idx);
    }

    result.emplace_back();
    result.back().implicant_indices = std::vector<size_t>(combined.begin(), combined.end());
    return result;
}

// 检查是否被另一个项子集
bool PetrickTerm::is_subset_of(const PetrickTerm& other) const {
    std::set<size_t> this_set(implicant_indices.begin(), implicant_indices.end());
    std::set<size_t> other_set(other.implicant_indices.begin(), other.implicant_indices.end());
    return std::includes(other_set.begin(), other_set.end(),
                         this_set.begin(), this_set.end());
}

// PetrickMethod 构造函数
PetrickMethod::PetrickMethod(const std::vector<QMCImplicant>& prime_implicants,
                             const std::vector<uint64_t>& minterms,
                             const std::vector<std::set<size_t>>& implicant_table)
    : prime_implicants_(prime_implicants),
      minterms_(minterms),
      implicant_table_(implicant_table),
      num_minterms_(minterms.size()) {}

// 构建Petrick表达式（积之和形式）
std::vector<PetrickTerm> PetrickMethod::build_petrick_expression() {
    std::vector<PetrickTerm> product;

    // 对每个最小项，找到覆盖它的质蕴涵项
    for (size_t minterm_idx = 0; minterm_idx < num_minterms_; ++minterm_idx) {
        std::vector<size_t> covering_implicants;

        for (size_t imp_idx = 0; imp_idx < prime_implicants_.size(); ++imp_idx) {
            if (implicant_table_[imp_idx].find(minterm_idx) != implicant_table_[imp_idx].end()) {
                covering_implicants.push_back(imp_idx);
            }
        }

        // 创建和项（OR了所有覆盖该最小项的质蕴涵项）
        std::vector<PetrickTerm> sum;
        for (size_t idx : covering_implicants) {
            sum.emplace_back(idx);
        }

        // 将和项与积项相与
        if (product.empty()) {
            product = sum;
        } else {
            std::vector<PetrickTerm> new_product;
            for (const auto& p : product) {
                for (const auto& s : sum) {
                    auto combined = p.and_with(s);
                    new_product.insert(new_product.end(), combined.begin(), combined.end());
                }
            }
            product = new_product;
        }
    }

    return product;
}

// 简化Petrick表达式
std::vector<PetrickTerm> PetrickMethod::simplify_petrick_expression(
    const std::vector<PetrickTerm>& terms) {

    if (terms.empty()) return {};

    // 第一步：消除被其他项子集覆盖的项
    std::vector<PetrickTerm> simplified;

    // 按长度排序，优先处理短的项
    std::vector<PetrickTerm> sorted_terms = terms;
    std::sort(sorted_terms.begin(), sorted_terms.end(),
              [](const PetrickTerm& a, const PetrickTerm& b) {
                  return a.implicant_indices.size() < b.implicant_indices.size();
              });

    std::set<size_t> covered;

    for (const auto& term : sorted_terms) {
        bool is_covered = false;
        for (size_t idx : covered) {
            if (term.is_subset_of(simplified[idx])) {
                is_covered = true;
                break;
            }
        }
        if (!is_covered) {
            simplified.push_back(term);
            covered.insert(simplified.size() - 1);
        }
    }

    return simplified;
}

// 执行Petrick方法，返回最小覆盖的质蕴涵项索引
std::vector<size_t> PetrickMethod::find_minimum_cover() {
    auto terms = build_petrick_expression();
    terms = simplify_petrick_expression(terms);

    if (terms.empty()) return {};

    // 选择代价最小的项
    size_t best_idx = 0;
    size_t best_cost = terms[0].cost(prime_implicants_);

    for (size_t i = 1; i < terms.size(); ++i) {
        size_t c = terms[i].cost(prime_implicants_);
        if (c < best_cost) {
            best_cost = c;
            best_idx = i;
        }
    }

    return terms[best_idx].implicant_indices;
}

// 获取所有最小覆盖
std::vector<std::vector<size_t>> PetrickMethod::find_all_minimum_covers() {
    auto terms = build_petrick_expression();
    terms = simplify_petrick_expression(terms);

    if (terms.empty()) return {};

    // 找到最小代价
    size_t min_cost = terms[0].cost(prime_implicants_);
    for (const auto& term : terms) {
        size_t c = term.cost(prime_implicants_);
        if (c < min_cost) {
            min_cost = c;
        }
    }

    // 收集所有达到最小代价的项
    std::vector<std::vector<size_t>> covers;
    for (const auto& term : terms) {
        if (term.cost(prime_implicants_) == min_cost) {
            covers.push_back(term.implicant_indices);
        }
    }

    return covers;
}

} // namespace logic
