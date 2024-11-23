#pragma once

#include "tokens.h"
#include "expr.h"

#include <vector>
#include <span>

struct Parser {
    // TODO: Just pass in a span instead?
    Parser(std::vector<Token> tokens)
        : m_tokens(tokens)
    {}

private:
    Expr* expression();
    Expr* equality();
    Expr* comparison();
    Expr* term();

    bool match(std::span<const TokenType> types);
    Token advance();

    bool check(TokenType ty) const;
    bool is_at_end() const;
    Token peek() const;
    Token previous() const;

    std::vector<Token> m_tokens;
    int m_current = 0;
};