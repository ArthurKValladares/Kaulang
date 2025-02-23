#include "resolver.h"

#include "compiler.h"

void Resolver::init(Arena* arena) {
    scopes.init(arena);
}

void Resolver::resolve(KauCompiler* compiler, Array<Stmt> stmts) {
    for (u64 i = 0; i < stmts.size(); ++i) {
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
            visit_static_fn_call_expr(compiler, expr);
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
        case Expr::Type::SUPER: {
            visit_super_expr(compiler, expr);
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
    begin_scope(compiler);
    resolve(compiler, stmt->s_block.stmts);
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

    begin_scope(compiler);

    if (stmt->s_class.superclass != nullptr) {
        if (stmt->s_class.name->m_lexeme == stmt->s_class.superclass->expr.literal->val->m_lexeme) {
            compiler->error(stmt->s_class.superclass->expr.literal->val->m_line, CREATE_STRING("Class can't inherit from itself"));
            exit(-1);
        }

        current_class = ClassType::SUBCLASS;

        resolve_expr(compiler, stmt->s_class.superclass);

        String super_str = CREATE_STRING("super");
        VariableStatus* status = (VariableStatus*) compiler->global_arena->push_struct_no_zero<VariableStatus>();
        *status = VariableStatus {
            .defined = true,
            .uses = 1
        };
        scopes.back().insert(compiler->global_arena, (void*) &super_str, sizeof(String), HASH_STR(super_str), status); 
    }
    
    String this_str = CREATE_STRING("this");
    VariableStatus* status = (VariableStatus*) compiler->global_arena->push_struct_no_zero<VariableStatus>();
    *status = VariableStatus {
        .defined = true,
        .uses = 1
    };
    scopes.back().insert(compiler->global_arena, (void*) &this_str, sizeof(String), HASH_STR(this_str), status); 

    for (u64 i = 0; i < stmt->s_class.members.size(); ++i) {
        Stmt* class_stmt = &stmt->s_class.members[i];
        if (class_stmt->ty == Stmt::Type::FN_DECLARATION) {
            FunctionType fn_type = FunctionType::METHOD;
            if (class_stmt->fn_declaration.name->m_lexeme == CREATE_STRING("init")) {
                fn_type = FunctionType::INITIALIZER;
            }
            resolve_fn(compiler, class_stmt, fn_type);
        } else if (class_stmt->ty == Stmt::Type::VAR_DECL) {
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

void Resolver::visit_return_stmt(KauCompiler* compiler, Stmt* stmt) {
    ReturnPayload& ret = stmt->s_return;

    if (current_function == FunctionType::NONE) {
        compiler->error(ret.keyword->m_line, CREATE_STRING("Can't return from top-level code"));
        exit(-1);
    }

    if (stmt->s_return.expr != nullptr) {
        if (current_function == FunctionType::INITIALIZER) {
            compiler->error(ret.keyword->m_line, CREATE_STRING("Can't return value fron initializer"));
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
    if (!scopes.empty()) {
        u64 hashed_str = HASH_STR(token->m_lexeme);

        Map& scope = scopes.back();
        VariableStatus* get = (VariableStatus*) scope.get(hashed_str);

        if (get != nullptr && get->defined == false) {
            compiler->error(token->m_line, CREATE_STRING("Can't read local varaible in its own initializer"));
            return;
        }
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

    for (u64 i = 0; i < expr->expr.fn_call->arguments.size(); ++i) {
        resolve_expr(compiler, expr->expr.fn_call->arguments[i]);
    }
}

void Resolver::visit_static_fn_call_expr(KauCompiler* compiler, Expr* expr) {
    resolve_expr(compiler, expr->expr.static_fn_call->class_expr);
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
    ThisExpr* this_expr = expr->expr.this_expr;
    if (current_class == ClassType::NONE) {
        compiler->error(this_expr->val->m_line, CREATE_STRING("Can't use `this` outside of class"));
        return;
    }
    resolve_local(compiler, expr, this_expr->val);
}

void Resolver::visit_super_expr(KauCompiler* compiler, Expr* expr) {
    SuperExpr* super_expr = expr->expr.super_expr;

    if (current_class == ClassType::NONE) {
        compiler->error(super_expr->keyword->m_line, CREATE_STRING("Can't use `super` outside of class"));
        return;
    } else if (current_class != ClassType::SUBCLASS) {
        compiler->error(super_expr->keyword->m_line, CREATE_STRING("Can't use `super` in a class with no superclass."));
        return;
    }
    resolve_local(compiler, expr, super_expr->keyword);
}

void Resolver::resolve_local(KauCompiler* compiler, Expr* expr, const Token* token) {
    for (i64 i = scopes.size() - 1; i >= 0; --i) {
        u64 hashed_lexeme = HASH_STR(token->m_lexeme);
        VariableStatus* get = (VariableStatus*) scopes[i].get(hashed_lexeme);
        if (get != nullptr) {
            mark_resolved(compiler, expr, scopes.size() - 1 - i);
            get->uses += 1;
            return;
        }
    }
}

void Resolver::resolve_fn(KauCompiler* compiler, Stmt* stmt, FunctionType ty) {
    const FunctionType enclosing_function = current_function;
    current_function = ty;

    begin_scope(compiler);
    for (u64 i = 0; i < stmt->fn_declaration.params.size(); ++i) {
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

    Map& scope = scopes.back();
    String str = name->m_lexeme;
    u64 hashed_lexeme = HASH_STR(str);
    VariableStatus* get = (VariableStatus*) scope.get(hashed_lexeme);
    if (get != nullptr) {
        compiler->error(name->m_line, CREATE_STRING("Already a variable with this name in this scope"));
        return;
    }
    VariableStatus* status = (VariableStatus*) compiler->global_arena->push_struct_no_zero<VariableStatus>();
    *status = VariableStatus{
        .defined = false,
        .uses = 0,
    };
    scope.insert(compiler->global_arena, (void*) &str, sizeof(String), hashed_lexeme, status);
}

void Resolver::define(Token* name) {
    if (scopes.empty()) {
        return;
    }

    Map& scope = scopes.back();
    VariableStatus* status = (VariableStatus*) scope.get(HASH_STR(name->m_lexeme));
    status->defined = true;
}

void Resolver::begin_scope(KauCompiler* compiler) {
    // TODO: This needs to be aligned
    Map* curr = scopes.curr_ptr();
    scopes.advance();

    *curr = Map();
    curr->allocate(compiler->global_arena);
}

void Resolver::end_scope() {
    Map& scope = scopes.back();
    for (u64 i = 0; i < scope.num_buckets; ++i) {
        MapNode* node = scope.buckets[i];
        while (node != nullptr) {
            VariableStatus* status = (VariableStatus*) node->value;
            if (status->uses == 0) { 
                String* fn_name = (String*) node->key;
                fprintf(stdout, "Warn: unused variable %.*s\n", (u32) fn_name->len, fn_name->chars);
            }
            node = node->next;
        }
    }
    
    scopes.pop();
}

void Resolver::mark_resolved(KauCompiler* compiler, Expr* expr, int depth) {
    u64* depth_ptr = (u64*) compiler->global_arena->push_struct_no_zero<u64>();
    *depth_ptr = depth;
    compiler->locals.insert(compiler->global_arena, depth_ptr, sizeof(u64), (u64) expr, depth_ptr);
}