#ifndef LOGIC_OPT_QMC_H
#define LOGIC_OPT_QMC_H

#include <vector>
#include <cstdint>
#include <set>
#include <map>
#include <string>

namespace logic {

// QMCImplicant 表示一个质蕴涵项
// 使用组合方式表示：0=原始变量, 1=反变量, 2=无关项
struct QMCImplicant {
    uint64_t terms;       // 覆盖的最小项（minterm mask）
    std::vector<int> bits;  // 每个变量的值：0=原变量, 1=反变量, -1=无关
    bool is_prime;        // 是否是质蕴涵项

    QMCImplicant() : terms(0), bits(), is_prime(false) {}

    QMCImplicant(uint64_t t, const std::vector<int>& b)
        : terms(t), bits(b), is_prime(false) {}

    // 获取变量数量
    size_t num_variables() const { return bits.size(); }

    // 检查是否与另一个蕴涵项可以合并
    // 返回：-1表示不能合并，否则返回可合并的位置
    int can_merge_with(const QMCImplicant& other) const;

    // 合并两个蕴涵项
    QMCImplicant merge(const QMCImplicant& other) const;

    // 检查是否覆盖特定的最小项
    bool covers(uint64_t minterm) const;

    // 转换为字符串表示（用变量索引）
    std::string to_string(const std::vector<std::string>& var_names) const;

    // 比较运算符（用于set）
    bool operator<(const QMCImplicant& other) const;

    // 相等运算符（用于find）
    bool operator==(const QMCImplicant& other) const;
};

// Quine-McCluskey 算法实现
class QuineMcCluskey {
public:
    // 构造函数：使用最小项和变量名
    QuineMcCluskey(const std::vector<uint64_t>& minterms,
                   const std::vector<std::string>& var_names);

    // 主算法：执行化简
    void minimize();

    // 获取最简SOP的质蕴涵项
    const std::vector<QMCImplicant>& get_prime_implicants() const {
        return prime_implicants_;
    }

    // 获取关键质蕴涵项（essential prime implicants）
    std::vector<QMCImplicant> get_essential_prime_implicants() const;

    // 获取覆盖所有最小项的最小质蕴涵项集
    std::vector<QMCImplicant> get_minimum_cover() const;

    // 检查是否已化简
    bool is_minimized() const { return minimized_; }

private:
    std::vector<uint64_t> minterms_;
    std::vector<std::string> var_names_;
    size_t num_vars_;

    // 所有质蕴涵项
    std::vector<QMCImplicant> prime_implicants_;

    // 质蕴涵表：prime_implicant index -> set of minterm indices
    std::vector<std::set<size_t>> implicant_table_;

    bool minimized_;

    // 生成初始蕴涵项（从最小项）
    std::vector<QMCImplicant> generate_initial_implicants();

    // 迭代合并以找到所有质蕴涵项
    std::vector<QMCImplicant> find_prime_implicants();

    // 检查一个蕴涵项是否是质蕴涵项
    bool is_prime_implicant(const QMCImplicant& imp) const;
};

} // namespace logic

#endif // LOGIC_OPT_QMC_H
