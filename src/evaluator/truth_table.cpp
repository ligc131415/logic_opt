#include "truth_table.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <functional>

namespace logic {

// 从表达式提取所有变量（去重，排序）
std::vector<std::string> extract_variables(const ExprPtr& root) {
    std::set<std::string> var_set;

    // 深度优先遍历AST
    std::function<void(const ExpressionNode&)> traverse = [&](const ExpressionNode& node) {
        if (node.is_variable()) {
            var_set.insert(node.get_var_name());
        }
        for (const auto& child : node.children) {
            traverse(*child);
        }
    };

    traverse(*root);

    // 转换为排序后的向量
    std::vector<std::string> vars(var_set.begin(), var_set.end());
    std::sort(vars.begin(), vars.end());
    return vars;
}

// 构造函数：从表达式生成真值表
TruthTable::TruthTable(const ExprPtr& root, std::vector<std::string> variables)
    : variables_(std::move(variables)) {
    // 建立变量到索引的映射
    for (size_t i = 0; i < variables_.size(); ++i) {
        var_index_[variables_[i]] = i;
    }

    size_t num_combinations = static_cast<size_t>(std::pow(2, variables_.size()));

    // 遍历所有可能的输入组合
    for (uint64_t mask = 0; mask < num_combinations; ++mask) {
        InputAssignment assign;

        // 根据掩码设置变量值
        for (size_t i = 0; i < variables_.size(); ++i) {
            assign[variables_[i]] = (mask & (1ULL << i)) != 0;
        }

        // 计算输出
        bool output = evaluate_node(*root, assign);

        entries_.emplace_back(mask, output, assign);
    }
}

// 递归求值AST
bool TruthTable::evaluate_node(const ExpressionNode& node, const InputAssignment& assign) const {
    switch (node.op) {
        case OpType::VARIABLE: {
            auto it = assign.find(node.get_var_name());
            if (it != assign.end()) {
                return it->second;
            }
            return false;  // 未找到视为false
        }

        case OpType::TRUE:
            return true;

        case OpType::FALSE:
            return false;

        case OpType::NOT: {
            if (node.children.empty()) return false;
            return !evaluate_node(*node.children[0], assign);
        }

        case OpType::AND: {
            if (node.children.empty()) return true;
            bool result = true;
            for (const auto& child : node.children) {
                result = result && evaluate_node(*child, assign);
                if (!result) break;  // 短路求值
            }
            return result;
        }

        case OpType::OR: {
            if (node.children.empty()) return false;
            bool result = false;
            for (const auto& child : node.children) {
                result = result || evaluate_node(*child, assign);
                if (result) break;  // 短路求值
            }
            return result;
        }

        case OpType::XOR: {
            if (node.children.size() < 2) return false;
            bool result = evaluate_node(*node.children[0], assign);
            for (size_t i = 1; i < node.children.size(); ++i) {
                result = result != evaluate_node(*node.children[i], assign);
            }
            return result;
        }
    }
    return false;
}

// 获取满足条件的条目（输出为true）- 即最小项
std::vector<uint64_t> TruthTable::get_minterms() const {
    std::vector<uint64_t> minterms;
    for (const auto& entry : entries_) {
        if (entry.output) {
            minterms.push_back(entry.input_mask);
        }
    }
    return minterms;
}

// 获取不满足条件的条目（输出为false）- 即最大项
std::vector<uint64_t> TruthTable::get_maxterms() const {
    std::vector<uint64_t> maxterms;
    for (const auto& entry : entries_) {
        if (!entry.output) {
            maxterms.push_back(entry.input_mask);
        }
    }
    return maxterms;
}

// 根据输入掩码查找输出
bool TruthTable::evaluate(uint64_t input_mask) const {
    for (const auto& entry : entries_) {
        if (entry.input_mask == input_mask) {
            return entry.output;
        }
    }
    return false;
}

// 打印真值表（用于调试）
void TruthTable::print() const {
    std::cout << "Truth Table (" << variables_.size() << " variables, "
              << entries_.size() << " entries):\n";

    // 打印表头
    for (const auto& var : variables_) {
        std::cout << var << " ";
    }
    std::cout << "| Output\n";
    std::cout << std::string((variables_.size() * 2) + 10, '-') << "\n";

    // 打印每一行
    for (const auto& entry : entries_) {
        // 打印输入值
        for (size_t i = 0; i < variables_.size(); ++i) {
            std::cout << ((entry.input_mask & (1ULL << i)) ? '1' : '0') << " ";
        }
        std::cout << "| " << (entry.output ? '1' : '0') << "\n";
    }
}

} // namespace logic
