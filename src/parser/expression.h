#ifndef LOGIC_OPT_EXPRESSION_H
#define LOGIC_OPT_EXPRESSION_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace logic {

// Operation types for AST nodes
enum class OpType {
    VARIABLE,   // A variable (e.g., A, B, C1)
    AND,        // Logical AND (&)
    OR,         // Logical OR (|)
    NOT,        // Logical NOT (~)
    XOR,        // Logical XOR (^)
    TRUE,       // Constant true
    FALSE       // Constant false
};

// Forward declaration
struct ExpressionNode;

// Type alias for node pointer
using ExprPtr = std::unique_ptr<ExpressionNode>;

// Structure representing a node in the expression AST
struct ExpressionNode {
    OpType op;
    std::string var_name;  // Used for VARIABLE nodes
    std::vector<ExprPtr> children;

    // Constructor for leaf nodes (VARIABLE, TRUE, FALSE)
    ExpressionNode(OpType operation, std::string name = "")
        : op(operation), var_name(std::move(name)) {}

    // Constructor for unary operators (NOT)
    ExpressionNode(OpType operation, ExprPtr child)
        : op(operation) {
        children.push_back(std::move(child));
    }

    // Constructor for binary operators (AND, OR, XOR)
    ExpressionNode(OpType operation, ExprPtr left, ExprPtr right)
        : op(operation) {
        children.push_back(std::move(left));
        children.push_back(std::move(right));
    }

    // Check if node is a leaf
    bool is_leaf() const {
        return op == OpType::VARIABLE || op == OpType::TRUE || op == OpType::FALSE;
    }

    // Check if node is a variable
    bool is_variable() const {
        return op == OpType::VARIABLE;
    }

    // Check if node is a constant
    bool is_constant() const {
        return op == OpType::TRUE || op == OpType::FALSE;
    }

    // Get variable name (valid only for VARIABLE nodes)
    const std::string& get_var_name() const {
        return var_name;
    }

    // Recursive printing for debugging
    std::string to_string(int indent = 0) const {
        std::string pad(indent * 2, ' ');
        std::string result;

        switch (op) {
            case OpType::VARIABLE:
                result = pad + "VARIABLE: " + var_name;
                break;
            case OpType::TRUE:
                result = pad + "TRUE";
                break;
            case OpType::FALSE:
                result = pad + "FALSE";
                break;
            case OpType::NOT:
                result = pad + "NOT\n";
                if (!children.empty()) {
                    result += children[0]->to_string(indent + 1);
                }
                break;
            case OpType::AND:
                result = pad + "AND\n";
                for (size_t i = 0; i < children.size(); ++i) {
                    result += children[i]->to_string(indent + 1);
                    if (i < children.size() - 1) result += "\n";
                }
                break;
            case OpType::OR:
                result = pad + "OR\n";
                for (size_t i = 0; i < children.size(); ++i) {
                    result += children[i]->to_string(indent + 1);
                    if (i < children.size() - 1) result += "\n";
                }
                break;
            case OpType::XOR:
                result = pad + "XOR\n";
                for (size_t i = 0; i < children.size(); ++i) {
                    result += children[i]->to_string(indent + 1);
                    if (i < children.size() - 1) result += "\n";
                }
                break;
        }

        return result;
    }
};

// Helper function to create a variable node
inline ExprPtr make_var(std::string name) {
    return std::make_unique<ExpressionNode>(OpType::VARIABLE, std::move(name));
}

// Helper function to create a constant true node
inline ExprPtr make_true() {
    return std::make_unique<ExpressionNode>(OpType::TRUE);
}

// Helper function to create a constant false node
inline ExprPtr make_false() {
    return std::make_unique<ExpressionNode>(OpType::FALSE);
}

// Helper function to create a NOT node
inline ExprPtr make_not(ExprPtr child) {
    return std::make_unique<ExpressionNode>(OpType::NOT, std::move(child));
}

// Helper function to create an AND node
inline ExprPtr make_and(ExprPtr left, ExprPtr right) {
    return std::make_unique<ExpressionNode>(OpType::AND, std::move(left), std::move(right));
}

// Helper function to create an OR node
inline ExprPtr make_or(ExprPtr left, ExprPtr right) {
    return std::make_unique<ExpressionNode>(OpType::OR, std::move(left), std::move(right));
}

// Helper function to create an XOR node
inline ExprPtr make_xor(ExprPtr left, ExprPtr right) {
    return std::make_unique<ExpressionNode>(OpType::XOR, std::move(left), std::move(right));
}

// Operator precedence levels
inline int get_precedence(OpType op) {
    switch (op) {
        case OpType::NOT:    return 4;
        case OpType::AND:    return 3;
        case OpType::XOR:    return 2;
        case OpType::OR:     return 1;
        default:             return 0;
    }
}

// Check if operator is right-associative
inline bool is_right_associative(OpType op) {
    return op == OpType::NOT;
}

// Inline output stream operator for OpType
inline std::ostream& operator<<(std::ostream& os, OpType op) {
    switch (op) {
        case OpType::VARIABLE: os << "VARIABLE"; break;
        case OpType::TRUE:     os << "TRUE"; break;
        case OpType::FALSE:    os << "FALSE"; break;
        case OpType::NOT:      os << "NOT"; break;
        case OpType::AND:      os << "AND"; break;
        case OpType::OR:       os << "OR"; break;
        case OpType::XOR:      os << "XOR"; break;
    }
    return os;
}

} // namespace logic

#endif // LOGIC_OPT_EXPRESSION_H
