#pragma once

#include "lib/arena.h"
#include  "lib/span.h"

#include "tokens.h"
#include "expr.h"
#include "scanner.h"


struct Parser {
    Parser(Array<Token> tokens)
        : m_tokens(tokens)
    {}
    Array<Stmt> parse(Arena* arena);

private:
    void error(const Token* token, std::string_view message);

    Array<Stmt> program(Arena* arena);
    Stmt declaration(Arena* arena);
    Stmt var_declaration(Arena* arena);
    Stmt fn_declaration(Arena* arena, bool is_static);
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

    bool match(Span<const TokenType> types);
    bool match(const TokenType ty);
    
    Token* advance();

    bool check(TokenType ty);
    bool is_at_end() const;
    const Token* peek() const;
    Token* previous();

    Token* consume(TokenType ty, std::string_view message);

    void syncronize();

    Array<Token> m_tokens;
    int m_current = 0;

    bool m_had_error = false;
};