#include "resolver.h"

#include "compiler.h"

void Resolver::resolve(KauCompiler* compiler, Stmt* stmts, u64 stmts_count) {
    for (u64 i = 0; i < stmts_count; ++i) {
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
        case Stmt::Type::CLASS_DECLARATION: {
            visit_class_stmt(compiler, stmt);
            break;
        }
        case Stmt::Type::RETURN: {
            visit_return_stmt(compiler, stmt);
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
        case Expr::Type::STATIC_FN_CALL: {
            // TODO
            break;
        }
        case Expr::Type::GET: {
            visit_get_expr(compiler, expr);
            break;
        }
        case Expr::Type::SET: {
            visit_set_expr(compiler, expr);
            break;
        }
        case Expr::Type::THIS: {
            visit_this_expr(compiler, expr);
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
    declare(compiler, stmt->s_var_decl.name);
    if (stmt->s_var_decl.initializer != nullptr) {
        resolve_expr(compiler, stmt->s_var_decl.initializer);
    }
    define(stmt->s_var_decl.name);
}

void Resolver::visit_fn_stmt(KauCompiler* compiler, Stmt* stmt) {
    declare(compiler, stmt->fn_declaration.name);
    define(stmt->fn_declaration.name);

    resolve_fn(compiler, stmt,  FunctionType::FUNCTION);
}

void Resolver::visit_class_stmt(KauCompiler* compiler, Stmt* stmt) {
    ClassType enclosing_class = current_class;
    current_class = ClassType::CLASS;

    declare(compiler, stmt->s_class.name);
    define(stmt->s_class.name);

    begin_scope();

    String this_str = String {
        "this",
        4
    };
    scopes.back()[this_str] = 
    VariableStatus {
        .defined = true,
        .uses = 1
    };

    for (u64 i = 0; i < stmt->s_class.members_count; ++i) {
        Stmt* class_stmt = &stmt->s_class.members[i];
        if (class_stmt->ty == Stmt::Type::FN_DECLARATION) {
            FunctionType fn_type = FunctionType::METHOD;
            if (class_stmt->fn_declaration.name->m_lexeme == String{"init", 4}) {
                fn_type = FunctionType::INITIALIZER;
            }
            resolve_fn(compiler, class_stmt, fn_type);
        } else if (class_stmt->ty == Stmt::Type::VAR_DECL) {
            // TODO: Think about this.
        } else {
            assert(false);
        }
        
    }

    end_scope();

    current_class = enclosing_class;
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
    if (current_function == FunctionType::NONE) {
        // TODO: Get actual line number
        compiler->error(0, "Can't return from top-level code");
        exit(-1);
    }

    if (stmt->s_return.expr != nullptr) {
        if (current_function == FunctionType::INITIALIZER) {
            // TODO: Get actual line number
            compiler->error(0, "Can't return value fron initializer");
            exit(-1);
        }
        resolve_expr(compiler, stmt->s_return.expr);
    }
}

void Resolver::visit_while_stmt(KauCompiler* compiler, Stmt* stmt) {
    resolve_expr(compiler, stmt->s_while.condition);
    resolve_stmt(compiler, stmt->s_while.body);
}

void Resolver::visit_variable_expr(KauCompiler* compiler, Expr* expr) {
    const Token* token = expr->expr.literal->val;
    if (!scopes.empty() &&
        scopes.back().contains(token->m_lexeme) &&
        scopes.back().at(token->m_lexeme).defined == false) {
        compiler->error(0, "Can't read local varaible in its own initializer");
        return;
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

    for (u64 i = 0; i < expr->expr.fn_call->arguments_count; ++i) {
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

void Resolver::visit_get_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.get->class_expr);
}

void Resolver::visit_set_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.set->get);
    resolve_expr(compiler, expr->expr.set->right);
}

void Resolver::visit_this_expr(KauCompiler* compiler, Expr* expr) {
    if (current_class == ClassType::NONE) {
        compiler->error(0, "Can't use `this` outside of class");
        return;
    }
    resolve_local(compiler, expr, expr->expr.this_expr->val);
}

void Resolver::resolve_local(KauCompiler* compiler, Expr* expr, const Token* token) {
    for (i64 i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i].contains(token->m_lexeme)) {
            mark_resolved(compiler, expr, scopes.size() - 1 - i);
            scopes[i][token->m_lexeme].uses += 1;
            return;
        }
    }
}

void Resolver::resolve_fn(KauCompiler* compiler, Stmt* stmt, FunctionType ty) {
    const FunctionType enclosing_function = current_function;
    current_function = ty;

    begin_scope();
    for (u64 i = 0; i < stmt->fn_declaration.params_count; ++i) {
        Token* param = stmt->fn_declaration.params[i];
        declare(compiler, param);
        define(param);
    }
    resolve_stmt(compiler, stmt->fn_declaration.body);
    end_scope();

    current_function = enclosing_function;
}

void Resolver::declare(KauCompiler* compiler, Token* name) {
    if (scopes.empty()) {
        return;
    }

    ScopeMap& scope = scopes.back();
    if (scope.contains(name->m_lexeme)) {
        compiler->error(0, "Already a variable with this name in this scope");
        return;
    }
    scope[name->m_lexeme] = VariableStatus{
        .defined = false,
        .uses = 0,
    };
}

void Resolver::define(Token* name) {
    if (scopes.empty()) {
        return;
    }

    VariableStatus& status = scopes.back()[name->m_lexeme];
    status.defined = true;
}

void Resolver::begin_scope() {
    scopes.push_back(ScopeMap());
}

void Resolver::end_scope() {
    for (auto const& [name, status] : scopes.back()) {
        if (status.uses == 0) {
            std::println("Warn: unused variable {}", name.to_string_view());
        }
    }
    scopes.pop_back();
}

void Resolver::mark_resolved(KauCompiler* compiler, Expr* expr, int depth) {
    compiler->locals[expr] = depth;
}