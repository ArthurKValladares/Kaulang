#include "lib/span.h"

#include "expr.h"
#include "defs.h"
#include "environment.h"
#include "compiler.h"

#define TEST_BINARY_OP(VALUE_IN_TYPE, VALUE_IN_FIELD, VALUE_OUT_TYPE, VALUE_OUT_FIELD, OPERATOR) do {\
    if (left_val.ty == Value::Type::VALUE_IN_TYPE) {\
        in_value = Value {\
            .ty = Value::Type::VALUE_OUT_TYPE,\
            .VALUE_OUT_FIELD = left_val.VALUE_IN_FIELD OPERATOR right_val.VALUE_IN_FIELD\
        };\
        return RuntimeError::ok();\
    }\
} while(0)

#define CHECK_ERR(err) do {\
    RuntimeError err_binding = err;\
    if (!err_binding.is_ok()) {\
        return err_binding;\
    }\
} while(0)

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

    Callable construct_callable(FnDeclarationPayload fn_declaration) {
        String fn_name = fn_declaration.name->m_lexeme;

        return Callable(fn_declaration.params.size(), [fn_declaration](Array<Value> args, KauCompiler* compiler, Arena* arena, Environment* env) {
            Environment new_env = {};
            new_env.init(arena);
            new_env.enclosing = env;

            for (size_t i = 0; i < fn_declaration.params.size(); ++i) {
                new_env.define(arena, fn_declaration.params[i]->m_lexeme, args[i]);
            }

            return fn_declaration.body->evaluate(compiler, arena, &new_env, false, false);
        });
    }

    Callable construct_callable_class(FnDeclarationPayload fn_declaration, Class* class_ptr) {
        String fn_name = fn_declaration.name->m_lexeme;
        String this_str = CREATE_STRING("this");
        bool is_initializer = fn_name == this_str;

        return Callable(fn_declaration.params.size(), [fn_declaration, class_ptr, this_str, is_initializer](Array<Value> args, KauCompiler* compiler, Arena* arena, Environment* env) {
            env->define(
                arena,
                this_str,
                Value{
                    .ty = Value::Type::CLASS,
                    .m_class = class_ptr
                }
            );
            if (class_ptr->superclass != nullptr) {
                String super_str = CREATE_STRING("super");

                env->define(
                    arena,
                    super_str,
                    Value{
                        .ty = Value::Type::CLASS,
                        .m_class = class_ptr->superclass
                    }
                );
            }

            Environment new_env = {};
            new_env.init(arena);
            new_env.enclosing = env;

            for (size_t i = 0; i < fn_declaration.params.size(); ++i) {
                new_env.define(arena, fn_declaration.params[i]->m_lexeme, args[i]);
            }

            Value body_val = fn_declaration.body->evaluate(compiler, arena, &new_env, false, false);
            if (is_initializer) {
                return *env->get(this_str);
            } else {
                return body_val;
            }
        });
    }

    String mangled_name(Arena* arena, String left, String right) {
        const String dot = CREATE_STRING(".");
        const String* strings[3] = {
            &left,
            &dot,
            &right
        };
        Span<const String*> strings_span = Span<const String*>(strings, 3);
        return concatenated_strings(arena, strings_span);
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
        .message = CREATE_STRING("Unsupported literal")
    };
}

RuntimeError RuntimeError::unsupported_binary_op(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = CREATE_STRING("Unsupported binary operation")
    };
}

RuntimeError RuntimeError::unsupported_unary_op(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = CREATE_STRING("Unsupported unary operation")
    };
}

RuntimeError RuntimeError::operands_must_be_equal(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = CREATE_STRING("Operands must be equal")
    };
}

RuntimeError RuntimeError::operands_must_be_floats(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = CREATE_STRING("Operands must be floats")
    };
}

RuntimeError RuntimeError::operands_must_be_strings(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = CREATE_STRING("Operands must be strings")
    };
}

