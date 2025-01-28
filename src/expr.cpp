#include "expr.h"
#include "defs.h"
#include "environment.h"
#include "compiler.h"

#define TEST_BINARY_OP(VALUE_IN_TYPE, VALUE_IN_FIELD, VALUE_OUT_TYPE, VALUE_OUT_FIELD, OPERATOR) {\
    if (left_val.ty == Value::Type::VALUE_IN_TYPE) {\
        in_value = Value {\
            .ty = Value::Type::VALUE_OUT_TYPE,\
            .VALUE_OUT_FIELD = left_val.VALUE_IN_FIELD OPERATOR right_val.VALUE_IN_FIELD\
        };\
        return RuntimeError::ok();\
    }\
}

namespace {
    bool is_numeric(Value::Type ty) {
        return ty == Value::Type::DOUBLE ||
            ty == Value::Type::FLOAT ||
            ty == Value::Type::LONG ||
            ty == Value::Type::INT;
    }

    Value::Type highest_precision_type(Value::Type a, Value::Type b) {
        assert(is_numeric(a) && is_numeric(b));

        if (a == Value::Type::DOUBLE || b == Value::Type::DOUBLE) {
            return Value::Type::DOUBLE;
        }
        if (a == Value::Type::FLOAT || b == Value::Type::FLOAT) {
            return Value::Type::FLOAT;
        }
        if (a == Value::Type::LONG || b == Value::Type::LONG) {
            return Value::Type::LONG;
        }
        return Value::Type::INT;
    }
};

RuntimeError RuntimeError::ok() {
    return RuntimeError {
        .ty = Type::Ok
    };
}

RuntimeError RuntimeError::unsupported_literal(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_LITERAL,
        .token = token,
        .message = "Unsupported literal"
    };
}

RuntimeError RuntimeError::unsupported_binary_op(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Unsupported binary operation"
    };
}

RuntimeError RuntimeError::unsupported_unary_op(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Unsupported unary operation"
    };
}

RuntimeError RuntimeError::operands_must_be_equal(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be equal"
    };
}

RuntimeError RuntimeError::operands_must_be_floats(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be floats"
    };
}

RuntimeError RuntimeError::operands_must_be_strings(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be strings"
    };
}

RuntimeError RuntimeError::operand_must_be_float(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operand must be float"
    };
}

RuntimeError RuntimeError::operand_must_be_bool(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operand must be bool"
    };
}

RuntimeError RuntimeError::divide_by_zero(const Token* token) {
    return RuntimeError {
        .ty = Type::DIVIDE_BY_ZERO,
        .token = token,
        .message = "Divide by zero"
    };
}

RuntimeError RuntimeError::operands_do_not_support_operator(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Operands do not support operator"
    };
}

RuntimeError RuntimeError::undeclared_variable(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = "Undeclared variable"
    };
}

RuntimeError RuntimeError::undefined_variable(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = "Undefined variable"
    };
}

RuntimeError RuntimeError::undeclared_function(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = "Undeclared function"
    };
}

RuntimeError RuntimeError::invalid_function_identifier(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_IDENTIFIER,
        .token = token,
        .message = "invalid identifier"
    };
}

RuntimeError RuntimeError::invalid_function_argument(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = "invalid argument"
    };
}

RuntimeError RuntimeError::object_must_be_struct(const Token* token){
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = "object must be struct"
    };
}

RuntimeError RuntimeError::class_does_not_have_field(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = "class does not have field"
    };
}

// TODO: error messages will be better later, need to add a string param
// so i can set `expected` and `found` numbers
RuntimeError RuntimeError::wrong_number_arguments(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_NUMBER_ARGUMENTS,
        .token = token,
        .message = "wrong number of arguments"
    };
}

bool RuntimeError::is_ok() const {
    return ty == Type::Ok;
}

