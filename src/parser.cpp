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

    Expr* new_comma(Expr* left, Token* op, Expr* right) {
        CommaExpr* comma = (CommaExpr*) malloc(sizeof(CommaExpr));
        comma->left = left;
        comma->op = op;
        comma->right = right;

        Expr* expr = new_expr(
            Expr::Type::COMMA,
            ExprPayload{.comma = comma}
        );

        return expr;
    }

    Expr* new_ternary(Expr* left, Token* left_op, Expr* middle, Token* right_op, Expr* right) {
        TernaryExpr* ternary = (TernaryExpr*) malloc(sizeof(TernaryExpr));
        ternary->left = left;
        ternary->left_op = left_op;
        ternary->middle = middle;
        ternary->right_op = right_op;
        ternary->right = right;

        Expr* expr = new_expr(
            Expr::Type::TERNARY,
            ExprPayload{.ternary = ternary}
        );

        return expr;
    }

    Stmt new_print_stmt(Expr* val) {
        return Stmt {
            .ty = Stmt::Type::PRINT,
            .expr = val
        };
    }

    Stmt new_expr_stmt(Expr* expr) {
        return Stmt {
            .ty = Stmt::Type::EXPR,
            .expr = expr
        };
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

std::vector<Stmt> Parser::parse() {
    //TODO: error-handling and syncronization here later
    return program();
}

std::vector<Stmt> Parser::program() {
    std::vector<Stmt> statements = {};
    while (!is_at_end()) {
        statements.emplace_back(statement());
    }
    return statements;
}

Stmt Parser::statement() {
    if (match(std::initializer_list<TokenType>{TokenType::PRINT})) {
        return print_statement();
    } else {
        return expr_statement();
    }
}

Stmt Parser::expr_statement() {
    Expr* val = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return new_expr_stmt(val);
}

Stmt Parser::print_statement() {
    Expr* val = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value");
    return new_print_stmt(val);
}

Expr* Parser::expression() {
    return comma();
}

Expr* Parser::comma() {
    Expr* expr = ternary();

    while (match(std::initializer_list<TokenType>{TokenType::COMMA})) {
        Token* op = previous();
        Expr* right = comma();
        Expr* comma = new_comma(expr, op, right);
        expr = comma;
    }

    return expr;
}

Expr* Parser::ternary() {
    Expr* expr = equality();

    if (match(std::initializer_list<TokenType>{TokenType::QUESTION_MARK})) {
        Token* left_op = previous();
        Expr* middle = ternary();

        if (match(std::initializer_list<TokenType>{TokenType::COLON})) {
            Token* right_op = previous();
            Expr* right = ternary();
            return new_ternary(expr, left_op, middle, right_op, right);
        }

        error(peek(), "ternary operator expected `:`.");

        return nullptr;
    } else {
        return expr;
    }
}

Expr* Parser::equality() {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        error(peek(), "equality operator without right-hand side expression.");
        return nullptr;
    }

    Expr* expr = comparison();

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::comparison() {
    constexpr TokenType TOKEN_TYPES[4] = {TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESSER, TokenType::LESSER_EQUAL};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        error(peek(), "comparison operator without right-hand side expression.");
        return nullptr;
    }

    Expr* expr = term();

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::term() {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::MINUS, TokenType::PLUS};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        error(peek(), "term operator without right-hand side expression.");
        return nullptr;
    }

    Expr* expr = factor();

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = factor();
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::factor() {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::SLASH, TokenType::STAR};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term();
        error(peek(), "factor operator without right-hand side expression.");
        return nullptr;
    }

    Expr* expr = unary();

    while (match(TOKEN_TYPES)) {
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
    if (match(std::initializer_list<TokenType>{
        TokenType::FALSE, TokenType::TRUE, TokenType::NIL, 
        TokenType::NUMBER_INT, TokenType::NUMBER_FLOAT, TokenType::NUMBER_DOUBLE,
        TokenType::STRING
    })) {
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
    const auto test = peek()->m_type;
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