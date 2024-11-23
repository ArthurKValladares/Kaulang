#include "parser.h"

Expr* Parser::expression() {
    return equality();
}

Expr* Parser::equality() {
    Expr* expr = comparison();

    while (match(std::initializer_list<TokenType>{TokenType::BANG, TokenType::BANG_EQUAL})) {
        Token op = previous();
        Expr* right = term();
        // TODO:
        // expr = ...
    }

    return expr;
}

Expr* Parser::comparison() {
    // TODO
    return nullptr;
}

Expr* Parser::term() {
    // TODO
    return nullptr;
}

bool Parser::is_at_end() const {
    return peek().m_type == TokenType::_EOF;
}

bool Parser::match(std::span<const TokenType> types) {
    if (is_at_end()) {
        return false;
    }

    for (const TokenType ty : types) {
        if (check(ty)) {
            advance();
            return true;
        }
    }

    return false;
}

Token Parser::advance() {
    if (!is_at_end()) {
        ++m_current;
    }
    return previous();
}

bool Parser::check(TokenType ty) const {
    if (is_at_end()) {
        return false;
    }
    return peek().m_type == ty;
}

Token Parser::peek() const {
    return m_tokens[m_current];
}

Token Parser::previous() const {
    return m_tokens[m_current - 1];
}