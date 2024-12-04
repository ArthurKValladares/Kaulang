#include "expr.h"
#include "defs.h"

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

RuntimeError RuntimeError::operands_must_be_floats(Token* token) {
    return RuntimeError {
        .ty = Type::WRONG_OPERANDS,
        .token = token,
        .message = "Operands must be floats"
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

static RuntimeError divide_by_zero(Token* token) {
    return RuntimeError {
        .ty = Type::DIVIDE_BY_ZERO,
        .token = token,
        .message = "Divide by zero"
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
    }
}

RuntimeError Expr::evaluate(Value& in_value) {
    // TODO: error reporting
    switch (ty)
    {
        case Type::LITERAL: {
            LiteralExpr* literal = expr.literal;
            switch (literal->val->m_type)
            {
                case TokenType::FALSE: {
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .f = false
                    };
                    return RuntimeError::ok();
                }
                case TokenType::TRUE: {
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .f = true
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NIL: {
                    in_value = Value {
                        .ty = Value::Type::NIL,
                    };
                    return RuntimeError::ok();
                }
                case TokenType::NUMBER: {
                    in_value = Value {
                        .ty = Value::Type::FLOAT,
                        .f = literal->val->data.data.f
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
                default:  {
                    return RuntimeError::unsupported_literal(literal->val);
                }
            }

            return RuntimeError::ok();
        }
        case Type::UNARY: {
            UnaryExpr* unary = expr.unary;

            Value right_val = {};
            RuntimeError right_err = unary->right->evaluate(right_val);
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
            RuntimeError left_err = binary->left->evaluate(left_val);
            if (!left_err.is_ok()) {
                return left_err;
            }

            Value right_val = {};
            RuntimeError right_err = binary->right->evaluate(right_val);
            if (!right_err.is_ok()) {
                return right_err;
            }

            switch (binary->op->m_type)
            {
                case TokenType::PLUS: {
                    if (left_val.ty == Value::Type::STRING && right_val.ty == Value::Type::STRING) {
                        // TODO: do this right later once I use my own string stuff
                        in_value = Value {
                            .ty = Value::Type::STRING,
                            .str = left_val.str
                        };
                    } else {
                        if (left_val.ty != Value::Type::FLOAT ||
                            right_val.ty != Value::Type::FLOAT) {
                                return RuntimeError::operand_must_be_float(binary->op);
                        }
                        in_value = Value {
                            .ty = Value::Type::FLOAT,
                            .f = left_val.f + right_val.f
                        };
                    }
                    return RuntimeError::ok();
                }
                case TokenType::MINUS: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_val.f - right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::SLASH: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    if (right_val.f == 0.0) {
                        return RuntimeError::divide_by_zero();
                    }
                    in_value = Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_val.f / right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::STAR: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_val.f * right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::GREATER: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f > right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::GREATER_EQUAL: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f >= right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::LESSER: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f < right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::LESSER_EQUAL: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f <= right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::BANG_EQUAL: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f != right_val.f
                    };
                    return RuntimeError::ok();
                }
                case TokenType::EQUAL_EQUAL: {
                    if (left_val.ty != Value::Type::FLOAT ||
                        right_val.ty != Value::Type::FLOAT) {
                            return RuntimeError::operand_must_be_float(binary->op);
                    }
                    in_value = Value {
                        .ty = Value::Type::BOOL,
                        .b = left_val.f == right_val.f
                    };
                    return RuntimeError::ok();
                }
                default: {
                    return RuntimeError::unsupported_binary_op(binary->op);
                }
            }
        }
        case Type::GROUPING: {
            GroupingExpr* grouping = expr.grouping;

            return RuntimeError::ok();
        }
        case Type::COMMA: {
            CommaExpr* comma = expr.comma;

            return RuntimeError::ok();
        }
        case Type::TERNARY: {
            TernaryExpr* ternary = expr.ternary;

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
        case Type::STRING: {
            std::println("{}", str);
            break;
        }
    }
}