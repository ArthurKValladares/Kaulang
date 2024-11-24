#include "parser.h"

#include "defs.h"

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

    Expr* new_unary(Token* op, Expr* right) {
        UnaryExpr* unary = (UnaryExpr*) malloc(sizeof(UnaryExpr));
        unary->op = op;
        unary->right = right;

        Expr* expr = new_expr(
            Expr::Type::UNARY,
            ExprPayload{.unary = unary}
        );

        return expr;
    }

    Expr* new_literal(Token* val) {
        LiteralExpr* literal = (LiteralExpr*) malloc(sizeof(LiteralExpr));
        literal->val = val;

        Expr* expr = new_expr(
            Expr::Type::LITERAL,
            ExprPayload{.literal = literal}
        );

        return expr;
    }

    Expr* new_grouping(Expr* grouping_expr) {
        GroupingExpr* grouping = (GroupingExpr*) malloc(sizeof(GroupingExpr));
        grouping->expr = grouping_expr;

        Expr* expr = new_expr(
            Expr::Type::GROUPING,
            ExprPayload{.grouping = grouping}
        );

        return expr;
    }

    // TODO: Will need to set some sort of ParserError state here to syncronyze later
    void error(Token* token, std::string_view message) {
        if (token->m_type == TokenType::_EOF) {
            std::println(stderr, "Parser Error: {} at end.", message);
        } else {
            std::println(stderr, "Parser Error: {} of token: {} at {}.", message,
                    token->m_lexeme, token->m_line);
        }
    }
};

Expr* Parser::parse() {
    //TODO: error-handling and syncronization here later
    return expression();
}

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
    Expr* expr = term();

    while (match(std::initializer_list<TokenType>{TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESSER, TokenType::LESSER_EQUAL})) {
        Token* op = previous();
        Expr* right = term();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::term() {
    Expr* expr = factor();

    while (match(std::initializer_list<TokenType>{TokenType::MINUS, TokenType::PLUS})) {
        Token* op = previous();
        Expr* right = factor();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::factor() {
    Expr* expr = unary();

    while (match(std::initializer_list<TokenType>{TokenType::SLASH, TokenType::STAR})) {
        Token* op = previous();
        Expr* right = unary();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::unary() {
    if (match(std::initializer_list<TokenType>{TokenType::BANG, TokenType::MINUS})) {
        Token* op = previous();
        Expr* right = unary();
        return new_unary(op, right);
    } else {
        return primary();
    }
}

Expr* Parser::primary() {
    if (match(std::initializer_list<TokenType>{TokenType::FALSE, TokenType::TRUE, TokenType::NIL, TokenType::NUMBER, TokenType::STRING})) {
        return new_literal(previous());
    }

    if (match(std::initializer_list<TokenType>{TokenType::LEFT_PAREN})) {
        Expr* expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return new_grouping(expr);
    }

    error(peek(), "Expected expression.");

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

Token* Parser::consume(TokenType ty, std::string_view message) {
    if (check(ty)) {
        return advance();
    }

    error(peek(), message);

    return nullptr;
}

void Parser::syncronize() {
    advance();
    while (!is_at_end()) {
        if (previous()->m_type == TokenType::SEMICOLON) {
            return;
        }

        switch (peek()->m_type)
        {
            case TokenType::CLASS:
            case TokenType::FN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
        }

        advance();
    }
}