#include "qmc.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

namespace logic {

// 检查是否与另一个蕴涵项可以合并
int QMCImplicant::can_merge_with(const QMCImplicant& other) const {
    if (bits.size() != other.bits.size()) return -1;

    int diff_count = 0;
    int diff_pos = -1;

    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != other.bits[i]) {
            // 两个都必须是确定值（非-1）且不同，才能算作差异
            if (bits[i] != -1 && other.bits[i] != -1) {
                diff_count++;
                diff_pos = static_cast<int>(i);
            } else if (bits[i] == -1 && other.bits[i] == -1) {
                // 都是无关项，不算差异
            } else {
                // 一个是-1，另一个不是 - 不能合并
                return -1;
            }
        }
    }

    if (diff_count == 1) {
        return diff_pos;
    }
    return -1;
}

// 合并两个蕴涵项
QMCImplicant QMCImplicant::merge(const QMCImplicant& other) const {
    QMCImplicant result;
    result.bits = bits;
    result.terms = terms | other.terms;

    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != other.bits[i]) {
            result.bits[i] = -1;  // 无关项
        }
    }

    return result;
}

// 检查是否覆盖特定的最小项
bool QMCImplicant::covers(uint64_t minterm) const {
    for (size_t i = 0; i < bits.size(); ++i) {
        int bit_val = (minterm >> i) & 1;
        if (bits[i] != -1 && bits[i] != static_cast<int>(bit_val)) {
            return false;
        }
    }
    return true;
}

// 转换为字符串表示
std::string QMCImplicant::to_string(const std::vector<std::string>& var_names) const {
    std::string result;
    bool first = true;

    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == -1) continue;  // 跳过无关项

        if (!first) {
            result += "&";
        }
        first = false;

        const std::string& var = var_names[i];
        // bits[i]=1 表示变量为1（原变量），bits[i]=0 表示变量为0（非）
        if (bits[i] == 1) {
            result += var;
        } else {
            result += "~" + var;
        }
    }

    // 如果全部是无关项（应该不会发生），返回1
    if (result.empty()) {
        result = "1";
    }

    return result;
}

// 比较运算符
bool QMCImplicant::operator<(const QMCImplicant& other) const {
    if (bits.size() != other.bits.size()) {
        return bits.size() < other.bits.size();
    }
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != other.bits[i]) {
            return bits[i] < other.bits[i];
        }
    }
    return terms < other.terms;
}

// 相等运算符
bool QMCImplicant::operator==(const QMCImplicant& other) const {
    if (bits.size() != other.bits.size() || terms != other.terms) {
        return false;
    }
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != other.bits[i]) {
            return false;
        }
    }
    return true;
}

// 生成初始蕴涵项（从最小项）
std::vector<QMCImplicant> QuineMcCluskey::generate_initial_implicants() {
    std::vector<QMCImplicant> implicants;

    for (uint64_t minterm : minterms_) {
        std::vector<int> bits(num_vars_, -1);  // 初始都是无关项
        for (size_t i = 0; i < num_vars_; ++i) {
            if (minterm & (1ULL << i)) {
                bits[i] = 1;
            } else {
                bits[i] = 0;
            }
        }
        implicants.emplace_back(minterm, bits);
    }

    return implicants;
}

// 迭代合并以找到所有质蕴涵项
std::vector<QMCImplicant> QuineMcCluskey::find_prime_implicants() {
    // 当前轮次的蕴涵项
    std::vector<QMCImplicant> current = generate_initial_implicants();

    // 标记哪些被合并过
    std::set<uint64_t> merged_mask;

    // 用于存储所有质蕴涵项
    std::set<QMCImplicant> all_prime_implicants;

    while (!current.empty()) {
        std::vector<QMCImplicant> next;
        std::vector<bool> used(current.size(), false);

        // 尝试合并每对蕴涵项
        for (size_t i = 0; i < current.size(); ++i) {
            for (size_t j = i + 1; j < current.size(); ++j) {
                int merge_pos = current[i].can_merge_with(current[j]);
                if (merge_pos != -1) {
                    used[i] = true;
                    used[j] = true;
                    merged_mask.insert(current[i].terms);
                    merged_mask.insert(current[j].terms);

                    QMCImplicant merged = current[i].merge(current[j]);
                    // 检查是否已存在
                    if (std::find(next.begin(), next.end(), merged) == next.end()) {
                        next.push_back(merged);
                    }
                }
            }
        }

        // 收集未被合并的（质蕴涵项）
        for (size_t i = 0; i < current.size(); ++i) {
            if (!used[i]) {
                all_prime_implicants.insert(current[i]);
            }
        }

        current = next;
    }

    return std::vector<QMCImplicant>(all_prime_implicants.begin(), all_prime_implicants.end());
}