void Expr::print() const {
    switch (ty)
    {
        case Type::LITERAL: {
            LiteralExpr* literal = expr.literal;

            literal->val->print();

            break;
        }
        case Type::UNARY: {
            UnaryExpr* unary = expr.unary;

            unary->op->print();
            std::print(" ");
            unary->right->print();

            break;
        }
        case Type::BINARY: {
            BinaryExpr* binary = expr.binary;

            binary->left->print();
            std::print(" ");
            binary->op->print();
            std::print(" ");
            binary->right->print();

            break;
        }
        case Type::GROUPING: {
            GroupingExpr* grouping = expr.grouping;

            grouping->expr->print();

            break;
        }
        case Type::TERNARY: {
            TernaryExpr* ternary = expr.ternary;

            ternary->left->print();
            std::print(" ? ");
            ternary->middle->print();
            std::print(" : ");
            ternary->right->print();

            break;
        }
        case Type::ASSIGNMENT: {
            AssignmentExpr* assingment = expr.assignment;

            assingment->id->print();
            std::print(" = ");
            assingment->right->print();

            break;
        }
        case Type::AND: {
            LogicalBinaryExpr* logical_and = expr.logical_binary;

            logical_and->left->print();
            std::print(" and ");
            logical_and->right->print();

            break;
        }
        case Type::OR: {
            LogicalBinaryExpr* logical_or = expr.logical_binary;

            logical_or->left->print();
            std::print(" or ");
            logical_or->right->print();

            break;
        }
        case Type::FN_CALL: {
            FnCallExpr* fn_call = expr.fn_call;

            fn_call->callee->print();
            std::print("(");
            for (size_t i = 0; i < fn_call->arguments_count; ++i) {
                fn_call->arguments[i]->print();
                if (i != fn_call->arguments_count - 1) {
                    std::print(", ");
                }
            }
            std::print(")");

            break;
        }
        case Type::GET: {
            GetExpr* get = expr.get;

            get->class_expr->print();
            std::print(".");
            get->member->print();
            std::println();

            break;
        }
    }
}

