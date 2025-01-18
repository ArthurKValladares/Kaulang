#pragma once

#include "lib/arena.h"

#include "tokens.h"
#include "expr.h"
#include "scanner.h"

#include <vector>
#include <span>

struct Parser {
    Parser(Token* tokens, u64 tokens_size)
        : m_tokens(tokens)
        , m_tokens_size(tokens_size)
    {}
    std::vector<Stmt> parse(Arena* arena);

private:
    void error(const Token* token, std::string_view message);

    std::vector<Stmt> program(Arena* arena);
    Stmt declaration(Arena* arena);
    Stmt var_declaration();
    Stmt fn_declaration(Arena* arena);
    Stmt statement(Arena* arena);
    Stmt expr_statement();
    Stmt block_statement(Arena* arena);
    Stmt print_statement();
    Stmt if_statement(Arena* arena);
    Stmt while_statement(Arena* arena);
    Stmt for_statement(Arena* arena);
    Stmt break_statement();
    Stmt continue_statement();
    Stmt return_statement();
    Expr* expression();
    Expr* assignment();
    Expr* logic_or();
    Expr* logic_and();
    Expr* ternary();
    Expr* equality();
    Expr* comparison();
    Expr* term();
    Expr* factor();
    Expr* unary();
    Expr* fn_call();
    Expr* primary();

    Expr* finish_call(Expr* callee);

    bool match(std::span<const TokenType> types);
    Token* advance();

    bool check(TokenType ty);
    bool is_at_end() const;
    const Token* peek() const;
    Token* previous();

    Token* consume(TokenType ty, std::string_view message);

    void syncronize();

    Token* m_tokens;
    u64 m_tokens_size;
    int m_current = 0;

    bool m_had_error = false;
};