RuntimeError RuntimeError::operand_must_be_float(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = CREATE_STRING("Operand must be float")
    };
}

RuntimeError RuntimeError::operand_must_be_bool(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = CREATE_STRING("Operand must be bool")
    };
}

RuntimeError RuntimeError::divide_by_zero(const Token* token) {
    return RuntimeError {
        .ty = Type::DIVIDE_BY_ZERO,
        .token = token,
        .message = CREATE_STRING("Divide by zero")
    };
}

RuntimeError RuntimeError::operands_do_not_support_operator(const Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = CREATE_STRING("Operands do not support operator")
    };
}

RuntimeError RuntimeError::undeclared_variable(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = CREATE_STRING("Undeclared variable")
    };
}

RuntimeError RuntimeError::undefined_variable(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = CREATE_STRING("Undefined variable")
    };
}

RuntimeError RuntimeError::undeclared_function(const Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = CREATE_STRING("Undeclared function")
    };
}

RuntimeError RuntimeError::invalid_function_identifier(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_IDENTIFIER,
        .token = token,
        .message = CREATE_STRING("invalid function identifier")
    };
}

RuntimeError RuntimeError::invalid_function_argument(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = CREATE_STRING("invalid argument")
    };
}

RuntimeError RuntimeError::object_must_be_struct(const Token* token){
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = CREATE_STRING("object must be struct")
    };
}

RuntimeError RuntimeError::class_does_not_have_field(const Token* token) {
    return RuntimeError {
        .ty = Type::INVALID_ARGUMENT,
        .token = token,
        .message = CREATE_STRING("class does not have field")
    };
}

