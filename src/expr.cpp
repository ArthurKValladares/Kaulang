#include "expr.h"
#include "defs.h"

void Expr::print() {
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

Value Expr::evaluate() {
    // TODO: error reporting
    switch (ty)
    {
        case Type::LITERAL: {
            LiteralExpr* literal = expr.literal;

            return Value{};
        }
        case Type::UNARY: {
            UnaryExpr* unary = expr.unary;

            Value right_result = unary->right->evaluate();

            switch (unary->op->m_type)
            {
                case TokenType::BANG: {
                    // TODO: Error reporting
                    assert(right_result.ty == Value::Type::BOOL);
                    right_result.b = !right_result.b;
                    break;
                }
                case TokenType::MINUS: {
                    // TODO: Error reporting
                    assert(right_result.ty == Value::Type::FLOAT);
                    right_result.f = -right_result.f;
                    break;
                }
                default: {
                    assert(false);
                    return Value{};
                }
            }

            return right_result;
        }
        case Type::BINARY: {
            BinaryExpr* binary = expr.binary;

            Value left_result = binary->left->evaluate();
            Value right_result = binary->right->evaluate();

            switch (binary->op->m_type)
            {
                case TokenType::PLUS: {
                    if (left_result.ty == Value::Type::STRING && right_result.ty == Value::Type::STRING) {
                        // TODO: do this right later once I use my own string stuff};
                        return Value {
                            .ty = Value::Type::STRING,
                            .str = left_result.str
                        };
                    } else {
                        assert(left_result.ty == Value::Type::FLOAT);
                        assert(right_result.ty == Value::Type::FLOAT);
                        return Value {
                            .ty = Value::Type::FLOAT,
                            .f = left_result.f + right_result.f
                        };
                    }
                }
                case TokenType::MINUS: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_result.f - right_result.f
                    };
                }
                case TokenType::SLASH: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_result.f / right_result.f
                    };
                }
                case TokenType::STAR: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::FLOAT,
                        .f = left_result.f * right_result.f
                    };
                }
                case TokenType::GREATER: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f > right_result.f
                    };
                }
                case TokenType::GREATER_EQUAL: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f >= right_result.f
                    };
                }
                case TokenType::LESSER: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f < right_result.f
                    };
                }
                case TokenType::LESSER_EQUAL: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f <= right_result.f
                    };
                }
                case TokenType::BANG_EQUAL: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f != right_result.f
                    };
                }
                case TokenType::EQUAL_EQUAL: {
                    assert(left_result.ty == Value::Type::FLOAT);
                    assert(right_result.ty == Value::Type::FLOAT);
                    return Value {
                        .ty = Value::Type::BOOL,
                        .b = left_result.f == right_result.f
                    };
                }
                default: {
                    assert(false);
                    return Value{};
                }
            }
        }
        case Type::GROUPING: {
            GroupingExpr* grouping = expr.grouping;

            return Value{};
        }
        case Type::COMMA: {
            CommaExpr* comma = expr.comma;

            return Value{};
        }
        case Type::TERNARY: {
            TernaryExpr* ternary = expr.ternary;

            return Value{};
        }
    }
}