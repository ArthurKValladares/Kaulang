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

    Expr* new_this(const Token* val) {
        ThisExpr* this_expr = (ThisExpr*) malloc(sizeof(ThisExpr));
        this_expr->val = val;
        Expr* expr = new_expr(
            Expr::Type::THIS,
            ExprPayload{.this_expr = this_expr}
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
        LogicalBinaryExpr* logical_binary = (LogicalBinaryExpr*) malloc(sizeof(LogicalBinaryExpr));
        assert(logical_binary != nullptr);
        logical_binary->left = left;
        logical_binary->op = op;
        logical_binary->right = right;

        Expr* expr = new_expr(
            Expr::Type::AND,
            ExprPayload{.logical_binary = logical_binary}
        );

        return expr;
    }

    Expr* new_or(Expr* left, Token* op, Expr* right) {
        LogicalBinaryExpr* logical_binary = (LogicalBinaryExpr*) malloc(sizeof(LogicalBinaryExpr));
        assert(logical_binary != nullptr);
        logical_binary->left = left;
        logical_binary->op = op;
        logical_binary->right = right;

        Expr* expr = new_expr(
            Expr::Type::OR,
            ExprPayload{.logical_binary = logical_binary}
        );

        return expr;
    }

    Expr* new_fn_call(Expr* callee, const Token* paren, Expr** arguments, u64 arguments_count) {
        FnCallExpr* fn_call = (FnCallExpr*) malloc(sizeof(FnCallExpr));
        assert(fn_call != nullptr);
        fn_call->callee = callee;
        fn_call->paren = paren;
        fn_call->arguments = arguments;
        fn_call->arguments_count = arguments_count;

        Expr* expr = new_expr(
            Expr::Type::FN_CALL,
            ExprPayload{.fn_call = fn_call}
        );

        return expr;
    }

    Expr* new_get(Expr* class_expr, Token* member) {
        GetExpr* get_expr = (GetExpr*) malloc(sizeof(GetExpr));
        assert(get_expr != nullptr);
        get_expr->class_expr = class_expr;
        get_expr->member = member;

        Expr* expr = new_expr(
            Expr::Type::GET,
            ExprPayload{.get = get_expr}
        );

        return expr;
    }

    Expr* new_set(Expr* get, Expr* right) {
        SetExpr* set = (SetExpr*) malloc(sizeof(SetExpr));
        set->get = get;
        set->right = right;

        Expr* expr = new_expr(
            Expr::Type::SET,
            ExprPayload{.set = set}
        );

        return expr;
    }

    Stmt* allocated_stmt(Stmt stmt) {
        Stmt* ret = (Stmt*) malloc(sizeof(Stmt));
        *ret = stmt;
        return ret;
    }

    Stmt new_block_stmt(Stmt* stmts, int size) {
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

    Stmt new_fn_declaration(Token* name, Token** params, u64 params_count, Stmt* body) {
        return Stmt {
            .ty = Stmt::Type::FN_DECLARATION,
            .fn_declaration = FnDeclarationPayload{
                name, 
                params,
                params_count,
                body
            },
        };
    }

    Stmt new_class_declaration(Token* name, Stmt* members, u64 members_count) {
        return Stmt {
            .ty = Stmt::Type::CLASS_DECLARATION,
            .s_class = ClassDeclarationPayload{
                name, 
                members,
                members_count
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

Stmt* Parser::parse(Arena* arena, u64& input_len) {
    return program(arena, input_len);
}

Stmt* Parser::program(Arena* arena, u64& input_len) {
    arena->child_arena = alloc_arena();

    u64 stmts_count = 0;
    Stmt* statements = (Stmt*) arena->push_no_zero(0);
    while (!is_at_end()) {
        arena->push_struct_no_zero<Stmt>();
        statements[stmts_count++] = declaration(arena->child_arena);
    }
    input_len = stmts_count;
    return statements;
}

Stmt Parser::declaration(Arena* arena) {
    Stmt ret;
    if (match(std::initializer_list<TokenType>{TokenType::VAR})) {
        ret = var_declaration(arena);
    } else if (match(std::initializer_list<TokenType>{TokenType::FN})) {
        ret = fn_declaration(arena);
    } else if (match(std::initializer_list<TokenType>{TokenType::CLASS})) {
        ret = class_declaration(arena);
    } else {
        ret = statement(arena);
    }

    if (ret.ty == Stmt::Type::ERR) {
        syncronize();
    }

    return ret;
}

Stmt Parser::fn_declaration(Arena* arena) {
    Token* name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");

    int param_count = 0;
    Token** params = (Token**) arena->push_no_zero(0);
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arena->push_struct_no_zero<Token*>();
            params[param_count++] = consume(TokenType::IDENTIFIER, "Expecteded parameter name");
        } while(match(std::initializer_list<TokenType>{TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters");

    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");
    Stmt* body = allocated_stmt(block_statement(arena));

    return new_fn_declaration(name, params, param_count, body);
}

Stmt Parser::class_declaration(Arena* arena) {
    arena->child_arena = alloc_arena();

    Token* name = consume(TokenType::IDENTIFIER, "Expected class name");

    consume(TokenType::LEFT_BRACE, "Expected '{' after class name");

    u64 methods_count = 0;
    Stmt* methods = (Stmt*) arena->push_no_zero(0);
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        arena->push_struct_no_zero<Stmt>();
        if (match(std::initializer_list<TokenType>{TokenType::VAR})) {
            methods[methods_count++] = var_declaration(arena->child_arena);
        } else if (match(std::initializer_list<TokenType>{TokenType::FN})) {
            methods[methods_count++] = fn_declaration(arena->child_arena);
        } else {
            error(peek(), "Statements inside class declaration must either be functions or variables.");
        }
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after class body");
    consume(TokenType::SEMICOLON, "Expected ';' after class declaration");
    
    return new_class_declaration(name, methods, methods_count);
}

Stmt Parser::var_declaration(Arena* arena) {
    Token* name = consume(TokenType::IDENTIFIER, "Expected variable name");

    Expr* initializer = nullptr;
    if (match(std::initializer_list<TokenType>{TokenType::EQUAL})) {
        initializer = expression(arena);
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return new_var_decl(name, initializer);
}

Stmt Parser::statement(Arena* arena) {
    if (match(std::initializer_list<TokenType>{TokenType::IF})) {
        return if_statement(arena);
    }
    if (match(std::initializer_list<TokenType>{TokenType::WHILE})) {
        return while_statement(arena);
    }
    if (match(std::initializer_list<TokenType>{TokenType::FOR})) {
        return for_statement(arena);
    }
    if (match(std::initializer_list<TokenType>{TokenType::LEFT_BRACE})) {
        return block_statement(arena);
    }
    if (match(std::initializer_list<TokenType>{TokenType::BREAK})) {
        return break_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::CONTINUE})) {
        return continue_statement();
    }
    if (match(std::initializer_list<TokenType>{TokenType::RETURN})) {
        return return_statement(arena);
    }
    return expr_statement(arena);
}

Stmt Parser::expr_statement(Arena* arena) {
    Expr* val = expression(arena);

    if (m_had_error) {
        return new_err_stmt();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return new_expr_stmt(val);
}

Stmt Parser::block_statement(Arena* arena) {
    int stmt_count = 0;

    arena->child_arena = alloc_arena();

    Stmt* stmts = (Stmt*) arena->push_no_zero(0);
    while (!is_at_end() && !check(TokenType::RIGHT_BRACE)) {
        arena->push_struct_no_zero<Stmt>();
        stmts[stmt_count++] = declaration(arena->child_arena);
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");

    return new_block_stmt(stmts, stmt_count);
}


Stmt Parser::if_statement(Arena* arena) {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    Expr* expr = expression(arena);
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in if statement");

    Stmt if_stmt = statement(arena);

    Stmt else_stmt = {};
    if (match(std::initializer_list<TokenType>{TokenType::ELSE})) {
        else_stmt = statement(arena);
    }

    return new_if_stmt(expr, if_stmt, else_stmt);
}

Stmt Parser::while_statement(Arena* arena) {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    Expr* expr = expression(arena);
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in while statement");

    Stmt stmt = statement(arena);

    return new_while_stmt(expr, stmt);
}

Stmt Parser::for_statement(Arena* arena) {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");

    Stmt initializer = {};
    if (match(std::initializer_list<TokenType>{TokenType::SEMICOLON})) {
    } else if (match(std::initializer_list<TokenType>{TokenType::VAR})) {
        initializer = var_declaration(arena);
    } else {
        initializer = expr_statement(arena);
    }

    Expr* condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression(arena);
    }
    consume(TokenType::SEMICOLON, "Expected ';' after 'for' condition");

    Expr* increment = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        increment = expression(arena);
    }
    consume(TokenType::RIGHT_PAREN, "Unterminated parentheses in for statement");

    Stmt body = statement(arena);

    if (increment != nullptr) {
        const int size = 2;
        Stmt* stmts = (Stmt*) arena->push_array_no_zero<Stmt>(2);
        stmts[0] = body;
        stmts[1] = new_expr_stmt(increment);
        body = new_block_stmt(stmts, size);
    }
    if (condition == nullptr) {
        condition = new_literal(&true_token);
    }
    body = new_while_stmt(condition, body);
    if (initializer.ty != Stmt::Type::ERR) {
        const int size = 2;
        Stmt* stmts = (Stmt*) arena->push_array_no_zero<Stmt>(2);
        stmts[0] = initializer;
        stmts[1] = body;
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

Stmt Parser::return_statement(Arena* arena) {
    Token* return_keyword = previous();
    
    Expr* value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression(arena);
    }
    consume(TokenType::SEMICOLON, "Expected ';' after retur value");

    return new_return_stmt(return_keyword, value);
}

Expr* Parser::expression(Arena* arena) {
    return assignment(arena);
}

Expr* Parser::assignment(Arena* arena) {
    Expr* expr = logic_or(arena);

    if (match(std::initializer_list<TokenType>{TokenType::EQUAL})) {
        Token* equals = previous();
        Expr* right = assignment(arena);

        if (expr->ty == Expr::Type::LITERAL && expr->expr.literal->val->m_type == TokenType::IDENTIFIER) {
            const Token* id = expr->expr.literal->val;
            return new_assignment(id, right);
        } else if (expr->ty == Expr::Type::GET) {
            return new_set(expr, right);
        } else {
            error(peek(), "invalid assignment target.");
            return nullptr;
        }
    }

    return expr;
}

Expr* Parser::logic_or(Arena* arena) {
    Expr* expr = logic_and(arena);

    while (match(std::initializer_list<TokenType>{TokenType::OR})) {
        Token* op = previous();
        Expr* right = logic_and(arena);
        return new_or(expr, op, right);
    }

    return expr;
}

Expr* Parser::logic_and(Arena* arena) {
    Expr* expr = ternary(arena);

    while (match(std::initializer_list<TokenType>{TokenType::AND})) {
        Token* op = previous();
        Expr* right = ternary(arena);
        return new_and(expr, op, right);
    }

    return expr;
}

Expr* Parser::ternary(Arena* arena) {
    Expr* expr = equality(arena);

    if (match(std::initializer_list<TokenType>{TokenType::QUESTION_MARK})) {
        Token* left_op = previous();
        Expr* middle = ternary(arena);

        if (match(std::initializer_list<TokenType>{TokenType::COLON})) {
            Token* right_op = previous();
            Expr* right = ternary(arena);
            return new_ternary(expr, left_op, middle, right_op, right);
        }

        error(peek(), "ternary operator expected `:`.");

        return nullptr;
    } else {
        return expr;
    }
}

Expr* Parser::equality(Arena* arena) {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);

        error(peek(), "equality operator without right-hand side expression.");

        return nullptr;
    }

    Expr* expr = comparison(arena);

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::comparison(Arena* arena) {
    constexpr TokenType TOKEN_TYPES[4] = {TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESSER, TokenType::LESSER_EQUAL};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);

        error(peek(), "comparison operator without right-hand side expression.");

        return nullptr;
    }

    Expr* expr = term(arena);

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::term(Arena* arena) {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::MINUS, TokenType::PLUS};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);

        error(peek(), "term operator without right-hand side expression.");

        m_had_error = true;
        return nullptr;
    }

    Expr* expr = factor(arena);

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = factor(arena);
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::factor(Arena* arena) {
    constexpr TokenType TOKEN_TYPES[2] = {TokenType::SLASH, TokenType::STAR};

    if (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = term(arena);

        error(peek(), "factor operator without right-hand side expression.");

        return nullptr;
    }

    Expr* expr = unary(arena);

    while (match(TOKEN_TYPES)) {
        Token* op = previous();
        Expr* right = unary(arena);
        Expr* binary = new_binary(expr, op, right);
        expr = binary;
    }

    return expr;
}

Expr* Parser::unary(Arena* arena) {
    if (match(std::initializer_list<TokenType>{TokenType::BANG, TokenType::MINUS})) {
        Token* op = previous();
        Expr* right = unary(arena);
        return new_unary(op, right);
    } else {
        return fn_call(arena);
    }
}

Expr* Parser::fn_call(Arena* arena) {
    Expr* expr = primary(arena);

    while (true) {
        if (match(std::initializer_list<TokenType>{TokenType::LEFT_PAREN})) {
            expr = finish_call(arena, expr);
        } else if (match(std::initializer_list<TokenType>{TokenType::DOT})) {
            Token* name = consume(TokenType::IDENTIFIER, "Expected identifier after '.'");
            expr = new_get(expr, name);
        } else {
            break;
        }
    }

    return expr;
}

Expr* Parser::finish_call(Arena* arena, Expr* callee) {
    arena->child_arena = alloc_arena();

    u64 arguments_count = 0;
    Expr** arguments = (Expr**) arena->push_no_zero(0);

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arena->push_struct_no_zero<Expr*>();
            arguments[arguments_count++] = expression(arena->child_arena);
        } while(match(std::initializer_list<TokenType>{TokenType::COMMA}));
    }

    Token* paren = consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");

    return new_fn_call(callee, paren, arguments, arguments_count);
}

Expr* Parser::primary(Arena* arena) {
    if (match(std::initializer_list<TokenType>{
        TokenType::FALSE, TokenType::TRUE, TokenType::NIL, 
        TokenType::NUMBER_INT, TokenType::NUMBER_FLOAT, TokenType::NUMBER_DOUBLE,
        TokenType::STRING
    })) {
        return new_literal(previous());
    }

    if (match(std::initializer_list<TokenType>{TokenType::THIS})) {
        return new_this(previous());
    }

    if (match(std::initializer_list<TokenType>{TokenType::IDENTIFIER})) {
        Token* id = previous();
        return new_literal(id);
    }

    if (match(std::initializer_list<TokenType>{TokenType::LEFT_PAREN})) {
        Expr* expr = expression(arena);
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
            case TokenType::RETURN:
                return;
        }

        advance();
    }

    m_had_error = false;
}