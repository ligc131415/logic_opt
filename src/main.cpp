#include "parser/lexer.h"
#include "parser/parser.h"
#include "evaluator/truth_table.h"
#include "minimizer/qmc.h"
#include "minimizer/petrick.h"
#include "minimizer/output.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// 解析表达式字符串
static logic::ExprPtr parse_expression(const std::string& input) {
    logic::Parser parser(input);
    return parser.parse();
}

// 提取变量并生成真值表
static logic::TruthTable generate_truth_table(const logic::ExprPtr& root) {
    auto variables = logic::extract_variables(root);
    return logic::TruthTable(root, variables);
}

// 主流程
static void process_expression(const std::string& input) {
    std::cout << "Input expression: " << input << "\n\n";

    // 解析表达式
    auto root = parse_expression(input);

    // 生成真值表
    auto table = generate_truth_table(root);

    // 获取最小项和最大项
    auto minterms = table.get_minterms();
    auto maxterms = table.get_maxterms();

    // 打印变量信息
    std::cout << "Variables: ";
    const auto& vars = table.get_variables();
    for (size_t i = 0; i < vars.size(); ++i) {
        std::cout << vars[i];
        if (i < vars.size() - 1) std::cout << ", ";
    }
    std::cout << "\n\n";

    // 打印真值表
    std::cout << "Truth Table:\n";
    table.print();
    std::cout << "\n";

    // 打印最小项和最大项
    std::cout << "Minterms (Σm): ";
    if (minterms.empty()) {
        std::cout << "none";
    } else {
        for (size_t i = 0; i < minterms.size(); ++i) {
            std::cout << minterms[i];
            if (i < minterms.size() - 1) std::cout << ",";
        }
    }
    std::cout << "\n";

    std::cout << "Maxterms (ΠM): ";
    if (maxterms.empty()) {
        std::cout << "none";
    } else {
        for (size_t i = 0; i < maxterms.size(); ++i) {
            std::cout << maxterms[i];
            if (i < maxterms.size() - 1) std::cout << ",";
        }
    }
    std::cout << "\n\n";

    // 使用Quine-McCluskey化简
    std::cout << "Quine-McCluskey Minimization:\n";
    std::cout << "============================\n\n";

    if (minterms.empty()) {
        std::cout << "Simplified SOP: 0\n";
        std::cout << "Simplified POS: " << (vars.empty() ? "1" : "0") << "\n";
    } else if (minterms.size() == (1ULL << vars.size())) {
        std::cout << "Simplified SOP: 1\n";
        std::cout << "Simplified POS: 1\n";
    } else {
        // SOP化简
        logic::QuineMcCluskey qmc_sop(minterms, vars);
        qmc_sop.minimize();

        auto prime_implicants = qmc_sop.get_prime_implicants();
        auto essential = qmc_sop.get_essential_prime_implicants();
        auto cover = qmc_sop.get_minimum_cover();

        std::cout << "Prime Implicants (" << prime_implicants.size() << "):\n";
        for (const auto& imp : prime_implicants) {
            std::cout << "  " << imp.to_string(vars) << "\n";
        }

        std::cout << "\nEssential Prime Implicants (" << essential.size() << "):\n";
        for (const auto& imp : essential) {
            std::cout << "  " << imp.to_string(vars) << "\n";
        }

        std::cout << "\nMinimum Cover (" << cover.size() << " terms):\n";
        std::string sop = logic::OutputFormatter::format_implicants_as_sop(cover, vars);
        std::cout << "  " << sop << "\n";

        // POS化简（使用最大项）
        if (!maxterms.empty()) {
            logic::QuineMcCluskey qmc_pos(maxterms, vars);
            qmc_pos.minimize();

            auto cover_pos = qmc_pos.get_minimum_cover();
            std::string pos = logic::OutputFormatter::format_implicants_as_pos(cover_pos, vars);
            std::cout << "\nPOS form:\n  " << pos << "\n";
        }
    }
}

// 显示帮助
static void show_help() {
    std::cout << "Digital Logic Simplifier\n";
    std::cout << "========================\n\n";
    std::cout << "Usage: logic_opt [expression]\n\n";
    std::cout << "Operators:\n";
    std::cout << "  &   - Logical AND\n";
    std::cout << "  |   - Logical OR\n";
    std::cout << "  ~   - Logical NOT\n";
    std::cout << "  ^   - Logical XOR\n";
    std::cout << "  ()  - Parentheses for grouping\n\n";
    std::cout << "Examples:\n";
    std::cout << "  logic_opt \"(A & B) | (A & ~B)\"\n";
    std::cout << "  logic_opt \"(A | B) & (~A | C)\"\n";
    std::cout << "  logic_opt \"A ^ B ^ C\"\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string input;
    if (argc == 2) {
        input = argv[1];
    }

    // 移除空格
    input.erase(std::remove(input.begin(), input.end(), ' '), input.end());

    try {
        process_expression(input);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