RuntimeError Expr::evaluate(KauCompiler* compiler, Arena* arena, Environment* env, Value& in_value) {
    switch (ty)
    {
        case Type::LITERAL: {
            LiteralExpr* literal = expr.literal;
            switch (literal->val->m_type)
            {
                case TokenType::FALSE: {
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = false
                    };
                    return RuntimeError::ok();
                }
                case TokenType::TRUE: {
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = true
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NIL: {
                    in_value = Value {
                        .ty = Value::Type::NIL,
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NUMBER_INT: {
                    in_value = Value {
                        .ty = Value::Type::INT,
                        .i = literal->val->data.data.i
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NUMBER_LONG: {
                    in_value = Value {
                        .ty = Value::Type::LONG,
                        .l = literal->val->data.data.l
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NUMBER_FLOAT: {
                    in_value = Value {
                        .ty = Value::Type::FLOAT,
                        .f = literal->val->data.data.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NUMBER_DOUBLE: {
                    in_value = Value {
                        .ty = Value::Type::DOUBLE,
                        .d = literal->val->data.data.d
                    };
                    return RuntimeError::ok();
                }
                case TokenType::STRING: {
                    in_value = Value {
                        .ty = Value::Type::STRING,
                        .str = literal->val->m_lexeme
                    };
                    return RuntimeError::ok();
                }
                case TokenType::IDENTIFIER: {
                    return compiler->lookup_variable(env, literal->val, this, in_value);
                }
                default:  {
                    return RuntimeError::unsupported_literal(literal->val);
                }
            }

            return RuntimeError::ok();
        }
        case Type::UNARY: {
            UnaryExpr* unary = expr.unary;

            Value right_val = {};
            RuntimeError right_err = unary->right->evaluate(compiler, arena, env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            switch (unary->op->m_type)
            {
                case TokenType::BANG: {
                    if(right_val.ty != Value::Type::BOOL) {
                        return RuntimeError::operand_must_be_bool(unary->op);
                    }
                    right_val.b = !right_val.b;
                    break;
                }
                case TokenType::MINUS: {
                    if(right_val.ty == Value::Type::FLOAT) {
                        return RuntimeError::operand_must_be_float(unary->op);
                    }
                    right_val.f = -right_val.f;
                    break;
                }
                default: {
                    return RuntimeError::unsupported_unary_op(unary->op);
                }
            }

            in_value = right_val;
            return RuntimeError::ok();
        }
        case Type::BINARY: {
            BinaryExpr* binary = expr.binary;

            Value left_val = {};
            RuntimeError left_err = binary->left->evaluate(compiler, arena, env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            Value right_val = {};
            RuntimeError right_err = binary->right->evaluate(compiler, arena, env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            switch (binary->op->m_type)
            {
                case TokenType::PLUS: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, FLOAT, f, +);
                    TEST_BINARY_OP(DOUBLE, d, DOUBLE, d, +);
                    TEST_BINARY_OP(INT, i, INT, i, +);

                    if (left_val.ty == Value::Type::STRING) {
                        in_value = Value {
                            .ty = Value::Type::STRING,
                            .str = concatenated_string(arena, left_val.str, right_val.str)
                        };
                        return RuntimeError::ok();
                    }

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::MINUS: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, FLOAT, f, -);
                    TEST_BINARY_OP(DOUBLE, d, DOUBLE, d, -);
                    TEST_BINARY_OP(INT, i, INT, i, -);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::SLASH: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    if (left_val.ty == Value::Type::FLOAT) {
                        if (right_val.f == 0.0) {
                            return RuntimeError::divide_by_zero(binary->op);
                        }

                        in_value = Value {
                            .ty = Value::Type::FLOAT,
                            .f = left_val.f / right_val.f
                        };
                        return RuntimeError::ok();
                    }

                    if (left_val.ty == Value::Type::DOUBLE) {
                        if (right_val.d == 0.0) {
                            return RuntimeError::divide_by_zero(binary->op);
                        }

                        in_value = Value {
                            .ty = Value::Type::DOUBLE,
                            .d = left_val.d / right_val.d
                        };
                        return RuntimeError::ok();
                    }

                    if (left_val.ty == Value::Type::INT) {
                        if (right_val.i == 0) {
                            return RuntimeError::divide_by_zero(binary->op);
                        }
                        
                        in_value = Value {
                            .ty = Value::Type::INT,
                            .i = left_val.i / right_val.i
                        };
                        return RuntimeError::ok();
                    }

                    if (left_val.ty == Value::Type::LONG) {
                        if (right_val.l == 0) {
                            return RuntimeError::divide_by_zero(binary->op);
                        }
                        
                        in_value = Value {
                            .ty = Value::Type::LONG,
                            .l = left_val.l / right_val.l
                        };
                        return RuntimeError::ok();
                    }

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::STAR: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, FLOAT, f, *);
                    TEST_BINARY_OP(DOUBLE, d, DOUBLE, d, *);
                    TEST_BINARY_OP(INT, i, INT, i, *);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::GREATER: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, >);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, >);
                    TEST_BINARY_OP(INT, i, BOOL, b, >);
                    TEST_BINARY_OP(STRING, str, BOOL, b, >);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::GREATER_EQUAL: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, >=);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, >=);
                    TEST_BINARY_OP(INT, i, BOOL, b, >=);
                    TEST_BINARY_OP(STRING, str, BOOL, b, >=);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::LESSER: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, <);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, <);
                    TEST_BINARY_OP(INT, i, BOOL, b, <);
                    TEST_BINARY_OP(STRING, str, BOOL, b, <);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::LESSER_EQUAL: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, <=);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, <=);
                    TEST_BINARY_OP(INT, i, BOOL, b, <=);
                    TEST_BINARY_OP(STRING, str, BOOL, b, <=);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::BANG_EQUAL: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, !=);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, !=);
                    TEST_BINARY_OP(INT, i, BOOL, b, !=);
                    TEST_BINARY_OP(STRING, str, BOOL, b, !=);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                case TokenType::EQUAL_EQUAL: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, BOOL, b, ==);
                    TEST_BINARY_OP(DOUBLE, d, BOOL, b, ==);
                    TEST_BINARY_OP(INT, i, BOOL, b, ==);
                    TEST_BINARY_OP(STRING, str, BOOL, b, ==);

                    return RuntimeError::operands_do_not_support_operator(binary->op);
                }
                default: {
                    return RuntimeError::unsupported_binary_op(binary->op);
                }
            }
        }
        case Type::GROUPING: {
            GroupingExpr* grouping = expr.grouping;

            Value val = {};
            RuntimeError err = grouping->expr->evaluate(compiler, arena, env, val);
            if (!err.is_ok()) {
                return err;
            }

            return RuntimeError::ok();
        }
        case Type::TERNARY: {
            TernaryExpr* ternary = expr.ternary;

            Value left_val = {};
            RuntimeError left_err = ternary->left->evaluate(compiler, arena, env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(ternary->left_op);
            }

            if (left_val.b) {
                Value middle_val = {};
                RuntimeError middle_err = ternary->middle->evaluate(compiler, arena, env, middle_val);
                if (!middle_err.is_ok()) {
                    return middle_err;
                }

                in_value = middle_val;

                return RuntimeError::ok();
            } else {
                Value right_val = {};
                RuntimeError right_err = ternary->right->evaluate(compiler, arena, env, right_val);
                if (!right_err.is_ok()) {
                    return right_err;
                }

                in_value = right_val;
                
                return RuntimeError::ok();
            }

            return RuntimeError::ok();
        }
        case Type::ASSIGNMENT: {
            AssignmentExpr* assignment = expr.assignment;

            Value right_val = {};
            RuntimeError right_err = assignment->right->evaluate(compiler, arena, env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            in_value = right_val;

            if (env->contains(assignment->id)) {
                env->set(assignment->id, right_val);
            } else {
                return RuntimeError::undefined_variable(assignment->id);
            }
            

            return RuntimeError::ok();
        }
        case Type::AND: {
            LogicalBinaryExpr* logical_and = expr.logical_binary;

            Value left_val = {};
            RuntimeError left_err = logical_and->left->evaluate(compiler, arena, env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }
            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(logical_and->op);
            }

            in_value = left_val;
            // TODO: so far this allow the right-side expression to evaluate to something other
            // than bool in the case the left-side is false. Think about that.
            if (left_val.b == true) {
                Value right_val = {};
                RuntimeError right_err = logical_and->right->evaluate(compiler, arena, env, right_val);
                if (!right_err.is_ok()) {
                    return right_err;
                }
                if (right_val.ty != Value::Type::BOOL) {
                    return RuntimeError::operand_must_be_bool(logical_and->op);
                }

                in_value = right_val;
            }

            return RuntimeError::ok();
        }
        case Type::OR: {
            LogicalBinaryExpr* logical_or = expr.logical_binary;

            Value left_val = {};
            RuntimeError left_err = logical_or->left->evaluate(compiler, arena, env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }
            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(logical_or->op);
            }

            Value right_val = {};
            RuntimeError right_err = logical_or->right->evaluate(compiler, arena, env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }
            if (right_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(logical_or->op);
            }

            in_value = Value {
                .ty = Value::Type::BOOL,
                .b = left_val.b || right_val.b
            };

            return RuntimeError::ok();
        }
        case Type::FN_CALL:  {
            FnCallExpr* fn_call = expr.fn_call;
            
            Expr* callee = fn_call->callee;
            LiteralExpr* callee_literal = callee->expr.literal;
            if (callee_literal->val->m_type != TokenType::IDENTIFIER) {
                return RuntimeError::invalid_function_identifier(callee_literal->val);
            }
    
            Callable callable = {};
            RuntimeError err = env->get_callable(callee_literal->val, callable);
            if (!err.is_ok()) {
                return err;
            }

            if (callable.m_arity != fn_call->arguments_count) {
                return RuntimeError::wrong_number_arguments(callee_literal->val);
            }

            std::vector<Value> values;
            values.resize(fn_call->arguments_count);
            for (size_t i = 0; i < fn_call->arguments_count; ++i) {
                Value arg_val = {};
                RuntimeError err = fn_call->arguments[i]->evaluate(compiler, arena, env, arg_val);
                if (!err.is_ok()) {
                    return RuntimeError::invalid_function_argument(callee_literal->val);
                }
                values[i] = arg_val;
            }
            
            
            const Value ret_value = callable.m_callback(values, compiler, arena, env);
            in_value = ret_value;
            
            compiler->hit_return = false;

            return RuntimeError::ok();
        }
        case Type::GET: {
            GetExpr* get = expr.get;

            Value expr_val = {};
            RuntimeError expr_err = get->class_expr->evaluate(compiler, arena, env, expr_val);
            if (!expr_err.is_ok()) {
                return expr_err;
            }
            if (expr_val.ty != Value::Type::CLASS) {
                return RuntimeError::object_must_be_struct(get->member);
            }

            const bool has_field = expr_val.m_class.get(get->member->m_lexeme, in_value);
            if (has_field) {
                return RuntimeError::ok();
            } else {
                return RuntimeError::class_does_not_have_field(get->member);
            }
        }
    }
}

