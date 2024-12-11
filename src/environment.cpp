#include "environment.h"

void Environment::define(Token* token, Value value) {
    // TODO: allocation
    values[std::string(token->m_lexeme)] = value;
}

RuntimeError Environment::get(Token* token, Value& in_value) {
    std::string str_name = std::string(token->m_lexeme);
    if (values.contains(str_name)) {
        in_value = values.at(str_name);
        return RuntimeError::ok();
    } else {
        return RuntimeError::undefined_variable(token);
    }
}