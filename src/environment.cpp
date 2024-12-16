#include "environment.h"

void Environment::define(Token* token, Value value) {
    // TODO: allocation
    values[std::string(token->m_lexeme)] = value;
}

bool Environment::contains(Token* token) const {
    std::string str_name = std::string(token->m_lexeme);
    if(values.contains(str_name)) {
        return true;
    } else {
        if (enclosing != nullptr) {
            return enclosing->contains(token);
        }

        return false;
    }
}

RuntimeError Environment::get(Token* token, Value& in_value) {
    std::string str_name = std::string(token->m_lexeme);

    if (values.contains(str_name)) {
        in_value = values.at(str_name);
        if (in_value.ty == Value::Type::NIL) {
            return RuntimeError::undefined_variable(token);
        }
        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get(token, in_value);
        }

        return RuntimeError::undeclared_variable(token);
    }
}