void Value::print() const {
    switch (ty)
    {
        case Type::NIL: {
            std::println("nil");
            break;
        }
        case Type::BOOL: {
            std::println("{}", b);
            break;
        }
        case Type::FLOAT: {
            std::println("{}", f);
            break;
        }
        case Type::DOUBLE: {
            std::println("{}", d);
            break;
        }
        case Type::INT: {
            std::println("{}", i);
            break;
        }
        case Type::LONG: {
            std::println("{}", l);
            break;
        }
        case Type::STRING: {
            std::println("{}", str.to_string_view());
            break;
        }
        case Type::CLASS: {
            m_class.print();
        }
    }
}

Value Stmt::evaluate(KauCompiler* compiler, Arena* arena, Environment* env, bool from_prompt, bool in_loop) {
    Value expr_val = {};

    switch (ty)
    {
        case Stmt::Type::EXPR: {
            RuntimeError expr_err = s_expr.expr->evaluate(compiler, arena, env, expr_val);
            if (!expr_err.is_ok()) {
                compiler->runtime_error(expr_err.token->m_line, expr_err.message);
            }
            break;
        }
        case Stmt::Type::VAR_DECL: {
            if (s_var_decl.initializer != nullptr) {
                RuntimeError expr_err = s_var_decl.initializer->evaluate(compiler, arena, env, expr_val);
                if (!expr_err.is_ok()) {
                    compiler->runtime_error(expr_err.token->m_line, expr_err.message);
                }
            }
            env->define(s_var_decl.name, expr_val);
            break;
        }
        case Stmt::Type::BLOCK: {
            Environment new_env = {};
            new_env.enclosing = env;
            for (int i = 0; i < s_block.size; ++i) {
                expr_val = s_block.stmts[i].evaluate(compiler, arena, &new_env, from_prompt, in_loop);
                // continue statement stops current block from exeuting further, like a break.
                if (expr_val.ty == Value::Type::BREAK ||
                    expr_val.ty == Value::Type::CONTINUE ||
                    compiler->hit_return
                ) {
                    break;
                }
            }
            break;
        }
        case Stmt::Type::IF: {
            Value test_expr_val = {};
            RuntimeError expr_err = s_if.condition->evaluate(compiler, arena, env, test_expr_val);
            if (!expr_err.is_ok()) {
                compiler->runtime_error(expr_err.token->m_line, expr_err.message);
            }
            if (test_expr_val.ty != Value::Type::BOOL) {
                compiler->runtime_error(expr_err.token->m_line, "if test expression must evaluate to bool");
            }
            bool if_result = test_expr_val.b;

            if (if_result) {
                expr_val = s_if.if_stmt->evaluate(compiler, arena, env, from_prompt, in_loop);
            } else if (s_if.else_stmt->ty != Stmt::Type::ERR) {
                expr_val = s_if.else_stmt->evaluate(compiler, arena, env, from_prompt, in_loop);
            }

            break;
        }
        case Stmt::Type::WHILE: {
            while (true) {
                Value test_expr_val = {};
                RuntimeError expr_err = s_while.condition->evaluate(compiler, arena, env, test_expr_val);
                if (!expr_err.is_ok()) {
                    compiler->runtime_error(expr_err.token->m_line, expr_err.message);
                }
                if (test_expr_val.ty != Value::Type::BOOL) {
                    compiler->runtime_error(expr_err.token->m_line, "while test expression must evaluate to bool");
                }
                if(!test_expr_val.b) {
                    break;
                }

                expr_val = s_while.body->evaluate(compiler, arena, env, from_prompt, true);
                if (expr_val.ty == Value::Type::BREAK || compiler->hit_return) {
                    break;
                }
            }
            break;
        }
        case Stmt::Type::BREAK: {
            if (!in_loop) {
                compiler->runtime_error(s_break_continue.line, "'break' statement can only be used in a loop.");
            }
            expr_val = Value {
                .ty = Value::Type::BREAK
            };
            break;
        }
        case Stmt::Type::CONTINUE: {
            if (!in_loop) {
                compiler->runtime_error(s_break_continue.line, "'continue' statement can only be used in a loop.");
            }
            expr_val = Value {
                .ty = Value::Type::CONTINUE
            };
            break;
        }
        case Stmt::Type::FN_DECLARATION: {
            String fn_name = fn_declaration.name->m_lexeme;
            FnDeclarationPayload fn = fn_declaration;

            env->define_callable(fn_name, Callable(fn_declaration.params_count, [fn = std::move(fn)](std::vector<Value> const& args, KauCompiler* compiler, Arena* arena, Environment* env) {
                Environment new_env = {};
                new_env.enclosing = env;

                for (size_t i = 0; i < fn.params_count; ++i) {
                    new_env.define(fn.params[i], args[i]);
                }

                return fn.body->evaluate(compiler, arena, &new_env, false, false);
            }));

            break;
        }
        case Stmt::Type::CLASS_DECLARATION: {
            Token* class_name_token = s_class.name;
            String class_name = class_name_token->m_lexeme;

            env->define_class(class_name_token, Class(class_name));

            env->define_callable(class_name, Callable(0, [class_name_token](std::vector<Value> const& args, KauCompiler* compiler, Arena* arena, Environment* env) {
                Class in_class = {};
                env->get_class(class_name_token, in_class);

                return Value{
                    .ty = Value::Type::CLASS,
                    .m_class = in_class,
                };
            }));

            break;
        }
        case Stmt::Type::RETURN: {
            RuntimeError expr_err = s_return.expr->evaluate(compiler, arena, env, expr_val);
            if (!expr_err.is_ok()) {
                compiler->runtime_error(expr_err.token->m_line, expr_err.message);
            }
            compiler->hit_return = true;
            break;
        }
        case Stmt::Type::ERR: {
            assert(false);
            break;
        }
    }

    if (from_prompt) {
        expr_val.print();
    }

    return expr_val;
}

