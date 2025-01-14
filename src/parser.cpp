#include "parser.h"

#include "defs.h"

const Token true_token = Token {
    TokenType::TRUE,
    String{},
    0,
    TokenData{},
};

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

    Expr* new_literal(const Token* val) {
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

    Expr* new_assignment(const Token* id, Expr* right) {
        AssignmentExpr* assignment = (AssignmentExpr*) malloc(sizeof(AssignmentExpr));
        assignment->id = id;
        assignment->right = right;

        Expr* expr = new_expr(
            Expr::Type::ASSIGNMENT,
            ExprPayload{.assignment = assignment}
        );

        return expr;
    }

    Expr* new_and(Expr* left, Token* op, Expr* right) {
        AndExpr* logical_and = (AndExpr*) malloc(sizeof(AndExpr));
        assert(logical_and != nullptr);
        logical_and->left = left;
        logical_and->op = op;
        logical_and->right = right;

        Expr* expr = new_expr(
            Expr::Type::AND,
            ExprPayload{.logical_and = logical_and}
        );

        return expr;
    }

    Expr* new_or(Expr* left, Token* op, Expr* right) {
        OrExpr* logical_or = (OrExpr*) malloc(sizeof(OrExpr));
        assert(logical_or != nullptr);
        logical_or->left = left;
        logical_or->op = op;
        logical_or->right = right;

        Expr* expr = new_expr(
            Expr::Type::OR,
            ExprPayload{.logical_or = logical_or}
        );

        return expr;
    }

    Expr* new_fn_call(Expr* callee, const Token* paren, std::vector<Expr*> arguments) {
        // TODO: do away with the vector later so i can use malloc
        FnCallExpr* fn_call = new FnCallExpr;
        assert(fn_call != nullptr);
        fn_call->callee = callee;
        fn_call->paren = paren;
        fn_call->arguments = std::move(arguments);

        Expr* expr = new_expr(
            Expr::Type::FN_CALL,
            ExprPayload{.fn_call = fn_call}
        );

        return expr;
    }

    // TODO: This function is probably temporary
    Stmt* allocated_stmt(Stmt stmt) {
        Stmt* ret = (Stmt*) malloc(sizeof(Stmt));
        *ret = stmt;
        return ret;
    }

    Stmt new_print_stmt(Expr* val) {
        return Stmt {
            .ty = Stmt::Type::EXPR,
            .s_expr = ExprStmtPayload{val},
            .should_print = true,
        };
    }

    Stmt new_block_stmt(Stmt** stmts, int size) {
        return Stmt {
            .ty = Stmt::Type::BLOCK,
            .s_block = BlockPayload{stmts, size}
        };
    }

    Stmt new_expr_stmt(Expr* expr) {
        return Stmt {
            .ty = Stmt::Type::EXPR,
            .s_expr = ExprStmtPayload{expr},
        };
    }

    Stmt new_var_decl(Token* name, Expr* expr) {
        return Stmt {
            .ty = Stmt::Type::VAR_DECL,
            .s_var_decl = VarDeclPayload{name, expr},
        };
    }

    Stmt new_if_stmt(Expr* expr, Stmt if_stmt, Stmt else_stmt) {
        return Stmt {
            .ty = Stmt::Type::IF,
            .s_if = IfPayload{
                expr,
                allocated_stmt(if_stmt),
                allocated_stmt(else_stmt)
            }
        };
    }

    Stmt new_while_stmt(Expr* expr, Stmt stmt) {
        return Stmt {
            .ty = Stmt::Type::WHILE,
            .s_while = WhilePayload{
                expr,
                allocated_stmt(stmt)
            }
        };
    }

    Stmt new_err_stmt() {
        return Stmt {
            .ty = Stmt::Type::ERR,
        };
    }

    Stmt new_break_stmt(int line) {
        return Stmt {
            .ty = Stmt::Type::BREAK,
            .s_break_continue = BreakContinuePayload{line},
        };
    }

    Stmt new_continue_stmt(int line) {
        return Stmt {
            .ty = Stmt::Type::CONTINUE,
            .s_break_continue = BreakContinuePayload{line},
        };
    }

    Stmt new_return_stmt(Token* keyword, Expr* expr) {
        return Stmt {
            .ty = Stmt::Type::RETURN,
            .s_return = ReturnPayload{
                keyword,
                expr
            },
        };
    }

    Stmt new_fn_declaration(Token* name, Token** params, int params_size, Stmt* body) {
        return Stmt {
            .ty = Stmt::Type::FN_DECLARATION,
            .fn_declaration = FnDeclarationPayload{
                name, 
                params,
                params_size,
                body
            },
        };
    }
};

