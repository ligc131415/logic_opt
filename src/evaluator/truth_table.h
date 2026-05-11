#ifndef LOGIC_OPT_TRUTH_TABLE_H
#define LOGIC_OPT_TRUTH_TABLE_H

#include "expression.h"
#include <vector>
#include <map>
#include <cstdint>
#include <string>

namespace logic {

//代表一个输入组合（变量赋值）
using InputAssignment = std::map<std::string, bool>;

// 真值表条目
struct TruthTableEntry {
    uint64_t input_mask;     // 输入变量的二进制值（按变量索引）
    bool output;             // 输出值
    InputAssignment assign;  // 完整的变量赋值（用于调试）

    TruthTableEntry() : input_mask(0), output(false) {}
    TruthTableEntry(uint64_t mask, bool out, InputAssignment a)
        : input_mask(mask), output(out), assign(std::move(a)) {}
};

// 真值表类
class TruthTable {
public:
    // 构造函数：从表达式生成真值表
    TruthTable(const ExprPtr& root, std::vector<std::string> variables);

    // 获取所有变量
    const std::vector<std::string>& get_variables() const { return variables_; }

    // 获取变量数量
    size_t get_variable_count() const { return variables_.size(); }

    // 获取真值表条目数量（2^n）
    size_t size() const { return entries_.size(); }

    // 获取所有条目
    const std::vector<TruthTableEntry>& get_entries() const { return entries_; }

    // 获取满足条件的条目（输出为true）
    std::vector<uint64_t> get_minterms() const;

    // 获取不满足条件的条目（输出为false）
    std::vector<uint64_t> get_maxterms() const;

    // 根据输入掩码查找输出
    bool evaluate(uint64_t input_mask) const;

    // 打印真值表（用于调试）
    void print() const;

private:
    std::vector<std::string> variables_;
    std::map<std::string, size_t> var_index_;  // 变量名到索引的映射
    std::vector<TruthTableEntry> entries_;

    // 递归求值AST
    bool evaluate_node(const ExpressionNode& node, const InputAssignment& assign) const;
};

// 从表达式提取所有变量（去重，排序）
std::vector<std::string> extract_variables(const ExprPtr& root);

} // namespace logic

#endif // LOGIC_OPT_TRUTH_TABLE_H