// Quine-McCluskey 构造函数
QuineMcCluskey::QuineMcCluskey(const std::vector<uint64_t>& minterms,
                               const std::vector<std::string>& var_names)
    : minterms_(minterms), var_names_(var_names), num_vars_(var_names.size()), minimized_(false) {
    // 对最小项排序并去重
    std::set<uint64_t> unique_terms(minterms_.begin(), minterms_.end());
    minterms_ = std::vector<uint64_t>(unique_terms.begin(), unique_terms.end());
}

// 主算法：执行化简
void QuineMcCluskey::minimize() {
    if (minimized_) return;

    // 找到所有质蕴涵项
    prime_implicants_ = find_prime_implicants();

    // 构建质蕴涵表
    implicant_table_.resize(prime_implicants_.size());
    for (size_t i = 0; i < prime_implicants_.size(); ++i) {
        for (size_t j = 0; j < minterms_.size(); ++j) {
            if (prime_implicants_[i].covers(minterms_[j])) {
                implicant_table_[i].insert(j);
            }
        }
    }

    minimized_ = true;
}

// 获取关键质蕴涵项（essential prime implicants）
std::vector<QMCImplicant> QuineMcCluskey::get_essential_prime_implicants() const {
    std::vector<QMCImplicant> essential;

    if (!minimized_) return essential;

    // 对每个最小项，检查有多少个质蕴涵项覆盖它
    std::map<size_t, std::vector<size_t>> minterm_to_implicant;

    for (size_t i = 0; i < prime_implicants_.size(); ++i) {
        for (size_t minterm_idx : implicant_table_[i]) {
            minterm_to_implicant[minterm_idx].push_back(i);
        }
    }

    // 找到只被一个质蕴涵项覆盖的最小项
    std::set<size_t> essential_indices;
    for (const auto& pair : minterm_to_implicant) {
        if (pair.second.size() == 1) {
            essential_indices.insert(pair.second[0]);
        }
    }

    // 添加这些关键质蕴涵项
    for (size_t idx : essential_indices) {
        essential.push_back(prime_implicants_[idx]);
    }

    return essential;
}

// 获取覆盖所有最小项的最小质蕴涵项集
std::vector<QMCImplicant> QuineMcCluskey::get_minimum_cover() const {
    if (!minimized_) return {};

    // 首先找出所有关键质蕴涵项
    std::set<size_t> essential_indices;
    std::map<size_t, std::vector<size_t>> minterm_to_implicant;

    for (size_t i = 0; i < prime_implicants_.size(); ++i) {
        for (size_t minterm_idx : implicant_table_[i]) {
            minterm_to_implicant[minterm_idx].push_back(i);
        }
    }

    // 找到只被一个质蕴涵项覆盖的最小项
    for (const auto& pair : minterm_to_implicant) {
        if (pair.second.size() == 1) {
            essential_indices.insert(pair.second[0]);
        }
    }

    // 使用索引追踪已选择的质蕴涵项
    std::vector<size_t> cover_indices;
    std::set<size_t> covered_minterms;
    std::vector<bool> used(prime_implicants_.size(), false);

    // 添加所有关键质蕴涵项
    for (size_t idx : essential_indices) {
        cover_indices.push_back(idx);
        used[idx] = true;
        for (size_t minterm_idx : implicant_table_[idx]) {
            covered_minterms.insert(minterm_idx);
        }
    }

    // 使用贪心算法选择剩余的质蕴涵项
    while (covered_minterms.size() < minterms_.size()) {
        size_t best_idx = 0;
        size_t max_coverage = 0;

        for (size_t i = 0; i < prime_implicants_.size(); ++i) {
            if (used[i]) continue;

            size_t coverage = 0;
            for (size_t minterm_idx : implicant_table_[i]) {
                if (covered_minterms.find(minterm_idx) == covered_minterms.end()) {
                    coverage++;
                }
            }

            if (coverage > max_coverage) {
                max_coverage = coverage;
                best_idx = i;
            }
        }

        // 添加最佳选择
        cover_indices.push_back(best_idx);
        used[best_idx] = true;
        for (size_t minterm_idx : implicant_table_[best_idx]) {
            covered_minterms.insert(minterm_idx);
        }
    }

    // 转换为 QMCImplicant 向量
    std::vector<QMCImplicant> cover;
    for (size_t idx : cover_indices) {
        cover.push_back(prime_implicants_[idx]);
    }

    return cover;
}

} // namespace logic
