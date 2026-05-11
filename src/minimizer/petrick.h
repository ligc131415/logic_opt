#ifndef LOGIC_OPT_PETRICK_H
#define LOGIC_OPT_PETRICK_H

#include "qmc.h"
#include <vector>
#include <string>
#include <set>

namespace logic {

// Petrick's Method 实现
// 用于在质蕴涵表中找到最小覆盖

// Petrick项表示一个乘积项（多个质蕴涵项的AND）
struct PetrickTerm {
    std::vector<size_t> implicant_indices;  // 质蕴涵项索引列表

    PetrickTerm() = default;
    explicit PetrickTerm(size_t idx) { implicant_indices.push_back(idx); }

    // 计算项的代价（质蕴涵项数量和无关项数量）
    size_t cost(const std::vector<QMCImplicant>& prime_implicants) const;

    // 合并两个Petrick项（OR操作）
    std::vector<PetrickTerm> or_with(const PetrickTerm& other) const;

    // 与另一个Petrick项合并（AND操作）
    std::vector<PetrickTerm> and_with(const PetrickTerm& other) const;

    // 检查是否被另一个项子集
    bool is_subset_of(const PetrickTerm& other) const;
};

// Petrick方法类
class PetrickMethod {
public:
    // 构造函数
    // prime_implicants: 所有质蕴涵项
    // minterms: 最小项列表
    // implicant_table: 质蕴涵表，implicant_table[i] = 覆盖的最小项索引集合
    PetrickMethod(const std::vector<QMCImplicant>& prime_implicants,
                  const std::vector<uint64_t>& minterms,
                  const std::vector<std::set<size_t>>& implicant_table);

    // 执行Petrick方法，返回最小覆盖的质蕴涵项索引
    std::vector<size_t> find_minimum_cover();

    // 获取所有最小覆盖
    std::vector<std::vector<size_t>> find_all_minimum_covers();

private:
    const std::vector<QMCImplicant>& prime_implicants_;
    const std::vector<uint64_t>& minterms_;
    const std::vector<std::set<size_t>>& implicant_table_;
    size_t num_minterms_;

    // 构建Petrick表达式（积之和形式）
    std::vector<PetrickTerm> build_petrick_expression();

    // 简化Petrick表达式
    std::vector<PetrickTerm> simplify_petrick_expression(
        const std::vector<PetrickTerm>& terms);
};

} // namespace logic

#endif // LOGIC_OPT_PETRICK_H
