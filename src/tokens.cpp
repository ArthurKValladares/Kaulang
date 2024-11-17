#include "tokens.h"

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
    case TokenType::SLASH: {
        return "/";
    }
    case TokenType::STAR: {
        return "*";
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
        // user-defined
        return "";
    }
    case TokenType::STRING: {
        // User-defined
        return "";
    }
    case TokenType::NUMBER: {
        // User-defined
        return "";
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

TokenData TokenData::new_float(float val) {
    TokenData data = {};
    data.ty = TokenData::Type::FLOAT;
    data.data = TokenData::InnerData {
        .f = val
    };
    return data;
}