void Stmt::print() {
    // TODO: Review some of this stuff, its not great
    switch (ty)
    {
        case Type::VAR_DECL: {
            std::print("VAR DECL: ");
            std::print("{}", s_var_decl.name->m_lexeme.to_string_view());
            if (s_var_decl.initializer != nullptr) {
                std::print(" = ");
                s_var_decl.initializer->print();
            }
            std::println("");
            break;
        }
        case Type::EXPR: {
            std::print("EXPR: ");
            s_expr.expr->print();
            std::println("");
            break;
        }
        case Type::BLOCK: {
            std::println("BLOCK");
            for (int i = 0; i < s_block.size; ++i) {
                s_block.stmts[i].print();
            }
            break;
        }
        case Type::IF: {
            std::print("IF: ");
            s_if.if_stmt->print();
            if (s_if.else_stmt->ty != Stmt::Type::ERR) {
                s_if.else_stmt->print();
            }
            break;
        }
        case Type::WHILE: {
            std::print("WHILE: ");
            s_while.body->print();
            break;
        }
        case Type::BREAK: {
            std::println("BREAK");
            break;
        }
        case Type::CONTINUE: {
            std::println("CONTINUE");
            break;
        }
        case Type::RETURN: {
            std::print("RETURN");
            s_return.expr->print();
            std::println("");
            break;
        }
        case Type::FN_DECLARATION: {
            std::print("FN DECLARATION: ");
            std::print("{} params:", fn_declaration.name->m_lexeme.to_string_view());
            for (u64 i = 0; i < fn_declaration.params_count; ++i) {
                std::print(" {}", fn_declaration.params[i]->m_lexeme.to_string_view());
            }
            std::println();
            break;
        }
        case Type::CLASS_DECLARATION: {
            std::print("CLASS DECLARATION: ");
            std::println("{}", s_class.name->m_lexeme.to_string_view());
            for (u64 i = 0; i < s_class.methods_count; ++i) {
                s_class.methods[i].print();
            }
            std::println();
            break;
        }
        //
        case Type::ERR: {
            std::println("ERR");
            return;
        }
    }
}

bool Class::get(String field, Value& in_value) {
    if (fields.contains(field)) {
        in_value = *((Value*)fields.get(field));
        return true;
    } else {
        return false;
    }
}