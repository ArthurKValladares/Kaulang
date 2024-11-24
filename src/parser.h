#pragma once

#include "tokens.h"
#include "expr.h"
#include "scanner.h"

#include <vector>
#include <span>

// TODO: Probably makes sense to make a lot of this stuff const
struct Parser {
    // TODO: Make sure both these constructors can just move the data

    // TODO: Just pass in a span instead?
    Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {}
    Parser(Scanner&& scanner)
        : m_tokens(std::move(scanner.m_tokens))
    {}

    Expr* parse();

private:
    Expr* expression();
    Expr* equality();
    Expr* comparison();
    Expr* term();
    Expr* factor();
    Expr* unary();
    Expr* primary();

    bool match(std::span<const TokenType> types);
    Token* advance();

    bool check(TokenType ty);
    bool is_at_end();
    Token* peek();
    Token* previous();

    Token* consume(TokenType ty, std::string_view message);

    void syncronize();

    std::vector<Token> m_tokens;
    int m_current = 0;
};