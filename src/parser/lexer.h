#ifndef LOGIC_OPT_LEXER_H
#define LOGIC_OPT_LEXER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace logic {

// Token types for the lexer
enum class TokenType {
    // Variables (identifiers starting with letter)
    VARIABLE,

    // Constants
    CONST_TRUE,   // 1
    CONST_FALSE,  // 0

    // Logical operators
    AND,        // &
    OR,         // |
    NOT,        // ~
    XOR,        // ^

    // Delimiters
    LPAREN,     // (
    RPAREN,     // )

    // End of input
    EOF_TOKEN,

    // Error token
    INVALID
};

// Token structure
struct Token {
    TokenType type;
    std::string value;  // For VARIABLE tokens
    size_t line;
    size_t column;

    Token(TokenType t = TokenType::INVALID, std::string v = "", size_t l = 0, size_t c = 0)
        : type(t), value(std::move(v)), line(l), column(c) {}

    std::string to_string() const {
        std::string name;
        switch (type) {
            case TokenType::VARIABLE:    name = "VARIABLE:" + value; break;
            case TokenType::CONST_TRUE:  name = "CONST_TRUE"; break;
            case TokenType::CONST_FALSE: name = "CONST_FALSE"; break;
            case TokenType::AND:         name = "AND"; break;
            case TokenType::OR:          name = "OR"; break;
            case TokenType::NOT:         name = "NOT"; break;
            case TokenType::XOR:         name = "XOR"; break;
            case TokenType::LPAREN:      name = "LPAREN"; break;
            case TokenType::RPAREN:      name = "RPAREN"; break;
            case TokenType::EOF_TOKEN:   name = "EOF"; break;
            case TokenType::INVALID:     name = "INVALID"; break;
        }
        return name + "(" + std::to_string(line) + ":" + std::to_string(column) + ")";
    }
};

// Lexer class for tokenizing logical expressions
class Lexer {
public:
    explicit Lexer(const std::string& input)
        : input_(input), pos_(0), line_(1), column_(1) {}

    // Get all tokens from the input
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        Token token;
        do {
            token = next_token();
            tokens.push_back(token);
        } while (token.type != TokenType::EOF_TOKEN);
        return tokens;
    }

    // Get next token
    Token next_token() {
        skip_whitespace();

        if (pos_ >= input_.size()) {
            return Token(TokenType::EOF_TOKEN, "", line_, column_);
        }

        char c = input_[pos_];

        // Check for constants first
        if (c == '1') {
            ++pos_;
            return Token(TokenType::CONST_TRUE, "1", line_, column_++);
        }
        if (c == '0') {
            ++pos_;
            return Token(TokenType::CONST_FALSE, "0", line_, column_++);
        }

        // Check for operators
        switch (c) {
            case '&':
                ++pos_;
                return Token(TokenType::AND, "", line_, column_++);
            case '|':
                ++pos_;
                return Token(TokenType::OR, "", line_, column_++);
            case '~':
                ++pos_;
                return Token(TokenType::NOT, "", line_, column_++);
            case '^':
                ++pos_;
                return Token(TokenType::XOR, "", line_, column_++);
            case '(':
                ++pos_;
                return Token(TokenType::LPAREN, "", line_, column_++);
            case ')':
                ++pos_;
                return Token(TokenType::RPAREN, "", line_, column_++);
            default:
                // Check if it's a variable identifier
                if (is_alpha(c)) {
                    return tokenize_variable();
                }
                // Unknown character
                ++pos_;
                return Token(TokenType::INVALID, std::string(1, c), line_, column_++);
        }
    }

private:
    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;

    void skip_whitespace() {
        while (pos_ < input_.size() && is_whitespace(input_[pos_])) {
            if (input_[pos_] == '\n') {
                ++line_;
                column_ = 1;
            } else {
                ++column_;
            }
            ++pos_;
        }
    }

    bool is_whitespace(char c) const {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    bool is_alpha(char c) const {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool isalnum(char c) const {
        return is_alpha(c) || (c >= '0' && c <= '9');
    }

    Token tokenize_variable() {
        size_t start = pos_;
        size_t start_col = column_;

        // Variable must start with a letter, followed by letters or digits
        if (is_alpha(input_[pos_])) {
            ++pos_;
            ++column_;
            while (pos_ < input_.size() && isalnum(input_[pos_])) {
                ++pos_;
                ++column_;
            }
        }

        return Token(TokenType::VARIABLE, input_.substr(start, pos_ - start), line_, start_col);
    }
};

// Helper function to print tokens
inline void print_tokens(const std::vector<Token>& tokens) {
    for (const auto& tok : tokens) {
        std::cout << tok.to_string() << std::endl;
    }
}

} // namespace logic

#endif // LOGIC_OPT_LEXER_H
