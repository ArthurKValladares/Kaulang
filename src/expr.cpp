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

RuntimeError RuntimeError::ok() {
    return RuntimeError {
        .ty = Type::Ok
    };
}

RuntimeError RuntimeError::unsupported_literal(Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_LITERAL,
        .token = token,
        .message = "Unsupported literal"
    };
}

RuntimeError RuntimeError::unsupported_binary_op(Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Unsupported binary operation"
    };
}

RuntimeError RuntimeError::unsupported_unary_op(Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Unsupported unary operation"
    };
}

RuntimeError RuntimeError::operands_must_be_equal(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be equal"
    };
}

RuntimeError RuntimeError::operands_must_be_floats(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be floats"
    };
}

RuntimeError RuntimeError::operands_must_be_strings(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be strings"
    };
}

RuntimeError RuntimeError::operand_must_be_float(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operand must be float"
    };
}

RuntimeError RuntimeError::operand_must_be_bool(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operand must be bool"
    };
}

RuntimeError RuntimeError::divide_by_zero(Token* token) {
    return RuntimeError {
        .ty = Type::DIVIDE_BY_ZERO,
        .token = token,
        .message = "Divide by zero"
    };
}

RuntimeError RuntimeError::operands_do_not_support_operator(Token* token) {
    return RuntimeError {
        .ty = Type::UNSUPPORTED_OPERATOR,
        .token = token,
        .message = "Operands do not support operator"
    };
}

RuntimeError RuntimeError::undeclared_variable(Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = "Undeclared variable"
    };
}

RuntimeError RuntimeError::undefined_variable(Token* token) {
    return RuntimeError {
        .ty = Type::UNDEFINED_VARIABLE,
        .token = token,
        .message = "Undefined variable"
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
        case Type::COMMA: {
            CommaExpr* comma = expr.comma;

            comma->left->print();
            std::print(" , ");
            comma->right->print();

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
    }
}

RuntimeError Expr::evaluate(Environment* env, Value& in_value) {
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
                    return env->get(literal->val, in_value);
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
            RuntimeError right_err = unary->right->evaluate(env, right_val);
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
            RuntimeError left_err = binary->left->evaluate(env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            Value right_val = {};
            RuntimeError right_err = binary->right->evaluate(env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            switch (binary->op->m_type)
            {
                // TODO: Tons of repetition, clean up later
                case TokenType::PLUS: {
                    if (left_val.ty != right_val.ty) {
                            return RuntimeError::operands_must_be_equal(binary->op);
                    }

                    TEST_BINARY_OP(FLOAT, f, FLOAT, f, +);
                    TEST_BINARY_OP(DOUBLE, d, DOUBLE, d, +);
                    TEST_BINARY_OP(INT, i, INT, i, +);

                    if (left_val.ty == Value::Type::STRING) {
                        // TODO: Correctly handle later when i have better string support
                        in_value = Value {
                            .ty = Value::Type::STRING,
                            .str = left_val.str
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
            RuntimeError err = grouping->expr->evaluate(env, val);
            if (!err.is_ok()) {
                return err;
            }

            return RuntimeError::ok();
        }
        case Type::COMMA: {
            CommaExpr* comma = expr.comma;

            Value left_val = {};
            RuntimeError left_err = comma->left->evaluate(env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            Value right_val = {};
            RuntimeError right_err = comma->right->evaluate(env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            in_value = right_val;

            return RuntimeError::ok();
        }
        case Type::TERNARY: {
            TernaryExpr* ternary = expr.ternary;

            Value left_val = {};
            RuntimeError left_err = ternary->left->evaluate(env, left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            if (left_val.ty != Value::Type::BOOL) {
                return RuntimeError::operand_must_be_bool(ternary->left_op);
            }

            if (left_val.b) {
                Value middle_val = {};
                RuntimeError middle_err = ternary->middle->evaluate(env, middle_val);
                if (!middle_err.is_ok()) {
                    return middle_err;
                }

                in_value = middle_val;

                return RuntimeError::ok();
            } else {
                Value right_val = {};
                RuntimeError right_err = ternary->right->evaluate(env, right_val);
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
            RuntimeError right_err = assignment->right->evaluate(env, right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            in_value = right_val;

            if (env->contains(assignment->id)) {
                env->define(assignment->id, right_val);
            } else {
                return RuntimeError::undefined_variable(assignment->id);
            }
            

            return RuntimeError::ok();
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
        case Type::STRING: {
            std::println("{}", str);
            break;
        }
    }
}

Value Stmt::evaluate(KauCompiler* compiler, Environment* env) {
    if (ty == Stmt::Type::BLOCK) {
        Value expr_val = {};
        Environment new_env = {};
        new_env.enclosing = env;
        for (Stmt& stmt : stmts) {
            expr_val = stmt.evaluate(compiler, &new_env);
        }
        return expr_val;
    }

    if (ty == Stmt::Type::IF) {
        Value test_expr_val = {};
        RuntimeError expr_err = expr->evaluate(env, test_expr_val);
        if (!expr_err.is_ok()) {
            compiler->runtime_error(expr_err.token->m_line, expr_err.message);
        }
        if (test_expr_val.ty != Value::Type::BOOL) {
            compiler->runtime_error(expr_err.token->m_line, "if test expression must evaluate to bool");
        }
        bool if_result = test_expr_val.b;

        Value expr_val = {};
        Environment new_env = {};
        new_env.enclosing = env;
        if (if_result) {
            expr_val = stmts[0].evaluate(compiler, &new_env);
        } else if (stmts[1].ty != Stmt::Type::ERR) {
            expr_val = stmts[1].evaluate(compiler, &new_env);
        }

        return expr_val;
    }

    // For now a variable declaration, like `var a;` has no expr.
    // Maybe it should have a no-op one instead, or just some sort of 
    // expr that evaluates to a default val.
    Value expr_val = {};
    if (expr != nullptr) {
        RuntimeError expr_err = expr->evaluate(env, expr_val);
        if (!expr_err.is_ok()) {
            compiler->runtime_error(expr_err.token->m_line, expr_err.message);
        }
    }

    if (ty == Stmt::Type::VAR_DECL) {
        env->define(name, expr_val);
    }

    return expr_val;
}

void Stmt::print() {
    
    switch (ty)
    {
        case Type::VAR_DECL: {
            std::print("VAR DECL: ");
            std::print("{}", name->m_lexeme);
            if (expr != nullptr) {
                std::print(" = ");
                expr->print();
            }
            std::println("");
            break;
        }
        case Type::PRINT: {
            std::print("PRINT: ");
            expr->print();
            std::println("");
            break;
        }
        case Type::EXPR: {
            std::print("EXPR: ");
            expr->print();
            std::println("");
            break;
        }
        case Type::BLOCK: {
            std::print("BLOCK: ");
            for (Stmt& stmt : stmts) {
                stmt.print();
            }
            break;
        }
        case Type::IF: {
            std::print("IF: ");
            stmts[0].print();
            if (stmts[1].ty != Stmt::Type::ERR) {
                stmts[1].print();
            }
            break;
        }
        //
        case Type::ERR: {
            std::println("ERR");
            return;
        }
    }
}