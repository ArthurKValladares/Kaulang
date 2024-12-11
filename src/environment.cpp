#include "environment.h"

void Environment::define(std::string_view name, Value value) {
    values[name] = value;
}

RuntimeError Environment::get(Token* token, Value& in_value) {
    if (values.contains(token->m_lexeme)) {
        //in_value = values.at(token->m_lexeme);
        return RuntimeError::ok();
    } else {
        return RuntimeError::undefined_variable(token);
    }
}