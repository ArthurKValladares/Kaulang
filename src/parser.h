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
    Stmt* parse(Arena* arena, u64& input_count);

private:
    void error(const Token* token, std::string_view message);

    Stmt* program(Arena* arena, u64& input_count);
    Stmt declaration(Arena* arena);
    Stmt var_declaration(Arena* arena);
    Stmt fn_declaration(Arena* arena);
    Stmt class_declaration(Arena* arena);
    Stmt statement(Arena* arena);
    Stmt expr_statement(Arena* arena);
    Stmt block_statement(Arena* arena);
    Stmt print_statement(Arena* arena);
    Stmt if_statement(Arena* arena);
    Stmt while_statement(Arena* arena);
    Stmt for_statement(Arena* arena);
    Stmt break_statement();
    Stmt continue_statement();
    Stmt return_statement(Arena* arena);
    Expr* expression(Arena* arena);
    Expr* assignment(Arena* arena);
    Expr* logic_or(Arena* arena);
    Expr* logic_and(Arena* arena);
    Expr* ternary(Arena* arena);
    Expr* equality(Arena* arena);
    Expr* comparison(Arena* arena);
    Expr* term(Arena* arena);
    Expr* factor(Arena* arena);
    Expr* unary(Arena* arena);
    Expr* fn_call(Arena* arena);
    Expr* primary(Arena* arena);

    Expr* finish_call(Arena* arena, Expr* callee);

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