#ifndef LOGIC_OPT_PARSER_H
#define LOGIC_OPT_PARSER_H

#include "lexer.h"
#include "expression.h"
#include <string>
#include <memory>

namespace logic {

// Parser class (Recursive Descent Parser)
class Parser {
public:
    explicit Parser(const std::string& input)
        : tokens_(Lexer(input).tokenize()), pos_(0) {}

    // Parse the input and return the AST root node
    ExprPtr parse() {
        if (tokens_.empty() || tokens_.back().type != TokenType::EOF_TOKEN) {
            throw std::runtime_error("Parser requires EOF-terminated tokens");
        }
        pos_ = 0;
        auto expr = parse_expression();
        // Check for extra tokens (unconsumed)
        if (pos_ < tokens_.size() - 1) {
            throw std::runtime_error(
                "Unexpected token after expression: " + token_type_name(tokens_[pos_].type)
            );
        }
        return expr;
    }

private:
    std::vector<Token> tokens_;
    size_t pos_;

    // Current token
    Token current() const {
        return tokens_[pos_];
    }

    // Advance to next token
    void advance() {
        if (pos_ < tokens_.size()) {
            ++pos_;
        }
    }

    // Check if current token matches type
    bool check(TokenType type) const {
        return current().type == type;
    }

    // Expect a specific token type, throw if mismatch
    void expect(TokenType type) {
        if (!check(type)) {
            throw std::runtime_error(
                "Expected " + token_type_name(type) + ", got " + token_type_name(current().type)
            );
        }
        advance();
    }

    std::string token_type_name(TokenType type) const {
        switch (type) {
            case TokenType::VARIABLE:    return "VARIABLE";
            case TokenType::CONST_TRUE:  return "1";
            case TokenType::CONST_FALSE: return "0";
            case TokenType::AND:         return "&";
            case TokenType::OR:          return "|";
            case TokenType::NOT:         return "~";
            case TokenType::XOR:         return "^";
            case TokenType::LPAREN:      return "(";
            case TokenType::RPAREN:      return ")";
            case TokenType::EOF_TOKEN:   return "EOF";
            case TokenType::INVALID:     return "INVALID";
        }
        return "UNKNOWN";
    }

    // expression → xor_expr (('|' xor_expr)*)
    ExprPtr parse_expression() {
        return parse_or_expression();
    }

    // or_expr → xor_expr (('|' xor_expr)*)
    ExprPtr parse_or_expression() {
        auto left = parse_xor_expression();
        while (check(TokenType::OR)) {
            advance();
            auto right = parse_xor_expression();
            left = make_or(std::move(left), std::move(right));
        }
        return left;
    }

    // xor_expr → and_expr (('^' and_expr)*)
    ExprPtr parse_xor_expression() {
        auto left = parse_and_expression();
        while (check(TokenType::XOR)) {
            advance();
            auto right = parse_and_expression();
            left = make_xor(std::move(left), std::move(right));
        }
        return left;
    }

    // and_expr → not_expr (('&' not_expr)*)
    ExprPtr parse_and_expression() {
        auto left = parse_not_expression();
        while (check(TokenType::AND)) {
            advance();
            auto right = parse_not_expression();
            left = make_and(std::move(left), std::move(right));
        }
        return left;
    }

    // not_expr → ('~')* primary
    ExprPtr parse_not_expression() {
        size_t negations = 0;
        while (check(TokenType::NOT)) {
            advance();
            ++negations;
        }

        auto expr = parse_primary();

        // Apply negations
        for (size_t i = 0; i < negations; ++i) {
            expr = make_not(std::move(expr));
        }
        return expr;
    }

    // primary → VARIABLE | '(' expression ')' | '1' | '0'
    ExprPtr parse_primary() {
        if (check(TokenType::VARIABLE)) {
            auto tok = current();
            advance();
            return make_var(tok.value);
        }

        if (check(TokenType::CONST_TRUE)) {
            advance();
            return make_true();
        }

        if (check(TokenType::CONST_FALSE)) {
            advance();
            return make_false();
        }

        if (check(TokenType::LPAREN)) {
            advance();
            auto expr = parse_expression();
            expect(TokenType::RPAREN);
            return expr;
        }

        throw std::runtime_error(
            "Unexpected token in primary: " + token_type_name(current().type)
        );
    }
};

} // namespace logic

#endif // LOGIC_OPT_PARSER_H
