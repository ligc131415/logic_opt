#ifndef LOGIC_OPT_OUTPUT_H
#define LOGIC_OPT_OUTPUT_H

#include "qmc.h"
#include "truth_table.h"
#include <string>
#include <vector>

namespace logic {

// 输出格式化类
class OutputFormatter {
public:
    // 格式化SOP表达式
    static std::string format_sop(const std::vector<uint64_t>& minterms,
                                  const std::vector<std::string>& var_names);

    // 格式化POS表达式
    static std::string format_pos(const std::vector<uint64_t>& maxterms,
                                  const std::vector<std::string>& var_names);

    // 从最小项获取SOP（使用Quine-McCluskey）
    static std::string get_sop_from_minterms(const std::vector<uint64_t>& minterms,
                                             const std::vector<std::string>& var_names);

    // 从最大项获取POS（使用Quine-McCluskey）
    static std::string get_pos_from_maxterms(const std::vector<uint64_t>& maxterms,
                                             const std::vector<std::string>& var_names);

    // 格式化质蕴涵项为SOP
    static std::string format_implicants_as_sop(
        const std::vector<QMCImplicant>& implicants,
        const std::vector<std::string>& var_names);

    // 格式化质蕴涵项为POS
    static std::string format_implicants_as_pos(
        const std::vector<QMCImplicant>& implicants,
        const std::vector<std::string>& var_names);

    // 打印完整的化简结果
    static void print_minimization_result(
        const std::vector<uint64_t>& minterms,
        const std::vector<uint64_t>& maxterms,
        const std::vector<std::string>& var_names);

    // 获取变量的字母名称（public for testing）
    static std::string get_variable_name(size_t index);

    // 将单个乘积项转换为字符串（public for testing）
    static std::string format_product_term(const std::vector<int>& bits,
                                           const std::vector<std::string>& var_names,
                                           bool positive_logic);
};

} // namespace logic

#endif // LOGIC_OPT_OUTPUT_H