void Parser::error(const Token* token, std::string_view message) {
    if (token->m_type == TokenType::_EOF) {
        std::println(stderr, "Parser Error: {} at end.", message);
    } else {
        std::println(stderr, "Parser Error: {} of token: {} at {}.", message,
                token->m_lexeme.to_string_view(), token->m_line);
    }
    m_had_error = true;
}

std::vector<Stmt> Parser::parse() {
    return program();
}

std::vector<Stmt> Parser::program() {
    std::vector<Stmt> statements = {};
    while (!is_at_end()) {
        statements.emplace_back(declaration());
    }
    return statements;
}

Stmt Parser::declaration() {
    Stmt ret;
    if (match(std::initializer_list<TokenType>{TokenType::VAR})) {
        ret = var_declaration();
    } else if (match(std::initializer_list<TokenType>{TokenType::FN})) {
        ret = fn_declaration();
    } else {
        ret = statement();
    }

    if (ret.ty == Stmt::Type::ERR) {
        syncronize();
    }

    return ret;
}

Stmt Parser::fn_declaration() {
    Token* name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<Token*> params = {};
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            params.push_back(consume(TokenType::IDENTIFIER, "Expecteded parameter name"));
        } while(match(std::initializer_list<TokenType>{TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters");

    // TODO: Probably can make this better without this double `copy` step
    const int size = params.size();
    Token** params_c = (Token**) malloc(size * sizeof(Token*));
    for (int i = 0; i < size; ++i) {
        params_c[i] = params[i];
    }

    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");
    Stmt* body = allocated_stmt(block_statement());

    return new_fn_declaration(name, params_c, size, body);
}


Stmt Parser::var_declaration() {
    Token* name = consume(TokenType::IDENTIFIER, "Expected variable name");

    Expr* initializer = nullptr;
    if (match(std::initializer_list<TokenType>{TokenType::EQUAL})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return new_var_decl(name, initializer);
}

Stmt Parser::statement() {
    if (match(std::initializer_list<TokenType>{TokenType::IF})) {
        return if_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::WHILE})) {
        return while_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::FOR})) {
        return for_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::PRINT})) {
        return print_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::LEFT_BRACE})) {
        return block_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::BREAK})) {
        return break_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::CONTINUE})) {
        return continue_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::RETURN})) {
        return return_statement();
    }
    return expr_statement();
}

Stmt Parser::expr_statement() {
    Expr* val = expression();

    if (m_had_error) {
        return new_err_stmt();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return new_expr_stmt(val);
}

Stmt Parser::block_statement() {
    std::vector<Stmt> stmts = {};

    while (!is_at_end() && !check(TokenType::RIGHT_BRACE)) {
        stmts.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");

    // TODO: Probably can make this better without this double `copy` step
    const int size = stmts.size();
    Stmt** stmts_c = (Stmt**) malloc(size * sizeof(Stmt*));
    for (int i = 0; i < size; ++i) {
        stmts_c[i] = allocated_stmt(stmts[i]);
    }
    return new_block_stmt(stmts_c, size);
}

Stmt Parser::print_statement() {
    Expr* val = expression();
    if (val == nullptr) {
        return new_err_stmt();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after value");
    return new_print_stmt(val);
}

Stmt Parser::if_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    Expr* expr = expression();
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in if statement");

    Stmt if_stmt = statement();

    Stmt else_stmt = {};
    if (match(std::initializer_list<TokenType>{TokenType::ELSE})) {
        else_stmt = statement();
    }

    return new_if_stmt(expr, if_stmt, else_stmt);
}

Stmt Parser::while_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    Expr* expr = expression();
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in while statement");

    Stmt stmt = statement();

    return new_while_stmt(expr, stmt);
}

Stmt Parser::for_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");

    Stmt initializer = {};
    if (match(std::initializer_list<TokenType>{TokenType::SEMICOLON})) {
    } else if (match(std::initializer_list<TokenType>{TokenType::VAR})) {
        initializer = var_declaration();
    } else {
        initializer = expr_statement();
    }

    Expr* condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after 'for' condition");

    Expr* increment = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in for statement");

    Stmt body = statement();

    if (increment != nullptr) {
        const int size = 2;
        Stmt** stmts = (Stmt**) malloc(size * sizeof(Stmt*));
        stmts[0] = allocated_stmt(body);
        stmts[1] = allocated_stmt(new_expr_stmt(increment));
        body = new_block_stmt(stmts, size);
    }
    if (condition == nullptr) {
        condition = new_literal(&true_token);
    }
    body = new_while_stmt(condition, body);
    if (initializer.ty != Stmt::Type::ERR) {
        const int size = 2;
        Stmt** stmts = (Stmt**) malloc(size * sizeof(Stmt*));
        stmts[0] = allocated_stmt(initializer);
        stmts[1] = allocated_stmt(body);
        body = new_block_stmt(stmts, size);
    }
    
    return body;
}