// TODO: error messages will be better later, need to add a string param
// so i can set `expected` and `found` numbers
RuntimeError RuntimeError::wrong_number_arguments(const Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_NUMBER_ARGUMENTS,
        .token = token,
        .message = CREATE_STRING("wrong number of arguments")
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
            fprintf(stdout, " ");
            unary->right->print();

            break;
        }
        case Type::BINARY: {
            BinaryExpr* binary = expr.binary;

            binary->left->print();
            fprintf(stdout, " ");
            binary->op->print();
            fprintf(stdout, " ");
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
            fprintf(stdout, " ? ");
            ternary->middle->print();
            fprintf(stdout, " : ");
            ternary->right->print();

            break;
        }
        case Type::ASSIGNMENT: {
            AssignmentExpr* assingment = expr.assignment;

            assingment->id->print();
            fprintf(stdout, " = ");
            assingment->right->print();

            break;
        }
        case Type::AND: {
            LogicalBinaryExpr* logical_and = expr.logical_binary;

            logical_and->left->print();
            fprintf(stdout, " and ");
            logical_and->right->print();

            break;
        }
        case Type::OR: {
            LogicalBinaryExpr* logical_or = expr.logical_binary;

            logical_or->left->print();
            fprintf(stdout, " or ");
            logical_or->right->print();

            break;
        }
        case Type::FN_CALL: {
            FnCallExpr* fn_call = expr.fn_call;

            fn_call->callee->print();
            fprintf(stdout, "(");
            for (size_t i = 0; i < fn_call->arguments.size(); ++i) {
                fn_call->arguments[i]->print();
                if (i != fn_call->arguments.size() - 1) {
                    fprintf(stdout, ", ");
                }
            }
            fprintf(stdout, ")");

            break;
        }
        case Type::GET: {
            GetExpr* get = expr.get;

            get->class_expr->print();
            fprintf(stdout, ".");
            get->member->print();

            break;
        }
        case Type::SET: {
            SetExpr* set = expr.set;

            set->get->print();
            fprintf(stdout, " = ");
            set->right->print();

            break;
        }
        case Type::THIS: {
            fprintf(stdout, "this");
            break;
        }
        case Type::SUPER: {
            expr.super_expr->keyword->print();
            fprintf(stdout, ".");
            expr.super_expr->method->print();
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
            CHECK_ERR(unary->right->evaluate(compiler, arena, env, right_val));

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
            CHECK_ERR(binary->left->evaluate(compiler, arena, env, left_val));

            Value right_val = {};
            CHECK_ERR(binary->right->evaluate(compiler, arena, env, right_val));

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
            CHECK_ERR(ternary->left->evaluate(compiler, arena, env, left_val));

            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(ternary->left_op);
            }

            if (left_val.b) {
                Value middle_val = {};
                CHECK_ERR(ternary->middle->evaluate(compiler, arena, env, middle_val));

                in_value = middle_val;

                return RuntimeError::ok();
            } else {
                Value right_val = {};
                CHECK_ERR(ternary->right->evaluate(compiler, arena, env, right_val));

                in_value = right_val;
                
                return RuntimeError::ok();
            }

            return RuntimeError::ok();
        }
        case Type::ASSIGNMENT: {
            AssignmentExpr* assignment = expr.assignment;

            Value right_val = {};
            CHECK_ERR(assignment->right->evaluate(compiler, arena, env, right_val));

            in_value = right_val;

            if (env->contains(assignment->id->m_lexeme)) {
                const bool was_set = env->set(assignment->id->m_lexeme, right_val);
                assert(was_set);
            } else {
                return RuntimeError::undefined_variable(assignment->id);
            }
            

            return RuntimeError::ok();
        }
        case Type::AND: {
            LogicalBinaryExpr* logical_and = expr.logical_binary;

            Value left_val = {};
            CHECK_ERR(logical_and->left->evaluate(compiler, arena, env, left_val));
            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(logical_and->op);
            }

            in_value = left_val;

            if (left_val.b == true) {
                Value right_val = {};
                CHECK_ERR(logical_and->right->evaluate(compiler, arena, env, right_val));
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
            CHECK_ERR(logical_or->left->evaluate(compiler, arena, env, left_val));
            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(logical_or->op);
            }

            Value right_val = {};
            CHECK_ERR(logical_or->right->evaluate(compiler, arena, env, right_val));

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
            
            Callable* callable = nullptr;
            const Token* calllable_name = nullptr;
            if (callee->ty == Expr::Type::LITERAL) {
                LiteralExpr* callee_literal = callee->expr.literal;
                if (callee_literal->val->m_type != TokenType::IDENTIFIER) {
                    return RuntimeError::invalid_function_identifier(callee_literal->val);
                }

                Callable* literal_callable = env->get_callable(callee_literal->val->m_lexeme);
                if (literal_callable == nullptr) {
                    return RuntimeError::undeclared_function(callee_literal->val);
                }

                callable = literal_callable;
                calllable_name = callee_literal->val;
            } else if (callee->ty == Expr::Type::GET) {
                Value get_value = {};
                CHECK_ERR(callee->evaluate(compiler, arena, env, get_value));
                assert(get_value.ty == Value::Type::CALLABLE);

                callable = get_value.callable;
                calllable_name = callee->expr.get->member;
            }
            else if (callee->ty == Expr::Type::STATIC_FN_CALL) {
                StaticFnCallExpr* static_fn = callee->expr.static_fn_call;
                assert(static_fn->class_expr->ty == Expr::Type::LITERAL);
                Expr* class_expr = static_fn->class_expr;
                const Token* class_name = class_expr->expr.literal->val;

                String static_fn_name = mangled_name(arena, class_name->m_lexeme, static_fn->fn_name->m_lexeme);
                Callable* class_callable = compiler->global_env.get_callable(static_fn_name);
                if (class_callable == nullptr) {
                    return RuntimeError::undeclared_function(static_fn->fn_name);
                }

                callable = class_callable;
                calllable_name = static_fn->fn_name;
            }
            else if (callee->ty == Expr::Type::SUPER) {
                SuperExpr* super_expr = callee->expr.super_expr;

                Value super_value = {};
                CHECK_ERR(compiler->lookup_variable(env, super_expr->keyword, this, super_value));
                assert(super_value.ty == Value::Type::CLASS);
                Class* super_class = super_value.m_class;

                Callable* super_method = super_class->get_method(super_expr->method->m_lexeme);
                if (super_method == nullptr) {
                    return RuntimeError::undeclared_function(super_expr->method);
                }

                callable = super_method;
                calllable_name = super_expr->method;
            } else {
                assert(false);
            }

            if (callable->m_arity != fn_call->arguments.size()) {
                return RuntimeError::wrong_number_arguments(calllable_name);
            }

            Array<Value> values;
            values.init(arena, fn_call->arguments.size());
            for (size_t i = 0; i < fn_call->arguments.size(); ++i) {
                Value arg_val = {};
                RuntimeError err = fn_call->arguments[i]->evaluate(compiler, arena, env, arg_val);
                if (!err.is_ok()) {
                    return err;
                }
                values[i] = arg_val;
            }


            const Value ret_value = callable->m_callback(values, compiler, arena, env);
            in_value = ret_value;

            compiler->hit_return = false;

            return RuntimeError::ok();
        }
        case Type::GET: {
            GetExpr* get = expr.get;

            Value expr_val = {};
            CHECK_ERR(get->class_expr->evaluate(compiler, arena, env, expr_val));
            if (expr_val.ty != Value::Type::CLASS) {
                return RuntimeError::object_must_be_struct(get->member);
            }

            const bool has_field = expr_val.m_class->get(get->member->m_lexeme, in_value);
            if (has_field) {
                return RuntimeError::ok();
            } else {
                return RuntimeError::class_does_not_have_field(get->member);
            }
        }
        case Type::SET: {
            SetExpr* set = expr.set;
            GetExpr* get = set->get->expr.get;

            Value class_val = {};
            CHECK_ERR(get->class_expr->evaluate(compiler, arena, env, class_val));
            if (class_val.ty != Value::Type::CLASS) {
                return RuntimeError::object_must_be_struct(get->member);
            }

            const bool has_field = class_val.m_class->contains_field(get->member->m_lexeme);
            if (has_field) {
                Value right_val = {};
                CHECK_ERR(set->right->evaluate(compiler, arena, env, right_val));

                class_val.m_class->set_field(get->member->m_lexeme, right_val);

                return RuntimeError::ok();
            } else {
                return RuntimeError::class_does_not_have_field(get->member);
            }
        }
        case Type::THIS: {
            ThisExpr* this_expr = expr.this_expr;
            return compiler->lookup_variable(env, this_expr->val, this, in_value);
        }
    }
}

