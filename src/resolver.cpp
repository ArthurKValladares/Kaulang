#include "resolver.h"

#include "compiler.h"

void Resolver::resolve(KauCompiler* compiler, Stmt* stmts, u64 stmts_len) {
    for (u64 i = 0; i < stmts_len; ++i) {
        resolve_stmt(compiler, &stmts[i]);
    }
}

void Resolver::resolve_stmt(KauCompiler* compiler, Stmt* stmt) {
    switch (stmt->ty) {
        case Stmt::Type::EXPR: {
            visit_expr_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::VAR_DECL: {
            visit_var_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::BLOCK: {
            visit_block_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::IF: {
            visit_if_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::WHILE: {
            visit_while_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::FN_DECLARATION: {
            visit_fn_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::RETURN: {
            visit_return_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::PRINT: {
            visit_print_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::BREAK: {
            break;
        }
        case Stmt::Type::CONTINUE: {
            break;
        }
        case Stmt::Type::ERR: {
            assert(false);
            break;
        }
    }
}

void Resolver::resolve_expr(KauCompiler* compiler, Expr* expr) {
    switch (expr->ty) {
        case Expr::Type::LITERAL: {
            // TODO: Don't love that this is how I need to detect a variable, make it better
            if (expr->expr.literal->val->m_type == TokenType::IDENTIFIER) {
                visit_variable_expr(compiler, expr);
            }
            break;
        }
        case Expr::Type::UNARY: {
            visit_unary_expr(compiler, expr);
            break;
        }
        case Expr::Type::BINARY: {
            visit_binary_expr(compiler, expr);
            break;
        }
        case Expr::Type::GROUPING: {
            visit_grouping_expr(compiler, expr);
            break;
        }
        case Expr::Type::TERNARY: {
            visit_ternary_expr(compiler, expr);
            break;
        }
        case Expr::Type::ASSIGNMENT: {
            visit_assign_expr(compiler, expr);
            break;
        }
        case Expr::Type::AND: {
            visit_logical_expr(compiler, expr);
            break;
        }
        case Expr::Type::OR: {
            visit_logical_expr(compiler, expr);
            break;
        }
        case Expr::Type::FN_CALL: {
            visit_fn_call_expr(compiler, expr);
            break;
        }
        case Expr::Type::ERR: {
            assert(false);
            break;
        }
    }   
}

void Resolver::visit_expr_stmt(KauCompiler* compiler, Stmt* stmt) {
    resolve_expr(compiler, stmt->s_expr.expr);
}

void Resolver::visit_block_stmt(KauCompiler* compiler, Stmt* stmt) {
    begin_scope();
    resolve(compiler, stmt->s_block.stmts, stmt->s_block.size);
    end_scope();
}

void Resolver::visit_var_stmt(KauCompiler* compiler, Stmt* stmt) {
    declare(stmt->s_var_decl.name);
    if (stmt->s_var_decl.initializer != nullptr) {
        resolve_expr(compiler, stmt->s_var_decl.initializer);
    }
    define(stmt->s_var_decl.name);
}

void Resolver::visit_fn_stmt(KauCompiler* compiler, Stmt* stmt) {
    declare(stmt->fn_declaration.name);
    define(stmt->fn_declaration.name);

    resolve_fn(compiler, stmt);
}

void Resolver::visit_if_stmt(KauCompiler* compiler, Stmt* stmt) {
    resolve_expr(compiler, stmt->s_if.condition);
    resolve_stmt(compiler, stmt->s_if.if_stmt);
    if (stmt->s_if.else_stmt->ty != Stmt::Type::ERR) {
        resolve_stmt(compiler, stmt->s_if.else_stmt);
    }
}

void Resolver::visit_print_stmt(KauCompiler* compiler, Stmt* stmt) {
    // TODO: Print Stmt should be its own thing
    resolve_expr(compiler, stmt->s_expr.expr);
}

void Resolver::visit_return_stmt(KauCompiler* compiler, Stmt* stmt) {
    resolve_expr(compiler, stmt->s_return.expr);
}

void Resolver::visit_while_stmt(KauCompiler* compiler, Stmt* stmt) {
    resolve_expr(compiler, stmt->s_while.condition);
    resolve_stmt(compiler, stmt->s_while.body);
}

void Resolver::visit_variable_expr(KauCompiler* compiler, Expr* expr) {
    const Token* token = expr->expr.literal->val;
    if (!scopes.empty() &&
        scopes.back().contains(token->m_lexeme) &&
        scopes.back().at(token->m_lexeme) == false) {
        std::println(stderr, "Can't read local varaible in its own initializer");
        exit(-1);
    }
    resolve_local(compiler, expr, token);
}

void Resolver::visit_assign_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.assignment->right);
    resolve_local(compiler, expr, expr->expr.literal->val);
}

void Resolver::visit_binary_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.binary->left);
    resolve_expr(compiler, expr->expr.binary->right);
}

void Resolver::visit_fn_call_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.fn_call->callee);

    for (u64 i = 0; i < expr->expr.fn_call->arguments_len; ++i) {
        resolve_expr(compiler, expr->expr.fn_call->arguments[i]);
    }
}

void Resolver::visit_grouping_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.grouping->expr);
}

void Resolver::visit_logical_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.logical_binary->left);
    resolve_expr(compiler, expr->expr.logical_binary->right);
}

void Resolver::visit_unary_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.unary->right);
}

void Resolver::visit_ternary_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.ternary->left);
    resolve_expr(compiler, expr->expr.ternary->middle);
    resolve_expr(compiler, expr->expr.ternary->right);
}

void Resolver::resolve_local(KauCompiler* compiler, Expr* expr, const Token* token) {
    for (i64 i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i].contains(token->m_lexeme)) {
            mark_resolved(compiler, expr, scopes.size() - 1 - i);
            return;
        }
    }
}

void Resolver::resolve_fn(KauCompiler* compiler, Stmt* stmt) {
    begin_scope();
    for (u64 i = 0; i < stmt->fn_declaration.params_size; ++i) {
        Token* param = stmt->fn_declaration.params[i];
        declare(param);
        define(param);
    }
    resolve_stmt(compiler, stmt->fn_declaration.body);
    end_scope();
}

void Resolver::declare(Token* name) {
    if (scopes.empty()) {
        return;
    }

    scopes.back()[name->m_lexeme] = false;
}

void Resolver::define(Token* name) {
    if (scopes.empty()) {
        return;
    }

    scopes.back()[name->m_lexeme] = true;
}

void Resolver::begin_scope() {
    scopes.push_back(ScopeMap());
}

void Resolver::end_scope() {
    scopes.pop_back();
}

void Resolver::mark_resolved(KauCompiler* compiler, Expr* expr, int depth) {
    compiler->locals[expr] = depth;
}