#include "output.h"
#include <algorithm>
#include <sstream>
#include <set>

namespace logic {

// 将单个乘积项转换为字符串
std::string OutputFormatter::format_product_term(const std::vector<int>& bits,
                                                 const std::vector<std::string>& var_names,
                                                 bool positive_logic) {
    std::stringstream ss;

    bool first = true;
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == -1) continue;  // 跳过无关项

        // 对于POS，反转逻辑
        int value = positive_logic ? bits[i] : (bits[i] == 0 ? 1 : 0);

        if (!first) {
            ss << "&";
        }
        first = false;

        const std::string& var = var_names.empty() ? get_variable_name(i) : var_names[i];

        // value=1 表示变量为1（原变量），value=0 表示变量为0（非）
        if (value == 1) {
            ss << var;
        } else {
            ss << "~" << var;
        }
    }

    // 如果全部是无关项，返回"1"（对于SOP）或"0"（对于POS）
    if (ss.str().empty()) {
        ss << (positive_logic ? "1" : "0");
    }

    return ss.str();
}

// 获取变量的字母名称
std::string OutputFormatter::get_variable_name(size_t index) {
    const std::string vars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (index < vars.size()) {
        return std::string(1, vars[index]);
    }
    return "X" + std::to_string(index);
}

// 格式化SOP表达式
std::string OutputFormatter::format_sop(const std::vector<uint64_t>& minterms,
                                        const std::vector<std::string>& var_names) {
    if (minterms.empty()) {
        return "0";
    }

    // 如果所有最小项都覆盖了，返回"1"
    size_t total = 1ULL << var_names.size();
    if (minterms.size() == total) {
        return "1";
    }

    std::stringstream ss;
    bool first = true;

    for (uint64_t minterm : minterms) {
        std::vector<int> bits(var_names.size(), -1);
        for (size_t i = 0; i < var_names.size(); ++i) {
            if (minterm & (1ULL << i)) {
                bits[i] = 1;
            } else {
                bits[i] = 0;
            }
        }

        if (!first) {
            ss << " | ";
        }
        first = false;

        ss << format_product_term(bits, var_names, true);
    }

    return ss.str();
}

// 格式化POS表达式
std::string OutputFormatter::format_pos(const std::vector<uint64_t>& maxterms,
                                        const std::vector<std::string>& var_names) {
    if (maxterms.empty()) {
        return "1";
    }

    // 如果所有最大项都覆盖了，返回"0"
    size_t total = 1ULL << var_names.size();
    if (maxterms.size() == total) {
        return "0";
    }

    std::stringstream ss;
    bool first = true;

    for (uint64_t maxterm : maxterms) {
        std::vector<int> bits(var_names.size(), -1);
        for (size_t i = 0; i < var_names.size(); ++i) {
            if (maxterm & (1ULL << i)) {
                bits[i] = 1;
            } else {
                bits[i] = 0;
            }
        }

        if (!first) {
            ss << " & ";
        }
        first = false;

        ss << "(" << format_product_term(bits, var_names, false) << ")";
    }

    return ss.str();
}

// 从最小项获取SOP（使用Quine-McCluskey）
std::string OutputFormatter::get_sop_from_minterms(const std::vector<uint64_t>& minterms,
                                                   const std::vector<std::string>& var_names) {
    if (minterms.empty()) {
        return "0";
    }

    QuineMcCluskey qmc(minterms, var_names);
    qmc.minimize();

    auto cover = qmc.get_minimum_cover();
    std::vector<QMCImplicant> selected = cover;

    return format_implicants_as_sop(selected, var_names);
}

