#include "parser.h"

namespace {
    Expr* new_expr(Expr::Type ty, ExprPayload payload) {
        Expr* expr = (Expr*) malloc(sizeof(Expr));
        expr->expr = payload;
        expr->ty = ty;
        return expr;
    }

    Expr* new_binary(Expr* left, Token* op, Expr* right) {
        BinaryExpr* binary = (BinaryExpr*) malloc(sizeof(BinaryExpr));
        binary->left = left;
        binary->op = op;
        binary->right = right;

        Expr* expr = new_expr(
            Expr::Type::BINARY,
            ExprPayload{.binary = binary}
        );

        return expr;
    }
};

Expr* Parser::expression() {
    return equality();
}

Expr* Parser::equality() {
    Expr* expr = comparison();

    while (match(std::initializer_list<TokenType>{TokenType::BANG, TokenType::BANG_EQUAL})) {
        Token* op = previous();
        Expr* right = term();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
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

Expr* factor() {
    // TODO
    return nullptr;
}

Expr* unary() {
    // TODO
    return nullptr;
}

Expr* primary() {
    // TODO
    return nullptr;
}

bool Parser::is_at_end() {
    return peek()->m_type == TokenType::_EOF;
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

Token* Parser::advance() {
    if (!is_at_end()) {
        ++m_current;
    }
    return previous();
}

bool Parser::check(TokenType ty) {
    if (is_at_end()) {
        return false;
    }
    return peek()->m_type == ty;
}

Token* Parser::peek() {
    return &m_tokens[m_current];
}

Token* Parser::previous() {
    return &m_tokens[m_current - 1];
}