void Value::print() const {
    switch (ty)
    {
        case Type::NIL: {
            fprintf(stdout, "nil\n");
            break;
        }
        case Type::BOOL: {
            fprintf(stdout, "%s\n", b ? "true" : "false");
            break;
        }
        case Type::FLOAT: {
            fprintf(stdout, "%f\n", f);
            break;
        }
        case Type::DOUBLE: {
            fprintf(stdout, "%lf\n", d);
            break;
        }
        case Type::INT: {
            fprintf(stdout, "%d\n", i);
            break;
        }
        case Type::LONG: {
            fprintf(stdout, "%ld\n", l);
            break;
        }
        case Type::STRING: {
            fprintf(stdout, "%.*s\n", (u32) str.len, str.chars);
            break;
        }
        case Type::CLASS: {
            m_class->print();
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
            env->define(arena, s_var_decl.name->m_lexeme, expr_val);
            break;
        }
        case Stmt::Type::BLOCK: {
            Environment new_env = {};
            new_env.init(arena);
            new_env.enclosing = env;
            for (int i = 0; i < s_block.stmts.size(); ++i) {
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
                compiler->runtime_error(expr_err.token->m_line, CREATE_STRING("if test expression must evaluate to bool"));
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
                    compiler->runtime_error(expr_err.token->m_line, CREATE_STRING("while test expression must evaluate to bool"));
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
                compiler->runtime_error(s_break_continue.token->m_line, CREATE_STRING("'break' statement can only be used in a loop."));
            }
            expr_val = Value {
                .ty = Value::Type::BREAK
            };
            break;
        }
        case Stmt::Type::CONTINUE: {
            if (!in_loop) {
                compiler->runtime_error(s_break_continue.token->m_line, CREATE_STRING("'continue' statement can only be used in a loop."));
            }
            expr_val = Value {
                .ty = Value::Type::CONTINUE
            };
            break;
        }
        case Stmt::Type::FN_DECLARATION: {
            String fn_name = fn_declaration.name->m_lexeme;
            FnDeclarationPayload fn = fn_declaration;

            env->define_callable(arena, fn_name, construct_callable(fn));

            break;
        }
        case Stmt::Type::CLASS_DECLARATION: {
            Token* class_name_token = s_class.name;
            String class_name = class_name_token->m_lexeme;
            Class* superclass = nullptr;

            if (s_class.superclass != nullptr) {
                Value superclass_val = {};
                assert(s_class.superclass->ty == Expr::Type::LITERAL);
                const Token* superclass_token = s_class.superclass->expr.literal->val;
                superclass = env->get_class(superclass_token->m_lexeme);
                if (superclass == nullptr) {
                    compiler->runtime_error(superclass_token->m_line, CREATE_STRING("superclass must be a class."));
                }
            }

            env->define_class(arena, class_name_token->m_lexeme, Class());
            Class* new_class = nullptr;
            new_class = env->get_class(class_name_token->m_lexeme);
            assert(new_class != nullptr);
            new_class->m_name = class_name;
            new_class->m_methods.allocate(arena);
            new_class->m_fields.allocate(arena);
            new_class->superclass = superclass;

            for (u64 i = 0; i < s_class.members.size(); ++i) {
                Stmt* stmt = &s_class.members[i];
                if (stmt->ty == Stmt::Type::FN_DECLARATION) {
                    FnDeclarationPayload fn = stmt->fn_declaration;
                    if (fn.is_static) {
                        String fn_name = mangled_name(arena, new_class->m_name, fn.name->m_lexeme);
                        compiler->global_env.define_callable(arena, fn_name, construct_callable(fn));
                    } else {
                        String str = fn.name->m_lexeme;
                        Callable callable = construct_callable_class(fn, new_class);
                        new_class->m_methods.insert(arena,str, HASH_STR(str), callable);
                    }
                } else if (stmt->ty == Stmt::Type::VAR_DECL) {
                    VarDeclPayload var_decl = stmt->s_var_decl;

                    Value value = {};
                    if (var_decl.initializer != nullptr) {
                        RuntimeError var_err = var_decl.initializer->evaluate(compiler, arena, env, value);
                        if (!var_err.is_ok()) {
                            compiler->runtime_error(var_err.token->m_line, var_err.message);
                        }
                    }

                    String str = var_decl.name->m_lexeme;
                    new_class->m_fields.insert(arena, str, HASH_STR(str), value);
                } else {
                    assert(false);
                }
            }

            Callable* class_init = new_class->get_method(CREATE_STRING("init"));
            if (class_init != nullptr) {
                env->define_callable(arena, class_name, Callable(class_init->m_arity, [class_name_token, class_init](Array<Value> args, KauCompiler* compiler, Arena* arena, Environment* env) {
                    Class* in_class = env->get_class(class_name_token->m_lexeme);
                    assert(in_class != nullptr);

                    const Value init_value = class_init->m_callback(args, compiler, arena, env);

                    return Value{
                        .ty = Value::Type::CLASS,
                        .m_class = in_class,
                    };
                }));
            } else {
                env->define_callable(arena, class_name, Callable(0, [class_name_token](Array<Value> args, KauCompiler* compiler, Arena* arena, Environment* env) {
                    Class* in_class = env->get_class(class_name_token->m_lexeme);
                    assert(in_class != nullptr);

                    return Value{
                        .ty = Value::Type::CLASS,
                        .m_class = in_class,
                    };
                }));
            }

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
            fprintf(stdout, "VAR DECL: %.*s", (u32) s_var_decl.name->m_lexeme.len, s_var_decl.name->m_lexeme.chars);
            if (s_var_decl.initializer != nullptr) {
                fprintf(stdout, " = ");
                s_var_decl.initializer->print();
            }
            break;
        }
        case Type::EXPR: {
            fprintf(stdout, "EXPR: ");
            s_expr.expr->print();
            break;
        }
        case Type::BLOCK: {
            fprintf(stdout, "BLOCK: ");
            for (int i = 0; i < s_block.stmts.size(); ++i) {
                s_block.stmts[i].print();
            }
            break;
        }
        case Type::IF: {
            fprintf(stdout, "IF: ");
            s_if.if_stmt->print();
            if (s_if.else_stmt->ty != Stmt::Type::ERR) {
                s_if.else_stmt->print();
            }
            break;
        }
        case Type::WHILE: {
            fprintf(stdout, "WHILE: ");
            s_while.body->print();
            break;
        }
        case Type::BREAK: {
            fprintf(stdout, "BREAK");
            break;
        }
        case Type::CONTINUE: {
            fprintf(stdout, "CONTINUE");
            break;
        }
        case Type::RETURN: {
            fprintf(stdout, "RETURN");
            s_return.expr->print();
            break;
        }
        case Type::FN_DECLARATION: {
            fprintf(stdout, "FN DECLARATION: ");
            fprintf(stdout, "%.*s params:", (u32) fn_declaration.name->m_lexeme.len, fn_declaration.name->m_lexeme.chars);
            for (u64 i = 0; i < fn_declaration.params.size(); ++i) {
                fprintf(stdout, " %.*s", (u32) fn_declaration.params[i]->m_lexeme.len,fn_declaration.params[i]->m_lexeme.chars);
            }
            break;
        }
        case Type::CLASS_DECLARATION: {
            fprintf(stdout, "CLASS DECLARATION: ");
            fprintf(stdout, "%.*s", (u32) s_class.name->m_lexeme.len, s_class.name->m_lexeme.chars);
            if (s_class.superclass != nullptr) {
                fprintf(stdout, " : ");
                s_class.superclass->print();
            }
            for (u64 i = 0; i < s_class.members.size(); ++i) {
                s_class.members[i].print();
            }
            break;
        }
        //
        case Type::ERR: {
            fprintf(stdout, "ERR");
            return;
        }
    }
}

bool Class::contains_field(String field) {
    return m_fields.get(HASH_STR(field)) != nullptr;
}

void Class::set_field(String field, Value in_value) {
    Value* field_val = (Value*) m_fields.get(HASH_STR(field));
    *field_val = in_value;
}

Callable* Class::get_method(String name) {
    Callable* method = (Callable*) m_methods.get(HASH_STR(name));
    if (method != nullptr) {
        return method;
    }
    if (superclass != nullptr) {
        return superclass->get_method(name);
    }
    return nullptr;
}

bool Class::get(String field, Value& in_value) {
    void* field_ret = m_fields.get(HASH_STR(field));
    if (field_ret != nullptr) {
        in_value = *((Value*) field_ret);
        return true;
    }

    Callable* method_ret = (Callable*) get_method(field);
    if (method_ret != nullptr) {
        in_value = Value {
            .ty = Value::Type::CALLABLE,
            .callable = method_ret,
        };
        return true;
    }

    return false;
}