// 从最大项获取POS（使用Quine-McCluskey）
std::string OutputFormatter::get_pos_from_maxterms(const std::vector<uint64_t>& maxterms,
                                                   const std::vector<std::string>& var_names) {
    if (maxterms.empty()) {
        return "1";
    }

    // 对于POS，我们直接使用最大项
    std::string result = format_pos(maxterms, var_names);

    // 如果结果被括号包围且是一个单独的术语，移除括号
    if (result.size() >= 2 && result.front() == '(' && result.back() == ')') {
        // 检查是否只有一个术语（没有&或|在括号内）
        size_t inner_start = 1;
        size_t inner_end = result.size() - 2;
        bool has_operator = false;
        for (size_t i = inner_start; i <= inner_end; ++i) {
            if (result[i] == '&' || result[i] == '|') {
                has_operator = true;
                break;
            }
        }
        if (!has_operator) {
            return result.substr(1, result.size() - 2);
        }
    }

    return result;
}

// 格式化质蕴涵项为SOP
std::string OutputFormatter::format_implicants_as_sop(
    const std::vector<QMCImplicant>& implicants,
    const std::vector<std::string>& var_names) {

    if (implicants.empty()) {
        return "0";
    }

    std::stringstream ss;
    bool first = true;

    for (const auto& imp : implicants) {
        if (!first) {
            ss << " | ";
        }
        first = false;

        ss << imp.to_string(var_names);
    }

    return ss.str();
}

// 格式化质蕴涵项为POS
// 输入是 f'（补函数）的质蕴涵项，输出为 f 的 POS 形式
// 每个质蕴涵项代表 f' 的一个乘积项，通过 De Morgan 变换转为 f 的和项：
//   f' 乘积项 ~A·B  →  f 和项 (A+~B)
//   极性反转：bit=0→var, bit=1→~var
//   连接符：乘积项用 & 连接（SOP 用 |）
std::string OutputFormatter::format_implicants_as_pos(
    const std::vector<QMCImplicant>& implicants,
    const std::vector<std::string>& var_names) {

    if (implicants.empty()) {
        return "1";
    }

    std::stringstream ss;
    bool first = true;

    for (const auto& imp : implicants) {
        if (!first) {
            ss << " & ";
        }
        first = false;

        // 将质蕴涵项（f' 的乘积项）格式化为 f 的和项
        std::string term;
        bool first_lit = true;
        for (size_t i = 0; i < imp.bits.size(); ++i) {
            if (imp.bits[i] == -1) continue;
            if (!first_lit) term += "|";
            first_lit = false;
            // POS 极性反转：bit=0 → 原变量, bit=1 → 反变量
            if (imp.bits[i] == 0) {
                term += var_names[i];
            } else {
                term += "~" + var_names[i];
            }
        }

        if (term.empty()) {
            // 全部无关项：f'=1 → f=0
            term = "0";
        }

        ss << "(" << term << ")";
    }

    return ss.str();
}

// 打印完整的化简结果
void OutputFormatter::print_minimization_result(
    const std::vector<uint64_t>& minterms,
    const std::vector<uint64_t>& maxterms,
    const std::vector<std::string>& var_names) {

    std::cout << "Minimization Results:\n";
    std::cout << "=====================\n\n";

    std::cout << "Variables: ";
    for (const auto& var : var_names) {
        std::cout << var << " ";
    }
    std::cout << "\n\n";

    // 原始形式
    std::cout << "Original SOP (minterms): ";
    if (minterms.empty()) {
        std::cout << "0";
    } else {
        std::cout << "Σm(";
        for (size_t i = 0; i < minterms.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << minterms[i];
        }
        std::cout << ")";
    }
    std::cout << "\n";

    std::cout << "Original POS (maxterms): ";
    if (maxterms.empty()) {
        std::cout << "1";
    } else {
        std::cout << "ΠM(";
        for (size_t i = 0; i < maxterms.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << maxterms[i];
        }
        std::cout << ")";
    }
    std::cout << "\n\n";

    // 最简形式
    std::string sop = get_sop_from_minterms(minterms, var_names);
    std::string pos = get_pos_from_maxterms(maxterms, var_names);

    std::cout << "Simplified SOP:\n  " << sop << "\n\n";
    std::cout << "Simplified POS:\n  " << pos << "\n";
}

} // namespace logic
