#include "tokens.h"

#include "defs.h"

// NOTE: reserved keywords in lowercase, user-defines data in uppercase
const char* token_type_to_string(TokenType ty) {
    switch (ty)
    {
    case TokenType::LEFT_PAREN: {
        return "(";
    }
    case TokenType::RIGHT_PAREN: {
        return ")";
    }
    case TokenType::LEFT_BRACE: {
        return "{";
    }
    case TokenType::RIGHT_BRACE: {
        return "}";
    }
    case TokenType::COMMA: {
        return ",";
    }
    case TokenType::DOT: {
        return ".";
    }
    case TokenType::MINUS: {
        return "-";
    }
    case TokenType::PLUS: {
        return "+";
    }
    case TokenType::SEMICOLON: {
        return ";";
    }
    case TokenType::COLON: {
        return ":";
    }
    case TokenType::SLASH: {
        return "/";
    }
    case TokenType::STAR: {
        return "*";
    }
    case TokenType::QUESTION_MARK: {
        return "?";
    }
    case TokenType::BANG: {
        return "!";
    }
    case TokenType::BANG_EQUAL: {
        return "!=";
    }
    case TokenType::EQUAL: {
        return "=";
    }
    case TokenType::EQUAL_EQUAL: {
        return "==";
    }
    case TokenType::GREATER: {
        return ">";
    }
    case TokenType::GREATER_EQUAL: {
        return ">=";
    }
    case TokenType::LESSER: {
        return "<";
    }
    case TokenType::LESSER_EQUAL: {
        return "<=";
    }
    case TokenType::IDENTIFIER: {
        return "IDENTIFIER";
    }
    case TokenType::STRING: {
        return "STRING";
    }
    case TokenType::NUMBER_INT: {
        return "INT";
    }
    case TokenType::NUMBER_FLOAT: {
        return "FLOAT";
    }
    case TokenType::NUMBER_DOUBLE: {
        return "DOUBLE";
    }
    case TokenType::AND: {
        return "and";
    }
    case TokenType::CLASS: {
        return "class";
    }
    case TokenType::ELSE: {
        return "else";
    }
    case TokenType::FALSE: {
        return "false";
    }
    case TokenType::FN: {
        return "fn";
    }
    case TokenType::FOR: {
        return "for";
    }
    case TokenType::IF: {
        return "if";
    }
    case TokenType::NIL: {
        return "nil";
    }
    case TokenType::OR: {
        return "or";
    }
    case TokenType::PRINT: {
        return "print";
    }
    case TokenType::RETURN: {
        return "return";
    }
    case TokenType::SUPER: {
        return "super";
    }
    case TokenType::THIS: {
        return "this";
    }
    case TokenType::TRUE: {
        return "true";
    }
    case TokenType::VAR: {
        return "var";
    }
    case TokenType::WHILE: {
        return "while";
    }
    case TokenType::_EOF: {
        return "EOF";
    }
    }
}

void Token::print() const {
    std::print("{}", token_type_to_string(m_type));
    if (!m_lexeme.empty()) {
        std::print(" -> {}", m_lexeme);
    }
    switch (data.ty) {
        case TokenData::Type::FLOAT: {
            std::print(" -> {}", data.data.f);
            break;
        }
        case TokenData::Type::DOUBLE: {
            std::print(" -> {}", data.data.d);
            break;
        }
        case TokenData::Type::INT: {
            std::print(" -> {}", data.data.i);
            break;
        }
        default: {
            break;
        }
    }
}

TokenData TokenData::new_float(float val) {
    TokenData data = {};
    data.ty = TokenData::Type::FLOAT;
    data.data = TokenData::InnerData {
        .f = val
    };
    return data;
}

TokenData TokenData::new_double(double val) {
    TokenData data = {};
    data.ty = TokenData::Type::DOUBLE;
    data.data = TokenData::InnerData {
        .d = val
    };
    return data;
}

TokenData TokenData::new_int(int val) {
    TokenData data = {};
    data.ty = TokenData::Type::INT;
    data.data = TokenData::InnerData {
        .i = val
    };
    return data;
}