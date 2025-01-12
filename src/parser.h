#pragma once

#include "tokens.h"
#include "expr.h"
#include "scanner.h"

#include <vector>
#include <span>

struct Parser {
    // TODO: Make sure both these constructors can just move the data

    // TODO: Just pass in a span instead?
    Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {}
    Parser(Scanner&& scanner)
        : m_tokens(std::move(scanner.m_tokens))
    {}

    std::vector<Stmt> parse();

private:
    void error(const Token* token, std::string_view message);

    std::vector<Stmt> program();
    Stmt declaration();
    Stmt var_declaration();
    Stmt fn_declaration();
    Stmt statement();
    Stmt expr_statement();
    Stmt block_statement();
    Stmt print_statement();
    Stmt if_statement();
    Stmt while_statement();
    Stmt for_statement();
    Stmt break_statement();
    Stmt continue_statement();
    Expr* expression();
    Expr* assignment();
    Expr* logic_or();
    Expr* logic_and();
    Expr* comma();
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

    std::vector<Token> m_tokens;
    int m_current = 0;

    bool m_had_error = false;
};