Stmt Parser::break_statement() {
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'");

    return new_break_stmt(previous()->m_line);
}

Stmt Parser::continue_statement() {
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'");

    return new_continue_stmt(previous()->m_line);
}

Stmt Parser::return_statement() {
    Token* return_keyword = previous();
    
    Expr* value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after retur value");

    return new_return_stmt(return_keyword, value);
}

Expr* Parser::expression() {
    return assignment();
}

Expr* Parser::assignment() {
    Expr* expr = logic_or();

    if (match(std::initializer_list<TokenType>{TokenType::EQUAL})) {
        Token* equals = previous();
        Expr* right = assignment();

        if (expr->ty == Expr::Type::LITERAL && expr->expr.literal->val->m_type == TokenType::IDENTIFIER) {
            const Token* id = expr->expr.literal->val;
            return new_assignment(id, right);
        } else {
            error(peek(), "invalid assignment target.");
            return nullptr;
        }
    }

    return expr;
}

Expr* Parser::logic_or() {
    Expr* expr = logic_and();

    while (match(std::initializer_list<TokenType>{TokenType::OR})) {
        Token* op = previous();
        Expr* right = logic_and();
        return new_or(expr, op, right);
    }

    return expr;
}

Expr* Parser::logic_and() {
    Expr* expr = ternary();

    while (match(std::initializer_list<TokenType>{TokenType::AND})) {
        Token* op = previous();
        Expr* right = ternary();
        return new_and(expr, op, right);
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

        m_had_error = true;
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
        return fn_call();
    }
}

Expr* Parser::fn_call() {
    Expr* expr = primary();

    while (true) {
        if (match(std::initializer_list<TokenType>{TokenType::LEFT_PAREN})) {
            expr = finish_call(expr);
        } else {
            break;
        }
    }

    return expr;
}

Expr* Parser::finish_call(Expr* callee) {
    std::vector<Expr*> arguments = {};

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while(match(std::initializer_list<TokenType>{TokenType::COMMA}));
    }

    Token* paren = consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");

    return new_fn_call(callee, paren, arguments);
}

Expr* Parser::primary() {
    if (match(std::initializer_list<TokenType>{
        TokenType::FALSE, TokenType::TRUE, TokenType::NIL, 
        TokenType::NUMBER_INT, TokenType::NUMBER_FLOAT, TokenType::NUMBER_DOUBLE,
        TokenType::STRING
    })) {
        return new_literal(previous());
    }

    if (match(std::initializer_list<TokenType>{TokenType::IDENTIFIER})) {
        Token* id = previous();
        return new_literal(id);
    }

    if (match(std::initializer_list<TokenType>{TokenType::LEFT_PAREN})) {
        Expr* expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return new_grouping(expr);
    }

    error(peek(), "Expected expression.");

    return nullptr;
}

bool Parser::is_at_end() const {
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

const Token* Parser::peek() const {
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

    m_had_error = false;
}